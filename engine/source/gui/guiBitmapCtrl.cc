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
#include "gui/guiBitmapCtrl.h"

IMPLEMENT_CONOBJECT(GuiBitmapCtrl);

GuiBitmapCtrl::GuiBitmapCtrl(void)
{
    mBitmapName = StringTable->EmptyString;
    startPoint.set(0, 0);
    mWrap = false;

    //Luma:	Ability to specify source rect for image UVs
    mUseSourceRect = false;
    mSourceRect.set(0, 0, 0, 0);
}

bool GuiBitmapCtrl::setBitmapName( void *obj, const char *data )
{
   // Prior to this, you couldn't do bitmap.bitmap = "foo.jpg" and have it work.
   // With protected console types you can now call the setBitmap function and
   // make it load the image.
   static_cast<GuiBitmapCtrl *>( obj )->setBitmap( data );

   // Return false because the setBitmap method will assign 'mBitmapName' to the
   // argument we are specifying in the call.
   return false;
}

void GuiBitmapCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addGroup("GuiBitmapCtrl");		
   addProtectedField( "bitmap", TypeFilename, Offset( mBitmapName, GuiBitmapCtrl ), &setBitmapName, &defaultProtectedGetFn, "" );
   //addField("bitmap", TypeFilename, Offset(mBitmapName, GuiBitmapCtrl));
   addField("wrap",   TypeBool,     Offset(mWrap,       GuiBitmapCtrl));
   endGroup("GuiBitmapCtrl");		

   //Luma:	ability to specify source rect for image UVs
   addGroup("Misc");
   //ability to specify source rect for image UVs
   addField( "useSourceRect", TypeBool, Offset( mUseSourceRect, GuiBitmapCtrl ));
   addField( "sourceRect", TypeRectI, Offset( mSourceRect, GuiBitmapCtrl ));
   endGroup("Misc");	
}

ConsoleMethod( GuiBitmapCtrl, setValue, void, 4, 4, "(int xAxis, int yAxis)"
              "Set the offset of the bitmap.\n"
              "@return No return value."
              )
{
   object->setValue(dAtoi(argv[2]), dAtoi(argv[3]));
}

ConsoleMethod( GuiBitmapCtrl, setBitmap, void, 3, 3, "( pathName ) Use the setBitmap method to change the bitmap this control uses.\n"
                                                                "@param pathName A path to a new texture for this control. Limited to 256x256.\n"
                                                                "@return No return value")
{
   object->setBitmap(argv[2]);
}

ConsoleMethod(GuiBitmapCtrl, getTextureWidth, S32, 2, 2, "Gets the Width of the Texture.\n"
              "@return Texture Width"
              )
{
   return object->getWidth();
}

ConsoleMethod(GuiBitmapCtrl, getTextureHeight, S32, 2, 2, "Gets the Height of the Texture.\n"
              "@return Texture Height"
              )
{
   return object->getHeight();
}

bool GuiBitmapCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);
   setBitmap(mBitmapName);
   return true;
}

void GuiBitmapCtrl::onSleep()
{
   mTextureObject = NULL;
   Parent::onSleep();
}

//-------------------------------------
void GuiBitmapCtrl::inspectPostApply()
{
   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the bitmap (if present)
   Parent::inspectPostApply();

   if (!mWrap && (getWidth() == 0) && (getHeight() == 0) && mTextureObject)
   {
//      TextureObject *texture = (TextureObject *) mTextureObject;
      setWidth( mTextureObject->getBitmapWidth());
      setHeight( mTextureObject->getBitmapHeight());
   }
}

void GuiBitmapCtrl::setBitmap(const char *name, bool resize)
{
//   mBitmapName = StringTable->insert(name);
    mBitmapName = name;

    if ( mBitmapName.isNotEmpty() )
	{
        if ( !mBitmapName.equal("texhandle", String::NoCase) )
            mTextureObject.set( mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__) );
        
        // Resize the control to fit the bitmap
        if ( mTextureObject && resize )
        {
            setExtent( mTextureObject->getBitmapWidth(), mTextureObject->getBitmapHeight() );
            updateSizing();
        }
    }
    else
        mTextureObject = NULL;
    
    setUpdate();
    
//    if (*mBitmapName) {
//      mTextureObject = GFXTexHandle(mBitmapName, GFXTexHandle::BitmapTexture, true);
//
//      // Resize the control to fit the bitmap
//      if (resize) {
////         TextureObject* texture = (TextureObject *) mTextureObject;
//         getWidth() = mTextureObject->getBitmapWidth();
//         getHeight() = mTextureObject->getBitmapHeight();
//         GuiControl *parent = getParent();
//         if( !parent ) {
//             Con::errorf( "GuiBitmapCtrl::setBitmap( %s ), trying to resize but object has no parent.", name ) ;
//         } else {
//             Point2I extent = parent->getExtent();
//         parentResized(extent,extent);
//      }
//   }
//   }
//   else
//      mTextureObject = NULL;
//   setUpdate();
}

void GuiBitmapCtrl::updateSizing()
{
    if(!getParent())
        return;

    RectI fakeBounds( getPosition(), getParent()->getExtent());
    parentResized( fakeBounds, fakeBounds);
}


void GuiBitmapCtrl::setBitmap(const GFXTexHandle &handle, bool resize)
{
   mTextureObject = handle;

   // Resize the control to fit the bitmap
   if (resize) {
       updateSizing();
   }
}


void GuiBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if (mTextureObject)
   {
      GFX->getDrawUtil()->clearBitmapModulation();
        if(mWrap)
        {
         // We manually draw each repeat because non power of two textures will 
         // not tile correctly when rendered with dglDrawBitmapTile(). The non POT
         // bitmap will be padded by the hardware, and we'll see lots of slack
         // in the texture. So... lets do what we must: draw each repeat by itself:
//         TextureObject* texture = (TextureObject *) mTextureObject;
            RectI srcRegion;
            RectI dstRegion;
            float xdone = ((float)getWidth()/(float)mTextureObject->getBitmapWidth())+1;
            float ydone = ((float)getHeight()/(float)mTextureObject->getBitmapHeight())+1;

            int xshift = startPoint.x%mTextureObject->getBitmapWidth();
            int yshift = startPoint.y%mTextureObject->getBitmapHeight();
            for(int y = 0; y < ydone; ++y)
                for(int x = 0; x < xdone; ++x)
                {
                    //Luma:	ability to specify source rect for image UVs
                    if(mUseSourceRect && mSourceRect.isValidRect())
                    {
                        srcRegion = mSourceRect;
                    }
                    else
                    {
                        srcRegion.set(0,0,mTextureObject->getBitmapWidth(),mTextureObject->getBitmapHeight());
                    }
                    dstRegion.set( ((mTextureObject->getBitmapWidth()*x)+offset.x)-xshift,
                                      ((mTextureObject->getBitmapHeight()*y)+offset.y)-yshift,
                                      mTextureObject->getBitmapWidth(),	
                                      mTextureObject->getBitmapHeight());
//                GFX->getDrawUtil()->drawBitmapStretchSR(texture,dstRegion, srcRegion, false);
                    GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, dstRegion, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear);
                }
        }
        else
      {
         RectI rect(offset, getExtent());
        
         //Luma:	ability to specify source rect for image UVs
         if(mUseSourceRect && mSourceRect.isValidRect() )
         {
            RectI srcRegion;
            srcRegion = mSourceRect;
//            GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject,rect, srcRegion, false);
             GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject,rect, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear);
        }
        else
        {
//            GFX->getDrawUtil()->drawBitmapStretch(mTextureObject, rect);
            GFX->getDrawUtil()->drawBitmapStretch(mTextureObject, rect, GFXBitmapFlip_None, GFXTextureFilterLinear, false);
        }
      }
   }

   if (mProfile->mBorder || !mTextureObject)
   {
      RectI rect(offset.x, offset.y, getWidth(), getHeight());
      GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
   }

   renderChildControls(offset, updateRect);
}

void GuiBitmapCtrl::setValue(S32 x, S32 y)
{
   if (mTextureObject)
   {
//        TextureObject* texture = (TextureObject *) mTextureObject;
        x+=mTextureObject->getBitmapWidth()/2;
        y+=mTextureObject->getBitmapHeight()/2;
    }
    while (x < 0)
        x += 256;
    startPoint.x = x % 256;
                
    while (y < 0)
        y += 256;
    startPoint.y = y % 256;
}

//Luma:	ability to specify source rect for image UVs
void GuiBitmapCtrl::setSourceRect(U32 x, U32 y, U32 width, U32 height) 
{ 
    mSourceRect.set(x, y, width, height); 
} 
void GuiBitmapCtrl::setUseSourceRect(bool bUse)
{
    mUseSourceRect = bUse;
}
