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
#include "./gfxOpenGLDevice.h"
#include "gfxOpenGLTextureTarget.h"
#include "gfxOpenGLTextureObject.h"
//#include "./gfxGLCubemap.h"
#include "graphics/gfxTextureManager.h"
#include "./gfxOpenGLUtils.h"

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXOpenGLTextureTargetDesc : public _GFXOpenGLTargetDesc
{
public:
   _GFXOpenGLTextureTargetDesc(GFXOpenGLTextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXOpenGLTargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXOpenGLTextureTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual GLenum getBinding() { return mTex->getBinding(); }
   
private:
   StrongRefPtr<GFXOpenGLTextureObject> mTex;
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



// Actual GFXOpenGLTextureTarget interface
GFXOpenGLTextureTarget::GFXOpenGLTextureTarget()
{
   for(U32 i=0; i<MaxRenderSlotId; i++)
      mTargets[i] = NULL;
   
   glGenFramebuffers(1, &mFramebuffer);

   GFXTextureManager::addEventDelegate( this, &GFXOpenGLTextureTarget::_onTextureEvent );
}

GFXOpenGLTextureTarget::~GFXOpenGLTextureTarget()
{
   glDeleteFramebuffers(1, &mFramebuffer);

   GFXTextureManager::removeEventDelegate( this, &GFXOpenGLTextureTarget::_onTextureEvent );
}

const Point2I GFXOpenGLTextureTarget::getSize()
{
    return mTargets[Color0].isValid() ? Point2I(mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight()) : Point2I(0, 0);
}

GFXFormat GFXOpenGLTextureTarget::getFormat()
{
   // TODO: Fix me!
   return GFXFormatR8G8B8A8;
}

void GFXOpenGLTextureTarget::attachTexture( GFXTextureObject *tex, RenderSlot slot, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXOpenGLTextureObject* glTexture = static_cast<GFXOpenGLTextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXOpenGLTextureTargetDesc(glTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

//void GFXOpenGLTextureTarget::attachTexture( RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel/*=0*/ )
//{
//   // No depth cubemaps, sorry
//   AssertFatal(slot != DepthStencil, "GFXOpenGLTextureTarget::attachTexture (cube) - Cube depth textures not supported!");
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

void GFXOpenGLTextureTarget::clearAttachments()
{
   deactivate();
   for(S32 i=1; i<MaxRenderSlotId; i++)
      attachTexture(NULL, (RenderSlot)i);
}

void GFXOpenGLTextureTarget::zombify()
{
   invalidateState();
}

void GFXOpenGLTextureTarget::resurrect()
{
   // Dealt with when the target is next bound
}


_GFXOpenGLTargetDesc * GFXOpenGLTextureTarget::getTargetDesc(RenderSlot slot) const
{
   // This can only be called by our implementations, and then will not actually store the pointer so this is (almost) safe
   return mTargets[slot].ptr();
}

void GFXOpenGLTextureTarget::_onTextureEvent( GFXTexCallbackCode code )
{
   invalidateState();
}

const String GFXOpenGLTextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}


