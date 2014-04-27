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
#include "2d/sceneobject/SceneObject.h"
#include <array>

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

BatchRender::BatchRender() :
    mTriangleCount( 0 ),
    mStrictOrderMode( false ),
    mpDebugStats( nullptr ),
    mBlendMode(true),
    mSrcBlendFactor( GFXBlendSrcAlpha),
    mDstBlendFactor( GFXBlendInvSrcAlpha),
    mBlendColor( ColorF(1.0f,1.0f,1.0f,1.0f) ),
    mBatchEnabled( true )
{
   mBatchDesc.setCullMode(GFXCullNone);
   mBatchDesc.zEnable = false;
   
   mWireframeBatchDesc.setCullMode(GFXCullNone);
   mWireframeBatchDesc.zEnable = false;
   mWireframeBatchDesc.fillMode = GFXFillWireframe;
}

//-----------------------------------------------------------------------------

BatchRender::~BatchRender()
{
    // Destroy index vectors in texture batch map.
    for ( auto itr:mTextureBatchMap )
        delete itr.second;

    mTextureBatchMap.clear();

    // Destroy index vectors in index vector pool.
    for ( indexedPrim* itr:mIndexVectorPool )
        delete itr;

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

// OMG this is slow - use SubmitIndexedTriangleStrip instead!
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
    AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );
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
//    // Do we have anything batched?
//    else if ( mTriangleCount > 0 )
//    {
//        flush( mpDebugStats->batchColorStateFlush );
//    }
   
    
    Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;
    Vector<U16>* indexBuffer = &mIndexBuffer;

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && !mVertexBuffer.empty() )
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
        vertBuffer = findTextureBatch( texture )->verts;
        indexBuffer = findTextureBatch(texture)->index;
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
        if (n != 0 && !vertBuffer->empty() )
        {
            indexBuffer->push_back(vertBuffer->push_back_unique(vertBuffer->back()));
            indexBuffer->push_back(vertBuffer->push_back_unique(vert[0]));
        }

        // Add textured vertices.
        // NOTE: We swap #2/#3 here.
        indexBuffer->push_back(vertBuffer->push_back_unique(vert[0]));
        indexBuffer->push_back(vertBuffer->push_back_unique(vert[1]));
        indexBuffer->push_back(vertBuffer->push_back_unique(vert[2]));
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

void BatchRender::SubmitTriangleStrip( const Vector<GFXVertexPCT> verts, GFXTexHandle& texture)
{
    // Sanity!
    AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );
    
    if (verts.empty())
        return;
    
    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitTriangleStrip);
    
    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + verts.size()/3) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
    
    Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;
    Vector<U16>* indexBuffer = &mIndexBuffer;
    
    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if ( texture != mStrictOrderTextureHandle && !mVertexBuffer.empty() )
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
        vertBuffer = findTextureBatch( texture )->verts;
        indexBuffer = findTextureBatch(texture)->index;
    }
    
    // degenerate linking triangle
    if (!vertBuffer->empty())
    {
        indexBuffer->push_back(vertBuffer->push_back_unique(vertBuffer->back()));
        indexBuffer->push_back(vertBuffer->push_back_unique(verts[0]));
    }
    
    for (auto itr:verts)
        indexBuffer->push_back(vertBuffer->push_back_unique(itr));

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


void BatchRender::SubmitIndexedTriangleStrip(const Vector<GFXVertexPCT> &verts, GFXTexHandle &texture, const Vector<U16> &in_indexBuffer)
{
   // Sanity!
   AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );
   
   // Debug Profiling.
   PROFILE_SCOPE(BatchRender_SubmitQuad);
   
   
   // Would we exceed the triangle buffer size?
   if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
   {
      // Yes, so flush.
      flush( mpDebugStats->batchBufferFullFlush );
   }
   
   Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;
   Vector<U16>* indexBuffer = &mIndexBuffer;
   
   // Strict order mode?
   if ( mStrictOrderMode )
   {
      // Yes, so is there a texture change?
      if ( texture != mStrictOrderTextureHandle && !mVertexBuffer.empty() )
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
      vertBuffer = findTextureBatch( texture )->verts;
      indexBuffer = findTextureBatch(texture)->index;
   }
   
   U16 indexOffset = vertBuffer->size();

   // degenerate linking triangle
   if (!vertBuffer->empty() )
   {
      indexBuffer->push_back(indexBuffer->back());
      indexBuffer->push_back(in_indexBuffer.front());
   }
   
   // Add textured vertices.
   vertBuffer->merge(verts);

   for (auto i:in_indexBuffer )
      indexBuffer->push_back(i+indexOffset);
   
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


void BatchRender::SubmitQuad(const std::array< GFXVertexPCT, 4> verts,
                             GFXTexHandle& texture)
{
    // Sanity!
    AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );
    
    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitQuad);

    // Would we exceed the triangle buffer size?
    if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
    {
        // Yes, so flush.
        flush( mpDebugStats->batchBufferFullFlush );
    }
   
    Vector<GFXVertexPCT>* vertBuffer = &mVertexBuffer;
    Vector<U16>* indexBuffer = &mIndexBuffer;

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Yes, so is there a texture change?
        if (texture != mStrictOrderTextureHandle && !mVertexBuffer.empty() )
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
        vertBuffer = findTextureBatch( texture )->verts;
        indexBuffer = findTextureBatch(texture)->index;
    }
    
   U16 indexOffset = vertBuffer->size();
   std::array< U16 , 4> in_indexBuffer = {0, 1, 3, 2};
   
   // degenerate linking triangle
   if (!vertBuffer->empty())
   {
//       Con::printf("indexBuffer->pushback %d %d", indexBuffer->back(), in_indexBuffer.front());

      indexBuffer->push_back(indexBuffer->back());
      indexBuffer->push_back(in_indexBuffer.front()+indexOffset);
   }
   
   // Add textured vertices.
   vertBuffer->merge(verts.begin(), verts.end());
   
   for (auto i:in_indexBuffer )
   {
      indexBuffer->push_back(i+indexOffset);
//      Con::printf("indexBuffer->pushback %d", i+indexOffset);
   }

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

void BatchRender::SubmitPoint( const Vector2 position, const float size, GFXTexHandle& texture,
                 const ColorF& color )
{
   GFXVertexPCS vert;
   vert.point.set(position.x, position.y, 0.0f);
   vert.color = color;
   vert.size = size;

   // Sanity!
   AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );
   
   // Debug Profiling.
   PROFILE_SCOPE(BatchRender_SubmitPoint);
   
   
   // Would we exceed the triangle buffer size?
   if ( (mTriangleCount + 2) > BATCHRENDER_MAXTRIANGLES )
   {
      // Yes, so flush.
      flush( mpDebugStats->batchBufferFullFlush );
   }
   
   Vector<GFXVertexPCS>* vertBuffer = &mPointBuffer;
   Vector<U16>* indexBuffer = &mPointIndexBuffer;
   
   // Strict order mode?
   if ( mStrictOrderMode )
   {
      // Yes, so is there a texture change?
      if ( texture != mStrictOrderTextureHandle && !mVertexBuffer.empty() )
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
      vertBuffer  = findTexturePointBatch( texture )->verts;
      indexBuffer = findTexturePointBatch(texture)->index;
   }
   
   U16 indexOffset = vertBuffer->size();
//   std::array< U16 , 4> in_indexBuffer = {0, 1, 3, 2};
//   
//   // degenerate linking triangle
//   if (!vertBuffer->empty())
//   {
//      //       Con::printf("indexBuffer->pushback %d %d", indexBuffer->back(), in_indexBuffer.front());
//      
//      indexBuffer->push_back(indexBuffer->back());
//      indexBuffer->push_back(in_indexBuffer.front()+indexOffset);
//   }

   vertBuffer->push_back(vert);
   indexBuffer->push_back(indexOffset);
   
   // Add textured vertices.
//   vertBuffer->merge(verts.begin(), verts.end());
   
//   for (auto i:in_indexBuffer )
//   {
//      //      Con::printf("indexBuffer->pushback %d", i+indexOffset);
//   }
   
   // Stats.
//   mpDebugStats->batchTrianglesSubmitted+=2;
   
   // Increase triangle count.
   mTriangleCount += 1;
   
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
    AssertFatal( mpDebugStats != nullptr, "Debug stats have not been configured." );

    // Debug Profiling.
    PROFILE_SCOPE(BatchRender_SubmitQuad);
    
    std::array< GFXVertexPCT, 4> verts;
   
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

    if ( mWireframeMode )
    {
//        mBatchDesc.setFillModeWireframe();
    }
    else
    {
        GFX->setTexture(0, mStrictOrderTextureHandle);
    }

    // Set blend mode.
    if ( mBlendMode )
    {
        mBatchDesc.setBlend(true, mSrcBlendFactor, mDstBlendFactor, GFXBlendOpAdd);
    }

    // Set alpha-blend mode.
    if ( mAlphaTestMode >= 0.0f )
    {
        mBatchDesc.setAlphaTest(true, GFXCmpGreaterEqual, mAlphaTestMode*255.0);
    }

    GFX->setStateBlockByDesc( mBatchDesc );

    // Strict order mode?
    if ( mStrictOrderMode )
    {
        // Bind the texture if not in wireframe mode.
        if ( !mWireframeMode )
        {
           if (!mIndexBuffer.empty())
              _lightAndDrawTriangleStrip(&mVertexBuffer, &mIndexBuffer, mStrictOrderTextureHandle);
           if (!mPointIndexBuffer.empty())
              _lightAndDrawPoints(&mPointBuffer, &mPointIndexBuffer, mStrictOrderTextureHandle);
        }
      else
            _drawWireframe(  &mVertexBuffer, &mIndexBuffer );

        // Stats.
        mpDebugStats->batchDrawCallsStrict++;

        // Stats.
        const size_t trianglesDrawn = mVertexBuffer.size()-2;
        if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
            mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

        // Stats.
        if ( mVertexBuffer.size() > mpDebugStats->batchMaxVertexBuffer )
            mpDebugStats->batchMaxVertexBuffer = mVertexBuffer.size();
    }
    else
    {
        // No, so iterate texture batch map.
        for( auto batchItr:mTextureBatchMap )
        {
            // Fetch indexedPrim
            indexedPrim* indexPrim = batchItr.second;
            _lightAndDrawTriangleStrip(indexPrim->verts, indexPrim->index, batchItr.first);

           // Stats.
            mpDebugStats->batchDrawCallsSorted++;

            // Stats.
            const size_t trianglesDrawn = indexPrim->verts->size()-2;
            if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
                mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

           // Stats.
           if ( indexPrim->verts->size() > mpDebugStats->batchMaxVertexBuffer )
              mpDebugStats->batchMaxVertexBuffer = indexPrim->verts->size();

           // Return index vector to pool.
            indexPrim->verts->clear();
            indexPrim->index->clear();
            mIndexVectorPool.push_back( indexPrim );
        }

        // Clear texture batch map.
        mTextureBatchMap.clear();

        // No, so iterate texture batch map.
        for( auto batchItr:mTexturePointBatchMap )
        {
            // Fetch indexedPrim
            indexedPoints* indexedPoints = batchItr.second;
            _lightAndDrawPoints(indexedPoints->verts, indexedPoints->index, batchItr.first);

            // Stats.
            mpDebugStats->batchDrawCallsSorted++;

            // Stats.
//            const size_t trianglesDrawn = indexedPoints->verts->size()-2;
//            if ( trianglesDrawn > mpDebugStats->batchMaxTriangleDrawn )
//                mpDebugStats->batchMaxTriangleDrawn = trianglesDrawn;

            // Stats.
//            if ( indexedPoints->verts->size() > mpDebugStats->batchMaxVertexBuffer )
//                mpDebugStats->batchMaxVertexBuffer = indexedPoints->verts->size();

            // Return index vector to pool.
            indexedPoints->verts->clear();
            indexedPoints->index->clear();
            mIndexPointVectorPool.push_back(indexedPoints);
        }
        // Clear texture batch map.
        mTexturePointBatchMap.clear();
    }
    // Reset batch state.
    mTriangleCount = 0;
    mVertexBuffer.clear();
    mIndexBuffer.clear();
    mPointBuffer.clear();
    mPointIndexBuffer.clear();
}

void BatchRender::_drawWireframe( Vector<GFXVertexPCT>* pVertexVector, Vector<U16>* pIndex)
{
    Vector<U16> wireVerts;
    if (pIndex->size() < 3)
       return;
   
    auto itr = pIndex->begin();
    while ((itr != pIndex->end()) && (itr+1 != pIndex->end()) && (itr+2 != pIndex->end()))
    {
       wireVerts.push_back(*itr);
       wireVerts.push_back(*(itr+1));
       wireVerts.push_back(*(itr+1));
       wireVerts.push_back(*(itr+2));
       wireVerts.push_back(*itr);
       wireVerts.push_back(*(itr+2));
       itr++;
    }
   
    mTempVertBuffHandle.set(GFX, (U32)pVertexVector->size(), GFXBufferTypeVolatile, pVertexVector->address(), (U32)wireVerts.size(), wireVerts.address() );
    GFX->setVertexBuffer( mTempVertBuffHandle );

    GFX->setupGenericShaders();
    GFX->drawIndexedPrimitive(GFXLineList, 0, 0, (U32)pVertexVector->size(), (U32)wireVerts.size(), (U32)wireVerts.size()/2);
}


void BatchRender::_lightAndDrawTriangleStrip(Vector<GFXVertexPCT> *pVertexVector, Vector<U16> *pIndex, GFXTexHandle handle)
{
   // Bind the texture if not in wireframe mode.
   if (!handle.isNull())
      GFX->setTexture(0, handle);
   
//   // vertex lighting
//   for (int i = 0; i < mVertexBuffer.size(); i++)
//   {
//      LightInfo* light = nullptr;
//      LightQuery query;
//      query.init( SphereF( pVertexVector->at(i).point, 500.0) );
//      query.getLights( &light, 1 );
//      if (light != nullptr)
//      {
//         F32 len = (light->getPosition()-pVertexVector->at(i).point).len();
//         F32 rad = light->getRange().x;
//         F32 factor = 1.0-mClampF( (len-rad)/rad, 0.0, 1.0 );
//         if (factor > 0.0)
//         {
//            U8 alpha = mVertexBuffer[i].color;
//            ColorF lightColor = light[0].getColor()*factor;
//            ColorF lightAdd = ColorF(mVertexBuffer[i].color) + lightColor;
//            lightAdd.clamp();
//            mVertexBuffer[i].color = lightAdd;
//            mVertexBuffer[i].color.alpha = alpha;
//         }
//      }
//   }
   
   mTempVertBuffHandle.set(GFX, (U32)pVertexVector->size(), GFXBufferTypeVolatile, pVertexVector->address(), (U32)pIndex->size(), pIndex->address() );
   GFX->setVertexBuffer( mTempVertBuffHandle );
   
   // Draw the triangles.
   if (mShader.isNull())
   {
       GFX->setupGenericShaders(GFXDevice::GSTexture);
   }
    else
   {
       MatrixF xform(GFX->getWorldMatrix());
       xform *= GFX->getViewMatrix();
       xform *= GFX->getProjectionMatrix();
       xform.transpose();
       GFX->setShader(mShader);
       GFX->setShaderConstBuffer(mShaderConstBuffer);
       mShaderConstBuffer->setSafe( mShader->getShaderConstHandle("$mvp_matrix"), xform );
       mShaderConstBuffer->setSafe( mShader->getShaderConstHandle("$sampler2d_0"), 0);
   }

    GFX->drawIndexedPrimitive(GFXTriangleStrip, 0, 0, (U32)pVertexVector->size(), (U32)pIndex->size(), (U32)pIndex->size()-2);
}


//-----------------------------------------------------------------------------

void BatchRender::_lightAndDrawPoints(Vector<GFXVertexPCS> *pVertexVector, Vector<U16> *pIndex, GFXTexHandle handle) {
    // Bind the texture if not in wireframe mode.
    if (!handle.isNull())
        GFX->setTexture(0, handle);

    mPointBuffHandle.set(GFX, (U32)pVertexVector->size(), GFXBufferTypeVolatile, pVertexVector->address(), (U32)pIndex->size(), pIndex->address() );
    GFX->setVertexBuffer( mPointBuffHandle );

    // Draw the triangles.
    if (mShader.isNull())
    {
        GFX->setupGenericShaders(GFXDevice::GSPoint);
    }
    else
    {
        MatrixF xform(GFX->getWorldMatrix());
        xform *= GFX->getViewMatrix();
        xform *= GFX->getProjectionMatrix();
        xform.transpose();
        GFX->setShader(mShader);
        GFX->setShaderConstBuffer(mShaderConstBuffer);
        mShaderConstBuffer->setSafe( mShader->getShaderConstHandle("$mvp_matrix"), xform );
        mShaderConstBuffer->setSafe( mShader->getShaderConstHandle("$sampler2d_0"), 0);
    }

    GFX->drawIndexedPrimitive(GFXPointList, 0, 0, (U32)pVertexVector->size(), (U32)pIndex->size(), (U32)pIndex->size());
}


//-----------------------------------------------------------------------------


BatchRender::indexedPrim* BatchRender::findTextureBatch( GFXTexHandle& handle )
{
//    Vector<GFXVertexPCT> * pIndexVector = nullptr;
    indexedPrim* pIndexVector = nullptr;

    // Find texture binding.
    textureBatchType::iterator itr = mTextureBatchMap.find( handle );

    // Did we find a texture binding?
    if ( itr == mTextureBatchMap.end() )
    {
        // No, so fetch index vector pool count.
        const size_t indexVectorPoolCount = mIndexVectorPool.size();

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
            pIndexVector = new indexedPrim( GFXTriangleStrip, 0, 0 );
        }

        // Insert into texture batch map.
        mTextureBatchMap.insert( handle, pIndexVector );
    }
    else
    {
        // Yes, so fetch it.
        pIndexVector = itr->second;
    }

    return pIndexVector;
}


BatchRender::indexedPoints *BatchRender::findTexturePointBatch(GFXTexHandle &handle) {
//    Vector<GFXVertexPCT> * pIndexVector = nullptr;
    indexedPoints* pIndexVector = nullptr;

    // Find texture binding.
    auto itr = mTexturePointBatchMap.find( handle );

    // Did we find a texture binding?
    if ( itr == mTexturePointBatchMap.end() )
    {
        // No, so fetch index vector pool count.
        const size_t indexPointVectorPoolCount = mIndexPointVectorPool.size();

        // Do we have any in the index vector pool?
        if ( indexPointVectorPoolCount > 0 )
        {
            // Yes, so use it.
            pIndexVector = mIndexPointVectorPool[indexPointVectorPoolCount-1];
            mIndexPointVectorPool.pop_back();
        }
        else
        {
            // No, so generate one.
            pIndexVector = new indexedPoints( GFXPointList, 0, 0 );
        }

        // Insert into texture batch map.
        mTexturePointBatchMap.insert( handle, pIndexVector );
    }
    else
    {
        // Yes, so fetch it.
        pIndexVector = itr->second;
    }

    return pIndexVector;
}
