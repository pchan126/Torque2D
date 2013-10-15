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

#ifndef _IMAGE_FRAME_PROVIDER_CORE_H
#define _IMAGE_FRAME_PROVIDER_CORE_H

#include "2d/assets/ImageAsset.h"
#include "2d/assets/AnimationAsset.h"
#include "platform/Tickable.h"
#include "assets/assetPtr.h"
#include "BatchRender.h"
#include "memory/factoryCache.h"
#include "gui/guiControl.h"

///-----------------------------------------------------------------------------

class ImageFrameProviderCore :
    public virtual Tickable,
    public IFactoryObjectReset,
    protected AssetPtrCallback
{
protected:
    bool                                    mSelfTick;

    bool                                    mStaticProvider;

    U32                                     mImageFrame;
    AssetPtr<ImageAsset>*                   mpImageAsset;
    AssetPtr<AnimationAsset>*               mpAnimationAsset;

    S32                                     mLastFrameIndex;
    S32                                     mCurrentFrameIndex;
    size_t                                mMaxFrameIndex;
    F32                                     mCurrentTime;
    F32                                     mPausedTime;
    F32                                     mCurrentModTime;
    F32                                     mAnimationTimeScale;
    F32                                     mTotalIntegrationTime;
    F32                                     mFrameIntegrationTime;
    bool                                    mAnimationPaused;
    bool                                    mAnimationFinished;

public:
    ImageFrameProviderCore();
    virtual ~ImageFrameProviderCore();

    void allocateAssets( AssetPtr<ImageAsset>* pImageAssetPtr, AssetPtr<AnimationAsset>* pAnimationAssetPtr );
    inline void deallocateAssets( void ) { mpImageAsset = nullptr; mpAnimationAsset = nullptr; }

    virtual void copyTo( ImageFrameProviderCore* pImageFrameProviderCore ) const;

    /// Integration.
    virtual bool update( const F32 elapsedTime );
    virtual void processTick();
    virtual void interpolateTick( F32 delta ) {};
    virtual void advanceTime( F32 timeDelta ) {};
    virtual void setProcessTicks( bool tick  ) { Tickable::setProcessTicks( mSelfTick ? tick : false ); }
    bool updateAnimation( const F32 elapsedTime );

    virtual bool validRender( void ) const;

    virtual void render(
        const bool flipX,
        const bool flipY,
        const Vector2& vertexPos0,
        const Vector2& vertexPos1,
        const Vector2& vertexPos2,
        const Vector2& vertexPos3,
        BatchRender* pBatchRenderer,
        const ColorF& color = ColorF(1.0f, 1.0f, 1.0f),
        const U32 rows = 1,
        const U32 columns = 1) const;

    void renderGui( GuiControl& owner, Point2I offset, const RectI &updateRect ) const;

    /// Static-Image Frame.
    inline bool setImage( const char* pImageAssetId ) { return setImage( pImageAssetId, mImageFrame ); }
    virtual bool setImage( const char* pImageAssetId, const U32 frame );
    inline StringTableEntry getImage( void ) const{ return mpImageAsset->getAssetId(); }
    virtual bool setImageFrame( const U32 frame );
    inline U32 getImageFrame( void ) const { return mImageFrame; }

    /// Animated-Image Frame.
    virtual bool setAnimation( const char* pAnimationAssetId );
    inline StringTableEntry getAnimation( void ) const { return mpAnimationAsset->getAssetId(); }
    void setAnimationFrame( const U32 frameIndex );
    void setAnimationTimeScale( const F32 scale ) { mAnimationTimeScale = scale; }
    inline F32 getAnimationTimeScale( void ) const { return mAnimationTimeScale; }
    bool playAnimation( const AssetPtr<AnimationAsset>& animationAsset);
    inline void pauseAnimation( const bool animationPaused ) { mAnimationPaused = animationPaused; }
    inline void stopAnimation( void ) { mAnimationFinished = true; mAnimationPaused = false; }
    inline void resetAnimationTime( void ) { mCurrentTime = 0.0f; }
    inline bool isAnimationPaused( void ) const { return mAnimationPaused; }
    inline bool isAnimationFinished( void ) const { return mAnimationFinished; };
    bool isAnimationValid( void ) const;

    /// Frame provision.
    inline bool isStaticFrameProvider( void ) const { return mStaticProvider; }
    inline std::shared_ptr<GFXTextureObject>& getProviderTexture( void ) const { return !validRender() ? BadTextureHandle : isStaticFrameProvider() ? (*mpImageAsset)->getImageTexture() : (*mpAnimationAsset)->getImage()->getImageTexture(); };
    inline const ImageAsset::FrameArea& getProviderImageFrameArea( void ) const  { return !validRender() ? BadFrameArea : isStaticFrameProvider() ? (*mpImageAsset)->getImageFrameArea(mImageFrame) : (*mpAnimationAsset)->getImage()->getImageFrameArea(getCurrentAnimationFrame()); };

    inline const AnimationAsset* getCurrentAnimation( void ) const { return mpAnimationAsset->notNull() ? *mpAnimationAsset : nullptr; };
    inline const StringTableEntry getCurrentAnimationAssetId( void ) const { return mpAnimationAsset->getAssetId(); };
    const U32 getCurrentAnimationFrame( void ) const;
    inline const F32 getCurrentAnimationTime( void ) const { return mCurrentTime; };

    void clearAssets( void );

    virtual void resetState( void );

protected:
    virtual void onAnimationEnd( void ) {}
    virtual void onAssetRefreshed( AssetPtrBase* pAssetPtrBase );
};


#endif // _IMAGE_FRAME_PROVIDER_CORE_H
