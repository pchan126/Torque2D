//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20TextureTarget_H_
#define _GFXOpenGLES20TextureTarget_H_

#include "graphics/OpenGL/gfxOpenGLTextureTarget.h"
#include "memory/autoPtr.h"

class GFXOpenGLES20TextureObject;

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard
/// 2D/Rect textures through the same interface
class _GFXOpenGLES20TargetDesc
{
public:
    _GFXOpenGLES20TargetDesc(U32 _mipLevel, U32 _zOffset) :
    mipLevel(_mipLevel), zOffset(_zOffset)
    {
    }
    
    virtual ~_GFXOpenGLES20TargetDesc() {}
    
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
class GFXOpenGLES20TextureTarget : public GFXOpenGLTextureTarget
{
public:
    GFXOpenGLES20TextureTarget();
    virtual ~GFXOpenGLES20TextureTarget();
    
    virtual void attachTexture(GFXTextureObject *tex, RenderSlot slot = Color0, U32 mipLevel=0, U32 zOffset = 0);
    virtual void attachTexture(GFXCubemap *tex, U32 face, RenderSlot slot, U32 mipLevel=0);

    void deactivate();
    virtual const String describeSelf() const;
    
    virtual void resolve();
    
    virtual void resolveTo(GFXTextureObject* obj);

    void applyState();

    void makeActive();

protected:
    
    friend class GFXOpenGLES20Device;
    
    /// Array of _GFXOpenGLES20TargetDesc's, an internal struct used to keep track of texture data.
    AutoPtr<_GFXOpenGLES20TargetDesc> mTargets[MaxRenderSlotId];
};

#endif
