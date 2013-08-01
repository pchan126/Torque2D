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

#ifndef _GFXOpenGLTextureTARGET_H_
#define _GFXOpenGLTextureTARGET_H_

#include "graphics/gfxTarget.h"
#include "memory/autoPtr.h"

class GFXOpenGLTextureObject;

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard
/// 2D/Rect textures through the same interface
class _GFXOpenGLTargetDesc
{
public:
    _GFXOpenGLTargetDesc(U32 _mipLevel, U32 _zOffset) :
    mipLevel(_mipLevel), zOffset(_zOffset)
    {
    }

    virtual ~_GFXOpenGLTargetDesc() {}

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

/// Render to texture support for OpenGL.
/// This class needs to make a number of assumptions due to the requirements
/// and complexity of render to texture in OpenGL.
/// 1) This class is only guaranteed to work with 2D textures or cubemaps.  3D textures
/// may or may not work.
/// 2) This class does not currently support multiple texture targets.  Regardless
/// of how many targets you bind, only Color0 will be used.
/// 3) This class requires that the DepthStencil and Color0 targets have identical
/// dimensions.
/// 4) If the DepthStencil target is GFXTextureTarget::sDefaultStencil, then the
/// Color0 target should be the same size as the current backbuffer and should also
/// be the same format (typically R8G8B8A8)
class GFXOpenGLTextureTarget : public GFXTextureTarget
{
public:
   friend GFXOpenGLDevice;
   GFXOpenGLTextureTarget();
   virtual ~GFXOpenGLTextureTarget();

   virtual const Point2I getSize();
   virtual GFXFormat getFormat();
    virtual void attachTexture(GFXTextureObject *tex, RenderSlot slot = Color0, U32 mipLevel=0, U32 zOffset = 0);
//   virtual void attachTexture(GFXCubemap *tex, U32 face, RenderSlot slot = Color0, U32 mipLevel=0);
   virtual void clearAttachments();

   /// Functions to query internal state
   /// @{

   /// Returns the internal structure for the given slot.  This should only be called by our internal implementations.
   _GFXOpenGLTargetDesc * getTargetDesc(RenderSlot slot) const;

   /// @}
   
   virtual void deactivate() = 0;
   void zombify();
   void resurrect();
   virtual const String describeSelf() const;
   
protected:

   friend class GFXOpenGLDevice;

   GLuint mFramebuffer;

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   virtual void _onTextureEvent( GFXTexCallbackCode code );
   
   /// Array of _GFXGLTargetDesc's, an internal struct used to keep track of texture data.
   AutoPtr<_GFXOpenGLTargetDesc> mTargets[MaxRenderSlotId];

   /// These redirect to our internal implementation
   /// @{
   
   virtual void applyState() = 0;
   virtual void makeActive() = 0;
   
   /// @}

};

#endif
