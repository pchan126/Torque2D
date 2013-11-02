//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30TextureObject_H
#define _GFXOpenGLES30TextureObject_H

#include "platform/platformGL.h"
#include "graphics/OpenGL/gfxOpenGLTextureObject.h"

class GFXOpenGLES30Device;

class GFXOpenGLES30TextureObject : public GFXOpenGLTextureObject
{
public:
   GFXOpenGLES30TextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGLES30TextureObject();
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData();

   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXOpenGLES30TextureManager to resurrect the texture.
      
private:
   friend class GFXOpenGLES30TextureManager;
   typedef GFXOpenGLTextureObject Parent;
   
protected:
    bool mClamp;

   /// Pointer to owner device
   GFXOpenGLES30Device* mGLDevice;
};

#endif