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
#include "graphics/gfxDrawUtil.h"
#include "console/consoleTypes.h"
#include "platform/platformAudio.h"
#include "gui/guiCanvas.h"
#include "gui/guiDefaultControlRender.h"
#include "gui/buttons/guiBitmapButtonCtrl.h"

IMPLEMENT_CONOBJECT(GuiBitmapButtonCtrl);

GuiBitmapButtonCtrl::GuiBitmapButtonCtrl()
{
   mBitmapName = StringTable->EmptyString;
   mBitmapNormal = StringTable->EmptyString;
   mBitmapHilight = StringTable->EmptyString;
   mBitmapDepressed = StringTable->EmptyString;
   mBitmapInactive = StringTable->EmptyString;

   mIsLegacyVersion = true;

   setExtent(140, 30);
}

void GuiBitmapButtonCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("isLegacyVersion", TypeBool, Offset(mIsLegacyVersion, GuiBitmapButtonCtrl), "Determines if this is a legacy version of the control (only uses bitmap field)");
   addField("bitmap", TypeFilename, Offset(mBitmapName, GuiBitmapButtonCtrl), "Base name for the bitmaps used in the button states.\n For example, you would only put \"button\""
                                                                              " to load button_n.png, button_d.png, button_h.png and button_i.png");
   addField("bitmapNormal", TypeFilename, Offset(mBitmapNormal, GuiBitmapButtonCtrl), "Name of texture used for the normal button state");
   addField("bitmapHilight", TypeFilename, Offset(mBitmapHilight, GuiBitmapButtonCtrl), "Name of texture used for the hilight/hover button state");
   addField("bitmapDepressed", TypeFilename, Offset(mBitmapDepressed, GuiBitmapButtonCtrl), "Name of texture used for the depressed button state");
   addField("bitmapInactive", TypeFilename, Offset(mBitmapInactive, GuiBitmapButtonCtrl), "Name of texture used for the inactive button state");
}

bool GuiBitmapButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   if(mIsLegacyVersion)
   {
       setBitmap(mBitmapName);
   }
   else
   {
       setBitmap(mBitmapNormal, NORMAL);
       setBitmap(mBitmapHilight, HILIGHT);
       setBitmap(mBitmapDepressed, DEPRESSED);
       setBitmap(mBitmapInactive, INACTIVE);
   }
   
   return true;
}

void GuiBitmapButtonCtrl::onSleep()
{
   mTextureNormal = NULL;
   mTextureHilight = NULL;
   mTextureDepressed = NULL;
   mTextureInactive = NULL;

   Parent::onSleep();
}

ConsoleMethod( GuiBitmapButtonCtrl, setBitmap, void, 3, 3, "(filepath name) Loads a bitmap from a given file\n"
              "@return No return value.")
{
   object->setBitmap(argv[2]);
}

ConsoleMethod( GuiBitmapButtonCtrl, setBitmapNormal, void, 3, 3, "(filepath name) Loads a bitmap from a given file for the \"up\" state\n"
              "@return No return value.")
{
   object->setBitmap(argv[2], NORMAL);
}
ConsoleMethod( GuiBitmapButtonCtrl, setBitmapHilight, void, 3, 3, "(filepath name) Loads a bitmap from a given file for the \"down\" state\n"
              "@return No return value.")
{
   object->setBitmap(argv[2], HILIGHT);
}
ConsoleMethod( GuiBitmapButtonCtrl, setBitmapDepressed, void, 3, 3, "(filepath name) Loads a bitmap from a given file for the \"hover\" state\n"
              "@return No return value.")
{
   object->setBitmap(argv[2], DEPRESSED);
}
ConsoleMethod( GuiBitmapButtonCtrl, setBitmapInactive, void, 3, 3, "(filepath name) Loads a bitmap from a given file for the \"inactive\" state\n"
              "@return No return value.")
{
   object->setBitmap(argv[2], INACTIVE);
}

void GuiBitmapButtonCtrl::inspectPostApply()
{
   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the normal bitmap (if present)
   Parent::inspectPostApply();

   if ((getWidth() == 0) && (getHeight() == 0) && mTextureNormal)
   {
      setExtent( mTextureNormal->getBitmapWidth(), mTextureNormal->getBitmapHeight());
   }
}

void GuiBitmapButtonCtrl::setBitmap(const String& name)
{
//   mBitmapName = StringTable->insert(name);
    mBitmapName = name;

   if(!isAwake())
       return;

   if (*mBitmapName)
   {
      char buffer[1024];
      char *p;
      dStrcpy(buffer, name);
      p = buffer + dStrlen(buffer);

       String baseName = mBitmapName;
       static String s_n = "_n";
       static String s_d = "_d";
       static String s_h = "_h";
       static String s_i = "_i";
       
       mTextureNormal = GFXTextureObject::create( baseName, &GFXDefaultPersistentProfile, avar("mTextureNormal"));
      if (!mTextureNormal)
      {
          mTextureNormal = GFXTextureObject::create( baseName + s_n, &GFXDefaultPersistentProfile, avar("mTextureNormal"));
      }
       mTextureHilight = GFXTextureObject::create( baseName + s_d, &GFXDefaultPersistentProfile, avar("mTextureHilight"));

       if (!mTextureHilight)
         mTextureHilight = mTextureNormal;

       mTextureDepressed = GFXTextureObject::create( baseName + s_h, &GFXDefaultPersistentProfile, avar("mTextureDepressed"));

       if (!mTextureDepressed)
         mTextureDepressed = mTextureHilight;

       mTextureInactive = GFXTextureObject::create( baseName + s_i, &GFXDefaultPersistentProfile, avar("mTextureInactive"));

       if (!mTextureInactive)
         mTextureInactive = mTextureNormal;
       
//      mTextureNormal = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
//      if (!mTextureNormal)
//      {
//         dStrcpy(p, "_n");
//         mTextureNormal = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
//      }
//      dStrcpy(p, "_h");
//      mTextureHilight = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
//      if (!mTextureHilight)
//         mTextureHilight = mTextureNormal;
//      dStrcpy(p, "_d");
//      mTextureDepressed = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
//      if (!mTextureDepressed)
//         mTextureDepressed = mTextureHilight;
//      dStrcpy(p, "_i");
//      mTextureInactive = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
//      if (!mTextureInactive)
//         mTextureInactive = mTextureNormal;
   }
   else
   {
      mTextureNormal = NULL;
      mTextureHilight = NULL;
      mTextureDepressed = NULL;
      mTextureInactive = NULL;
   }
   setUpdate();
}

void GuiBitmapButtonCtrl::setBitmap(const String& name, ButtonState state)
{
   if(!isAwake() && *name)
       return;
   
//   StringTableEntry temporaryName = StringTable->insert(name);
    String temporaryName = name;
   
   if (*temporaryName)
   {
       char buffer[1024];
       char *p;
       dStrcpy(buffer, name);
       p = buffer + dStrlen(buffer);

       switch (state)
       {
          case NORMAL:
              mBitmapNormal = temporaryName;
               mTextureNormal = GFXTextureObject::create( temporaryName, &GFXDefaultPersistentProfile, avar("mTextureNormal"));
//              mTextureNormal = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
              break;
          case HILIGHT:
              mBitmapHilight = temporaryName;
               mTextureHilight = GFXTextureObject::create( temporaryName, &GFXDefaultPersistentProfile, avar("mTextureHilight"));
//              mTextureHilight = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
              break;
          case DEPRESSED:
              mBitmapDepressed = temporaryName;
               mTextureDepressed = GFXTextureObject::create( temporaryName, &GFXDefaultPersistentProfile, avar("mTextureDepressed"));
//              mTextureDepressed = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
              break;
          case INACTIVE:
              mBitmapInactive = temporaryName;
               mTextureInactive = GFXTextureObject::create( temporaryName, &GFXDefaultPersistentProfile, avar("mTextureInactive"));
//              mTextureInactive = TextureHandle(buffer, TextureHandle::BitmapTexture, true);
              break;
       }
   }
   else
   {
       switch (state)
       {
          case NORMAL:
              mTextureNormal = nullptr;
              break;
          case HILIGHT:
              mTextureHilight = nullptr;
              break;
          case DEPRESSED:
              mTextureDepressed = nullptr;
              break;
          case INACTIVE:
              mTextureInactive = nullptr;
              break;
       } 
   }

   setUpdate();
}

//-------------------------------------
void GuiBitmapButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   ButtonState state = NORMAL;

   if (mActive)
   {
      if (mMouseOver)
          state = HILIGHT;

      if (mDepressed || mStateOn)
          state = DEPRESSED;
   }
   else
      state = INACTIVE;

   switch (state)
   {
      case NORMAL:
          renderButton(mTextureNormal, offset, updateRect);
          break;
      case HILIGHT:
          renderButton(mTextureHilight ? mTextureHilight : mTextureNormal, offset, updateRect);
          break;
      case DEPRESSED:
          renderButton(mTextureDepressed, offset, updateRect);
          break;
      case INACTIVE:
          renderButton(mTextureInactive ? mTextureInactive : mTextureNormal, offset, updateRect);
          break;
   }
}

//------------------------------------------------------------------------------

void GuiBitmapButtonCtrl::renderButton(std::shared_ptr<GFXTextureObject> &texture, Point2I &offset, const RectI& updateRect)
{
   GFX->getDrawUtil()->clearBitmapModulation();

   if (texture)
   {
      RectI rect(offset, getExtent());
      GFX->getDrawUtil()->drawBitmapStretch(texture, rect);
      renderChildControls( offset, updateRect);
   }
   else
      Parent::onRender(offset, updateRect);
}

//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(GuiBitmapButtonTextCtrl);

void GuiBitmapButtonTextCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   enum {
      NORMAL,
      HILIGHT,
      DEPRESSED,
      INACTIVE
   } state = NORMAL;

   if (mActive)
   {
      if (mMouseOver) state = HILIGHT;
      if (mDepressed || mStateOn) state = DEPRESSED;
   }
   else
      state = INACTIVE;

   std::shared_ptr<GFXTextureObject> texture;

   switch (state)
   {
      case NORMAL:
         texture = mTextureNormal;
         break;
      case HILIGHT:
         texture = mTextureHilight;
         break;
      case DEPRESSED:
         texture = mTextureDepressed;
         break;
      case INACTIVE:
         texture = mTextureInactive;
         if(!texture)
            texture = mTextureNormal;
         break;
   }
   if (texture)
   {
      RectI rect(offset, getExtent());
      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapStretch(texture, rect);

      Point2I textPos = offset;
      if(mDepressed)
         textPos += Point2I(1,1);

      // Make sure we take the profile's textOffset into account.
      textPos += mProfile->mTextOffset;

      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
      renderJustifiedText(textPos, getExtent(), mButtonText);

      renderChildControls( offset, updateRect);
   }
   else
      Parent::onRender(offset, updateRect);
}
