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
   mGLDevice(static_cast<GFXOpenGLES20Device*>(mDevice))
{
   AssertFatal(dynamic_cast<GFXOpenGLES20Device*>(mDevice), "GFXOpenGLES20TextureObject::GFXOpenGLES20TextureObject - Invalid device type, expected GFXOpenGLES20Device!");
}

GFXOpenGLES20TextureObject::~GFXOpenGLES20TextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}


U8* GFXOpenGLES20TextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
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




