//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "BatchRender.h"

#ifndef _SCENE_OBJECT_H_
#include "2d/sceneobject/SceneObject.h"
#endif

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

BatchRender::BatchRender() :
    mTriangleCount( 0 ),
    mVertexCount( 0 ),
    mIndexCount( 0 ),
    NoColor( -1.0f, -1.0f, -1.0f ),
    mStrictOrderMode( false ),
    mpDebugStats( NULL ),
    mBlendColor( ColorF(1.0f,1.0f,1.0f,1.0f) ),
    mBatchEnabled( false )
{
}

//-----------------------------------------------------------------------------

BatchRender::~BatchRender()
{
    // Destroy index vectors in texture batch map.
    for ( textureBatchType::iterator itr = mTextureBatchMap.begin(); itr != mTextureBatchMap.end(); ++itr )
    {
        delete itr->value;
    }
    mTextureBatchMap.clear();

    // Destroy index vectors in index vector pool.
    for ( VectorPtr< indexVectorType* >::iterator itr = mIndexVectorPool.begin(); itr != mIndexVectorPool.end(); ++itr )
    {
        delete (*itr);
    }
    mIndexVectorPool.clear();
}

//-----------------------------------------------------------------------------

void BatchRender::setBlendMode( const SceneRenderRequest* pSceneRenderRequest )
{
    // Are we blending?
    if ( pSceneRenderRequest->mBlendMode )
    {
        // Yes, so set blending to standard alpha-blending.
        setBlendMode(
            pSceneRenderRequest->mSrcBlendFactor,
            pSceneRenderRequest->mDstBlendFactor,
            pSceneRenderRequest->mBlendColor );                            
    }
    else
    {
        // No, so turn-off blending.
        setBlendOff();
    }
}

//-----------------------------------------------------------------------------

void BatchRender::setAlphaTestMode( const SceneRenderRequest* pSceneRenderRequest )
{
    // Set alpha-test mode.
    setAlphaTestMode( pSceneRenderRequest->mAlphaTest );
}

//-----------------------------------------------------------------------------

void BatchRender::SubmitTriangles(
        const U32 vertexCount,
        const Vector2* pVertexArray,
        const Vector2* pTextureArray,
        GFXTexHandle& texture,
        const ColorF& inColor )
{
    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitTriangles);

    // Sanity!
    AssertFatal( mpDebugStats != NULL, "Debug stats have not been configured." );
    AssertFatal( vertexCount % 3 == 0, "BatchRender::SubmitTriangles() - Invalid vertex count, cannot represent whole triangles." );
    AssertFatal( vertexCount <= BATCHRENDER_BUFFERSIZE, "BatchRender::SubmitTriangles() - Invalid vertex count." );

    // Calculate triangle count.
    const U32 triangleCount = vertexCount / 3;

    ColorF color = ColorF(1.0, 1.0f, 1.0f, 1.0f);
    
    // Is a color specified?
    if ( inColor != NoColor )
        color = inColor;

    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + triangleCount) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Do we have anything batched?
    else if ( mTriangleCount > 0 )
    {
        flush( mpDebugStats->batchColorStateFlush );
    }

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && mTriangleCount > 0 )
        {
            // Yes, so flush.
            flush( mpDebugStats->batchTextureChangeFlush );
        }

        // Fetch vertex index.
        U16 vertexIndex = (U16)mVertexCount;

        // Add new indices.
        for( U32 n = 0; n < triangleCount; ++n )
        {
            mIndexBuffer[mIndexCount++] = vertexIndex++;
            mIndexBuffer[mIndexCount++] = vertexIndex++;
            mIndexBuffer[mIndexCount++] = vertexIndex++;
        }

        // Set strict order mode texture handle.
        mStrictOrderTextureHandle = texture;
    }
    else
    {
        // No, so add triangle run.
        findTextureBatch( texture )->push_back( TriangleRun( TriangleRun::TRIANGLE, triangleCount, mVertexCount ) );
    }

    // Add textured vertices.
    for( U32 n = 0; n < triangleCount; ++n )
    {
        mVertexBuffer[mVertexCount].point       = Point3F(pVertexArray->x, pVertexArray->y, 0.0f);
        mVertexBuffer[mVertexCount].texCoord    = Point2F(pTextureArray->x, pTextureArray->y);
        mVertexBuffer[mVertexCount].color       = color;
        mVertexCount++;
        pVertexArray++;
        pTextureArray++;
    }

    // Stats.
    mpDebugStats->batchTrianglesSubmitted += triangleCount;

    // Increase triangle count.
    mTriangleCount += triangleCount;

    // Have we reached the buffer limit?
    if ( mTriangleCount == BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Is batching enabled?
    else if ( !mBatchEnabled )
    {
        // No, so flush immediately.
        // NOTE: Technically this is still batching but will still revert to using
        // more draw calls therefore can be used in comparison.
        flushInternal();
    }
}

void BatchRender::SubmitQuad(const GFXVertexPCT* vertex,
                             GFXTexHandle& texture,
                             const ColorF& inColor)
{
    // Sanity!
    AssertFatal( mpDebugStats != NULL, "Debug stats have not been configured." );
    
    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitQuad);
    
    ColorF color = ColorF(1.0, 1.0f, 1.0f, 1.0f);
    
    // Is a color specified?
    if ( inColor != NoColor )
        color = inColor;
    
    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Do we have anything batched?
    else if ( mTriangleCount > 0 )
    {
        flush( mpDebugStats->batchColorStateFlush );
    }
    
    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && mTriangleCount > 0 )
        {
            // Yes, so flush.
            flush( mpDebugStats->batchTextureChangeFlush );
        }
        
        // Add new indices.
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;
        
        // Set strict order mode texture handle.
        mStrictOrderTextureHandle = texture;
    }
    else
    {
        // No, so add triangle run.
        findTextureBatch( texture )->push_back( TriangleRun( TriangleRun::QUAD, 1, mVertexCount ) );
    }
    
    // Add textured vertices.
    // NOTE: We swap #2/#3 here.
    mVertexBuffer[mVertexCount++] = vertex[0];
    mVertexBuffer[mVertexCount++] = vertex[1];
    mVertexBuffer[mVertexCount++] = vertex[3];
    mVertexBuffer[mVertexCount++] = vertex[2];
    
    // Stats.
    mpDebugStats->batchTrianglesSubmitted+=2;
    
    // Increase triangle count.
    mTriangleCount += 2;
    
    // Have we reached the buffer limit?
    if ( mTriangleCount == BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Is batching enabled?
    else if ( !mBatchEnabled )
    {
        // No, so flush immediately.
        // NOTE: Technically this is still batching but will still revert to using
        // more draw calls therefore can be used in comparison.
        flushInternal();
    }}


//-----------------------------------------------------------------------------

void BatchRender::SubmitQuad(
        const Vector2& vertexPos0,
        const Vector2& vertexPos1,
        const Vector2& vertexPos2,
        const Vector2& vertexPos3,
        const Vector2& texturePos0,
        const Vector2& texturePos1,
        const Vector2& texturePos2,
        const Vector2& texturePos3,
        GFXTexHandle& texture,
        const ColorF& inColor )
{
    // Sanity!
    AssertFatal( mpDebugStats != NULL, "Debug stats have not been configured." );

    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitQuad);

    ColorF color = ColorF(1.0, 1.0f, 1.0f, 1.0f);

    // Is a color specified?
    if ( inColor != NoColor )
        color = inColor;

    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Do we have anything batched?
    else if ( mTriangleCount > 0 )
    {
        flush( mpDebugStats->batchColorStateFlush );
    }

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && mTriangleCount > 0 )
        {
            // Yes, so flush.
            flush( mpDebugStats->batchTextureChangeFlush );
        }

        // Add new indices.
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount++;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;
        mIndexBuffer[mIndexCount++] = (U16)mVertexCount--;

        // Set strict order mode texture handle.
        mStrictOrderTextureHandle = texture;
    }
    else
    {
        // No, so add triangle run.
        findTextureBatch( texture )->push_back( TriangleRun( TriangleRun::QUAD, 1, mVertexCount ) );
    }

    // Add textured vertices.
    // NOTE: We swap #2/#3 here.
    mVertexBuffer[mVertexCount].point       = Point3F(vertexPos0.x, vertexPos0.y, 0.0f);
    mVertexBuffer[mVertexCount].texCoord    = Point2F(texturePos0.x, texturePos0.y);
    mVertexBuffer[mVertexCount].color       = color;
    mVertexCount++;
    mVertexBuffer[mVertexCount].point       = Point3F(vertexPos1.x, vertexPos1.y, 0.0f);
    mVertexBuffer[mVertexCount].texCoord    = Point2F(texturePos1.x, texturePos1.y);
    mVertexBuffer[mVertexCount].color       = color;
    mVertexCount++;
    mVertexBuffer[mVertexCount].point       = Point3F(vertexPos3.x, vertexPos3.y, 0.0f);
    mVertexBuffer[mVertexCount].texCoord    = Point2F(texturePos3.x, texturePos3.y);
    mVertexBuffer[mVertexCount].color       = color;
    mVertexCount++;
    mVertexBuffer[mVertexCount].point       = Point3F(vertexPos2.x, vertexPos2.y, 0.0f);
    mVertexBuffer[mVertexCount].texCoord    = Point2F(texturePos2.x, texturePos2.y);
    mVertexBuffer[mVertexCount].color       = color;
    mVertexCount++;

    // Stats.
    mpDebugStats->batchTrianglesSubmitted+=2;

    // Increase triangle count.
    mTriangleCount += 2;

    // Have we reached the buffer limit?
    if ( mTriangleCount == BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Is batching enabled?
    else if ( !mBatchEnabled )
    {
        // No, so flush immediately.
        // NOTE: Technically this is still batching but will still revert to using
        // more draw calls therefore can be used in comparison.
        flushInternal();
    }
}

//-----------------------------------------------------------------------------

void BatchRender::flush( U32& reasonMetric )
{
    // Finish if no triangles to flush.
    if ( mTriangleCount == 0 )
        return;

    // Increase reason metric.
    reasonMetric++;

    // Flush.
    flushInternal();
}

//-----------------------------------------------------------------------------

void BatchRender::flush( void )
{
    // Finish if no triangles to flush.
    if ( mTriangleCount == 0 )
        return;

    // Increase reason metric.
    mpDebugStats->batchAnonymousFlush++;

    // Flush.
    flushInternal();
}

//-----------------------------------------------------------------------------

void BatchRender::flushInternal( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(T2D_BatchRender_flush);

    // Finish if no triangles to flush.
    if ( mTriangleCount == 0 )
        return;

    // Stats.
    mpDebugStats->batchFlushes++;

    GFXStateBlockDesc desc;

    if ( mWireframeMode )
    {
        desc.setFillModeWireframe();
    }
    else
    {
        GFX->setTexture(0, mStrictOrderTextureHandle);
    }

    // Set blend mode.
    if ( mBlendMode )
    {
        desc.setBlend(true, mSrcBlendFactor, mDstBlendFactor);
    }

    // Set alpha-blend mode.
    if ( mAlphaTestMode >= 0.0f )
    {
        desc.setAlphaTest(true, GFXCmpGreaterEqual, mAlphaTestMode*255.0);
    }

    GFX->setStateBlockByDesc( desc );

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Bind the texture if not in wireframe mode.
        if ( !mWireframeMode )
            GFX->setTexture(0, mStrictOrderTextureHandle);
        else
            Con::printf("!mWireframeMode");

        GFXVertexBufferHandle<GFXVertexPCT> vb(GFX, mVertexCount, GFXBufferTypeVolatile, mVertexBuffer, mIndexCount, mIndexBuffer );
        GFX->setVertexBuffer( vb );
        
        // Draw the triangles
        GFX->setupGenericShaders(GFXDevice::GSTexture);
        GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, 0, 0, mIndexCount/3);

        // Stats.
        mpDebugStats->batchDrawCallsStrict++;

        // Stats.
        const U32 trianglesDrawn = mIndexCount / 3;
        if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
            mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

        // Stats.
        if ( mVertexCount > mpDebugStats->batchMaxVertexBuffer )
            mpDebugStats->batchMaxVertexBuffer = mVertexCount;
    }
    else
    {
        // No, so iterate texture batch map.
        for( textureBatchType::iterator batchItr = mTextureBatchMap.begin(); batchItr != mTextureBatchMap.end(); ++batchItr )
        {
            // Reset index count.
            mIndexCount = 0;

            // Fetch index vector.
            indexVectorType* pIndexVector = batchItr->value;

            // Iterate indexes.
            for( indexVectorType::iterator indexItr = pIndexVector->begin(); indexItr != pIndexVector->end(); ++indexItr )
            {
                // Fetch triangle run.
                const TriangleRun& triangleRun = *indexItr;

                // Fetch primitivecount.
                const U32 primitiveCount = triangleRun.mPrimitiveCount;

                // Fetch triangle index start.
                U16 triangleIndex = (U16)triangleRun.mStartIndex;

                // Fetch primitive mode.
                const TriangleRun::PrimitiveMode& primitiveMode = triangleRun.mPrimitiveMode;

                // Handle primitive mode.
                if ( primitiveMode == TriangleRun::QUAD )
                {
                    // Add triangle run for quad.
                    for( U32 n = 0; n < primitiveCount; ++n )
                    {
                        // Add new indices.
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                        mIndexBuffer[mIndexCount++] = triangleIndex--;
                        mIndexBuffer[mIndexCount++] = triangleIndex--;
                        mIndexBuffer[mIndexCount++] = triangleIndex--;
                    }
                }
                else if ( primitiveMode == TriangleRun::TRIANGLE )
                {
                    // Add triangle run for triangles.
                    for( U32 n = 0; n < primitiveCount; ++n )
                    {
                        // Add new indices.
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                        mIndexBuffer[mIndexCount++] = triangleIndex++;
                    }
                }
                else
                {
                    // Sanity!
                    AssertFatal( false, "BatchRender::flushInternal() - Unrecognized primitive mode encountered for triangle run." );
                }
            }

            // Sanity!
            AssertFatal( mIndexCount > 0, "No batching indexes are present." );

            // Bind the texture if not in wireframe mode.
            GFX->setTexture(0, batchItr->key);

            GFXVertexBufferHandle<GFXVertexPCT> vb(GFX, mVertexCount, GFXBufferTypeVolatile, mVertexBuffer, mIndexCount, mIndexBuffer );
            GFX->setVertexBuffer( vb );

            // Draw the triangles.
            GFX->setupGenericShaders(GFXDevice::GSTexture);
            GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, 0, 0, mIndexCount/3);

            // Stats.
            mpDebugStats->batchDrawCallsSorted++;

            // Stats.
            if ( mVertexCount > mpDebugStats->batchMaxVertexBuffer )
                mpDebugStats->batchMaxVertexBuffer = mVertexCount;

            // Stats.
            const U32 trianglesDrawn = mIndexCount / 3;
            if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
                mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

            // Return index vector to pool.
            pIndexVector->clear();
            mIndexVectorPool.push_back( pIndexVector );
        }

        // Clear texture batch map.
        mTextureBatchMap.clear();
    }

    // Reset common render state.
//    glDisableClientState( GL_VERTEX_ARRAY );
//    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
//    glDisableClientState( GL_COLOR_ARRAY );
//    glDisable( GL_ALPHA_TEST );
//    glDisable( GL_BLEND );
//    glDisable( GL_TEXTURE_2D );
//    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // Reset batch state.
    mTriangleCount = 0;
    mVertexCount = 0;
    mIndexCount = 0;
}

//-----------------------------------------------------------------------------

BatchRender::indexVectorType* BatchRender::findTextureBatch( GFXTexHandle& handle )
{
    // Fetch texture binding.
//    const U32 textureBinding = handle.getGLName();

    indexVectorType* pIndexVector = NULL;

    // Find texture binding.
    textureBatchType::iterator itr = mTextureBatchMap.find( handle );

    // Did we find a texture binding?
    if ( itr == mTextureBatchMap.end() )
    {
        // No, so fetch index vector pool count.
        const U32 indexVectorPoolCount = mIndexVectorPool.size();

        // Do we have any in the index vector pool?
        if ( indexVectorPoolCount > 0 )
        {
            // Yes, so use it.
            pIndexVector = mIndexVectorPool[indexVectorPoolCount-1];
            mIndexVectorPool.pop_back();
        }
        else
        {
            // No, so generate one.
            pIndexVector = new indexVectorType( 6 * 6 );
        }

        // Insert into texture batch map.
        mTextureBatchMap.insert( handle, pIndexVector );
    }
    else
    {
        // Yes, so fetch it.
        pIndexVector = itr->value;
    }

    return pIndexVector;
}


