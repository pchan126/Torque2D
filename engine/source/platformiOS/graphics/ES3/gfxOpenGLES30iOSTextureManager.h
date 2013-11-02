//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30iOSTextureManager_H
#define _GFXOpenGLES30iOSTextureManager_H

#import <OpenGLES/ES3/glext.h>

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "./gfxOpenGLES30iOSTextureObject.h"
#include "graphics/OpenGL/ES30/gfxOpenGLES30TextureManager.h"

class GFXOpenGLES30iOSTextureManager : public GFXOpenGLES30TextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLES30iOSTextureManager();
   ~GFXOpenGLES30iOSTextureManager();
   
protected:

   // GFXTextureManager
    virtual GFXTextureObject *createTexture(  const String &fullPath,
                                            GFXTextureProfile *profile );

    virtual GFXTextureObject* createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp );

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
                                             GFXTextureObject *inTex = nullptr,
                                             void* data = nullptr);
    
   bool _loadTexture(GFXTextureObject *texture, GBitmap *bmp);
   bool _loadTexture(GFXTextureObject *texture, void *raw);
   bool _refreshTexture(GFXTextureObject *texture);
   bool _freeTexture(GFXTextureObject *texture, bool zombify = false);

private:
   friend class GFXOpenGLES30iOSTextureObject;
   
    void handleTextureCallback(void *textureInfo);
};

#endif