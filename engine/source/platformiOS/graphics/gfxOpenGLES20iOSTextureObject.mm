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
    GFXOpenGLES20TextureObject(aDevice, profile),
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
   GFXOpenGLES20TextureObject(aDevice, profile),
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


U8* GFXOpenGLES20iOSTextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
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


