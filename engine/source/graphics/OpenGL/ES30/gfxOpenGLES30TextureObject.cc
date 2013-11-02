//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platformGL.h"
#include "console/console.h"
#include "math/mRect.h"
#include "gfxOpenGLES30TextureObject.h"
#include "gfxOpenGLES30Device.h"
#include "gfxOpenGLES30EnumTranslate.h"
#include "gfxOpenGLES30TextureManager.h"
#include "gfxOpenGLES30Utils.h"
#include "graphics/gfxCardProfile.h"


GFXOpenGLES30TextureObject::GFXOpenGLES30TextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXOpenGLTextureObject(aDevice, profile),
   mGLDevice(static_cast<GFXOpenGLES30Device*>(mDevice))
{
   AssertFatal(dynamic_cast<GFXOpenGLES30Device*>(mDevice), "GFXOpenGLES30TextureObject::GFXOpenGLES30TextureObject - Invalid device type, expected GFXOpenGLES30Device!");
}

GFXOpenGLES30TextureObject::~GFXOpenGLES30TextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}


U8* GFXOpenGLES30TextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
}

void GFXOpenGLES30TextureObject::reloadFromCache()
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




