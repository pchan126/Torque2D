//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESTextureTarget_H_
#define _GFXOpenGLESTextureTarget_H_

#include "graphics/gfxTarget.h"
#include "memory/autoPtr.h"

class GFXOpenGLESTextureObject;
class _GFXOpenGLESTextureTargetImpl;

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard
/// 2D/Rect textures through the same interface
class _GFXOpenGLESTargetDesc
{
public:
    _GFXOpenGLESTargetDesc(U32 _mipLevel, U32 _zOffset) :
    mipLevel(_mipLevel), zOffset(_zOffset)
    {
    }
    
    virtual ~_GFXOpenGLESTargetDesc() {}
    
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
class GFXOpenGLESTextureTarget : public GFXTextureTarget
{
public:
    GFXOpenGLESTextureTarget();
    virtual ~GFXOpenGLESTextureTarget();
    
    virtual const Point2I getSize();
    virtual GFXFormat getFormat();
    virtual void attachTexture(RenderSlot slot, GFXTextureObject *tex, U32 mipLevel=0, U32 zOffset = 0);
    virtual void attachTexture(RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel=0);
    virtual void clearAttachments();
    
    /// Functions to query internal state
    /// @{
    
    /// Returns the internal structure for the given slot.  This should only be called by our internal implementations.
    _GFXOpenGLESTargetDesc* getTargetDesc(RenderSlot slot) const;
    
    /// @}
    
    void deactivate();
    void zombify();
    void resurrect();
    virtual const String describeSelf() const;
    
    virtual void resolve();
    
    virtual void resolveTo(GFXTextureObject* obj);
    
protected:
    
    friend class GFXOpenGLESDevice;
    
    /// The callback used to get texture events.
    /// @see GFXTextureManager::addEventDelegate
    void _onTextureEvent( GFXTexCallbackCode code );
    
    /// If true our implementation should use AUX buffers
    bool _needsAux;
    
    /// Pointer to our internal implementation
    AutoPtr<_GFXOpenGLESTextureTargetImpl> _impl;
    
    /// Array of _GFXOpenGLESTargetDesc's, an internal struct used to keep track of texture data.
    AutoPtr<_GFXOpenGLESTargetDesc> mTargets[MaxRenderSlotId];
    
    /// These redirect to our internal implementation
    /// @{
    
    void applyState();
    void makeActive();
    
    /// @}
    
};


// Internal implementations
class _GFXOpenGLESTextureTargetImpl
{
public:
    GFXOpenGLESTextureTarget* mTarget;
    
    virtual ~_GFXOpenGLESTextureTargetImpl() {}
    
    virtual void applyState() = 0;
    virtual void makeActive() = 0;
    virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXOpenGLESTextureTargetFBOImpl : public _GFXOpenGLESTextureTargetImpl
{
public:
    GLuint mFramebuffer, mRenderBuffer;
    
    _GFXOpenGLESTextureTargetFBOImpl(GFXOpenGLESTextureTarget* target);
    virtual ~_GFXOpenGLESTextureTargetFBOImpl();
    
    virtual void applyState();
    virtual void makeActive();
    virtual void finish();
};


#endif
