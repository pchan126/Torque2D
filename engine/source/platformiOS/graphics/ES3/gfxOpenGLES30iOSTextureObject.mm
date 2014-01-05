//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <OpenGLES/ES3/glext.h>

#include "console/console.h"
#include "math/mRect.h"
#include "./GFXOpenGLES30iOSTextureObject.h"
#include "./gfxOpenGLES30iOSDevice.h"
#include "./gfxOpenGLES30iOSEnumTranslate.h"
#include "./gfxOpenGLES30iOSTextureManager.h"
#include "./gfxOpenGLES30iOSUtils.h"
#include "graphics/gfxCardProfile.h"
#import <GLKit/GLKit.h>

GFXOpenGLES30iOSTextureObject::GFXOpenGLES30iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* textureInfo) :
    GFXOpenGLES30TextureObject(aDevice, profile),
    mBytesPerTexel(4),
    mLockedRectRect(0, 0, 0, 0),
    mGLDevice(static_cast<GFXOpenGLES30iOSDevice*>(mDevice)),
    mZombieCache(nullptr)
{
    mTextureSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBitmapSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBinding = [ textureInfo target];
    mHandle = [ textureInfo name ];
}


GFXOpenGLES30iOSTextureObject::GFXOpenGLES30iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXOpenGLES30TextureObject(aDevice, profile),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mGLDevice(static_cast<GFXOpenGLES30iOSDevice*>(mDevice)),
   mZombieCache(nullptr)
{
   AssertFatal(dynamic_cast<GFXOpenGLES30iOSDevice*>(mDevice), "GFXOpenGLES30iOSTextureObject::GFXOpenGLES30iOSTextureObject - Invalid device type, expected GFXOpenGLES30iOSDevice!");
   glGenTextures(1, &mHandle);
}

GFXOpenGLES30iOSTextureObject::~GFXOpenGLES30iOSTextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}

CIImage* GFXOpenGLES30iOSTextureObject::getCIImage()
{
   CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
   CIImage *ret = [CIImage imageWithTexture:mHandle size:CGSizeMake(mTextureSize.x, mTextureSize.y) flipped:NO colorSpace:cs];
   CGColorSpaceRelease(cs);
   return ret;
}


