//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSTextureObject_H
#define _GFXOpenGLES20iOSTextureObject_H

#include "graphics/OpenGL/ES20/gfxOpenGLES20TextureObject.h"
#import <OpenGLES/ES2/glext.h>

class GFXOpenGLES20iOSDevice;
@class GLKTextureInfo;

class GFXOpenGLES20iOSTextureObject : public GFXOpenGLES20TextureObject
{
public:
   GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* texInfo);
   GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGLES20iOSTextureObject();
   
   inline bool isZombie() const { return mIsZombie; }

    ///< Reloads texture from zombie cache, used by GFXOpenGLES20iOSTextureManager to resurrect the texture.
   
   bool mIsNPoT2;

private:
   friend class GFXOpenGLES20iOSTextureManager;
   typedef GFXOpenGLES20TextureObject Parent;
   /// Internal GL object
    GLuint mFilter;
    bool mClamp;
   
   U32 mBytesPerTexel;
   GFXLockedRect mLockedRect;
   RectI mLockedRectRect;

   /// Pointer to owner device
   GFXOpenGLES20iOSDevice* mGLDevice;
   
   bool mIsZombie;
   U8* mZombieCache;
};

#endif