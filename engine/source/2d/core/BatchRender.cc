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

//#include "platformOSX/graphics/gfxOpenGLESTextureObject.h"

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

BatchRender::BatchRender() :
    mTriangleCount( 0 ),
    mVertexCount( 0 ),
    mTextureCoordCount( 0 ),
    mIndexCount( 0 ),
    NoColor( -1.0f, -1.0f, -1.0f ),
    mStrictOrderMode( false ),
    mpDebugStats( NULL ),
    mBlendColor( ColorF(1.0f,1.0f,1.0f,1.0f) ),
    mAlphaTestMode( -1.0f ),
    mBatchEnabled( true )
{
    mGFXStateDesc.setCullMode (GFXCullNone);
    mGFXStateDesc.setZReadWrite( false );
    mGFXStateDesc.setBlend( true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
    mGFXStateDesc.samplersDefined = true;
    mGFXStateDesc.samplers[0].GFXSamplerStateDesc::getClampLinear();
    mGFXStateDesc.fillMode = GFXFillSolid;
    
    mGFXStateRef = GFX->createStateBlock(mGFXStateDesc);
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
        TextureHandle& texture,
        const ColorF& color )
{
    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitTriangles);

    // Sanity!
    AssertFatal( mpDebugStats != NULL, "Debug stats have not been configured." );
    AssertFatal( vertexCount % 3 == 0, "BatchRender::SubmitTriangles() - Invalid vertex count, cannot represent whole triangles." );
    AssertFatal( vertexCount <= BATCHRENDER_BUFFERSIZE, "BatchRender::SubmitTriangles() - Invalid vertex count." );

    // Calculate triangle count.
    const U32 triangleCount = vertexCount / 3;

    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + triangleCount) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Do we have anything batched?
    else if ( mTriangleCount > 0 )
    {
        // Yes, so do we have any existing colors?
        if ( mColorCount == 0 )
        {
            // No, so flush if color is specified.
            if ( color != NoColor  )
                flush( mpDebugStats->batchColorStateFlush );
        }
        else
        {
            // Yes, so flush if color is not specified.
            if ( color == NoColor  )
                flush( mpDebugStats->batchColorStateFlush );
        }
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

    // Is a color specified?
    if ( color != NoColor )
    {
        // Yes, so add colors.
        for( U32 n = 0; n < triangleCount; ++n )
        {
            mColorBuffer[mColorCount++] = color;
            mColorBuffer[mColorCount++] = color;
            mColorBuffer[mColorCount++] = color;
        }
    }

    // Add textured vertices.
    for( U32 n = 0; n < triangleCount; ++n )
    {
        mVertexBuffer[mVertexCount++]   = *(pVertexArray++);
        mVertexBuffer[mVertexCount++]   = *(pVertexArray++);
        mVertexBuffer[mVertexCount++]   = *(pVertexArray++);
        mTextureBuffer[mTextureCoordCount++] = *(pTextureArray++);
        mTextureBuffer[mTextureCoordCount++] = *(pTextureArray++);
        mTextureBuffer[mTextureCoordCount++] = *(pTextureArray++);
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
        const ColorF& color )
{
    // Sanity!
    AssertFatal( mpDebugStats != NULL, "Debug stats have not been configured." );

    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitQuad);

    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    // Do we have anything batched?
    else if ( mTriangleCount > 0 )
    {
        // Yes, so do we have any existing colors?
        if ( mColorCount == 0 )
        {
            // No, so flush if color is specified.
            if ( color != NoColor  )
                flush( mpDebugStats->batchColorStateFlush );
        }
        else
        {
            // Yes, so flush if color is not specified.
            if ( color == NoColor  )
                flush( mpDebugStats->batchColorStateFlush );
        }
    }

    // Is a color specified?
    if ( color != NoColor )
    {
        // Yes, so add colors.
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
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

    // Is a color specified?
    if ( color != NoColor )
    {
        // Yes, so add colors.
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
        mColorBuffer[mColorCount++] = color;
    }

    // Add textured vertices.
    // NOTE: We swap #2/#3 here.
    mVertexBuffer[mVertexCount++]   = vertexPos0;
    mVertexBuffer[mVertexCount++]   = vertexPos1;
    mVertexBuffer[mVertexCount++]   = vertexPos3;
    mVertexBuffer[mVertexCount++]   = vertexPos2;
    mTextureBuffer[mTextureCoordCount++] = texturePos0;
    mTextureBuffer[mTextureCoordCount++] = texturePos1;
    mTextureBuffer[mTextureCoordCount++] = texturePos3;
    mTextureBuffer[mTextureCoordCount++] = texturePos2;

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

    GFX->setStateBlock(mGFXStateRef);
    GFXVertexBufferHandle<GFXVertexPCT> vHandle( GFX, mVertexCount, GFXBufferTypeVolatile, mVertexBuffer);
    GFX->setVertexBuffer(vHandle);

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        GFX->setTexture(0, mStrictOrderTextureHandle);

        // Yes, so do we have a single quad?
        if ( mQuadCount == 1 )
        {
            // Yes, so draw the quad using a triangle-strip with indexes.
            glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );    

            // Stats.
            mpDebugStats->batchDrawCallsStrictSingle++;

            // Stats.
            if ( mpDebugStats->batchMaxTriangleDrawn < 2 )
                mpDebugStats->batchMaxTriangleDrawn = 2;
        }
        else
        {
            // Draw the quads using triangles with indexes.
            glDrawElements( GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, mIndexBuffer );

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
            // Fetch texture binding.
            const U32 textureBinding = batchItr->key;

            // Fetch index vector.
            indexVectorType* pIndexVector = batchItr->value;

            // Iterate indexes.
            GFXPrimitive temp;
            temp.type = GFXTriangleList;
            temp.startIndex = 0;
            temp.minIndex = 65535;
            temp.numVertices = 0;
            temp.numPrimitives = 0;

            for( indexVectorType::iterator indexItr = pIndexVector->begin(); indexItr != pIndexVector->end(); ++indexItr )
            {
                // Fetch triangle run.
                const TriangleRun& triangleRun = *indexItr;

                // Add new indices.
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex--;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex--;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex;
            }
            
            GFXPrimitiveBufferHandle pbHandle(GFX, mIndexCount, temp.numPrimitives, GFXBufferTypeVolatile, mIndexBuffer, &temp);
            
            // Sanity!
            AssertFatal( mIndexCount > 0, "No batching indexes are present." );

            // Bind the texture if not in wireframe mode.
            if ( !mWireframeMode )
                glBindTexture( GL_TEXTURE_2D, textureBinding );

            // Draw the quads using triangles with indexes.
            glDrawElements( GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, mIndexBuffer );

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

    // Reset batch state.
    mTriangleCount = 0;
    mVertexCount = 0;
    mTextureResidentCount = 0;
    mIndexCount = 0;
    mColorCount = 0;

    PROFILE_END();   // T2D_BatchRender_flush
}

//-----------------------------------------------------------------------------

BatchRender::indexVectorType* BatchRender::findTextureBatch( TextureHandle& handle )
{
    glBegin( GL_TRIANGLE_STRIP );
        glTexCoord2f( texturePos0.x, texturePos0.y );
        glVertex2f( vertexPos0.x, vertexPos0.y );
        glTexCoord2f( texturePos1.x, texturePos1.y );
        glVertex2f( vertexPos1.x, vertexPos1.y );
        glTexCoord2f( texturePos3.x, texturePos3.y );
        glVertex2f( vertexPos3.x, vertexPos3.y );
        glTexCoord2f( texturePos2.x, texturePos2.y );
        glVertex2f( vertexPos2.x, vertexPos2.y );
    glEnd();
}

        // Insert into texture batch map.
        mTextureBatchMap.insert( textureBinding, pIndexVector );
    }
    else
    {
        // Yes, so fetch it.
        pIndexVector = itr->value;
    }

    return pIndexVector;
}


