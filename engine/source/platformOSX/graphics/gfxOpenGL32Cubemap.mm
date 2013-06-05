//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGL32Device.h"
#include "./gfxOpenGL32TextureObject.h"
#include "./gfxOpenGL32EnumTranslate.h"
#include "./gfxOpenGL32Utils.h"
#include "./gfxOpenGL32Cubemap.h"
#include "./gfxOpenGL32TextureManager.h"
#include "graphics/OpenGL/gfxOpenGLCardProfiler.h"


GFXOpenGL32Cubemap::GFXOpenGL32Cubemap()
{
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXOpenGL32Cubemap::_onTextureEvent );
}

GFXOpenGL32Cubemap::~GFXOpenGL32Cubemap()
{
   glDeleteTextures(1, &mCubemap);
   GFXTextureManager::removeEventDelegate( this, &GFXOpenGL32Cubemap::_onTextureEvent );
}



void GFXOpenGL32Cubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
   mDynamicTexSize = texSize;
   mFaceFormat = faceFormat;

   glGenTextures(1, &mCubemap);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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


void GFXOpenGL32Cubemap::bind(U32 textureUnit) const
{
   glActiveTexture(GL_TEXTURE0 + textureUnit);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   
   GFXOpenGLStateBlockRef sb = static_cast<GFXOpenGLDevice*>(GFX)->getCurrentStateBlock();
   AssertFatal(sb, "GFXOpenGL32Cubemap::bind - No active stateblock!");
   if (!sb)
      return;   
      
   const GFXSamplerStateDesc& ssd = sb->getDesc().samplers[textureUnit];
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 0));   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU]);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV]);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GFXGLTextureAddress[ssd.addressModeW]);
}



void GFXOpenGL32Cubemap::fillCubeTextures(GFXTexHandle* faces)
{
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
   U32 reqWidth = faces[0]->getWidth();
   U32 reqHeight = faces[0]->getHeight();
   GFXFormat regFaceFormat = faces[0]->getFormat();
   mWidth = reqWidth;
   mHeight = reqHeight;
   mFaceFormat = regFaceFormat;
   mMipLevels = 1; // Lie for now
   AssertFatal(reqWidth == reqHeight, "GFXOpenGL32Cubemap::fillCubeTextures - Width and height must be equal!");
   
   for(U32 i = 0; i < 6; i++)
   {
      AssertFatal(faces[i], avar("GFXOpenGL32Cubemap::fillCubeFaces - texture %i is NULL!", i));
      AssertFatal((faces[i]->getWidth() == reqWidth) && (faces[i]->getHeight() == reqHeight), "GFXOpenGL32Cubemap::fillCubeFaces - All textures must have identical dimensions!");
      AssertFatal(faces[i]->getFormat() == regFaceFormat, "GFXOpenGL32Cubemap::fillCubeFaces - All textures must have identical formats!");
      
      mTextures[i] = faces[i];
      GFXFormat faceFormat = faces[i]->getFormat();
      
      GFXOpenGL32TextureObject* glTex = static_cast<GFXOpenGL32TextureObject*>(faces[i].getPointer());
      U8* buf = glTex->getTextureData();
      glTexImage2D(faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], faces[i]->getWidth(), faces[i]->getHeight(),
                   0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], buf);
      delete[] buf;
   }
   
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
   //   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}
