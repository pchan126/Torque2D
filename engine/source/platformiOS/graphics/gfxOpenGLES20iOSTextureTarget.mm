//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/GFXOpenGLES20iOSTextureTarget.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureObject.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSCubemap.h"
#include "graphics/gfxTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSUtils.h"

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXOpenGLES20iOSTextureTargetDesc : public _GFXOpenGLES20iOSTargetDesc
{
public:
   _GFXOpenGLES20iOSTextureTargetDesc(GFXOpenGLES20iOSTextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXOpenGLES20iOSTargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXOpenGLES20iOSTextureTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual GLenum getBinding() { return mTex->getBinding(); }
   
private:
   StrongRefPtr<GFXOpenGLES20iOSTextureObject> mTex;
};

/// Internal struct used to track Cubemap texture information for FBO attachment
class _GFXOpenGLES20iOSCubemapTargetDesc : public _GFXOpenGLES20iOSTargetDesc
{
public:
   _GFXOpenGLES20iOSCubemapTargetDesc(GFXOpenGLES20iOSCubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset)
      : _GFXOpenGLES20iOSTargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
   {
   }
   
   virtual ~_GFXOpenGLES20iOSCubemapTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return 0; }
   virtual bool hasMips() { return mTex->getNumMipLevels() != 1; }
   virtual GLenum getBinding() { return GFXOpenGLES20iOSCubemap::getEnumForFaceNumber(mFace); }
   
private:
   StrongRefPtr<GFXOpenGLES20iOSCubemap> mTex;
   U32 mFace;
};


// Actual GFXOpenGLES20iOSTextureTarget interface
GFXOpenGLES20iOSTextureTarget::GFXOpenGLES20iOSTextureTarget()
{
   for(U32 i=0; i<MaxRenderSlotId; i++)
      mTargets[i] = NULL;
   
   glGenFramebuffers(1, &mFramebuffer);
}

GFXOpenGLES20iOSTextureTarget::~GFXOpenGLES20iOSTextureTarget()
{
   glDeleteFramebuffers(1, &mFramebuffer);
//   GFXTextureManager::removeEventDelegate( this, &GFXOpenGLES20iOSTextureTarget::_onTextureEvent );
}


void GFXOpenGLES20iOSTextureTarget::attachTexture( GFXTextureObject *tex, RenderSlot slot, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXOpenGLES20iOSTextureObject* glTexture = static_cast<GFXOpenGLES20iOSTextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXOpenGLES20iOSTextureTargetDesc(glTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

void GFXOpenGLES20iOSTextureTarget::attachTexture( GFXCubemap *tex, U32 face, RenderSlot slot, U32 mipLevel/*=0*/ )
{
   // No depth cubemaps, sorry
   AssertFatal(slot != DepthStencil, "GFXOpenGLES20iOSTextureTarget::attachTexture (cube) - Cube depth textures not supported!");
   if(slot == DepthStencil)
      return;
    
   // Triggers an update when we next render
   invalidateState();
   
//   // We stash the texture and info into an internal struct.
//   GFXOpenGLESCubemap* glTexture = static_cast<GFXOpenGLESCubemap*>(tex);
//    if(tex)
//      mTargets[slot] = new _GFXOpenGLESCubemapTargetDesc(glTexture, face, mipLevel, 0);
//   else
      mTargets[slot] = NULL;
}

void GFXOpenGLES20iOSTextureTarget::zombify()
{
   invalidateState();
}

void GFXOpenGLES20iOSTextureTarget::resurrect()
{
   // Dealt with when the target is next bound
}

void GFXOpenGLES20iOSTextureTarget::makeActive()
{
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   _GFXOpenGLES20iOSTargetDesc* color0 = getTargetDesc(GFXTextureTarget::Color0);
   if(!color0 )
      return;
   
   // Generate mips if necessary
   // Assumes a 2D texture.
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, color0->getHandle());
}

void GFXOpenGLES20iOSTextureTarget::deactivate()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GFXOpenGLES20iOSTextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
   
   _GFXOpenGLES20iOSTextureTargetDesc* color0 = dynamic_cast<_GFXOpenGLES20iOSTextureTargetDesc*>(getTargetDesc(GFXTextureTarget::Color0));
   if(color0)
   {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0->getBinding(), color0->getHandle(), 0);
   }
   else
   {
      // Clears the texture (note that the binding is irrelevent)
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
   }
   
   _GFXOpenGLES20iOSTargetDesc* depthStencil = getTargetDesc(GFXTextureTarget::DepthStencil);
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
}

_GFXOpenGLES20iOSTargetDesc* GFXOpenGLES20iOSTextureTarget::getTargetDesc(RenderSlot slot) const
{
   // This can only be called by our implementations, and then will not actually store the pointer so this is (almost) safe
   return mTargets[slot].ptr();
}

void GFXOpenGLES20iOSTextureTarget::_onTextureEvent( GFXTexCallbackCode code )
{
   invalidateState();
}

const String GFXOpenGLES20iOSTextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXOpenGLES20iOSTextureTarget::resolve()
{
}

void GFXOpenGLES20iOSTextureTarget::resolveTo(GFXTextureObject* obj)
{
}
