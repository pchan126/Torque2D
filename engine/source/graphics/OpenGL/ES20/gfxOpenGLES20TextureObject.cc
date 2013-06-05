//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platformGL.h"
#include "console/console.h"
#include "math/mRect.h"
#include "./GFXOpenGLES20TextureObject.h"
#include "./GFXOpenGLES20Device.h"
#include "./GFXOpenGLES20EnumTranslate.h"
#include "./GFXOpenGLES20TextureManager.h"
#include "./GFXOpenGLES20Utils.h"
#include "graphics/gfxCardProfile.h"


GFXOpenGLES20TextureObject::GFXOpenGLES20TextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXOpenGLTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mGLDevice(static_cast<GFXOpenGLES20Device*>(mDevice)),
   mZombieCache(NULL)
{
   AssertFatal(dynamic_cast<GFXOpenGLES20Device*>(mDevice), "GFXOpenGLES20TextureObject::GFXOpenGLES20TextureObject - Invalid device type, expected GFXOpenGLES20Device!");
   glGenTextures(1, &mHandle);
}

GFXOpenGLES20TextureObject::~GFXOpenGLES20TextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}

GFXLockedRect* GFXOpenGLES20TextureObject::lock(U32 mipLevel, RectI *inRect)
{
//   AssertFatal(mBinding != GL_TEXTURE_3D, "GFXOpenGLES20TextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;

   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXOpenGLES20TextureObject::lock - Rectangle too big!");

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

void GFXOpenGLES20TextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;

   glActiveTexture(GL_TEXTURE0);
   U32 boundTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
   mLockedRect.bits = NULL;

   glBindTexture(GL_TEXTURE_2D, boundTexture);
}

void GFXOpenGLES20TextureObject::release()
{
   glDeleteTextures(1, &mHandle);
   mHandle = 0;
}


bool GFXOpenGLES20TextureObject::copyToBmp(GBitmap * bmp)
{
    // not supported in opengl es
   return false;
}


U8* GFXOpenGLES20TextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
}

void GFXOpenGLES20TextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
    
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
   glBindTexture(mBinding, 0);
}

void GFXOpenGLES20TextureObject::reloadFromCache()
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

void GFXOpenGLES20TextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXOpenGLES20TextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   glGenTextures(1, &mHandle);
}

const String GFXOpenGLES20TextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   ret += String::ToString("   GL Handle: %i", mHandle);
   
   return ret;
}

void GFXOpenGLES20TextureObject::setFilter(const GFXTextureFilterType filter)
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


