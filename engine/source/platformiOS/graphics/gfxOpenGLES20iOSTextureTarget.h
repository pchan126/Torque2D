//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSTextureTarget_H_
#define _GFXOpenGLES20iOSTextureTarget_H_

#include "graphics/OpenGL/ES20/gfxOpenGLES20TextureTarget.h"
#include "memory/autoPtr.h"

class GFXOpenGLES20iOSTextureObject;
class _GFXOpenGLES20iOSTextureTargetImpl;

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard
/// 2D/Rect textures through the same interface
class _GFXOpenGLES20iOSTargetDesc
{
public:
    _GFXOpenGLES20iOSTargetDesc(U32 _mipLevel, U32 _zOffset) :
    mipLevel(_mipLevel), zOffset(_zOffset)
    {
    }
    
    virtual ~_GFXOpenGLES20iOSTargetDesc() {}
    
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
class GFXOpenGLES20iOSTextureTarget : public GFXOpenGLES20TextureTarget
{
public:
    GFXOpenGLES20iOSTextureTarget();
    virtual ~GFXOpenGLES20iOSTextureTarget();
    
    virtual const Point2I getSize();
    virtual void attachTexture( GFXTextureObject *tex, RenderSlot slot = Color0, U32 mipLevel=0, U32 zOffset = 0);
    virtual void attachTexture(GFXCubemap *tex, U32 face, RenderSlot slot, U32 mipLevel=0);
    virtual void clearAttachments();
    
    /// Functions to query internal state
    /// @{
    
    /// Returns the internal structure for the given slot.  This should only be called by our internal implementations.
    _GFXOpenGLES20iOSTargetDesc* getTargetDesc(RenderSlot slot) const;
    
    /// @}
    
    void deactivate();
    void zombify();
    void resurrect();
    virtual const String describeSelf() const;
    
    virtual void resolve();
    
    virtual void resolveTo(GFXTextureObject* obj);
    
protected:
    
    friend class GFXOpenGLES20iOSDevice;
    
    /// The callback used to get texture events.
    /// @see GFXTextureManager::addEventDelegate
    void _onTextureEvent( GFXTexCallbackCode code );
    
    /// If true our implementation should use AUX buffers
    bool _needsAux;
    
    /// Pointer to our internal implementation
    AutoPtr<_GFXOpenGLES20iOSTextureTargetImpl> _impl;
    
    /// Array of _GFXOpenGLES20iOSTargetDesc's, an internal struct used to keep track of texture data.
    AutoPtr<_GFXOpenGLES20iOSTargetDesc> mTargets[MaxRenderSlotId];
    
    /// These redirect to our internal implementation
    /// @{
    
    void applyState();
    void makeActive();
    
    /// @}
    
};


// Internal implementations
class _GFXOpenGLES20iOSTextureTargetImpl
{
public:
    GFXOpenGLES20iOSTextureTarget* mTarget;
    
    virtual ~_GFXOpenGLES20iOSTextureTargetImpl() {}
    
    virtual void applyState() = 0;
    virtual void makeActive() = 0;
    virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXOpenGLES20iOSTextureTargetFBOImpl : public _GFXOpenGLES20iOSTextureTargetImpl
{
public:
    GLuint mFramebuffer, mRenderBuffer;
    
    _GFXOpenGLES20iOSTextureTargetFBOImpl(GFXOpenGLES20iOSTextureTarget* target);
    virtual ~_GFXOpenGLES20iOSTextureTargetFBOImpl();
    
    virtual void applyState();
    virtual void makeActive();
    virtual void finish();
};


#endif
