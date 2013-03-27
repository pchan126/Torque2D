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

#include "gui/guiControl.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxDrawUtil.h"

/// Renders a skinned border.
class GuiBitmapBorderCtrl : public GuiControl
{
   typedef GuiControl Parent;

   enum {
      BorderTopLeft,
      BorderTopRight,
      BorderTop,
      BorderLeft,
      BorderRight,
      BorderBottomLeft,
      BorderBottom,
      BorderBottomRight,
      NumBitmaps
   };
	RectI *mBitmapBounds;  ///< bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
   GFXTexHandle mTextureObject;
public:
   bool onWake();
   void onSleep();
   void onRender(Point2I offset, const RectI &updateRect);
   DECLARE_CONOBJECT(GuiBitmapBorderCtrl);
};

IMPLEMENT_CONOBJECT(GuiBitmapBorderCtrl);

bool GuiBitmapBorderCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the texture for the close, minimize, and maximize buttons
   mTextureObject = mProfile->mTextureHandle;
   bool result = mProfile->constructBitmapArray() >= NumBitmaps;
   AssertFatal(result, "Failed to create the bitmap array");
   if(!result)
      return false;

   mBitmapBounds = mProfile->mBitmapArrayRects.address();
   return true;
}

void GuiBitmapBorderCtrl::onSleep()
{
   mTextureObject = NULL;
   Parent::onSleep();
}

void GuiBitmapBorderCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   renderChildControls( offset, updateRect );
   GFX->setClipRect(updateRect);

   //draw the outline
   RectI winRect;
   winRect.point = offset;
   winRect.extent = getExtent();

	winRect.point.x += mBitmapBounds[BorderLeft].extent.x;
	winRect.point.y += mBitmapBounds[BorderTop].extent.y;

   winRect.extent.x -= mBitmapBounds[BorderLeft].extent.x + mBitmapBounds[BorderRight].extent.x;
   winRect.extent.y -= mBitmapBounds[BorderTop].extent.y + mBitmapBounds[BorderBottom].extent.y;

   if(mProfile->mOpaque)
      GFX->getDrawUtil()->drawRectFill(winRect, mProfile->mFillColor);

   GFX->getDrawUtil()->clearBitmapModulation();
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset, mBitmapBounds[BorderTopLeft]);
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, Point2I(offset.x + getWidth() - mBitmapBounds[BorderTopRight].extent.x, offset.y),
                   mBitmapBounds[BorderTopRight]);

   RectI destRect;
   destRect.point.x = offset.x + mBitmapBounds[BorderTopLeft].extent.x;
   destRect.point.y = offset.y;
   destRect.extent.x = getWidth() - mBitmapBounds[BorderTopLeft].extent.x - mBitmapBounds[BorderTopRight].extent.x;
	destRect.extent.y = mBitmapBounds[BorderTop].extent.y;
   RectI stretchRect = mBitmapBounds[BorderTop];
   stretchRect.inset(1,0);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   destRect.point.x = offset.x;
   destRect.point.y = offset.y + mBitmapBounds[BorderTopLeft].extent.y;
   destRect.extent.x = mBitmapBounds[BorderLeft].extent.x;
   destRect.extent.y = getHeight() - mBitmapBounds[BorderTopLeft].extent.y - mBitmapBounds[BorderBottomLeft].extent.y;
   stretchRect = mBitmapBounds[BorderLeft];
   stretchRect.inset(0,1);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   destRect.point.x = offset.x + getWidth() - mBitmapBounds[BorderRight].extent.x;
   destRect.extent.x = mBitmapBounds[BorderRight].extent.x;
   destRect.point.y = offset.y + mBitmapBounds[BorderTopRight].extent.y;
   destRect.extent.y = getHeight() - mBitmapBounds[BorderTopRight].extent.y - mBitmapBounds[BorderBottomRight].extent.y;

	stretchRect = mBitmapBounds[BorderRight];
   stretchRect.inset(0,1);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset + Point2I(0, getHeight() - mBitmapBounds[BorderBottomLeft].extent.y), mBitmapBounds[BorderBottomLeft]);
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset + getExtent() - mBitmapBounds[BorderBottomRight].extent, mBitmapBounds[BorderBottomRight]);

   destRect.point.x = offset.x + mBitmapBounds[BorderBottomLeft].extent.x;
   destRect.extent.x = getWidth() - mBitmapBounds[BorderBottomLeft].extent.x - mBitmapBounds[BorderBottomRight].extent.x;

   destRect.point.y = offset.y + getHeight() - mBitmapBounds[BorderBottom].extent.y;
   destRect.extent.y = mBitmapBounds[BorderBottom].extent.y;
   stretchRect = mBitmapBounds[BorderBottom];
   stretchRect.inset(1,0);

   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);
}
