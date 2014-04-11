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

#ifndef _GUIIMAGEBUTTON_H_
#include "2d/gui/GuiImageButtonCtrl.h"
#endif

#ifndef _RENDER_PROXY_H_
#include "2d/core/RenderProxy.h"
#endif

#include "graphics/gfxDrawUtil.h"

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _GUICANVAS_H_
#include "gui/guiCanvas.h"
#endif

#ifndef _H_GUIDEFAULTCONTROLRENDER_
#include "gui/guiDefaultControlRender.h"
#endif

/// Script bindings.
#include "guiImageButtonCtrl_ScriptBindings.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiImageButtonCtrl);

//-----------------------------------------------------------------------------

GuiImageButtonCtrl::GuiImageButtonCtrl() :
//    mNormalAssetId( StringTable->EmptyString ),
//    mHoverAssetId( StringTable->EmptyString ),
//    mDownAssetId( StringTable->EmptyString ),
//    mInactiveAssetId( StringTable->EmptyString ),
   mLeftBorder(0),
   mRightBorder(0),
   mTopBorder(0),
   mBottomBorder(0)
{
    setExtent(140, 30);
}

//-----------------------------------------------------------------------------

void GuiImageButtonCtrl::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

//    addProtectedField("NormalImage", TypeAssetId, Offset(mNormalAssetId, GuiImageButtonCtrl), &setNormalImage, &getNormalImage, "The image asset Id used for the normal button state.");
//    addProtectedField("HoverImage", TypeAssetId, Offset(mHoverAssetId, GuiImageButtonCtrl), &setHoverImage, &getHoverImage, "The image asset Id used for the hover button state.");
//    addProtectedField("DownImage", TypeAssetId, Offset(mDownAssetId, GuiImageButtonCtrl), &setDownImage, &getDownImage, "The image asset Id used for the Down button state.");
//    addProtectedField("InactiveImage", TypeAssetId, Offset(mInactiveAssetId, GuiImageButtonCtrl), &setInactiveImage, &getInactiveImage, "The image asset Id used for the inactive button state.");

    addProtectedField("LeftBorder", TypeS32, Offset(mLeftBorder, GuiImageButtonCtrl), &setLeftBorder, &defaultProtectedGetFn, &writeLeftBorder, "");
    addProtectedField("RightBorder", TypeS32, Offset(mRightBorder, GuiImageButtonCtrl), &setRightBorder, &defaultProtectedGetFn, &writeRightBorder, "");
    addProtectedField("TopBorder", TypeS32, Offset(mTopBorder, GuiImageButtonCtrl), &setTopBorder, &defaultProtectedGetFn, &writeTopBorder, "");
    addProtectedField("BottomBorder", TypeS32, Offset(mBottomBorder, GuiImageButtonCtrl), &setBottomBorder, &defaultProtectedGetFn, &writeBottomBorder, "");
}

//-----------------------------------------------------------------------------

bool GuiImageButtonCtrl::onWake()
{
    // Call parent.
    if (!Parent::onWake())
        return false;

//    // Is only the "normal" image specified?
//    if (    mNormalAssetId != StringTable->EmptyString &&
//            mHoverAssetId == StringTable->EmptyString &&
//            mDownAssetId == StringTable->EmptyString &&
//            mInactiveAssetId == StringTable->EmptyString )
//    {
//        // Yes, so use it for all states.
//        mImageNormalAsset = mNormalAssetId;
//        mImageHoverAsset = mNormalAssetId;
//        mImageDownAsset = mNormalAssetId;
//        mImageInactiveAsset = mNormalAssetId;
//    }
//    else
//    {
//        // No, so assign individual states.
//        mImageNormalAsset = mNormalAssetId;
//        mImageHoverAsset = mHoverAssetId;
//        mImageDownAsset = mDownAssetId;
//        mImageInactiveAsset = mInactiveAssetId;
//    }
   
    return true;
}

//-----------------------------------------------------------------------------

void GuiImageButtonCtrl::onSleep()
{
    // Clear assets.
//    mImageNormalAsset.clear();
//    mImageHoverAsset.clear();
//    mImageDownAsset.clear();
//    mImageInactiveAsset.clear();

    // Call parent.
    Parent::onSleep();
}

//-----------------------------------------------------------------------------

//void GuiImageButtonCtrl::setNormalImage( const char* pImageAssetId )
//{
//    // Sanity!
//    AssertFatal( pImageAssetId != nullptr, "Cannot use a NULL asset Id." );
//
//    // Fetch the asset Id.
//    mNormalAssetId = StringTable->insert(pImageAssetId);
//
//    // Assign asset if awake.
//    if ( isAwake() )
//        mImageNormalAsset = mNormalAssetId;
//
//    // Update control.
//    setUpdate();
//}

//-----------------------------------------------------------------------------

//void GuiImageButtonCtrl::setHoverImage( const char* pImageAssetId )
//{
//    // Sanity!
//    AssertFatal( pImageAssetId != nullptr, "Cannot use a NULL asset Id." );
//
//    // Fetch the asset Id.
//    mHoverAssetId = StringTable->insert(pImageAssetId);
//
//    // Assign asset if awake.
//    if ( isAwake() )
//        mImageHoverAsset = mHoverAssetId;
//
//    // Update control.
//    setUpdate();
//}

//-----------------------------------------------------------------------------

//void GuiImageButtonCtrl::setDownImage( const char* pImageAssetId )
//{
//    // Sanity!
//    AssertFatal( pImageAssetId != NULL, "Cannot use a NULL asset Id." );
//
//    // Fetch the asset Id.
//    mDownAssetId = StringTable->insert(pImageAssetId);
//
//    // Assign asset if awake.
//    if ( isAwake() )
//        mImageDownAsset = mDownAssetId;
//
//    // Update control.
//    setUpdate();
//}

//-----------------------------------------------------------------------------

//void GuiImageButtonCtrl::setInactiveImage( const char* pImageAssetId )
//{
//    // Sanity!
//    AssertFatal( pImageAssetId != nullptr, "Cannot use a NULL asset Id." );
//
//    // Fetch the asset Id.
//    mInactiveAssetId = StringTable->insert(pImageAssetId);
//
//    // Assign asset if awake.
//    if ( isAwake() )
//        mImageInactiveAsset = mInactiveAssetId;
//
//    // Update control.
//    setUpdate();
//}

//-----------------------------------------------------------------------------

void GuiImageButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
    // Reset button state.
    ButtonState state = NORMAL;

    // Calculate button state.
    if ( mActive )
    {
        if ( mMouseOver )
            state = HOVER;

        if ( mDepressed || mStateOn )
            state = DOWN;
    }
    else
    {
        state = INACTIVE;
    }

    switch (state)
    {
        case NORMAL:
            {
                // Render the "normal" asset.
                renderButton( mProfile->mImageNormalAsset, mProfile->mNormalFrame, offset, updateRect);

            } break;

        case HOVER:
            {
                // Render the "hover" asset.
                renderButton( mProfile->mImageHoverAsset, mProfile->mHoverFrame, offset, updateRect);

            } break;

        case DOWN:
            {
                // Render the "down" asset.
                renderButton( mProfile->mImageDownAsset, mProfile->mDownFrame, offset, updateRect);

            }  break;

        case INACTIVE:
            {
                // Render the "inactive" asset.
                renderButton( mProfile->mImageInactiveAsset, mProfile->mInactiveFrame, offset, updateRect);

            } break;
    }
}

//------------------------------------------------------------------------------

void GuiImageButtonCtrl::renderButton( ImageAsset* pImageAsset, const U32 frame, Point2I &offset, const RectI& updateRect )
{
    // Ignore an invalid datablock.
    if ( pImageAsset == nullptr )
        return;

    // Is the asset valid and has the specified frame?
    if ( pImageAsset->isAssetValid() && frame < pImageAsset->getFrameCount() )
    {
        // Yes, so calculate the source region.
        const ImageAsset::FrameArea::PixelArea& pixelArea = pImageAsset->getImageFrameArea( frame ).mPixelArea;
        RectI sourceRegion( pixelArea.mPixelOffset, Point2I(pixelArea.mPixelWidth, pixelArea.mPixelHeight) );

        // Calculate destination region.
        RectI destinationRegion(offset, getExtent());

        // Render image.
        GFX->getDrawUtil()->setBitmapModulation( mProfile->mFillColor );
        GFX->getDrawUtil()->drawBitmapStretchSR( pImageAsset->getImageTexture(), destinationRegion, sourceRegion );
        GFX->getDrawUtil()->clearBitmapModulation();
        renderChildControls( offset, updateRect);
    }
    else
    {
        // No, so fetch the 'cannot render' proxy.
        RenderProxy* pNoImageRenderProxy = Sim::findObject<RenderProxy>( CANNOT_RENDER_PROXY_NAME );

        // Finish if no render proxy available or it can't render.
        if ( pNoImageRenderProxy == nullptr || !pNoImageRenderProxy->validRender() )
            return;

        // Render using render-proxy..
        pNoImageRenderProxy->renderGui( *this, offset, updateRect );
    }

    // Update the control.
    setUpdate();
}

//------------------------------------------------------------------------------

void GuiImageButtonCtrl::setTopBorder(const S32 topBorder)
{
   // Ignore no change.
   if ( topBorder == mTopBorder )
      return;
   
   // Valid?
   if ( topBorder < 0 )
   {
      // No, so warn.
      Con::warnf( "Invalid top Border '%d'.", topBorder );
      return;
   }
   
   // Update.
   mTopBorder = topBorder;
   
   // Refresh the asset.
}


//------------------------------------------------------------------------------

void GuiImageButtonCtrl::setBottomBorder(const S32 bottomBorder)
{
   // Ignore no change.
   if ( bottomBorder == mBottomBorder )
      return;
   
   // Valid?
   if ( bottomBorder < 0 )
   {
      // No, so warn.
      Con::warnf( "Invalid bottom Border '%d'.", bottomBorder );
      return;
   }
   
   // Update.
   mBottomBorder = bottomBorder;
   
   // Refresh the asset.
}

//------------------------------------------------------------------------------

void GuiImageButtonCtrl::setLeftBorder(const S32 leftBorder)
{
   // Ignore no change.
   if ( leftBorder == mLeftBorder )
      return;
   
   // Valid?
   if ( leftBorder < 0 )
   {
      // No, so warn.
      Con::warnf( "Invalid left Border '%d'.", leftBorder );
      return;
   }
   
   // Update.
   mLeftBorder = leftBorder;
   
   // Refresh the asset.
}

//------------------------------------------------------------------------------

void GuiImageButtonCtrl::setRightBorder(const S32 rightBorder)
{
   // Ignore no change.
   if ( rightBorder == mRightBorder )
      return;
   
   // Valid?
   if ( rightBorder < 0 )
   {
      // No, so warn.
      Con::warnf( "Invalid right Border '%d'.", rightBorder );
      return;
   }
   
   // Update.
   mRightBorder = rightBorder;
   
   // Refresh the asset.
}

