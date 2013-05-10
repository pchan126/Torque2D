//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSTextureObject_H
#define _GFXOpenGLES20iOSTextureObject_H

#include "graphics/gfxTextureObject.h"
#import <OpenGLES/ES2/glext.h>

class GFXOpenGLES20iOSDevice;
@class GLKTextureInfo;

class GFXOpenGLES20iOSTextureObject : public GFXTextureObject
{
public:
   GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* texInfo);
   GFXOpenGLES20iOSTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGLES20iOSTextureObject();
   
   void release();
   
   inline GLuint getHandle() const { return mHandle; }
   inline GLenum getBinding() const { return mBinding; }
   
   inline bool isZombie() const { return mIsZombie; }

   /// Binds the texture to the given texture unit
   /// and applies the current sampler state because GL tracks
   /// filtering and wrapper per object, while GFX tracks per sampler.
   void bind(U32 textureUnit) const;
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData();
    
   virtual GBitmap *getBitmap();

   virtual F32 getMaxUCoord() const;
   virtual F32 getMaxVCoord() const;
   
   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXOpenGLES20iOSTextureManager to resurrect the texture.
   
#ifdef TORQUE_DEBUG
   virtual void pureVirtualCrash() {}
#endif

   /// Get/set data from texture (for dynamic textures and render targets)
   /// @attention DO NOT READ FROM THE RETURNED RECT! It is not guaranteed to work and may incur significant performance penalties.
   virtual GFXLockedRect* lock(U32 mipLevel = 0, RectI *inRect = NULL);
   virtual void unlock(U32 mipLevel = 0 );

   virtual bool copyToBmp(GBitmap *); ///< Not implemented
   
   bool mIsNPoT2;
    inline GLuint getFilter( void ) { return mFilter; }
    virtual void setFilter( const GFXTextureFilterType filter );
    
    // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   virtual const String describeSelf() const;
   
private:
   friend class GFXOpenGLES20iOSTextureManager;
   typedef GFXTextureObject Parent;
   /// Internal GL object
   GLuint mHandle;
   GLenum mBinding;
    GLuint mFilter;
    bool mClamp;
   
   U32 mBytesPerTexel;
   GFXLockedRect mLockedRect;
   RectI mLockedRectRect;

   /// Pointer to owner device
   GFXOpenGLES20iOSDevice* mGLDevice;
   
   bool mIsZombie;
   U8* mZombieCache;
   
   void copyIntoCache();

};

#endif