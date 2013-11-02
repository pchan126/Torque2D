//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30TextureManager_H
#define _GFXOpenGLES30TextureManager_H

#include "platform/platformGL.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "gfxOpenGLES30TextureObject.h"

class GFXOpenGLES30TextureManager : public GFXTextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLES30TextureManager();
   ~GFXOpenGLES30TextureManager();
   
protected:

   // GFXTextureManager
//    virtual GFXTextureObject* createTexture(  const String &path,
//                                            GFXTextureProfile *profile ) { };

    virtual GFXTextureObject* createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp );

    virtual GFXTextureObject* createTexture(  U32 width, U32 height, void *pixels, GFXFormat format, GFXTextureProfile *profile );
    
    GFXTextureObject* _createTexture(  GBitmap *bmp,
                                      const String &resourceName,
                                      GFXTextureProfile *profile,
                                      bool deleteBmp,
                                      GFXTextureObject *inObj );
    
   GFXTextureObject *_createTextureObject(   U32 height,
                                             U32 width,
                                             U32 depth,
                                             GFXFormat format,
                                             GFXTextureProfile *profile,
                                             U32 numMipLevels,
                                             bool forceMips = false,
                                             S32 antialiasLevel = 0,
                                             GFXTextureObject *inTex = NULL );
    
   bool _loadTexture(GFXTextureObject *texture, GBitmap *bmp);
   bool _loadTexture(GFXTextureObject *texture, void *raw);
   bool _refreshTexture(GFXTextureObject *texture);
   bool _freeTexture(GFXTextureObject *texture, bool zombify = false);

   /// Creates internal GL texture
   void innerCreateTexture(GFXOpenGLES30TextureObject *obj,
           U32 height, U32 width, U32 depth, GFXFormat format,
           GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false,
           void* data = nullptr);
   
private:
   friend class GFXOpenGLES30TextureObject;
   
    void handleTextureCallback(void *textureInfo);
};

#endif