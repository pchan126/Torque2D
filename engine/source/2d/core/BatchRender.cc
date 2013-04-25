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
    for ( VectorPtr< Vector<GFXVertexPCT> * >::iterator itr = mIndexVectorPool.begin(); itr != mIndexVectorPool.end(); ++itr )
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
    
    Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && mVertexBuffer.size() > 0 )
        {
            // Yes, so flush.
            flush( mpDebugStats->batchTextureChangeFlush );
        }

        // Set strict order mode texture handle.
        mStrictOrderTextureHandle = texture;
    }
    else
    {
        // No, so add triangle run.
        vertBuffer = findTextureBatch( texture );
    }

    // Add textured vertices.
    for( U32 n = 0; n < triangleCount; ++n )
    {
        GFXVertexPCT vert[3];
        vert[0].point.set(pVertexArray[n].x, pVertexArray[n].y, 0.0f);
        vert[0].texCoord.set(pTextureArray[n].x, pTextureArray[n].y);
        vert[0].color       = color;
        vert[1].point.set(pVertexArray[n+1].x, pVertexArray[n+1].y, 0.0f);
        vert[1].texCoord.set(pTextureArray[n+1].x, pTextureArray[n+1].y);
        vert[1].color       = color;
        vert[2].point.set(pVertexArray[n+2].x, pVertexArray[n+2].y, 0.0f);
        vert[2].texCoord.set(pTextureArray[n+2].x, pTextureArray[n+2].y);
        vert[2].color       = color;

        // degenerate joining triangle
        if (n != 0 && vertBuffer->size() > 0)
        {
            vertBuffer->push_back(vertBuffer->last());
            vertBuffer->push_back(vert[0]);
        }

        // Add textured vertices.
        // NOTE: We swap #2/#3 here.
        vertBuffer->push_back(vert[0]);
        vertBuffer->push_back(vert[1]);
        vertBuffer->push_back(vert[2]);
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
                             GFXTexHandle& texture)
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
        flush( mpDebugStats->batchColorStateFlush );
    }
    
    Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && mVertexBuffer.size() > 0 )
        {
            // Yes, so flush.
            flush( mpDebugStats->batchTextureChangeFlush );
        }
        
        // Set strict order mode texture handle.
        mStrictOrderTextureHandle = texture;
    }
    else
    {
        // No, so add triangle run.
        vertBuffer = findTextureBatch( texture );
    }
    
    // degenerate linking triangle
    if (mVertexBuffer.size() > 0)
    {
        vertBuffer->push_back(vertBuffer->last());
        vertBuffer->push_back(vertex[0]);
    }
    
    // Add textured vertices.
    // NOTE: We swap #2/#3 here.
    vertBuffer->push_back(vertex[0]);
    vertBuffer->push_back(vertex[1]);
    vertBuffer->push_back(vertex[3]);
    vertBuffer->push_back(vertex[2]);
    
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
    
    GFXVertexPCT verts[4];
    
    verts[0].point.set(vertexPos0.x, vertexPos0.y, 0.0f);
    verts[0].texCoord.set(texturePos0.x, texturePos0.y);
    verts[0].color = color;
    verts[1].point.set(vertexPos1.x, vertexPos1.y, 0.0f);
    verts[1].texCoord.set(texturePos1.x, texturePos1.y);
    verts[1].color = color;
    verts[2].point.set(vertexPos2.x, vertexPos2.y, 0.0f);
    verts[2].texCoord.set(texturePos2.x, texturePos2.y);
    verts[2].color = color;
    verts[3].point.set(vertexPos3.x, vertexPos3.y, 0.0f);
    verts[3].texCoord.set(texturePos3.x, texturePos3.y);
    verts[3].color = color;

    SubmitQuad(verts, texture);
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
    desc.setCullMode(GFXCullNone);

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

        mTempVertBuffHandle.set(GFX, mVertexBuffer.size(), GFXBufferTypeVolatile, mVertexBuffer.address() );
        GFX->setVertexBuffer( mTempVertBuffHandle );
        
        // Draw the triangles
        GFX->setupGenericShaders(GFXDevice::GSTexture);
        GFX->drawPrimitive(GFXTriangleStrip, 0, mVertexBuffer.size()-2);

        // Stats.
        mpDebugStats->batchDrawCallsStrict++;

        // Stats.
        const U32 trianglesDrawn = mVertexBuffer.size()-2;
        if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
            mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

        // Stats.
        if ( mVertexBuffer.size() > mpDebugStats->batchMaxVertexBuffer )
            mpDebugStats->batchMaxVertexBuffer = mVertexBuffer.size();
    }
    else
    {
        // No, so iterate texture batch map.
        for( textureBatchType::iterator batchItr = mTextureBatchMap.begin(); batchItr != mTextureBatchMap.end(); ++batchItr )
        {
            // Bind the texture if not in wireframe mode.
            GFX->setTexture(0, batchItr->key);
            Vector<GFXVertexPCT>* pVertexVector = batchItr->value;

            mTempVertBuffHandle.set(GFX, pVertexVector->size(), GFXBufferTypeVolatile, pVertexVector->address() );
            GFX->setVertexBuffer( mTempVertBuffHandle );

            // Draw the triangles.
            GFX->setupGenericShaders(GFXDevice::GSTexture);
            GFX->drawPrimitive(GFXTriangleStrip, 0, pVertexVector->size()-2);

            // Stats.
            mpDebugStats->batchDrawCallsSorted++;

            // Stats.
            if ( pVertexVector->size()-2 > mpDebugStats->batchMaxVertexBuffer )
                mpDebugStats->batchMaxVertexBuffer = pVertexVector->size()-2;

            // Stats.
            const U32 trianglesDrawn = pVertexVector->size()-2;
            if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
                mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

            // Return index vector to pool.
            pVertexVector->clear();
            mIndexVectorPool.push_back( pVertexVector );
        }

        // Clear texture batch map.
        mTextureBatchMap.clear();
    }
    // Reset batch state.
    mTriangleCount = 0;
    mVertexBuffer.clear();
}

//-----------------------------------------------------------------------------

Vector<GFXVertexPCT> * BatchRender::findTextureBatch( GFXTexHandle& handle )
{
    Vector<GFXVertexPCT> * pIndexVector = NULL;

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
            pIndexVector = new Vector<GFXVertexPCT>;
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


