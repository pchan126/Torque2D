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

#ifndef _GFXOpenGL33WinTextureObject_H
#define _GFXOpenGL33WinTextureObject_H

#include "graphics/OpenGL/gfxOpenGLTextureObject.h"
#include "platform/platformGL.h"

class GFXOpenGL33WinDevice;

class GFXOpenGL33WinTextureObject : public GFXOpenGLTextureObject
{
    friend class GFXOpenGLTextureManager;
public:
   GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, void* texInfo);
   GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile);
   virtual ~GFXOpenGL33WinTextureObject();
   
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData();
       
   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXOpenGLTextureManager to resurrect the texture.
   
   
   bool mIsNPoT2;
    inline GLuint getFilter( void ) { return mFilter; }
    virtual void setFilter( const GFXTextureFilterType filter );

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
   friend class GFXOpenGL33WinTextureManager;
   typedef GFXTextureObject Parent;
   /// Internal GL object
    GLuint mFilter;
    bool mClamp;
   
   /// Pointer to owner device
   GFXOpenGL33WinDevice* mGLDevice;
   
   void copyIntoCache();
};

#endif