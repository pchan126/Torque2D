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
#include "graphics/gfxDevice.h"
#include "gui/guiTypes.h"
#include "gui/guiControl.h"
#include "gui/guiConsole.h"
#include "gui/containers/guiScrollCtrl.h"
#include "graphics/gfxDrawUtil.h"

IMPLEMENT_CONOBJECT(GuiConsole);

GuiConsole::GuiConsole()
{
   setExtent(64, 64);
   mCellSize.set(1, 1);
   mSize.set(1, 0);
}

bool GuiConsole::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the font
   mFont = mProfile->mFont;

   return true;
}

S32 GuiConsole::getMaxWidth(S32 startIndex, S32 endIndex)
{
   //sanity check
   size_t size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);

   if(startIndex < 0 || (U32)endIndex >= size || startIndex > endIndex)
      return 0;

   S32 result = 0;
   for(S32 i = startIndex; i <= endIndex; i++)
      result = std::max(result, (S32)(mFont->getStrWidth((const UTF8 *)log[i].mString)));
   
   Con::unlockLog();
   
   return(result + 6);
}

void GuiConsole::onPreRender()
{
   //see if the size has changed
   size_t size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);
   Con::unlockLog(); // we unlock immediately because we only use size here, not log.

   U32 prevSize = getHeight() / mCellSize.y;
   if ( prevSize > size )
       prevSize = 0;

   if(size != prevSize)
   {
      //first, find out if the console was scrolled up
      bool scrolled = false;
      GuiScrollCtrl *parent = dynamic_cast<GuiScrollCtrl*>(getParent());

      if(parent)
         scrolled = parent->isScrolledToBottom();

      //find the max cell width for the new entries
      auto newMax = getMaxWidth(0, (S32)size - 1);
      if(newMax > mCellSize.x)
         mCellSize.set(newMax, mFont->getHeight());

      //set the array size
      mSize.set(1, (S32)size);

      //resize the control
      resize(getPosition(), Point2I(mCellSize.x, mCellSize.y * (S32)size));

      //if the console was not scrolled, make the last entry visible
      if (scrolled)
         scrollCellVisible(Point2I(0,mSize.y - 1));
   }
}

void GuiConsole::onRenderCell(Point2I offset, Point2I cell, bool /*selected*/, bool /*mouseOver*/)
{
   size_t size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);

   ConsoleLogEntry &entry = log[cell.y];
   switch (entry.mLevel)
   {
      case ConsoleLogEntry::Normal:   GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor); break;
      case ConsoleLogEntry::Warning:  GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorHL); break;
      case ConsoleLogEntry::Error:    GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorNA); break;
      case ConsoleLogEntry::NUM_CLASS: Con::errorf("Unhandled case in GuiConsole::onRenderCell, NUM_CLASS");
   }
   GFX->getDrawUtil()->drawText(mFont, Point2I(offset.x + 3, offset.y), entry.mString, mProfile->mFontColors);
   
   Con::unlockLog();
}
