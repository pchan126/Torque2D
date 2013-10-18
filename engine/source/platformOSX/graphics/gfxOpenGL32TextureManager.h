//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGL32TextureManager_H
#define _GFXOpenGL32TextureManager_H

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "./gfxOpenGL32TextureObject.h"

class GFXOpenGL32TextureManager : public GFXTextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGL32TextureManager(NSOpenGLContext* mContext);
   ~GFXOpenGL32TextureManager();
   
protected:

   // GFXTextureManager
    virtual GFXTexHandle & createTexture(GBitmap *bmp,
           const String &resourceName,
           GFXTextureProfile *profile,
           bool deleteBmp);
   
//    virtual GFXTextureObject *createTexture(  const String &path,
//                                            GFXTextureProfile *profile );
    
//    virtual GFXTextureObject *createTexture(  U32 width,
//                                            U32 height,
//                                            void *pixels,
//                                            GFXFormat format,
//                                            GFXTextureProfile *profile);
//    
//    virtual GFXTextureObject *createTexture(  U32 width,
//                                            U32 height,
//                                            U32 depth,
//                                            void *pixels,
//                                            GFXFormat format,
//                                            GFXTextureProfile *profile );
    
//    virtual GFXTextureObject *createTexture(  U32 width,
//                                            U32 height,
//                                            GFXFormat format,
//                                            GFXTextureProfile *profile,
//                                            U32 numMipLevels,
//                                            S32 antialiasLevel);
   
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

   GFXTexHandle & _createTexture(GBitmap *bmp,
           const String &resourceName,
           GFXTextureProfile *profile,
           bool deleteBmp,
           GFXTexHandle inObj);

   bool _loadTexture(GFXTexHandle &texture, GBitmapPtr &bmp);
   bool _loadTexture(GFXTexHandle &texture, void *raw);
   bool _refreshTexture(GFXTexHandle &texture);
   bool _freeTexture(GFXTexHandle &texture, bool zombify = false);

private:
   friend class GFXOpenGLTextureObject;
   
   /// Creates internal GL texture
   void innerCreateTexture(GFXOpenGL32TextureObject *obj, U32 height, U32 width, U32 depth, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false, void* data = nullptr);

   void handleTextureCallback(void *textureInfo);
};

#endif