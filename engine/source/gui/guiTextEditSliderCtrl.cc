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

#include "console/consoleTypes.h"
#include "console/console.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxDrawUtil.h"
#include "gui/guiCanvas.h"
#include "gui/guiTextEditSliderCtrl.h"

IMPLEMENT_CONOBJECT(GuiTextEditSliderCtrl);

GuiTextEditSliderCtrl::GuiTextEditSliderCtrl()
{
   mRange.set(0.0f, 1.0f);
   mIncAmount = 1.0f;
   mValue = 0.0f;
   mMulInc = 0;
   mIncCounter = 0.0f;
   mFormat = StringTable->insert("%3.2f");
   mTextAreaHit = None;
}

GuiTextEditSliderCtrl::~GuiTextEditSliderCtrl()
{
}

void GuiTextEditSliderCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("format",    TypeString,  Offset(mFormat, GuiTextEditSliderCtrl));
   addField("range",     TypePoint2F, Offset(mRange, GuiTextEditSliderCtrl));
   addField("increment", TypeF32,     Offset(mIncAmount,     GuiTextEditSliderCtrl));
}

void GuiTextEditSliderCtrl::getText(char *dest)
{
   Parent::getText(dest);
}

void GuiTextEditSliderCtrl::setText(const char *txt)
{
   mValue = dAtof(txt);
   checkRange();
   setValue();
}

bool GuiTextEditSliderCtrl::onKeyDown(const GuiEvent &event)
{
   return Parent::onKeyDown(event);
}

void GuiTextEditSliderCtrl::checkRange()
{
   if(mValue < mRange.x)
      mValue = mRange.x;
   else if(mValue > mRange.y)
      mValue = mRange.y;
}

void GuiTextEditSliderCtrl::setValue()
{
   char buf[20];
   dSprintf(buf,sizeof(buf),mFormat, mValue);
   Parent::setText(buf);
}

void GuiTextEditSliderCtrl::onMouseDown(const GuiEvent &event)
{
   char txt[20];
   Parent::getText(txt);
   mValue = dAtof(txt);

   mMouseDownTime = Sim::getCurrentTime();
   GuiControl *parent = getParent();
   if(!parent)
      return;
   Point2I camPos  = event.mousePoint;
   Point2I point = parent->localToGlobalCoord(getPosition());

   if(camPos.x > point.x + getWidth() - 14)
   {
      if(camPos.y > point.y + (getHeight()/2))
      {
         mValue -=mIncAmount;
         mTextAreaHit = ArrowDown;
         mMulInc = -0.15f;
      }
      else
      {
         mValue +=mIncAmount;
         mTextAreaHit = ArrowUp;
         mMulInc = 0.15f;
      }

      checkRange();
      setValue();
      mouseLock();
      return;
   }
   Parent::onMouseDown(event);
}

void GuiTextEditSliderCtrl::onMouseDragged(const GuiEvent &event)
{
   if(mTextAreaHit == None || mTextAreaHit == Slider)
   {
      mTextAreaHit = Slider;
      GuiControl *parent = getParent();
      if(!parent)
         return;
      Point2I camPos = event.mousePoint;
      Point2I point = parent->localToGlobalCoord(getPosition());
      F32 maxDis = 100;
      F32 val;
      if(camPos.y < point.y)
      {
         if(point.y < maxDis)
            maxDis = (F32)point.y;
         val = point.y - maxDis;
         if(point.y > 0)
            mMulInc= 1.0f-(((float)camPos.y - val) / maxDis);
         else
            mMulInc = 1.0f;
         checkIncValue();
         return;
      }
      else if(camPos.y > point.y + getHeight())
      {
         GuiCanvas *root = getRoot();
         val = (F32)(root->getHeight() - (point.y + getHeight()));
         if(val < maxDis)
            maxDis = val;
         if( val > 0)
            mMulInc= -(float)(camPos.y - (point.y + getHeight()))/maxDis;
         else
            mMulInc = -1.0f;
         checkIncValue();
         return;
      }
      mTextAreaHit = None;
      Parent::onMouseDragged(event);
   }
}

void GuiTextEditSliderCtrl::onMouseUp(const GuiEvent &event)
{
   mMulInc = 0.0f;
   mouseUnlock();
  //if we released the mouse within this control, then the parent will call
  //the mConsoleCommand other wise we have to call it.
   Parent::onMouseUp(event);
  //if we didn't release the mouse within this control, then perform the action
   if (!cursorInControl())
      Con::evaluate(mConsoleCommand, false);
   mTextAreaHit = None;
}
void GuiTextEditSliderCtrl::checkIncValue()
{
   if(mMulInc > 1.0f)
      mMulInc = 1.0f;
   else if(mMulInc < -1.0f)
      mMulInc = -1.0f;
}

void GuiTextEditSliderCtrl::timeInc(U32 elapseTime)
{
   S32 numTimes = elapseTime / 750;
   if(mTextAreaHit != Slider && numTimes > 0)
   {
      if(mTextAreaHit == ArrowUp)
         mMulInc = 0.15f * numTimes;
      else
         mMulInc = -0.15f * numTimes;

      checkIncValue();
   }
}

void GuiTextEditSliderCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if(mTextAreaHit != None)
   {
      U32 elapseTime = Sim::getCurrentTime() - mMouseDownTime;
      if(elapseTime > 750 || mTextAreaHit == Slider)
      {
         timeInc(elapseTime);
         mIncCounter += mMulInc;
         if(mIncCounter >= 1.0f || mIncCounter <= -1.0f)
         {
            mValue = (mMulInc > 0.0f) ? mValue+mIncAmount : mValue-mIncAmount;
            mIncCounter = (mIncCounter > 0.0f) ? mIncCounter-1 : mIncCounter+1;
            checkRange();
            setValue();
         }
      }
   }
	Parent::onRender(offset, updateRect);

   Point2I start(offset.x + getWidth() - 14, offset.y);
   Point2I midPoint(start.x + 7, start.y + (getHeight()/2));

   GFX->getDrawUtil()->drawRectFill(Point2I(start.x+1,start.y+1), Point2I(start.x+13,start.y+getHeight()-1) , mProfile->mFillColor);

   GFX->getDrawUtil()->drawLine(start, Point2I(start.x, start.y+getHeight()),mProfile->mFontColor);
   GFX->getDrawUtil()->drawLine(Point2I(start.x,midPoint.y),
               Point2I(start.x+14,midPoint.y),
               mProfile->mFontColor);

    GFXVertexPC verts[6];
    
    verts[0].color.set( 0, 0, 0 );
	verts[1].color.set( 0, 0, 0 );
	verts[2].color.set( 0, 0, 0 );
	verts[3].color.set( 0, 0, 0 );
	verts[4].color.set( 0, 0, 0 );
	verts[5].color.set( 0, 0, 0 );
    
    if(mTextAreaHit == ArrowUp)
    {
        verts[0].point.set( (F32)midPoint.x, (F32)start.y + 1.0f, 0.0f );
        verts[1].point.set( (F32)start.x + 11.0f, (F32)midPoint.y - 2.0f, 0.0f );
        verts[2].point.set( (F32)start.x + 3.0f, (F32)midPoint.y - 2.0f, 0.0f );
    }
    else
    {
        verts[0].point.set( (F32)midPoint.x, (F32)start.y + 2.0f, 0.0f );
        verts[1].point.set( (F32)start.x + 11.0f, (F32)midPoint.y - 1.0f, 0.0f );
        verts[2].point.set( (F32)start.x + 3.0f, (F32)midPoint.y - 1.0f, 0.0f );
    }
    
    if(mTextAreaHit == ArrowDown)
    {
        verts[3].point.set( (F32)midPoint.x, (F32)(start.y + getExtent().y - 1), 0.0f );
        verts[4].point.set( (F32)start.x + 11.0f, (F32)midPoint.y + 3.0f, 0.0f );
        verts[5].point.set( (F32)start.x + 3.0f, (F32)midPoint.y + 3.0f, 0.0f );
    }
    else
    {
        verts[3].point.set( (F32)midPoint.x, (F32)(start.y + getExtent().y - 2), 0.0f );
        verts[4].point.set( (F32)start.x + 11.0f, (F32)midPoint.y + 2.0f, 0.0f );
        verts[5].point.set( (F32)start.x + 3.0f, (F32)midPoint.y + 2.0f, 0.0f );
    }
    
    GFXVertexBufferHandle<GFXVertexPC> vb(GFX, 6, GFXBufferTypeVolatile, verts);
    
    GFX->setVertexBuffer( vb );
    GFX->drawPrimitive( GFXTriangleList, 0, 2 );
}

void GuiTextEditSliderCtrl::onPreRender()
{
   if (isFirstResponder())
   {
      U32 timeElapsed = Platform::getVirtualMilliseconds() - mTimeLastCursorFlipped;
      mNumFramesElapsed++;
      if ((timeElapsed > 500) && (mNumFramesElapsed > 3))
      {
         mCursorOn = !mCursorOn;
         mTimeLastCursorFlipped = Sim::getCurrentTime();
         mNumFramesElapsed = 0;
         setUpdate();
      }

      //update the cursor if the text is scrolling
      if (mDragHit)
      {
         if ((mScrollDir < 0) && (mCursorPos > 0))
         {
            mCursorPos--;
         }
         else if ((mScrollDir > 0) && (mCursorPos < (S32)dStrlen(mText)))
         {
            mCursorPos++;
         }
      }
   }
}


