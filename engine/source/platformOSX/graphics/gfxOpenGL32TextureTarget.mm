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
#include "./gfxOpenGL32Device.h"
#include "./GFXOpenGL32TextureTarget.h"
#include "./GFXOpenGL32TextureObject.h"
#include "./gfxOpenGL32Cubemap.h"
#include "graphics/gfxTextureManager.h"

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard 
/// 2D/Rect textures through the same interface

/// Internal struct used to track Cubemap texture information for FBO attachment
class _GFXOpenGL32CubemapTargetDesc : public _GFXOpenGLTargetDesc
{
public:
   _GFXOpenGL32CubemapTargetDesc(GFXOpenGL32Cubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset)
      : _GFXOpenGLTargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
   {
   }
   
   virtual ~_GFXOpenGL32CubemapTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return 0; }
   virtual bool hasMips() { return mTex->getNumMipLevels() != 1; }
   virtual GLenum getBinding() { return GFXOpenGL32Cubemap::getEnumForFaceNumber(mFace); }
   
private:
   StrongRefPtr<GFXOpenGL32Cubemap> mTex;
   U32 mFace;
};


// Handy macro for checking the status of a framebuffer.  Framebuffers can fail in
// all sorts of interesting ways, these are just the most common.  Further, no existing GL profiling 
// tool catches framebuffer errors when the framebuffer is created, so we actually need this.
#define CHECK_FRAMEBUFFER_STATUS()\
{\
GLenum status;\
status = glCheckFramebufferStatus(GL_FRAMEBUFFER);\
switch(status) {\
case GL_FRAMEBUFFER_COMPLETE:\
break;\
case GL_FRAMEBUFFER_UNSUPPORTED:\
AssertFatal(false, "Unsupported FBO");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:\
AssertFatal(false, "Incomplete FBO Attachment");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:\
AssertFatal(false, "Incomplete FBO dimensions");\
break;\
case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:\
AssertFatal(false, "Incomplete FBO formats");\
default:\
/* programming error; will fail on all hardware */\
AssertFatal(false, "Something really bad happened with an FBO");\
}\
}


// Actual GFXOpenGL32TextureTarget interface
GFXOpenGL32TextureTarget::GFXOpenGL32TextureTarget()
{
}

GFXOpenGL32TextureTarget::~GFXOpenGL32TextureTarget()
{
}


void GFXOpenGL32TextureTarget::attachTexture( GFXCubemap *tex, U32 face, RenderSlot slot, U32 mipLevel/*=0*/ )
{
   // No depth cubemaps, sorry
   AssertFatal(slot != DepthStencil, "GFXOpenGL32TextureTarget::attachTexture (cube) - Cube depth textures not supported!");
   if(slot == DepthStencil)
      return;
    
   // Triggers an update when we next render
   invalidateState();
   
   // We stash the texture and info into an internal struct.
   GFXOpenGL32Cubemap* glTexture = static_cast<GFXOpenGL32Cubemap*>(tex);
   if(tex)
      mTargets[slot] = std::unique_ptr<_GFXOpenGLTargetDesc>(new _GFXOpenGL32CubemapTargetDesc(glTexture, face, mipLevel, 0));
   else
      mTargets[slot] = nullptr;
}


void GFXOpenGL32TextureTarget::makeActive()
{
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
}

void GFXOpenGL32TextureTarget::deactivate()
{
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GFXOpenGL32TextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   // REMINDER: When we implement MRT support, check against GFXGLDevice::getNumRenderTargets()
    makeActive();

   _GFXOpenGLTargetDesc * color0 = getTargetDesc(GFXTextureTarget::Color0);
   if(color0)
   {
      if(color0->getDepth() == 0)
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0->getBinding(), color0->getHandle(), color0->getMipLevel());
      else
         glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0->getBinding(), color0->getHandle(), color0->getMipLevel(), color0->getZOffset());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
   _GFXOpenGLTargetDesc * depthStencil = getTargetDesc(GFXTextureTarget::DepthStencil);
   if(depthStencil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(color0, "GFXOpenGL32TextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
      AssertFatal(depthStencil->getWidth() == color0->getWidth(), "GFXOpenGL32TextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same width!");
      AssertFatal(depthStencil->getHeight() == color0->getHeight(), "GFXOpenGL32TextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same height!");
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthStencil->getBinding(), depthStencil->getHandle(), depthStencil->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
   }
}



