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

#ifndef _GUIIMAGEPROGRESSCTRL_H_
#define _GUIIMAGEPROGRESSCTRL_H_

#include "gui/guiProgressCtrl.h"

#ifndef _IMAGE_ASSET_H_
#include "2d/assets/ImageAsset.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

class GuiImageProgressCtrl : public GuiProgressCtrl
{
private:
   typedef GuiTextCtrl Parent;

   F32 mProgress;

protected:
   StringTableEntry                mProgressAssetId;

   AssetPtr<ImageAsset>            mProgressNormalAsset;
   
public:
   //creation methods
   DECLARE_CONOBJECT(GuiImageProgressCtrl);
   GuiImageProgressCtrl();

    static void initPersistFields();

   void setProgressImage( const char* pImageAssetId );
   inline StringTableEntry getProgressImage( void ) const { return mProgressAssetId; }

   //console related methods
   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect);

//    virtual void interpolateTick( F32 delta );
//    virtual void processTick();
//    virtual void advanceTime( F32 timeDelta );
protected:
   static bool setProgressImage(void* obj, const char* data) { static_cast<GuiImageProgressCtrl*>(obj)->setProgressImage( data ); return false; }
   static const char* getProgressImage(void* obj, const char* data) { return static_cast<GuiImageProgressCtrl*>(obj)->getProgressImage(); }

};

#endif
