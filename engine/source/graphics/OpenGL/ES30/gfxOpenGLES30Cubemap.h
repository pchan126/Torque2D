//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30Cubemap_H_
#define _GFXOpenGLES30Cubemap_H_

#include "graphics/OpenGL/gfxOpenGLCubemap.h"

class GFXOpenGLES30Cubemap : public GFXOpenGLCubemap
{
public:
   GFXOpenGLES30Cubemap();
   virtual ~GFXOpenGLES30Cubemap();

   virtual void initDynamic( U32 texSize, GFXFormat faceFormat = GFXFormatR8G8B8A8 );

protected:
   // should only be called by GFXDevice
   virtual void bind(U32 textureUnit); ///< Notifies our owning device that we want to be set to the given texture unit (used for GL internal state tracking)
   virtual void fillCubeTextures(GFXTexHandle* faces); ///< Copies the textures in faces into the cubemap
};

#endif
