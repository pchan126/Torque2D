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
#include "./gfxOpenGL33WinDevice.h"
#include "./GFXOpenGL33WinTextureTarget.h"
#include "./GFXOpenGL33WinTextureObject.h"
//#include "./gfxGLCubemap.h"
#include "graphics/gfxTextureManager.h"
#include "./gfxOpenGL33WinUtils.h"

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard 
/// 2D/Rect textures through the same interface
class _GFXGLTargetDesc
{
public:
   _GFXGLTargetDesc(U32 _mipLevel, U32 _zOffset) :
      mipLevel(_mipLevel), zOffset(_zOffset)
   {
   }
   
   virtual ~_GFXGLTargetDesc() {}
   
   virtual U32 getHandle() = 0;
   virtual U32 getWidth() = 0;
   virtual U32 getHeight() = 0;
   virtual U32 getDepth() = 0;
   virtual bool hasMips() = 0;
   virtual GLenum getBinding() = 0;
   
   U32 getMipLevel() { return mipLevel; }
   U32 getZOffset() { return zOffset; }
   
private:
   U32 mipLevel;
   U32 zOffset;
};

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXOpenGL33WinTextureTargetDesc : public _GFXGLTargetDesc
{
public:
   _GFXOpenGL33WinTextureTargetDesc(GFXOpenGL33WinTextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXGLTargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXOpenGL33WinTextureTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual GLenum getBinding() { return mTex->getBinding(); }
   
private:
   StrongRefPtr<GFXOpenGL33WinTextureObject> mTex;
};

///// Internal struct used to track Cubemap texture information for FBO attachment
//class _GFXGLCubemapTargetDesc : public _GFXGLTargetDesc
//{
//public:
//   _GFXGLCubemapTargetDesc(GFXGLCubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset) 
//      : _GFXGLTargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
//   {
//   }
//   
//   virtual ~_GFXGLCubemapTargetDesc() {}
//   
//   virtual U32 getHandle() { return mTex->getHandle(); }
//   virtual U32 getWidth() { return mTex->getWidth(); }
//   virtual U32 getHeight() { return mTex->getHeight(); }
//   virtual U32 getDepth() { return 0; }
//   virtual bool hasMips() { return mTex->getNumMipLevels() != 1; }
//   virtual GLenum getBinding() { return GFXGLCubemap::getEnumForFaceNumber(mFace); }
//   
//private:
//   StrongRefPtr<GFXGLCubemap> mTex;
//   U32 mFace;
//};

// Internal implementations
class _GFXOpenGL33WinTextureTargetImpl
{
public:
   GFXOpenGL33WinTextureTarget* mTarget;
   
   virtual ~_GFXOpenGL33WinTextureTargetImpl() {}
   
   virtual void applyState() = 0;
   virtual void makeActive() = 0;
   virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXOpenGL33WinTextureTargetFBOImpl : public _GFXOpenGL33WinTextureTargetImpl
{
public:
   GLuint mFramebuffer;
   
   _GFXOpenGL33WinTextureTargetFBOImpl(GFXOpenGL33WinTextureTarget* target);
   virtual ~_GFXOpenGL33WinTextureTargetFBOImpl();
   
   virtual void applyState();
   virtual void makeActive();
   virtual void finish();
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

_GFXOpenGL33WinTextureTargetFBOImpl::_GFXOpenGL33WinTextureTargetFBOImpl(GFXOpenGL33WinTextureTarget* target)
{
   mTarget = target;
   glGenFramebuffers(1, &mFramebuffer);
}

_GFXOpenGL33WinTextureTargetFBOImpl::~_GFXOpenGL33WinTextureTargetFBOImpl()
{
   glDeleteFramebuffers(1, &mFramebuffer);
}

void _GFXOpenGL33WinTextureTargetFBOImpl::applyState()
{   
   // REMINDER: When we implement MRT support, check against GFXGLDevice::getNumRenderTargets()
   
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    Con::printf("_GFXOpenGL33WinTextureTargetFBOImpl::applyState:: glBindFramebuffer ");
   
   _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
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
   
   _GFXGLTargetDesc* depthStencil = mTarget->getTargetDesc(GFXTextureTarget::DepthStencil);
   if(depthStencil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(color0, "GFXOpenGL33WinTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
      AssertFatal(depthStencil->getWidth() == color0->getWidth(), "GFXOpenGL33WinTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same width!");
      AssertFatal(depthStencil->getHeight() == color0->getHeight(), "GFXOpenGL33WinTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same height!");
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthStencil->getBinding(), depthStencil->getHandle(), depthStencil->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
   }
   
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Con::printf("_GFXOpenGL33WinTextureTargetFBOImpl::applyState:: glBindFramebuffer(GL_FRAMEBUFFER, 0); ");
}

void _GFXOpenGL33WinTextureTargetFBOImpl::makeActive()
{
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
    Con::printf("_GFXOpenGL33WinTextureTargetFBOImpl::makeActive glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer); ");
}

void _GFXOpenGL33WinTextureTargetFBOImpl::finish()
{
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
   
    Con::printf("_GFXOpenGL33WinTextureTargetFBOImpl::finish glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer); ");

    _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
   if(!color0 || !(color0->hasMips()))
      return;
   
   // Generate mips if necessary
   // Assumes a 2D texture.
   glActiveTexture(GL_TEXTURE0);
//   PRESERVE_2D_TEXTURE();
   glBindTexture(GL_TEXTURE_2D, color0->getHandle());
   glGenerateMipmap(GL_TEXTURE_2D);
}


// Actual GFXOpenGL33WinTextureTarget interface
GFXOpenGL33WinTextureTarget::GFXOpenGL33WinTextureTarget()
{
   for(U32 i=0; i<MaxRenderSlotId; i++)
      mTargets[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXOpenGL33WinTextureTarget::_onTextureEvent );

   _impl = new _GFXOpenGL33WinTextureTargetFBOImpl(this);
}

GFXOpenGL33WinTextureTarget::~GFXOpenGL33WinTextureTarget()
{
   GFXTextureManager::removeEventDelegate( this, &GFXOpenGL33WinTextureTarget::_onTextureEvent );
}

const Point2I GFXOpenGL33WinTextureTarget::getSize()
{
   if(mTargets[Color0].isValid())
      return Point2I(mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight());

   return Point2I(0, 0);
}

GFXFormat GFXOpenGL33WinTextureTarget::getFormat()
{
   // TODO: Fix me!
   return GFXFormatR8G8B8A8;
}

void GFXOpenGL33WinTextureTarget::attachTexture( RenderSlot slot, GFXTextureObject *tex, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXOpenGL33WinTextureObject* glTexture = static_cast<GFXOpenGL33WinTextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXOpenGL33WinTextureTargetDesc(glTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

//void GFXOpenGL33WinTextureTarget::attachTexture( RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel/*=0*/ )
//{
//   // No depth cubemaps, sorry
//   AssertFatal(slot != DepthStencil, "GFXOpenGL33WinTextureTarget::attachTexture (cube) - Cube depth textures not supported!");
//   if(slot == DepthStencil)
//      return;
//    
//   // Triggers an update when we next render
//   invalidateState();
//   
//   // We stash the texture and info into an internal struct.
//   GFXGLCubemap* glTexture = static_cast<GFXGLCubemap*>(tex);
//   if(tex)
//      mTargets[slot] = new _GFXGLCubemapTargetDesc(glTexture, face, mipLevel, 0);
//   else
//      mTargets[slot] = NULL;
//}

void GFXOpenGL33WinTextureTarget::clearAttachments()
{
   deactivate();
   for(S32 i=1; i<MaxRenderSlotId; i++)
      attachTexture((RenderSlot)i, NULL);
}

void GFXOpenGL33WinTextureTarget::zombify()
{
   invalidateState();
   
   // Will be recreated in applyState
   _impl = NULL;
}

void GFXOpenGL33WinTextureTarget::resurrect()
{
   // Dealt with when the target is next bound
}

void GFXOpenGL33WinTextureTarget::makeActive()
{
   _impl->makeActive();
}

void GFXOpenGL33WinTextureTarget::deactivate()
{
   _impl->finish();
}

void GFXOpenGL33WinTextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   _impl = new _GFXOpenGL33WinTextureTargetFBOImpl(this);
    
   _impl->applyState();
}

_GFXGLTargetDesc* GFXOpenGL33WinTextureTarget::getTargetDesc(RenderSlot slot) const
{
   // This can only be called by our implementations, and then will not actually store the pointer so this is (almost) safe
   return mTargets[slot].ptr();
}

void GFXOpenGL33WinTextureTarget::_onTextureEvent( GFXTexCallbackCode code )
{
   invalidateState();
}

const String GFXOpenGL33WinTextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXOpenGL33WinTextureTarget::resolve()
{
}

void GFXOpenGL33WinTextureTarget::resolveTo(GFXTextureObject* obj)
{
   AssertFatal(dynamic_cast<GFXOpenGL33WinTextureObject*>(obj), "GFXOpenGL33WinTextureTarget::resolveTo - Incorrect type of texture, expected a GFXOpenGL33WinTextureObject");
   GFXOpenGL33WinTextureObject* glTexture = static_cast<GFXOpenGL33WinTextureObject*>(obj);

//   PRESERVE_FRAMEBUFFER();
   
   GLuint dest;
   GLuint src;
   
   glGenFramebuffers(1, &dest);
   glGenFramebuffers(1, &src);
   
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture->getHandle(), 0);
   
   glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
   glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,mTargets[Color0]->getHandle(), 0);
   
   glBlitFramebuffer(0, 0, mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(),
      0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
   
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
   
   glDeleteFramebuffers(1, &dest);
   glDeleteFramebuffers(1, &src);
}
