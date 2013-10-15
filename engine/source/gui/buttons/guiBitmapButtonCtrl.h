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

#ifndef _GUIBITMAPBUTTON_H_
#define _GUIBITMAPBUTTON_H_

#include "gui/buttons/guiButtonCtrl.h"
//#ifndef _TEXTURE_MANAGER_H_
//#include "graphics/gfxTextureManager.h"
//#endif
#include "graphics/gfxTextureManager.h"

enum ButtonState
{
    NORMAL,
    HILIGHT,
    DEPRESSED,
    INACTIVE
};

class GuiBitmapButtonCtrl : public GuiButtonCtrl
{
private:
   typedef GuiButtonCtrl Parent;

protected:
   StringTableEntry mBitmapName;
   StringTableEntry mBitmapNormal;
   StringTableEntry mBitmapHilight;
   StringTableEntry mBitmapDepressed;
   StringTableEntry mBitmapInactive;
   bool mIsLegacyVersion;

   GFXTexHandle mTextureNormal;
   GFXTexHandle mTextureHilight;
   GFXTexHandle mTextureDepressed;
   GFXTexHandle mTextureInactive;

   void renderButton(GFXTexHandle &texture, Point2I &offset, const RectI& updateRect);

public:
   DECLARE_CONOBJECT(GuiBitmapButtonCtrl);
   GuiBitmapButtonCtrl();

   static void initPersistFields();

   //Parent methods
   bool onWake();
   void onSleep();
   void inspectPostApply();

   void setBitmap(const String& name, ButtonState state);
   void setBitmap(const String& name);

   void onRender(Point2I offset, const RectI &updateRect);
};

class GuiBitmapButtonTextCtrl : public GuiBitmapButtonCtrl
{
   typedef GuiBitmapButtonCtrl Parent;
public:
   DECLARE_CONOBJECT(GuiBitmapButtonTextCtrl);
   void onRender(Point2I offset, const RectI &updateRect);
};

#endif //_GUI_BITMAP_BUTTON_CTRL_H
