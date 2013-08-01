//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#ifndef _GFXOpenGLCubemap_H_
#define _GFXOpenGLCubemap_H_

#include "graphics/gfxCubemap.h"

class GFXOpenGLCubemap : public GFXCubemap
{
public:
   GFXOpenGLCubemap();
   virtual ~GFXOpenGLCubemap();

   virtual void initStatic( GFXTexHandle *faces );
   virtual void initDynamic( U32 texSize, GFXFormat faceFormat = GFXFormatR8G8B8A8 ) = 0;
   virtual U32 getSize() const { return mWidth; }
   virtual GFXFormat getFormat() const { return mFaceFormat; }

   // Convenience methods for GFXGLTextureTarget
   U32 getWidth() { return mWidth; }
   U32 getHeight() { return mHeight; }
   U32 getNumMipLevels() { return mMipLevels; }
   U32 getHandle() { return mCubemap; }
   
   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
   /// Called by texCB; this is to ensure that all textures have been resurrected before we attempt to res the cubemap.
   void tmResurrect();
   
   static GLenum getEnumForFaceNumber(U32 face) { return faceList[face]; } ///< Performs lookup to get a GLenum for the given face number

   void setParameter( GLenum pname, GLint param);
   GLint mMinFilter;
   GLint mMagFilter;
   GLint mWrapS;
   GLint mWrapT;

protected:

   friend class GFXDevice;
   friend class GFXOpenGLDevice;

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code );
   
   GLuint mBinding;
   GLuint mCubemap; ///< Internal GL handle
   U32 mDynamicTexSize; ///< Size of faces for a dynamic texture (used in resurrect)
   
   // Self explanatory
   U32 mWidth;
   U32 mHeight;
   U32 mMipLevels;
   GFXFormat mFaceFormat;
      
   GFXTexHandle mTextures[6]; ///< Keep refs to our textures for resurrection of static cubemaps
   
   // should only be called by GFXDevice
   virtual void setToTexUnit( U32 tuNum ); ///< Binds the cubemap to the given texture unit
   virtual void bind(U32 textureUnit) = 0; ///< Notifies our owning device that we want to be set to the given texture unit (used for GL internal state tracking)
   virtual void fillCubeTextures(GFXTexHandle* faces) = 0; ///< Copies the textures in faces into the cubemap
   
   static GLenum faceList[6]; ///< Lookup table
};

#endif
