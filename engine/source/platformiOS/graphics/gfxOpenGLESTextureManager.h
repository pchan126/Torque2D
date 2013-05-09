//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESTextureManager_H
#define _GFXOpenGLESTextureManager_H

#import <OpenGLES/ES2/glext.h>

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLESTextureObject.h"

class GFXOpenGLESTextureManager : public GFXTextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLESTextureManager();
   ~GFXOpenGLESTextureManager();
   
protected:

   // GFXTextureManager
    virtual GFXTextureObject* createTexture(  const String &path,
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
                                             GFXTextureObject *inTex = NULL );
    
   bool _loadTexture(GFXTextureObject *texture, GBitmap *bmp);
   bool _loadTexture(GFXTextureObject *texture, void *raw);
   bool _refreshTexture(GFXTextureObject *texture);
   bool _freeTexture(GFXTextureObject *texture, bool zombify = false);

private:
   friend class GFXOpenGLESTextureObject;
   
   /// Creates internal GL texture
   void innerCreateTexture(GFXOpenGLESTextureObject *obj, U32 height, U32 width, U32 depth, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false);
    
    void handleTextureCallback(void *textureInfo);
};

#endif