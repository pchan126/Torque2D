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

#include "graphics/OpenGL/gfxOpenGLDevice.h"
#include "graphics/OpenGL/gfxOpenGLTextureObject.h"
#include "graphics/OpenGL/gfxOpenGLEnumTranslate.h"
#include "graphics/OpenGL/gfxOpenGLUtils.h"
#include "graphics/OpenGL/gfxOpenGLCubemap.h"
#include "graphics/gfxTextureManager.h"
#include "graphics/gfxCardProfile.h"


GLenum GFXOpenGLCubemap::faceList[6] = 
{ 
   GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

GFXOpenGLCubemap::GFXOpenGLCubemap() :
      mCubemap(0), 
      mDynamicTexSize(0),
      mFaceFormat( GFXFormatR8G8B8A8 )
{
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXOpenGLCubemap::_onTextureEvent );
}

GFXOpenGLCubemap::~GFXOpenGLCubemap()
{
   glDeleteTextures(1, &mCubemap);
   GFXTextureManager::removeEventDelegate( this, &GFXOpenGLCubemap::_onTextureEvent );
}


void GFXOpenGLCubemap::initStatic(GFXTexHandle* faces)
{
   if(mCubemap)
      return;
      
   if(faces)
   {
      AssertFatal(faces[0], "GFXOpenGLCubemap::initStatic - empty texture passed");
      glGenTextures(1, &mCubemap);
      fillCubeTextures(faces);
   }
}

void GFXOpenGLCubemap::zombify()
{
   glDeleteTextures(1, &mCubemap);
   mCubemap = 0;
}

void GFXOpenGLCubemap::resurrect()
{
   // Handled in tmResurrect
}

void GFXOpenGLCubemap::tmResurrect()
{
   if(mDynamicTexSize)
      initDynamic(mDynamicTexSize,mFaceFormat);
   else
   {
      initStatic( mTextures );
   }
}

void GFXOpenGLCubemap::setToTexUnit(U32 tuNum)
{
   static_cast<GFXOpenGLDevice*>(getOwningDevice())->setCubemapInternal(tuNum, this);
}

void GFXOpenGLCubemap::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      zombify();
   else
      tmResurrect();
}
