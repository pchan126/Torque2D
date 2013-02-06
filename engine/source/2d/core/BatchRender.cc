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

#include "platformOSX/graphics/gfxOpenGLTextureObject.h"

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

BatchRender::BatchRender() :
    mQuadCount( 0 ),
    mVertexCount( 0 ),
    mIndexCount( 0 ),
    NoColor( -1.0f, -1.0f, -1.0f ),
    mStrictOrderMode( false ),
    mpDebugStats( NULL ),
    mBlendColor( ColorF(1.0f,1.0f,1.0f,1.0f) ),
    mAlphaTestMode( -1.0f ),
    mBatchEnabled( true )
{
    mGFXStateDesc.zDefined = true;
    mGFXStateDesc.zEnable = false;
    mGFXStateDesc.zWriteEnable = false;
    mGFXStateDesc.cullDefined = true;
    mGFXStateDesc.cullMode = GFXCullNone;
    mGFXStateDesc.blendDefined = true;
    mGFXStateDesc.blendEnable = true;
    mGFXStateDesc.blendSrc = GFXBlendSrcAlpha;
    mGFXStateDesc.blendDest = GFXBlendInvSrcAlpha;
    mGFXStateDesc.samplersDefined = true;
    mGFXStateDesc.samplers[0].alphaOp = GFXTOPModulate;
    mGFXStateDesc.samplers[0].magFilter = GFXTextureFilterLinear;
    mGFXStateDesc.samplers[0].minFilter = GFXTextureFilterLinear;
    
    mGFXStateDesc.samplers[0].addressModeU = GFXAddressClamp;
    mGFXStateDesc.samplers[0].addressModeV = GFXAddressClamp;
    mGFXStateDesc.samplers[0].alphaArg1 = GFXTATexture;
    mGFXStateDesc.samplers[0].alphaArg2 = GFXTADiffuse;
    mGFXStateDesc.samplers[0].textureColorOp = GFXTOPAdd;
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

    PROFILE_START(BatchRender_SubmitQuad);

    // Do we have anything batched?
    if ( mQuadCount > 0 )
    {
        flush( mpDebugStats->batchColorStateFlush );
    }

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Is there is a texture change.
        if ( texture != mStrictOrderTextureHandle )
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
        indexVectorType* pIndexVector = NULL;

        // Find texture binding.
        textureBatchType::iterator itr = mTextureBatchMap.find( texture );

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
            mTextureBatchMap.insert( texture, pIndexVector );
        }
        else
        {
            // Yes, so fetch it.
            pIndexVector = itr->value;
        }

        // Add vertex start.
        pIndexVector->push_back( mVertexCount );       
    }

    // Add textured vertices.
    // NOTE: We swap #2/#3 here.

    mVertexBuffer[mVertexCount+0].point.set(vertexPos0.x, vertexPos0.y, 0.0f);
    mVertexBuffer[mVertexCount+1].point.set(vertexPos1.x, vertexPos1.y, 0.0f);
    mVertexBuffer[mVertexCount+2].point.set(vertexPos3.x, vertexPos3.y, 0.0f);
    mVertexBuffer[mVertexCount+3].point.set(vertexPos2.x, vertexPos2.y, 0.0f);
    mVertexBuffer[mVertexCount+0].texCoord.set(texturePos0.x, texturePos0.y);
    mVertexBuffer[mVertexCount+1].texCoord.set(texturePos1.x, texturePos1.y);
    mVertexBuffer[mVertexCount+2].texCoord.set(texturePos3.x, texturePos3.y);
    mVertexBuffer[mVertexCount+3].texCoord.set(texturePos2.x, texturePos2.y);
    mVertexBuffer[mVertexCount+0].color.set(color);
    mVertexBuffer[mVertexCount+1].color.set(color);
    mVertexBuffer[mVertexCount+2].color.set(color);
    mVertexBuffer[mVertexCount+3].color.set(color);
    mVertexCount += 4;
    
    // Stats.
    mpDebugStats->batchTrianglesSubmitted+=2;

    // Increase quad count.
    mQuadCount++;

    // Have we reached the buffer limit?
    if ( mQuadCount == BATCHRENDER_MAXQUADS )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }

    // Is batching enabled?
    if ( !mBatchEnabled )
    {
        // No, so flush immediately.
        // NOTE: Technically this is still batching but will still revert to using
        // more draw calls therefore can be used in comparison.
        flushInternal();
    }

    PROFILE_END();   // BatchRender_SubmitQuad
}

//-----------------------------------------------------------------------------

void BatchRender::flush( U32& reasonMetric )
{
    // Finish if no quads to flush.
    if ( mQuadCount == 0 )
        return;

    // Increase reason metric.
    reasonMetric++;

    // Flush.
    flushInternal();
}

//-----------------------------------------------------------------------------

void BatchRender::flush( void )
{
    // Finish if no quads to flush.
    if ( mQuadCount == 0 )
        return;

    // Increase reason metric.
    mpDebugStats->batchAnonymousFlush++;

    // Flush.
    flushInternal();
}

//-----------------------------------------------------------------------------

void BatchRender::flushInternal( void )
{
    // Finish if no quads to flush.
    if ( mQuadCount == 0 )
        return;

    PROFILE_START(T2D_BatchRender_flush);

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
            GFX->setupGenericShaders(GFXDevice::GSTexture);
            GFX->drawPrimitive(GFXTriangleStrip, 0, 2);

            // Stats.
            mpDebugStats->batchDrawCallsStrictSingle++;

            // Stats.
            if ( mpDebugStats->batchMaxTriangleDrawn < 2 )
                mpDebugStats->batchMaxTriangleDrawn = 2;
        }
        else
        {
            GFXPrimitive temp;
            temp.type = GFXTriangleList;
            temp.startIndex = 0;
            temp.minIndex = 0;
            temp.numPrimitives = mQuadCount*2;
            temp.numVertices = mQuadCount*4;

            GFXPrimitiveBufferHandle pbHandle(GFX, mIndexCount, temp.numPrimitives, GFXBufferTypeVolatile, mIndexBuffer, &temp);
            GFX->setPrimitiveBuffer(pbHandle);
            GFX->setupGenericShaders(GFXDevice::GSTexture);
            GFX->drawPrimitives();

            // Stats.
            mpDebugStats->batchDrawCallsStrictMultiple++;

            // Stats.
            const U32 trianglesDrawn = mIndexCount / 3;
            if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
                mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;
        }

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
            const GFXTexHandle textureHandle = batchItr->key;

            // Fetch index vector.
            indexVectorType* pIndexVector = batchItr->value;

            // Reset index count.
            mIndexCount = 0;

            // Iterate indexes.
            GFXPrimitive temp;
            temp.type = GFXTriangleList;
            temp.startIndex = 0;
            temp.minIndex = 65535;
            temp.numVertices = 0;
            temp.numPrimitives = 0;

            for( indexVectorType::iterator indexItr = pIndexVector->begin(); indexItr != pIndexVector->end(); ++indexItr )
            {
                // Fetch quad index.
                U32 quadIndex = (*indexItr);

                if (quadIndex < temp.minIndex)
                    temp.minIndex = quadIndex;
                
                if ((quadIndex+4 - temp.minIndex) > temp.numVertices)
                    temp.numVertices = quadIndex+4-temp.minIndex;

                // Add new indices.
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex++;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex--;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex--;
                mIndexBuffer[mIndexCount++] = (U16)quadIndex;
                temp.numPrimitives = +2;
            }
            
            GFXPrimitiveBufferHandle pbHandle(GFX, mIndexCount, temp.numPrimitives, GFXBufferTypeVolatile, mIndexBuffer, &temp);
            
            // Sanity!
            AssertFatal( mIndexCount > 0, "No batching indexes are present." );

            GFX->setTexture(0, textureHandle);
            GFX->setPrimitiveBuffer(pbHandle);
            GFX->setupGenericShaders(GFXDevice::GSTexture);
            GFX->drawPrimitives();

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
    mQuadCount = 0;
    mVertexCount = 0;
    mIndexCount = 0;

    PROFILE_END();   // T2D_BatchRender_flush
}

//-----------------------------------------------------------------------------

void BatchRender::RenderQuad(
        const Vector2& vertexPos0,
        const Vector2& vertexPos1,
        const Vector2& vertexPos2,
        const Vector2& vertexPos3,
        const Vector2& texturePos0,
        const Vector2& texturePos1,
        const Vector2& texturePos2,
        const Vector2& texturePos3 )
{
    GFXVertexPCT verts[4];
    
    verts[0].point.set(vertexPos0.x, vertexPos0.y, 0.0f);
    verts[1].point.set(vertexPos1.x, vertexPos1.y, 0.0f);
    verts[2].point.set(vertexPos3.x, vertexPos3.y, 0.0f);
    verts[3].point.set(vertexPos2.x, vertexPos2.y, 0.0f);
    verts[0].texCoord.set(texturePos0.x, texturePos0.y);
    verts[1].texCoord.set(texturePos1.x, texturePos1.y);
    verts[2].texCoord.set(texturePos3.x, texturePos3.y);
    verts[3].texCoord.set(texturePos2.x, texturePos2.y);
    verts[0].color.set(255, 255, 255);
    verts[1].color.set(255, 255, 255);
    verts[2].color.set(255, 255, 255);
    verts[3].color.set(255, 255, 255);
    
    GFXVertexBufferHandle<GFXVertexPCT> vHandle( GFX, 4, GFXBufferTypeVolatile, verts);
    GFX->setVertexBuffer(vHandle);
    
    GFX->setupGenericShaders(GFXDevice::GSTexture);
    GFX->drawPrimitive(GFXTriangleStrip, 0, 2);
}


