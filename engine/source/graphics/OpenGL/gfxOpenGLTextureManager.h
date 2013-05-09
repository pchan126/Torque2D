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

#ifndef _GFXOpenGLTextureManager_H
#define _GFXOpenGLTextureManager_H

#include "graphics/gfxDevice.h"
#include "graphics/gfxTextureManager.h"
#include "./gfxOpenGLTextureObject.h"

class GFXOpenGLTextureManager : public GFXTextureManager
{   
public:
    typedef GFXTextureManager Parent;
    
   GFXOpenGLTextureManager();
   ~GFXOpenGLTextureManager();
   
protected:

   // GFXTextureManager
//    virtual GFXTextureObject *createTexture(  GBitmap *bmp,
//                                            const String &resourceName,
//                                            GFXTextureProfile *profile,
//                                            bool deleteBmp);
    
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
//    
//    virtual GFXTextureObject *createTexture(  U32 width,
//                                            U32 height,
//                                            GFXFormat format,
//                                            GFXTextureProfile *profile,
//                                            U32 numMipLevels,
//                                            S32 antialiasLevel);
    
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
   friend class GFXOpenGLTextureObject;
   
   /// Creates internal GL texture
   void innerCreateTexture(GFXOpenGLTextureObject *obj, U32 height, U32 width, U32 depth, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false);

   void handleTextureCallback(void *textureInfo);
    // t2d texture mananger
public:
    bool mDGLRender;
    GLenum mTextureCompressionHint;
};

#endif