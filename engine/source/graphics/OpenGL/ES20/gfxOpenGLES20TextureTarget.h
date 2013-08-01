//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20TextureTarget_H_
#define _GFXOpenGLES20TextureTarget_H_

#include "graphics/OpenGL/gfxOpenGLTextureTarget.h"
#include "memory/autoPtr.h"

class GFXOpenGLES20TextureObject;

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
   
    void deactivate();
    virtual const String describeSelf() const;
    
    virtual void resolve();
    
    virtual void resolveTo(GFXTextureObject* obj);

    void applyState();

    void makeActive();

protected:
    
    friend class GFXOpenGLES20Device;
};

#endif
