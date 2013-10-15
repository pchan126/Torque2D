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


//-------------------------------------
//
// Icon Button Control
// Draws the bitmap within a special button control.  Only a single bitmap is used and the
// button will be drawn in a highlighted mode when the mouse hovers over it or when it
// has been clicked.
//
// Use mTextLocation to choose where within the button the text will be drawn, if at all.
// Use mTextMargin to set the text away from the button sides or from the bitmap.
// Use mButtonMargin to set everything away from the button sides.
// Use mErrorBitmapName to set the name of a bitmap to draw if the main bitmap cannot be found.
// Use mFitBitmapToButton to force the bitmap to fill the entire button extent.  Usually used
// with no button text defined.
//
// if the extent is set to (0,0) in the gui editor and appy hit, this control will
// set it's extent to be exactly the size of the normal bitmap (if present)
//


#include "console/console.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxDrawUtil.h"
#include "console/consoleTypes.h"
#include "platform/platformAudio.h"
#include "gui/guiCanvas.h"
#include "gui/guiDefaultControlRender.h"
#include "gui/buttons/guiIconButtonCtrl.h"

static ColorI colorWhite(255,255,255);
static ColorI colorBlack(0,0,0);

IMPLEMENT_CONOBJECT(GuiIconButtonCtrl);

//-------------------------------------
GuiIconButtonCtrl::GuiIconButtonCtrl()
{
   mBitmapName = StringTable->EmptyString;
   mTextLocation = TextLocLeft;
   mIconLocation = IconLocLeft;
   mTextMargin = 4;
   mButtonMargin.set(4,4);

   mFitBitmapToButton = false;

   mErrorBitmapName = StringTable->EmptyString;
   mErrorTextureHandle = NULL;

   setExtent(140, 30);
}

static EnumTable::Enums textLocEnums[] = 
{
   EnumTable::Enums( GuiIconButtonCtrl::TextLocNone, "None" ),
   EnumTable::Enums( GuiIconButtonCtrl::TextLocBottom, "Bottom" ),
   EnumTable::Enums( GuiIconButtonCtrl::TextLocRight, "Right" ),
   EnumTable::Enums( GuiIconButtonCtrl::TextLocTop, "Top" ),
   EnumTable::Enums( GuiIconButtonCtrl::TextLocLeft, "Left" ),
   EnumTable::Enums( GuiIconButtonCtrl::TextLocCenter, "Center" ),
};

static EnumTable gTextLocTable(6, &textLocEnums[0]); 


static EnumTable::Enums iconLocEnums[] = 
{
   EnumTable::Enums( GuiIconButtonCtrl::IconLocLeft, "Left" ),
   EnumTable::Enums( GuiIconButtonCtrl::IconLocRight, "Right" ),
   EnumTable::Enums( GuiIconButtonCtrl::IconLocNone, "None" ),
};
static EnumTable gIconLocTable(3, &iconLocEnums[0]); 


//-------------------------------------
void GuiIconButtonCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("buttonMargin", TypePoint2I,    Offset(mButtonMargin, GuiIconButtonCtrl));
   addField("iconBitmap", TypeFilename,   Offset(mBitmapName, GuiIconButtonCtrl));
   addField("iconLocation", TypeEnum, Offset(mIconLocation, GuiIconButtonCtrl), 1, &gIconLocTable);
   addField("sizeIconToButton", TypeBool,   Offset(mFitBitmapToButton, GuiIconButtonCtrl));
   addField("textLocation", TypeEnum, Offset(mTextLocation, GuiIconButtonCtrl), 1, &gTextLocTable);
   addField("textMargin", TypeS32,    Offset(mTextMargin, GuiIconButtonCtrl));   

}


//-------------------------------------
bool GuiIconButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);

   setBitmap(mBitmapName);

   if( mProfile )
      mProfile->constructBitmapArray();

   return true;
}


//-------------------------------------
void GuiIconButtonCtrl::onSleep()
{
   mTextureNormal = NULL;
   Parent::onSleep();
}


//-------------------------------------

ConsoleMethod( GuiIconButtonCtrl, setBitmap, void, 3, 3, "(filepath name) Loads bitmap from file\n"
              "@param name The path of the desired bitmap file\n"
              "@return No Return Value.")
{
   char* argBuffer = Con::getArgBuffer( 512 );
   Platform::makeFullPathName( argv[2], argBuffer, 512 );
   object->setBitmap( argBuffer );
}

//-------------------------------------
void GuiIconButtonCtrl::inspectPostApply()
{
   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the normal bitmap (if present)
   Parent::inspectPostApply();

   if ((getWidth() == 0) && (getHeight() == 0) && mTextureNormal)
   {
//      TextureObject *texture = (TextureObject *) mTextureNormal;
       Parent::inspectPostApply();
   }
}


void GuiIconButtonCtrl::onStaticModified(const char* slotName, const char* newValue)
{
    if ( isProperlyAdded() && !dStricmp(slotName, "autoSize") )
        resize( getPosition(), getExtent() );
}

bool GuiIconButtonCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
    if ( !mAutoSize || !mProfile->mFont )
        return Parent::resize( newPosition, newExtent );
    
    Point2I autoExtent( mMinExtent );
    
    if ( mIconLocation != IconLocNone )
    {
        autoExtent.y = mTextureNormal->getHeight() + mButtonMargin.y * 2;
        autoExtent.x = mTextureNormal->getWidth() + mButtonMargin.x * 2;
    }
    
    if ( mTextLocation != TextLocNone && mButtonText && mButtonText[0] )
    {
        U32 strWidth = mProfile->mFont->getStrWidthPrecise( mButtonText );
        
        if ( mTextLocation == TextLocLeft || mTextLocation == TextLocRight )
        {
            autoExtent.x += strWidth + mTextMargin * 2;
        }
        else // Top, Bottom, Center
        {
            strWidth += mTextMargin * 2;
            if ( strWidth > autoExtent.x )
                autoExtent.x = strWidth;
        }
    }
    
    return Parent::resize( newPosition, autoExtent );
}

//-------------------------------------
void GuiIconButtonCtrl::setBitmap(const char *name)
{
   mBitmapName = StringTable->insert(name);
   if(!isAwake())
      return;

   if (*mBitmapName)
   {
       mTextureNormal = GFXTextureObject::create( name, &GFXDefaultPersistentProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__) );
//      mTextureNormal = TextureHandle(name, TextureHandle::BitmapTexture, true);
   }
   else
   {
      mTextureNormal = nullptr;
   }
   setUpdate();
}   

//-------------------------------------
void GuiIconButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   renderButton( offset, updateRect);
}

//------------------------------------------------------------------------------

void GuiIconButtonCtrl::renderButton( Point2I &offset, const RectI& updateRect )
{
   bool highlight = mMouseOver;
   bool depressed = mDepressed;

   ColorI fontColor;

   if (!mActive)
       fontColor = mProfile->mFontColorNA;
   else
   {
       if (highlight)
           fontColor = mProfile->mFontColorHL;
       else if (mStateOn)
           fontColor = mProfile->mFontColorSEL;
       else
           fontColor = mProfile->mFontColor;
   }

   ColorI backColor   = mActive ? mProfile->mFillColor : mProfile->mFillColorNA; 
   ColorI borderColor = mActive ? mProfile->mBorderColor : mProfile->mBorderColorNA;

   RectI boundsRect(offset, getExtent());

   if (mDepressed || mStateOn)
   {
      // If there is a bitmap array then render using it.  Otherwise use a standard
      // fill.
      if( mProfile->mBitmapArrayRects.size() )
         renderBitmapArray(boundsRect, statePressed);
      else
         renderLoweredBox(boundsRect, mProfile);
   }
   else if(mMouseOver && mActive)
   {
      // If there is a bitmap array then render using it.  Otherwise use a standard
      // fill.
      if(mProfile->mBitmapArrayRects.size())
         renderBitmapArray(boundsRect, stateMouseOver);
      else
         renderRaisedBox(boundsRect, mProfile);
   }
   else
   {
      // If there is a bitmap array then render using it.  Otherwise use a standard
      // fill.
      if(mProfile->mBitmapArrayRects.size())
      {
         if(mActive)
            renderBitmapArray(boundsRect, stateNormal);
         else
            renderBitmapArray(boundsRect, stateDisabled);
      }
      else
      {
         GFX->getDrawUtil()->drawRectFill(boundsRect, mProfile->mFillColorNA);
         GFX->getDrawUtil()->drawRect(boundsRect, mProfile->mBorderColorNA);
      }
   }

   Point2I textPos = offset;
   if(depressed)
      textPos += Point2I(1,1);

   // Render the icon
   if ( mTextureNormal && mIconLocation != GuiIconButtonCtrl::IconLocNone )
   {
      // Render the normal bitmap
      GFX->getDrawUtil()->clearBitmapModulation();
//      std::shared_ptr<GFXTextureObject> *texture = (std::shared_ptr<GFXTextureObject> *) mTextureNormal;

      // Maintain the bitmap size or fill the button?
      if(!mFitBitmapToButton)
      {
         RectI iconRect(offset + mButtonMargin, Point2I(mTextureNormal->getBitmapWidth(),mTextureNormal->getBitmapHeight()));
         Point2I textureSize( mTextureNormal->getBitmapWidth(), mTextureNormal->getBitmapHeight() );

         if( mIconLocation == IconLocRight )         
            iconRect.set( offset + getExtent() - ( mButtonMargin + textureSize ), textureSize  );
         else if( mIconLocation == IconLocLeft )
            iconRect.set(offset + mButtonMargin, textureSize );

         GFX->getDrawUtil()->drawBitmapStretch(mTextureNormal, iconRect);

      } 
      else
      {
         RectI rect(offset + mButtonMargin, getExtent() - (mButtonMargin * 2) );        
         GFX->getDrawUtil()->drawBitmapStretch(mTextureNormal, rect);
      }

   }

   // Render text
   if(mTextLocation != TextLocNone)
   {
      GFX->getDrawUtil()->setBitmapModulation( fontColor );
      S32 textWidth = mProfile->mFont->getStrWidth(mButtonText);

      if(mTextLocation == TextLocRight)
      {

         Point2I start( mTextMargin, (getHeight()-mProfile->mFont->getHeight())/2 );
         if( mTextureNormal && mIconLocation != GuiIconButtonCtrl::IconLocNone )
         {
//            TextureObject *texture = (TextureObject *) mTextureNormal;
            start.x = mTextureNormal->getBitmapWidth() + mButtonMargin.x + mTextMargin;
         }

         GFX->getDrawUtil()->drawText( mProfile->mFont, start + offset, mButtonText, mProfile->mFontColors );

      }

      if(mTextLocation == TextLocCenter)
      {
         Point2I start;
         if( mTextureNormal && mIconLocation != GuiIconButtonCtrl::IconLocNone )
         {
//            TextureObject *texObject = (TextureObject *) mTextureNormal;
            start.set( ( (getWidth() - textWidth - mTextureNormal->getBitmapWidth())/2) + mTextureNormal->getBitmapWidth(), (getHeight()-mProfile->mFont->getHeight())/2 );
         }
         else
            start.set( (getWidth() - textWidth)/2, (getHeight()-mProfile->mFont->getHeight())/2 );
         GFX->getDrawUtil()->setBitmapModulation( fontColor );
         GFX->getDrawUtil()->drawText( mProfile->mFont, start + offset, mButtonText, mProfile->mFontColors );

      }
   }

   renderChildControls( offset, updateRect);
}

// Draw the bitmap array's borders according to the button's state.
void GuiIconButtonCtrl::renderBitmapArray(RectI &bounds, S32 state)
{
   switch(state)
   {
   case stateNormal:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 1, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 1, mProfile);
      break;

   case stateMouseOver:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 2, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 2, mProfile);
      break;

   case statePressed:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 3, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 3, mProfile);
      break;

   case stateDisabled:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 4, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 4, mProfile);
      break;
   }
}
