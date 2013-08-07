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
#include "./gfxOpenGL33WinTextureObject.h"
#include "./gfxOpenGL33WinDevice.h"
#include "./gfxOpenGL33WinEnumTranslate.h"
#include "./gfxOpenGL33WinTextureManager.h"
#include "./gfxOpenGL33WinUtils.h"
#include "graphics/gfxCardProfile.h"
#include "platform/platformGL.h"

GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, void* texInfo) :
    GFXOpenGLTextureObject(aDevice, profile),
    mGLDevice(static_cast<GFXOpenGL33WinDevice*>(mDevice)),
    mFilter( GL_NEAREST )
{
    mBitmap = NULL;
//    setTexture(texInfo);
}


GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXOpenGLTextureObject(aDevice, profile),
   mGLDevice(static_cast<GFXOpenGL33WinDevice*>(mDevice)),
   mFilter( GL_NEAREST )
{
   AssertFatal(dynamic_cast<GFXOpenGL33WinDevice*>(mDevice), "GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject - Invalid device type, expected GFXOpenGL33WinDevice!");
   glGenTextures(1, &mHandle);
}

GFXOpenGL33WinTextureObject::~GFXOpenGL33WinTextureObject() 
{
   delete[] mZombieCache;
   kill();
}


U8* GFXOpenGL33WinTextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
   return data;
}

void GFXOpenGL33WinTextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
//   if(mBinding == GL_TEXTURE_3D)
//      cacheSize *= mTextureSize.z;
    
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
//   glGetTexImage(mBinding, 0, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   glBindTexture(mBinding, 0);
}

void GFXOpenGL33WinTextureObject::reloadFromCache()
{
   if(!mZombieCache)
      return;
      
//   if(mBinding == GL_TEXTURE_3D)
//   {
//      static_cast<GFXGLTextureManager*>(TEXMGR)->_loadTexture(this, mZombieCache);
//      delete[] mZombieCache;
//      mZombieCache = NULL;
//      return;
//   }
   
   glBindTexture(mBinding, mHandle);
   glTexSubImage2D(mBinding, 0, 0, 0, mTextureSize.x, mTextureSize.y, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   
//   if(GFX->getCardProfiler()->queryProfile("GL::Workaround::needsExplicitGenerateMipmap") && mMipLevels != 1)
//      glGenerateMipmap(mBinding);
    
   delete[] mZombieCache;
   mZombieCache = NULL;
   mIsZombie = false;
}

void GFXOpenGL33WinTextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXOpenGL33WinTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   glGenTextures(1, &mHandle);
}

void GFXOpenGL33WinTextureObject::setFilter(const GFXTextureFilterType filter)
{
    // Set filter.
    mFilter = GFXGLTextureFilter[filter];
    
    // Finish if no GL texture name.
    if ( mHandle == 0 )
        return;
    
    // Set texture state.
    glBindTexture( GL_TEXTURE_2D, mHandle );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter );
}


