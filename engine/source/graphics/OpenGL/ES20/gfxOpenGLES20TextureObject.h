//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20TextureObject_H
#define _GFXOpenGLES20TextureObject_H

#include "platform/platformGL.h"
#include "graphics/OpenGL/gfxOpenGLTextureObject.h"

class GFXOpenGLES20Device;

class GFXOpenGLES20TextureObject : public GFXOpenGLTextureObject
{
public:
   GFXOpenGLES20TextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGLES20TextureObject();
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData();

   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXOpenGLES20TextureManager to resurrect the texture.
      
private:
   friend class GFXOpenGLES20TextureManager;
   typedef GFXOpenGLTextureObject Parent;
   
protected:
    bool mClamp;

   /// Pointer to owner device
   GFXOpenGLES20Device* mGLDevice;
};

#endif