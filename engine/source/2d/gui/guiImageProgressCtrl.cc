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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxDrawUtil.h"
#include "2d/gui/guiImageProgressCtrl.h"

#ifndef _RENDER_PROXY_H_
#include "2d/core/RenderProxy.h"
#endif

/// Script bindings.
#include "guiImageProgressCtrl_ScriptBindings.h"

IMPLEMENT_CONOBJECT(GuiImageProgressCtrl);

GuiImageProgressCtrl::GuiImageProgressCtrl() :
   mProgressAssetId( StringTable->EmptyString )
{
   mProgress = 0.0f;
}

void GuiImageProgressCtrl::initPersistFields()
{
    Parent::initPersistFields();

   addProtectedField("ProgressImage", TypeAssetId, Offset(mProgressAssetId, GuiImageProgressCtrl), &setProgressImage, &getProgressImage, "The image asset Id used for the progress bar.");
}


const char* GuiImageProgressCtrl::getScriptValue()
{
   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%g", mProgress);
   return ret;
}

void GuiImageProgressCtrl::setScriptValue(const char *value)
{
   //set the value
   if (! value)
      mProgress = 0.0f;
   else
      mProgress = dAtof(value);

   //validate the value
   mProgress = mClampF(mProgress, 0.f, 1.f);
   setUpdate();
}

void GuiImageProgressCtrl::onPreRender()
{
   const char * var = getVariable();
   if(var)
   {
      F32 value = mClampF(dAtof(var), 0.f, 1.f);
      if(value != mProgress)
      {
         mProgress = value;
         setUpdate();
      }
   }
}

void GuiImageProgressCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   RectI ctrlRect(offset, getExtent());

   //draw the progress
   S32 width = (S32)((F32)getWidth() * mProgress);

   if ( mProgressNormalAsset == nullptr)
      return;
   
//   if (width > 0)
//   {
//      RectI progressRect = ctrlRect;
//      progressRect.extent.x = width;
//      GFX->getDrawUtil()->drawRectFill(progressRect, mProfile->mFillColor);
//   }

   // Is the asset valid and has the specified frame?
   if ( mProgressNormalAsset->isAssetValid() && 0 < mProgressNormalAsset->getFrameCount() )
   {
      RectI progressRect = ctrlRect;
      progressRect.extent.x = width;

      // Yes, so calculate the source region.
      const ImageAsset::FrameArea::PixelArea& pixelArea = mProgressNormalAsset->getImageFrameArea( (U32)0 ).mPixelArea;
      RectI sourceRegion( pixelArea.mPixelOffset, Point2I(pixelArea.mPixelWidth*mProgress, pixelArea.mPixelHeight) );
      
      // Calculate destination region.
      RectI destinationRegion(offset, progressRect.extent);
      
      // Render image.
      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFillColor );
      GFX->getDrawUtil()->drawBitmapStretchSR( mProgressNormalAsset->getImageTexture(), destinationRegion, sourceRegion );
      GFX->getDrawUtil()->clearBitmapModulation();

      ColorI fontColor = mProfile->mFontColor;
		GFX->getDrawUtil()->setBitmapModulation(fontColor);
		renderJustifiedText(offset+mProfile->mTextOffset, getExtent(), (char*)mText, mFont);
      GFX->getDrawUtil()->clearBitmapModulation();
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

//   //now draw the border
//   if (mProfile->mBorder)
//      GFX->getDrawUtil()->drawRect(ctrlRect, mProfile->mBorderColor);
//
//
//   //render the children
//   renderChildControls(offset, updateRect);
}


//-----------------------------------------------------------------------------

void GuiImageProgressCtrl::setProgressImage( const char* pImageAssetId )
{
   // Sanity!
   AssertFatal( pImageAssetId != nullptr, "Cannot use a NULL asset Id." );
   
   // Fetch the asset Id.
   mProgressAssetId = StringTable->insert(pImageAssetId);
   
   mProgressNormalAsset = mProgressAssetId;
   
   // Update control.
   setUpdate();
}


