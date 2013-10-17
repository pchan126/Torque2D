//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSTextureManager_H
#define _GFXOpenGLES20iOSTextureManager_H

#import <OpenGLES/ES2/glext.h>

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureObject.h"
#include "graphics/OpenGL/ES20/gfxOpenGLES20TextureManager.h"

class GFXOpenGLES20iOSTextureManager : public GFXOpenGLES20TextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLES20iOSTextureManager();
   ~GFXOpenGLES20iOSTextureManager();
   
protected:

   // GFXTextureManager
    virtual GFXTexHandle createTexture(const String &fullPath,
           GFXTextureProfile *profile);

    virtual GFXTexHandle createTexture(GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp);

   GFXTexHandle _createTexture(GBitmap *bmp,
           const String &resourceName,
           GFXTextureProfile *profile,
           bool deleteBmp,
           GFXTexHandle inObj);
    
   GFXTexHandle _createTextureObject(U32 height,
           U32 width,
           U32 depth,
           GFXFormat format,
           GFXTextureProfile *profile,
           U32 numMipLevels,
           bool forceMips = false,
           S32 antialiasLevel = 0,
           GFXTexHandle inTex = nullptr,
           void *data = nullptr);
    
   bool _loadTexture(GFXTexHandle &texture, GBitmap *bmp);
   bool _loadTexture(GFXTexHandle &texture, void *raw);
   bool _refreshTexture(GFXTexHandle &texture);
   bool _freeTexture(GFXTexHandle &texture, bool zombify = false);

private:
   friend class GFXOpenGLES20iOSTextureObject;
   
    void handleTextureCallback(void *textureInfo);
};

#endif