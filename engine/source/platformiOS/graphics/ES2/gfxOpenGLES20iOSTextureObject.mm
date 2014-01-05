//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <OpenGLES/ES2/glext.h>

#include "console/console.h"
#include "math/mRect.h"
#include "./GFXOpenGLES20iOSTextureObject.h"
#include "./gfxOpenGLES20iOSDevice.h"
#include "./gfxOpenGLES20iOSEnumTranslate.h"
#include "./gfxOpenGLES20iOSTextureManager.h"
#include "./gfxOpenGLES20iOSUtils.h"
#include "graphics/gfxCardProfile.h"
#import <GLKit/GLKit.h>

GFXOpenGLES20iOSTextureObject::GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* textureInfo) :
    GFXOpenGLES20TextureObject(aDevice, profile),
    mBytesPerTexel(4),
    mLockedRectRect(0, 0, 0, 0),
    mGLDevice(static_cast<GFXOpenGLES20iOSDevice*>(mDevice)),
    mZombieCache(nullptr)
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
   mZombieCache(nullptr)
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

CIImage* GFXOpenGLES20iOSTextureObject::getCIImage()
{
   CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
   CIImage *ret = [CIImage imageWithTexture:mHandle size:CGSizeMake(mTextureSize.x, mTextureSize.y) flipped:NO colorSpace:cs];
   CGColorSpaceRelease(cs);
   return ret;
}


