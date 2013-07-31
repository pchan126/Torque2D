//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "./GFXOpenGLES20Device.h"
#include "./GFXOpenGLES20TextureTarget.h"
#include "./GFXOpenGLES20TextureObject.h"
#include "./GFXOpenGLES20Cubemap.h"
#include "graphics/gfxTextureManager.h"
#include "./GFXOpenGLES20Utils.h"

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXOpenGLES20TextureTargetDesc : public _GFXOpenGLES20TargetDesc
{
public:
   _GFXOpenGLES20TextureTargetDesc(GFXOpenGLES20TextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXOpenGLES20TargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXOpenGLES20TextureTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual GLenum getBinding() { return mTex->getBinding(); }
   
private:
   StrongRefPtr<GFXOpenGLES20TextureObject> mTex;
};

/// Internal struct used to track Cubemap texture information for FBO attachment
class _GFXOpenGLESCubemapTargetDesc : public _GFXOpenGLES20TargetDesc
{
public:
   _GFXOpenGLESCubemapTargetDesc(GFXOpenGLES20Cubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset) 
      : _GFXOpenGLES20TargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
   {
   }
   
   virtual ~_GFXOpenGLESCubemapTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return 0; }
   virtual bool hasMips() { return mTex->getNumMipLevels() != 1; }
   virtual GLenum getBinding() { return GFXOpenGLES20Cubemap::getEnumForFaceNumber(mFace); }
   
private:
   StrongRefPtr<GFXOpenGLES20Cubemap> mTex;
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


// Actual GFXOpenGLES20TextureTarget interface
GFXOpenGLES20TextureTarget::GFXOpenGLES20TextureTarget()
{
}

GFXOpenGLES20TextureTarget::~GFXOpenGLES20TextureTarget()
{
}

void GFXOpenGLES20TextureTarget::attachTexture( GFXTextureObject *tex, RenderSlot slot, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXOpenGLES20TextureObject* glTexture = static_cast<GFXOpenGLES20TextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXOpenGLES20TextureTargetDesc(glTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

void GFXOpenGLES20TextureTarget::attachTexture( GFXCubemap *tex, U32 face, RenderSlot slot, U32 mipLevel/*=0*/ )
{
   // No depth cubemaps, sorry
   AssertFatal(slot != DepthStencil, "GFXOpenGLES20TextureTarget::attachTexture (cube) - Cube depth textures not supported!");
   if(slot == DepthStencil)
      return;
    
   // Triggers an update when we next render
   invalidateState();
   
//   // We stash the texture and info into an internal struct.
//   GFXOpenGLES20Cubemap* glTexture = static_cast<GFXOpenGLES20Cubemap*>(tex);
//    if(tex)
//      mTargets[slot] = new _GFXOpenGLESCubemapTargetDesc(glTexture, face, mipLevel, 0);
//   else
      mTargets[slot] = NULL;
}


void GFXOpenGLES20TextureTarget::makeActive()
{
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   _GFXOpenGLES20TargetDesc* color0 = dynamic_cast<_GFXOpenGLES20TargetDesc*>(getTargetDesc(GFXTextureTarget::Color0));
   if(!color0) // || !(color0->hasMips()))
      return;
   
   // Generate mips if necessary
   // Assumes a 2D texture.
   glActiveTexture(GL_TEXTURE0);
//   PRESERVE_2D_TEXTURE();
   glBindTexture(GL_TEXTURE_2D, color0->getHandle());
//   glGenerateMipmap(GL_TEXTURE_2D);
}

void GFXOpenGLES20TextureTarget::deactivate()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GFXOpenGLES20TextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   
   _GFXOpenGLES20TargetDesc* color0 = dynamic_cast<_GFXOpenGLES20TargetDesc*>(getTargetDesc(GFXTextureTarget::Color0));
   if(color0)
   {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0->getBinding(), color0->getHandle(), color0->getMipLevel());
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
   }
   
   _GFXOpenGLES20TargetDesc* depthStencil = dynamic_cast<_GFXOpenGLES20TargetDesc*>(getTargetDesc(GFXTextureTarget::DepthStencil));
   if(depthStencil)
   {
      // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
      AssertFatal(color0, "GFXGLTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
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


const String GFXOpenGLES20TextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXOpenGLES20TextureTarget::resolve()
{
}

void GFXOpenGLES20TextureTarget::resolveTo(GFXTextureObject* obj)
{
}
