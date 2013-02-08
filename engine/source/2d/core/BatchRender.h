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

#ifndef _VECTOR2_H_
#include "2d/core/Vector2.h"
#endif

#ifndef _UTILITY_H_
#include "2d/core/Utility.h"
#endif

#ifndef _DEBUG_STATS_H_
#include "2d/scene/DebugStats.h"
#endif

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureHandle.h"

#ifndef _HASHTABLE_H
#include "collection/hashTable.h"
#endif

#ifndef _COLOR_H_
#include "graphics/color.h"
#endif

//-----------------------------------------------------------------------------

#define BATCHRENDER_BUFFERSIZE      (65535)
#define BATCHRENDER_MAXQUADS        (BATCHRENDER_BUFFERSIZE/6)

//-----------------------------------------------------------------------------

class SceneRenderRequest;

//-----------------------------------------------------------------------------

class BatchRender
{
public:
    BatchRender();
    virtual ~BatchRender();

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
        if (    mGFXStateDesc.blendEnable &&
                mGFXStateDesc.blendSrc == srcFactor &&
                mGFXStateDesc.blendDest == dstFactor &&
                mBlendColor == blendColor )
                return;

        // Flush.
        flush( mpDebugStats->batchBlendStateFlush );

        mGFXStateDesc.blendEnable = true;
        mGFXStateDesc.blendSrc = srcFactor;
        mGFXStateDesc.blendDest = dstFactor;
        mBlendColor = blendColor;
        mGFXStateRef = GFX->createStateBlock( mGFXStateDesc );
    }

    /// Turns-off blend mode.
    inline void setBlendOff( void )
    {
        // Ignore no change,
        if ( !mGFXStateDesc.blendEnable )
            return;

        // Flush.
        flush( mpDebugStats->batchBlendStateFlush );

        mGFXStateDesc.blendEnable = false;
        mGFXStateRef = GFX->createStateBlock( mGFXStateDesc );
    }

    // Set blend mode using a specific scene render request.
    void setBlendMode( const SceneRenderRequest* pSceneRenderRequest );

    /// Set alpha-test mode.
    inline void setAlphaTestMode( const F32 alphaTestMode )
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
        if ( mGFXStateDesc.fillMode == enabled ? GFXFillSolid : GFXFillWireframe )
            return;

        mGFXStateDesc.fillMode = enabled ? GFXFillSolid : GFXFillWireframe;
        mGFXStateRef = GFX->createStateBlock( mGFXStateDesc );
    }

    // Gets the wireframe mode.
    inline bool getWireframeMode( void ) const { return mGFXStateDesc.fillMode == GFXFillSolid; }

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

    /// Submit a quad for batching.
    /// Vertex and textures are indexed as:
    ///  3 ___ 2
    ///   |\  |
    ///   | \ |
    ///  0| _\|1
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
            const Vector2& texturePos3 );

    /// Flush (render) any pending batches with a reason metric.
    void flush( U32& reasonMetric );

    /// Flush (render) any pending batches.
    void flush( void );

private:
    /// Flush (render) any pending batches.
    void flushInternal( void );

private:
    typedef Vector<U32> indexVectorType;
    typedef HashMap<GFXTexHandle, indexVectorType*> textureBatchType;
    
    VectorPtr< indexVectorType* > mIndexVectorPool;
    textureBatchType    mTextureBatchMap;

    const ColorF        NoColor;

    GFXVertexPCT        mVertexBuffer[BATCHRENDER_BUFFERSIZE];
    U16                 mIndexBuffer[ BATCHRENDER_BUFFERSIZE ];
   
    U32                 mQuadCount;
    U32                 mVertexCount;
    U32                 mIndexCount;

    /// Render Options.
    GFXStateBlockDesc   mGFXStateDesc;
    GFXStateBlockRef    mGFXStateRef;

    ColorF              mBlendColor;
    F32                 mAlphaTestMode;

    bool                mStrictOrderMode;
    GFXTexHandle        mStrictOrderTextureHandle;
    DebugStats*         mpDebugStats;

    bool                mBatchEnabled;
};

#endif
