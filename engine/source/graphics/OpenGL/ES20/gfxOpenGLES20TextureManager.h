//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20TextureManager_H
#define _GFXOpenGLES20TextureManager_H

#include "platform/platformGL.h"
#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "gfxOpenGLES20TextureObject.h"

class GFXOpenGLES20TextureManager : public GFXTextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLES20TextureManager();
   ~GFXOpenGLES20TextureManager();
   
protected:

   // GFXTextureManager
//    virtual GFXTextureObject* createTexture(  const String &path,
//                                            GFXTextureProfile *profile ) { };

    virtual std::shared_ptr<GFXTextureObject> createTexture(GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp);

    virtual std::shared_ptr<GFXTextureObject> createTexture(U32 width, U32 height, void *pixels, GFXFormat format, GFXTextureProfile *profile);
    
    shared_ptr<GFXTextureObject> _createTexture(GBitmap *bmp,
            const String &resourceName,
            GFXTextureProfile *profile,
            bool deleteBmp,
            GFXTextureObject *inObj);
    
   GFXTextureObject *_createTextureObject(   U32 height,
                                             U32 width,
                                             U32 depth,
                                             GFXFormat format,
                                             GFXTextureProfile *profile,
                                             U32 numMipLevels,
                                             bool forceMips = false,
                                             S32 antialiasLevel = 0,
                                             GFXTextureObject *inTex = NULL );
    
   bool _loadTexture(GFXTexHandle &texture, GBitmap *bmp);
   bool _loadTexture(GFXTexHandle &texture, void *raw);
   bool _refreshTexture(GFXTexHandle &texture);
   bool _freeTexture(GFXTexHandle &texture, bool zombify = false);

   /// Creates internal GL texture
   void innerCreateTexture(GFXOpenGLES20TextureObject *obj,
           U32 height, U32 width, U32 depth, GFXFormat format,
           GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false,
           void* data = nullptr);
   
private:
   friend class GFXOpenGLES20TextureObject;
   
    void handleTextureCallback(void *textureInfo);
};

#endif