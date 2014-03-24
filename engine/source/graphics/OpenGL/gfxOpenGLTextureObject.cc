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

#include "math/mRect.h"
#include "./gfxOpenGLTextureObject.h"
#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLTextureManager.h"
#include "./gfxOpenGLUtils.h"
#include "graphics/gfxCardProfile.h"
#include "platform/platformGL.h"


GFXOpenGLTextureObject::GFXOpenGLTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, void* texInfo) : GFXTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mZombieCache(NULL),
   mIsNPoT2(true),
   mMinFilter(GL_LINEAR),
   mMagFilter(GL_LINEAR),
   mWrapS(GL_CLAMP_TO_EDGE),
   mWrapT(GL_CLAMP_TO_EDGE)
{
   
}

GFXOpenGLTextureObject::GFXOpenGLTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile): GFXTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mZombieCache(NULL),
   mIsNPoT2(true)
{
   
}

GFXOpenGLTextureObject::~GFXOpenGLTextureObject()
{
   
}

void GFXOpenGLTextureObject::bind(U32 textureUnit)
{
    GFXOpenGLDevice* device = dynamic_cast<GFXOpenGLDevice*>(GFX);
    AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGLTextureObject::bind - only GL_TEXTURE_2D supported");
   
   device->setTextureUnit(textureUnit);
   glBindTexture(mBinding, mHandle);

    GFXStateBlockRef sb =device->getCurrentStateBlock();
//   GFXOpenGLStateBlock* sb = dynamic_cast<GFXOpenGLStateBlock*>(device->getCurrentStateBlock().get());
   AssertFatal(sb, "GFXOpenGLTextureObject::bind - No active stateblock!");
   if (!sb)
      return;

   const GFXSamplerStateDesc ssd = std::dynamic_pointer_cast<GFXOpenGLStateBlock>(sb)->getDesc().samplers[textureUnit];
   setParameter( GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));
   setParameter( GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);

   setParameter(GL_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeU] : GL_CLAMP_TO_EDGE);
   setParameter(GL_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeV] : GL_CLAMP_TO_EDGE);
}

void GFXOpenGLTextureObject::release()
{
   glDeleteTextures(1, &mHandle);
   mHandle = 0;
}

void GFXOpenGLTextureObject::setFilter(const GFXTextureFilterType filter)
{
   GFXOpenGLDevice* device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGLTextureObject::bind - only GL_TEXTURE_2D supported");

   // Finish if no GL texture name.
    if ( mHandle == 0 )
        return;

    // Set texture state.
    device->setTextureUnit(0);
    glBindTexture( mBinding, mHandle );
    setParameter(GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[filter] );
    setParameter(GL_TEXTURE_MIN_FILTER, GFXGLTextureFilter[filter] );
}


void GFXOpenGLTextureObject::setParameter( GLenum pname, GLint param)
{
   switch (pname) {
      case GL_TEXTURE_MIN_FILTER:
         if (mMinFilter != param)
         {
            mMinFilter = param;
            glTexParameteri(mBinding, pname, mMinFilter);
         }
         break;
         
      case GL_TEXTURE_MAG_FILTER:
         if (mMagFilter != param)
         {
            mMagFilter = param;
            glTexParameteri(mBinding, pname, mMagFilter);
         }
         break;

      case GL_TEXTURE_WRAP_S:
         if (mWrapS != param)
         {
            mWrapS = param;
            glTexParameteri(mBinding, pname, mWrapS);
         }
         break;

      case GL_TEXTURE_WRAP_T:
         if (mWrapT != param)
         {
            mWrapT = param;
            glTexParameteri(mBinding, pname, mWrapT);
         }
         break;

      default:
         break;
   }
}

GFXLockedRect* GFXOpenGLTextureObject::lock(U32 mipLevel, RectI *inRect)
{
   AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGLTextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;
   
   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXOpenGLTextureObject::lock - Rectangle too big!");
      
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

void GFXOpenGLTextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;
   
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   device->setTextureUnit(0);
   U32 boundTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
   mLockedRect.bits = NULL;
   
   glBindTexture(GL_TEXTURE_2D, boundTexture);
}



F32 GFXOpenGLTextureObject::getMaxUCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getWidth();
}

F32 GFXOpenGLTextureObject::getMaxVCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getHeight();
}


void GFXOpenGLTextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
   
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
   glBindTexture(mBinding, 0);
}


void GFXOpenGLTextureObject::zombify()
{
   if(mIsZombie)
      return;
   
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
   
   release();
}

void GFXOpenGLTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
   
   glGenTextures(1, &mHandle);
}

const String GFXOpenGLTextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   ret += String::ToString("   GL Handle: %i", mHandle);
   
   return ret;
}


GBitmap* GFXOpenGLTextureObject::getBitmap()
{
   if (mBitmap)
      return mBitmap;
   
   if (mPath.isEmpty())
      return NULL;
   
   return NULL;
}

