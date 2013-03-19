//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "gfx/gles/GFXGLESDevice.h"
#include "gfx/gles/GFXGLESTextureObject.h"
#include "gfx/gles/gfxGLESEnumTranslate.h"
#include "gfx/gles/gfxGLESUtils.h"
#include "gfx/gles/GFXGLESCubemap.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxCardProfile.h"
#import <GLKit/GLKit.h>
//#include "gfx/bitmap/DDSFile.h"


GLenum GFXGLESCubemap::faceList[6] = 
{ 
   GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

GFXGLESCubemap::GFXGLESCubemap() :
      mCubemap(0), 
      mDynamicTexSize(0),
      mFaceFormat( GFXFormatR8G8B8A8 )
{
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
//   GFXTextureManager::addEventDelegate( this, &GFXGLESCubemap::_onTextureEvent );
}

GFXGLESCubemap::~GFXGLESCubemap()
{
   glDeleteTextures(1, &mCubemap);
//   GFXTextureManager::removeEventDelegate( this, &GFXGLESCubemap::_onTextureEvent );
}

void GFXGLESCubemap::fillCubeTextures(GFXTexHandle* faces)
{
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
   U32 reqWidth = faces[0]->getWidth();
   U32 reqHeight = faces[0]->getHeight();
   GFXFormat regFaceFormat = faces[0]->getFormat();
   mWidth = reqWidth;
   mHeight = reqHeight;
   mFaceFormat = regFaceFormat;
   mMipLevels = 1; // Lie for now
   AssertFatal(reqWidth == reqHeight, "GFXGLESCubemap::fillCubeTextures - Width and height must be equal!");
   
   for(U32 i = 0; i < 6; i++)
   {
      AssertFatal(faces[i], avar("GFXGLESCubemap::fillCubeFaces - texture %i is NULL!", i));
      AssertFatal((faces[i]->getWidth() == reqWidth) && (faces[i]->getHeight() == reqHeight), "GFXGLESCubemap::fillCubeFaces - All textures must have identical dimensions!");
      AssertFatal(faces[i]->getFormat() == regFaceFormat, "GFXGLESCubemap::fillCubeFaces - All textures must have identical formats!");
      
      mTextures[i] = faces[i];
      GFXFormat faceFormat = faces[i]->getFormat();

      GFXGLESTextureObject* glTex = static_cast<GFXGLESTextureObject*>(faces[i].getPointer());
      U8* buf = glTex->getTextureData();
      glTexImage2D(faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], faces[i]->getWidth(), faces[i]->getHeight(), 
         0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], buf);
      delete[] buf;
   }
   
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

void GFXGLESCubemap::initStatic(GFXTexHandle* faces)
{
   if(mCubemap)
      return;
      
   if(faces)
   {
      AssertFatal(faces[0], "GFXGLESCubemap::initStatic - empty texture passed");
      glGenTextures(1, &mCubemap);
      fillCubeTextures(faces);
   }
}


void GFXGLESCubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
   mDynamicTexSize = texSize;
   mFaceFormat = faceFormat;

   glGenTextures(1, &mCubemap);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   mWidth = texSize;
   mHeight = texSize;
   mMipLevels = 1;
   for(U32 i = 0; i < 6; i++)
   {
      glTexImage2D(  faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], texSize, texSize, 
                     0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], NULL);
   }
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GFXGLESCubemap::zombify()
{
   glDeleteTextures(1, &mCubemap);
   mCubemap = 0;
}

void GFXGLESCubemap::resurrect()
{
   // Handled in tmResurrect
}

void GFXGLESCubemap::tmResurrect()
{
   if(mDynamicTexSize)
      initDynamic(mDynamicTexSize,mFaceFormat);
   else
   {
         initStatic( mTextures );
   }
}

void GFXGLESCubemap::setToTexUnit(U32 tuNum)
{
   static_cast<GFXGLESDevice*>(getOwningDevice())->setCubemapInternal(tuNum, this);
}

void GFXGLESCubemap::bind(U32 textureUnit) const
{
   glActiveTexture(GL_TEXTURE0 + textureUnit);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   
   GFXGLESStateBlockRef sb = static_cast<GFXGLESDevice*>(GFX)->getCurrentStateBlock();
   AssertFatal(sb, "GFXGLESCubemap::bind - No active stateblock!");
   if (!sb)
      return;   
      
   const GFXSamplerStateDesc& ssd = sb->getDesc().samplers[textureUnit];
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 0));   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU]);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV]);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GFXGLTextureAddress[ssd.addressModeW]);

//   glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, ssd.mipLODBias);
}

void GFXGLESCubemap::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      zombify();
   else
      tmResurrect();
}
