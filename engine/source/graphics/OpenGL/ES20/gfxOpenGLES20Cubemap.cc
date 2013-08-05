//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGLES20Device.h"
#include "gfxOpenGLES20TextureObject.h"
#include "./gfxOpenGLES20EnumTranslate.h"
#include "./gfxOpenGLES20Utils.h"
#include "./gfxOpenGLES20Cubemap.h"
#include "./gfxOpenGLES20TextureManager.h"
#include "graphics/OpenGL/gfxOpenGLCardProfiler.h"


GFXOpenGLES20Cubemap::GFXOpenGLES20Cubemap()
{
   mBinding = (GL_TEXTURE_CUBE_MAP);
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXOpenGLES20Cubemap::_onTextureEvent );
}

GFXOpenGLES20Cubemap::~GFXOpenGLES20Cubemap()
{
   glDeleteTextures(1, &mCubemap);
   GFXTextureManager::removeEventDelegate( this, &GFXOpenGLES20Cubemap::_onTextureEvent );
}



void GFXOpenGLES20Cubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
   mDynamicTexSize = texSize;
   mFaceFormat = faceFormat;

   glGenTextures(1, &mCubemap);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   setParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   setParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   setParameter( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   setParameter( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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


void GFXOpenGLES20Cubemap::bind(U32 textureUnit)
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   device->setTextureUnit(textureUnit);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   
   GFXOpenGLStateBlockRef sb = static_cast<GFXOpenGLES20Device*>(GFX)->getCurrentStateBlock();
   AssertFatal(sb, "GFXOpenGLES20Cubemap::bind - No active stateblock!");
   if (!sb)
      return;   
      
   const GFXSamplerStateDesc& ssd = sb->getDesc().samplers[textureUnit];
   setParameter( GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 0));   
   setParameter( GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);   
   setParameter( GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU]);
   setParameter( GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV]);
}



void GFXOpenGLES20Cubemap::fillCubeTextures(GFXTexHandle* faces)
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   device->setTextureUnit(0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap);
   setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   
   U32 reqWidth = faces[0]->getWidth();
   U32 reqHeight = faces[0]->getHeight();
   GFXFormat regFaceFormat = faces[0]->getFormat();
   mWidth = reqWidth;
   mHeight = reqHeight;
   mFaceFormat = regFaceFormat;
   mMipLevels = 1; // Lie for now
   AssertFatal(reqWidth == reqHeight, "GFXOpenGLES20Cubemap::fillCubeTextures - Width and height must be equal!");
   
   for(U32 i = 0; i < 6; i++)
   {
      AssertFatal(faces[i], avar("GFXOpenGLES20Cubemap::fillCubeFaces - texture %i is NULL!", i));
      AssertFatal((faces[i]->getWidth() == reqWidth) && (faces[i]->getHeight() == reqHeight), "GFXOpenGLES20Cubemap::fillCubeFaces - All textures must have identical dimensions!");
      AssertFatal(faces[i]->getFormat() == regFaceFormat, "GFXOpenGLES20Cubemap::fillCubeFaces - All textures must have identical formats!");
      
      mTextures[i] = faces[i];
      GFXFormat faceFormat = faces[i]->getFormat();
      
      GFXOpenGLES20TextureObject* glTex = static_cast<GFXOpenGLES20TextureObject*>(faces[i].getPointer());
      U8* buf = glTex->getTextureData();
      glTexImage2D(faceList[i], 0, GFXGLTextureInternalFormat[faceFormat], faces[i]->getWidth(), faces[i]->getHeight(),
                   0, GFXGLTextureFormat[faceFormat], GFXGLTextureType[faceFormat], buf);
      delete[] buf;
   }
   
   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
   //   glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}
