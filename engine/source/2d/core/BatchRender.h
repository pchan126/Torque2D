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

#ifndef _BATCH_RENDER_H_
#define _BATCH_RENDER_H_

#include "2d/core/Vector2.h"
#include "2d/core/Utility.h"
#include "2d/scene/DebugStats.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureHandle.h"
#include "collection/hashTable.h"
#include "graphics/color.h"
#include "2d/assets/ShaderAsset.h"

//-----------------------------------------------------------------------------

#define BATCHRENDER_BUFFERSIZE      (65565)
#define BATCHRENDER_MAXTRIANGLES    (BATCHRENDER_BUFFERSIZE/3)

//-----------------------------------------------------------------------------

class SceneRenderRequest;

//-----------------------------------------------------------------------------

class BatchRender
{
private:
    struct indexedPrim
    {
        indexedPrim( const GFXPrimitiveType primitive, const U32 primitiveCount, const U32 startIndex ) :
            mPrimitiveMode( primitive ),
            mPrimitiveCount( primitiveCount ),
            mStartIndex( startIndex )
        {
            verts = new Vector<GFXVertexPCT>;
            index = new Vector<U16>;
        }
        
        ~indexedPrim()
        {
            delete verts;
            delete index;
        }
        
        GFXPrimitiveType mPrimitiveMode;
        U32 mPrimitiveCount;
        U32 mStartIndex;

        Vector<GFXVertexPCT>* verts;
        Vector<U16>* index;
    };

    typedef HashMap<GFXTexHandle, indexedPrim*> textureBatchType;
    Vector< indexedPrim* > mIndexVectorPool;
    textureBatchType    mTextureBatchMap;

    const ColorF        NoColor;

    Vector<GFXVertexPCT> mVertexBuffer;
    GFXVertexBufferHandle<GFXVertexPCT> mTempVertBuffHandle;
    Vector<U16>          mIndexBuffer;
    
   
    U32                 mTriangleCount;

    bool                mBlendMode;
    GFXBlend              mSrcBlendFactor;
    GFXBlend              mDstBlendFactor;
    ColorF              mBlendColor;
    F32                 mAlphaTestMode;

    bool                mStrictOrderMode;
    GFXTexHandle       mStrictOrderTextureHandle;
    DebugStats*         mpDebugStats;

    bool                mWireframeMode;
    bool                mBatchEnabled;

    GFXShaderRef            mShader;
    GFXShaderConstBufferRef mShaderConstBuffer;
public:
    BatchRender();
    virtual ~BatchRender();

    inline void setShader ( ShaderAsset shader, const bool forceFlush = false )
    {
        // Ignore if no change.
        if ( !forceFlush && shader.getShader() == mShader )
            return;

        // Flush.
        flushInternal();
        mShader = shader.getShader();
        mShaderConstBuffer = shader.getShaderConstBuffer();
    }

    inline void clearShader(const bool forceFlush = false )
    {
        if ( !forceFlush && mShader.isNull())
            return;

        flushInternal();
        mShader = nullptr;
        mShaderConstBuffer = nullptr;
    }

    /// Gets the strict order mode.
    inline GFXShaderRef getShader( void ) const { return mShader; }

    /// Set the strict order mode.
    inline void setStrictOrderMode( const bool strictOrder, const bool forceFlush = false )
    {
        // Ignore if no change.
        if ( !forceFlush && strictOrder == mStrictOrderMode )
            return;

        // Flush.
        flushInternal();

        // Update strict order mode.
        mStrictOrderMode = strictOrder;
    }

    /// Gets the strict order mode.
    inline bool getStrictOrderMode( void ) const { return mStrictOrderMode; }

    /// Turns-on blend mode with the specified blend factors and color.
    inline void setBlendMode( GFXBlend srcFactor, GFXBlend dstFactor, const ColorF& blendColor = ColorF(1.0f, 1.0f, 1.0f, 1.0f))
    {
        // Ignore no change.
        if (    mBlendMode &&
                mSrcBlendFactor == srcFactor &&
                mDstBlendFactor == dstFactor &&
                mBlendColor == blendColor )
                return;

        // Flush.
        flush( mpDebugStats->batchBlendStateFlush );

        mBlendMode = true;
        mSrcBlendFactor = srcFactor;
        mDstBlendFactor = dstFactor;
        mBlendColor = blendColor;
    }

    /// Turns-off blend mode.
    inline void setBlendOff( void )
    {
        // Ignore no change,
        if ( !mBlendMode )
            return;

        // Flush.
        flush( mpDebugStats->batchBlendStateFlush );

        mBlendMode = false;
    }

    // Set blend mode using a specific scene render request.
    void setBlendMode( const SceneRenderRequest* pSceneRenderRequest );

    /// Set alpha-test mode.
    void setAlphaTestMode( const F32 alphaTestMode )
    {
        // Ignore no change.
        if ( mIsEqual( mAlphaTestMode, alphaTestMode ) )
            return;

        // Flush.
        flush( mpDebugStats->batchAlphaStateFlush );

        // Stats.
        mpDebugStats->batchAlphaStateFlush++;

        mAlphaTestMode = alphaTestMode;
    }

    // Set alpha test mode using a specific scene render request.
    void setAlphaTestMode( const SceneRenderRequest* pSceneRenderRequest );

    /// Sets the wireframe mode.
    inline void setWireframeMode( const bool enabled )
    {
        // Ignore no change.
        if ( mWireframeMode == enabled )
            return;

        mWireframeMode = enabled;
    }

    // Gets the wireframe mode.
    inline bool getWireframeMode( void ) const { return mWireframeMode; }

    /// Sets the batch enabled mode.
    inline void setBatchEnabled( const bool enabled )
    {
        // Ignore no change.
        if ( mBatchEnabled == enabled )
            return;

        // Flush.
        flushInternal();

        mBatchEnabled = enabled;
    }

    /// Gets the batch enabled mode.
    inline bool getBatchEnabled( void ) const { return mBatchEnabled; }

    /// Sets the debug stats to use.
    inline void setDebugStats( DebugStats* pDebugStats ) { mpDebugStats = pDebugStats; }

    /// Submit triangles for batching.
    /// Vertex and textures are indexed as:
    ///  2        5
    ///   |\      |\
    ///   | \     | \
    ///  0| _\1  3| _\4
    void SubmitTriangles(
            const U32 vertexCount,
            const Vector2* pVertexArray,
            const Vector2* pTextureArray,
            GFXTexHandle& texture,
            const ColorF& color = ColorF(-1.0f, -1.0f, -1.0f) );

    void SubmitTriangleStrip( const Vector<GFXVertexPCT> verts, GFXTexHandle& texture);

    void SubmitIndexedTriangleStrip(const Vector<GFXVertexPCT> &verts,
                                   GFXTexHandle &texture,
                                   const Vector<U16> &indices);

   /// Submit a quad for batching.
    /// Vertex and textures are indexed as:
    ///  3 ___ 2
    ///   |\  |
    ///   | \ |
    ///  0| _\|1
    void SubmitQuad( const std::array< GFXVertexPCT, 4>,
                     GFXTexHandle& texture );
   
    void SubmitQuad(
            const Vector2& vertexPos0,
            const Vector2& vertexPos1,
            const Vector2& vertexPos2,
            const Vector2& vertexPos3,
            const Vector2& texturePos0,
            const Vector2& texturePos1,
            const Vector2& texturePos2,
            const Vector2& texturePos3,
            GFXTexHandle& texture,
            const ColorF& color = ColorF(1.0f, 1.0f, 1.0f) );

    /// Render a quad immediately without affecting current batch.
    /// All render state should be set beforehand directly.
    /// Vertex and textures are indexed as:
    ///  3 ___ 2
    ///   |\  |
    ///   | \ |
    ///  0| _\|1
    static void RenderQuad(
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

    /// Flush (render) any pending batches with a reason metric.
    void flush( U32& reasonMetric );

    /// Flush (render) any pending batches.
    void flush( void );

private:
    /// Flush (render) any pending batches.
    void flushInternal( void );

   void _lightAndDraw( Vector<GFXVertexPCT>* pVertexVector, Vector<U16>* pIndex, GFXTexHandle handle = NULL);

   /// Find texture batch.
    indexedPrim* findTextureBatch( GFXTexHandle& handle );

};

#endif
