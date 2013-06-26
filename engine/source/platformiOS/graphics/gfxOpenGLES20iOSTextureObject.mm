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


