//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30iOSTextureObject_H
#define _GFXOpenGLES30iOSTextureObject_H

#include "graphics/OpenGL/ES30/gfxOpenGLES30TextureObject.h"
#import <OpenGLES/ES3/glext.h>

class GFXOpenGLES30iOSDevice;
@class GLKTextureInfo;
@class CIImage;

class GFXOpenGLES30iOSTextureObject : public GFXOpenGLES30TextureObject
{
public:
   GFXOpenGLES30iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* texInfo);
   GFXOpenGLES30iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGLES30iOSTextureObject();
   
   inline bool isZombie() const { return mIsZombie; }
   CIImage* getCIImage();
    ///< Reloads texture from zombie cache, used by GFXOpenGLES30iOSTextureManager to resurrect the texture.
   
private:
   friend class GFXOpenGLES30iOSTextureManager;
   typedef GFXOpenGLES30TextureObject Parent;
   /// Internal GL object
    bool mClamp;
   
   U32 mBytesPerTexel;
   GFXLockedRect mLockedRect;
   RectI mLockedRectRect;

   /// Pointer to owner device
   GFXOpenGLES30iOSDevice* mGLDevice;
   
   bool mIsZombie;
   U8* mZombieCache;
};

#endif