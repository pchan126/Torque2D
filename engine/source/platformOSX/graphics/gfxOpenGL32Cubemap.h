//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGL32Cubemap_H_
#define _GFXOpenGL32Cubemap_H_

#include "graphics/OpenGL/gfxOpenGLCubemap.h"

class GFXOpenGL32Cubemap : public GFXOpenGLCubemap
{
public:
   GFXOpenGL32Cubemap();
   virtual ~GFXOpenGL32Cubemap();

   virtual void initDynamic( U32 texSize, GFXFormat faceFormat = GFXFormatR8G8B8A8 );

protected:
   // should only be called by GFXDevice
   virtual void bind(U32 textureUnit); ///< Notifies our owning device that we want to be set to the given texture unit (used for GL internal state tracking)
   virtual void fillCubeTextures(GFXTexHandle* faces); ///< Copies the textures in faces into the cubemap
};

#endif
