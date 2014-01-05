//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "gfxOpenGLES30Device.h"
#include "gfxOpenGLES30TextureTarget.h"
#include "gfxOpenGLES30TextureObject.h"
#include "gfxOpenGLES30Cubemap.h"
#include "graphics/gfxTextureManager.h"
#include "gfxOpenGLES30Utils.h"

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


// Actual GFXOpenGLES30TextureTarget interface
GFXOpenGLES30TextureTarget::GFXOpenGLES30TextureTarget()
{
}

GFXOpenGLES30TextureTarget::~GFXOpenGLES30TextureTarget()
{
}

void GFXOpenGLES30TextureTarget::makeActive()
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   _GFXOpenGLTargetDesc* color0 = dynamic_cast<_GFXOpenGLTargetDesc*>(getTargetDesc(GFXTextureTarget::Color0));
   if(!color0) // || !(color0->hasMips()))
      return;
   
   device->setTextureUnit(0);
   glBindTexture(GL_TEXTURE_2D, color0->getHandle());
}

void GFXOpenGLES30TextureTarget::deactivate()
{
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GFXOpenGLES30TextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   
   _GFXOpenGLTargetDesc* color0 = dynamic_cast<_GFXOpenGLTargetDesc*>(getTargetDesc(GFXTextureTarget::Color0));
   if(color0)
   {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0->getBinding(), color0->getHandle(), color0->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
   }
   
   _GFXOpenGLTargetDesc* depthStencil = dynamic_cast<_GFXOpenGLTargetDesc*>(getTargetDesc(GFXTextureTarget::DepthStencil));
   if(depthStencil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(color0, "GFXGLTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
      assert(color0 != nullptr);

      AssertFatal(depthStencil->getWidth() == color0->getWidth(), "GFXGLTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same width!");
      AssertFatal(depthStencil->getHeight() == color0->getHeight(), "GFXGLTextureTarget::applyState() - DepthStencil and Color0 targets MUST have the same height!");
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthStencil->getBinding(), depthStencil->getHandle(), depthStencil->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
   }
   
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


const String GFXOpenGLES30TextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", (mTargets[Color0]) ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", (mTargets[DepthStencil]) ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXOpenGLES30TextureTarget::resolve()
{
}

void GFXOpenGLES30TextureTarget::resolveTo(GFXTextureObject* obj)
{
}
