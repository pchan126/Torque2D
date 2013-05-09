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

void GFXOpenGLTextureObject::bind(U32 textureUnit) const
{
    GFXOpenGLDevice* device = dynamic_cast<GFXOpenGLDevice*>(GFX);
    AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGLES20TextureObject::bind - only GL_TEXTURE_2D supported");
   glActiveTexture(GL_TEXTURE0 + textureUnit);

    GLuint han = mHandle;
   glBindTexture(mBinding, han);

   GFXOpenGLStateBlockRef sb = device->getCurrentStateBlock();
   AssertFatal(sb, "GFXOpenGLTextureObject::bind - No active stateblock!");
   if (!sb)
      return;

   const GFXSamplerStateDesc ssd = sb->getDesc().samplers[textureUnit];
   glTexParameteri(mBinding, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));
   glTexParameteri(mBinding, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);

   glTexParameteri(mBinding, GL_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeU] : GL_CLAMP_TO_EDGE);
   glTexParameteri(mBinding, GL_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeV] : GL_CLAMP_TO_EDGE);
}

