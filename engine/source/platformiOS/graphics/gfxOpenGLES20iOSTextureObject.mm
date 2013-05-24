//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <OpenGLES/ES2/glext.h>

#include "console/console.h"
#include "math/mRect.h"
#include "platformiOS/graphics/GFXOpenGLES20iOSTextureObject.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSEnumTranslate.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSUtils.h"
#include "graphics/gfxCardProfile.h"
#import <GLKit/GLKit.h>

GFXOpenGLES20iOSTextureObject::GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* textureInfo) :
    GFXTextureObject(aDevice, profile),
    mBytesPerTexel(4),
    mLockedRectRect(0, 0, 0, 0),
    mGLDevice(static_cast<GFXOpenGLES20iOSDevice*>(mDevice)),
    mZombieCache(NULL),
    mFilter( GL_NEAREST )
{
    mTextureSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBitmapSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBinding = [ textureInfo target];
    mHandle = [ textureInfo name ];
}


GFXOpenGLES20iOSTextureObject::GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mGLDevice(static_cast<GFXOpenGLES20iOSDevice*>(mDevice)),
   mZombieCache(NULL)
{
   AssertFatal(dynamic_cast<GFXOpenGLES20iOSDevice*>(mDevice), "GFXOpenGLES20iOSTextureObject::GFXOpenGLES20iOSTextureObject - Invalid device type, expected GFXOpenGLES20iOSDevice!");
   glGenTextures(1, &mHandle);
}

GFXOpenGLES20iOSTextureObject::~GFXOpenGLES20iOSTextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}

GFXLockedRect* GFXOpenGLES20iOSTextureObject::lock(U32 mipLevel, RectI *inRect)
{
//   AssertFatal(mBinding != GL_TEXTURE_3D, "GFXOpenGLES20iOSTextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;

   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXOpenGLES20iOSTextureObject::lock - Rectangle too big!");

      mLockedRectRect = *inRect;
   }
   else
   {
      mLockedRectRect = RectI(0, 0, width, height);
   }
   
   mLockedRect.pitch = mLockedRectRect.extent.x * mBytesPerTexel;
   
   if( !mLockedRect.bits )
      return NULL;

   return &mLockedRect;
}

void GFXOpenGLES20iOSTextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;

   glActiveTexture(GL_TEXTURE0);
   U32 boundTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
   mLockedRect.bits = NULL;

   glBindTexture(GL_TEXTURE_2D, boundTexture);
}

void GFXOpenGLES20iOSTextureObject::release()
{
   glDeleteTextures(1, &mHandle);
   mHandle = 0;
}

GBitmap* GFXOpenGLES20iOSTextureObject::getBitmap()
{
    if (mBitmap)
        return mBitmap;
    
    if (mPath.isEmpty())
        return NULL;

    return NULL;
}


bool GFXOpenGLES20iOSTextureObject::copyToBmp(GBitmap * bmp)
{
    // not supported in opengl es
   return false;
}


U8* GFXOpenGLES20iOSTextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
}

void GFXOpenGLES20iOSTextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
    
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
   glBindTexture(mBinding, 0);
}

void GFXOpenGLES20iOSTextureObject::reloadFromCache()
{
   if(!mZombieCache)
      return;
      
   glBindTexture(mBinding, mHandle);
   glTexSubImage2D(mBinding, 0, 0, 0, mTextureSize.x, mTextureSize.y, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   
//   if(GFX->getCardProfiler()->queryProfile("GL::Workaround::needsExplicitGenerateMipmap") && mMipLevels != 1)
      glGenerateMipmap(mBinding);
      
   delete[] mZombieCache;
   mZombieCache = NULL;
   mIsZombie = false;
}

void GFXOpenGLES20iOSTextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXOpenGLES20iOSTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   glGenTextures(1, &mHandle);
}

F32 GFXOpenGLES20iOSTextureObject::getMaxUCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getWidth();
}

F32 GFXOpenGLES20iOSTextureObject::getMaxVCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getHeight();
}

const String GFXOpenGLES20iOSTextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   ret += String::ToString("   GL Handle: %i", mHandle);
   
   return ret;
}

void GFXOpenGLES20iOSTextureObject::setFilter(const GFXTextureFilterType filter)
{
//    // Set filter.
//    mFilter = GFXGLTextureFilter[filter];
//    
//    // Finish if no GL texture name.
//    if ( mHandle == 0 )
//        return;
//    
//    // Set texture state.
//    glBindTexture( GL_TEXTURE_2D, mHandle );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
}


