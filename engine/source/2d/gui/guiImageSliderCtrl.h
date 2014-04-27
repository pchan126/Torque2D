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

#ifndef _GUIIMAGESLIDERCTRL_H_
#define _GUIIMAGESLIDERCTRL_H_

#include "gui/guiSliderCtrl.h"

#ifndef _IMAGE_ASSET_H_
#include "2d/assets/ImageAsset.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

class GuiImageSliderCtrl : public GuiSliderCtrl
{
private:
   typedef GuiControl Parent;

protected:
//   Point2F mRange;
//   U32  mTicks;
//   F32  mValue;
//   RectI   mThumb;
//   Point2I mThumbSize;
//   void updateThumb( F32 value, bool snap = true, bool onWake = false );
//   S32 mShiftPoint;
//   S32 mShiftExtent;
//   bool mDisplayValue;
//   bool mDepressed;
//   bool mMouseOver;
//   bool mHasTexture;

//   enum
//   {
//	   SliderLineLeft = 0,
//	   SliderLineCenter,
//	   SliderLineRight,
//	   SliderButtonNormal,
//	   SliderButtonHighlight,
//	   NumBitmaps
//   };
//   	RectI *mBitmapBounds;
   
   Point2I                         mThumbSize;

   StringTableEntry                mThumbNormalAssetId;
   AssetPtr<ImageAsset>            mThumbNormalAsset;
   StringTableEntry                mThumbDepressedAssetId;
   AssetPtr<ImageAsset>            mThumbDepressedAsset;

public:
   //creation methods
   DECLARE_CONOBJECT(GuiImageSliderCtrl);
   GuiImageSliderCtrl();
   static void initPersistFields();

   void setThumbNormalImage( const char* pImageAssetId );
   inline StringTableEntry getThumbNormalImage( void ) const { return mThumbNormalAssetId; }
   void setThumbDepressedImage( const char* pImageAssetId );
   inline StringTableEntry getThumbDepressedImage( void ) const { return mThumbDepressedAssetId; }

   //   //Parental methods
//   bool onWake();
//
//   void onMouseDown(const GuiEvent &event);
//   void onMouseDragged(const GuiEvent &event);
//   void onMouseUp(const GuiEvent &);
//   void onMouseLeave(const GuiEvent &);
//   void onMouseEnter(const GuiEvent &);
//   F32 getValue() { return mValue; }
//   void setScriptValue(const char *val);

   void onRender(Point2I offset, const RectI &updateRect);

protected:
   static bool setThumbNormalImage(void* obj, const char* data) { static_cast<GuiImageSliderCtrl*>(obj)->setThumbNormalImage( data ); return false; }
   static const char* getThumbNormalImage(void* obj, const char* data) { return static_cast<GuiImageSliderCtrl*>(obj)->getThumbNormalImage(); }
   static bool setThumbDepressedImage(void* obj, const char* data) { static_cast<GuiImageSliderCtrl*>(obj)->setThumbDepressedImage( data ); return false; }
   static const char* getThumbDepressedImage(void* obj, const char* data) { return static_cast<GuiImageSliderCtrl*>(obj)->getThumbDepressedImage(); }
};

#endif
