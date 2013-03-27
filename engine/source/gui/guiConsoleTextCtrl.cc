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
#include "graphics/color.h"
#include "gui/guiConsoleTextCtrl.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxDrawUtil.h"
#include "gui/guiDefaultControlRender.h"

IMPLEMENT_CONOBJECT(GuiConsoleTextCtrl);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

GuiConsoleTextCtrl::GuiConsoleTextCtrl()
{
   //default fonts
   mConsoleExpression = NULL;
   mResult = NULL;
}

void GuiConsoleTextCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addGroup("GuiConsoleTextCtrl");		
   addField("expression",  TypeCaseString,  Offset(mConsoleExpression, GuiConsoleTextCtrl));
   endGroup("GuiConsoleTextCtrl");		
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiConsoleTextCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   mFont = mProfile->mFont;
   return true;
}

void GuiConsoleTextCtrl::onSleep()
{
   Parent::onSleep();
   mFont = NULL;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiConsoleTextCtrl::setText(const char *txt)
{
   //make sure we don't call this before onAdd();
   AssertFatal(mProfile, "Can't call setText() until setProfile() has been called.");

   if (txt)
      mConsoleExpression = StringTable->insert(txt);
   else
      mConsoleExpression = NULL;

   //Make sure we have a font
   mProfile->incRefCount();
   mFont = mProfile->mFont;

   setUpdate();

   //decrement the profile referrence
   mProfile->decRefCount();
}

void GuiConsoleTextCtrl::calcResize()
{
   if (!mResult)
      return;

   //resize
   if (mProfile->mAutoSizeWidth)
   {
      if (mProfile->mAutoSizeHeight)
         resize(getPosition(), Point2I(mFont->getStrWidth((const UTF8 *)mResult) + 4, mFont->getHeight() + 4));
      else
         resize(getPosition(), Point2I(mFont->getStrWidth((const UTF8 *)mResult) + 4, getHeight()));
   }
   else if (mProfile->mAutoSizeHeight)
   {
      resize(getPosition(), Point2I(getWidth(), mFont->getHeight() + 4));
   }
}


void GuiConsoleTextCtrl::onPreRender()
{
   if (mConsoleExpression)
      mResult = Con::evaluatef("$temp = %s;", mConsoleExpression);
   calcResize();
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiConsoleTextCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    RectI ctrlRect( offset, getExtent() );
    
    // if opaque, fill the update rect with the fill color
    if ( mProfile->mOpaque )
        GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColor );
    
    // if there's a border, draw the border
    if ( mProfile->mBorder )
        renderBorder( ctrlRect, mProfile );
    
    // If we have text to render.
    if ( mResult != StringTable->EmptyString )
    {
        GFont *font = mProfile->mFont;
        
        GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
        
        for ( U32 i = 0; i < mLineLen.size(); i++ )
        {
            Point2I tempOffset = offset;
            tempOffset += mProfile->mTextOffset;
            tempOffset.y += i * font->getHeight();
            
            const UTF8 *line = mResult + mStartLineOffset[i];
            U32 lineLen = mLineLen[i];
            GFX->getDrawUtil()->drawTextN( font, tempOffset, line, lineLen, mProfile->mFontColors );
        }
    }
    
    // render the child controlsmResult
    renderChildControls(offset, updateRect);
    
    //   // draw the background rectangle
//   RectI r(offset, getExtent());
//   GFX->getDrawUtil()->drawRectFill(r, ColorI(255,255,255));
//
//   // draw the border
//   r.extent += r.point;
//   glColor4ub(0, 0, 0, 0);
//
//#ifdef TORQUE_OS_IOS
//   Point2I topleft(r.point.x,  r.point.y);
//   Point2I bottomRight(r.extent.x-1, r.extent.y-1);
//	//this was the same drawing as GFX->getDrawUtil()->drawRect
//	GFX->getDrawUtil()->drawRect( topleft, bottomRight, ColorI(0, 0, 0, 0) );// PUAP -Mat untested
//#else
//   glBegin(GL_LINE_LOOP);
//   glVertex2i(r.point.x,  r.point.y);
//   glVertex2i(r.extent.x-1, r.point.y);
//   glVertex2i(r.extent.x-1, r.extent.y-1);
//   glVertex2i(r.point.x,  r.extent.y-1);
//   glEnd();
//#endif
//
//   if (mResult)
//   {
//      S32 txt_w = mFont->getStrWidth((const UTF8 *)mResult);
//      Point2I localStart;
//      switch (mProfile->mAlignment)
//      {
//         case GuiControlProfile::RightJustify:
//            localStart.set(getWidth() - txt_w-2, 0);
//            break;
//         case GuiControlProfile::CenterJustify:
//            localStart.set((getWidth() - txt_w) / 2, 0);
//            break;
//         default:
//            // GuiControlProfile::LeftJustify
//            localStart.set(2,0);
//            break;
//      }
//
//      Point2I globalStart = localToGlobalCoord(localStart);
//
//      //draw the text
//      GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor);
//      GFX->getDrawUtil()->drawText(mFont, globalStart, mResult, mProfile->mFontColors);
//   }
//
//   //render the child controls
//   renderChildControls(offset, updateRect);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

const char *GuiConsoleTextCtrl::getScriptValue()
{
   return getText();
}

void GuiConsoleTextCtrl::setScriptValue(const char *val)
{
   setText(val);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// EOF //
