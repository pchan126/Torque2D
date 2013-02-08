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

#include "platform/platform.h"

#include "./gfxOpenGLTextureManager.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "graphics/gfxCardProfile.h"
#include "memory/safeDelete.h"
#include "./gfxOpenGLUtils.h"
#include "platform/platformGL.h"

//#import <GLKit/GLKit.h>
//#include <squish.h>

#if (defined TORQUE_OS_IPHONE || defined TORQUE_OS_MAC)
#define EXT_ARRAY_SIZE 4
static const char* extArray[EXT_ARRAY_SIZE] = { "", "pvr", "jpg", "png"};
#else
#define EXT_ARRAY_SIZE 3
static const char* extArray[EXT_ARRAY_SIZE] = { "", "jpg", "png"};
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXOpenGLTextureManager::GFXOpenGLTextureManager()
{
    mDGLRender = true;
    mTextureCompressionHint = GL_FASTEST;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXOpenGLTextureManager::~GFXOpenGLTextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}

// build texture from GBitmap
//GFXTextureObject *GFXOpenGLTextureManager::createTexture(  GBitmap *bmp,
//                                        const String &resourceName,
//                                        GFXTextureProfile *profile,
//                                        bool deleteBmp)
//{
//    AssertFatal(bmp, "GFXTextureManager::createTexture() - Got NULL bitmap!");
//    
//    GFXTextureObject *cacheHit = _lookupTexture( resourceName, profile );
//    if( cacheHit != NULL)
//    {
//        // Con::errorf("Cached texture '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
//        if (deleteBmp)
//            delete bmp;
//        return cacheHit;
//    }
//    
//    return _createTexture( bmp, resourceName, profile, deleteBmp, NULL );
//}

//GFXTextureObject *GFXOpenGLTextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        void *pixels,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile)
//{
//    
//}
//
//GFXTextureObject *GFXOpenGLTextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        U32 depth,
//                                        void *pixels,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile )
//{
//    
//}
//
//GFXTextureObject *GFXOpenGLTextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile,
//                                        U32 numMipLevels,
//                                        S32 antialiasLevel)
//{
//    
//}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
//GFXTextureObject *GFXOpenGLTextureManager::createTexture( const String &path, GFXTextureProfile *profile )
//{
//    PROFILE_SCOPE( GFXTextureManager_createTexture );
//    GFXOpenGLDevice *device = (GFXOpenGLDevice*)GFX;
//    GLKTextureLoader *texLoader = (GLKTextureLoader*)device->getTextureLoader();
//    
//    AssertFatal((texLoader != NULL), "texture loader not initalized");
//    
//    // Resource handles used for loading.  Hold on to them
//    // throughout this function so that change notifications
//    // don't get added, then removed, and then re-added.
//    
//    String thePath = path;
//    bool textureExt = thePath.getExtension().isNotEmpty();
//    GLKTextureInfo *texture = nil;
//
//    String fullPath = path.getFullPath();
//    
//    // Check the cache first...
//    String pathNoExt;
//    pathNoExt = String::Join( thePath.getPath(), '/', thePath.getFileName() );
////    Con::printf("GFXOpenGLTextureManager::createTexture %s", pathNoExt.c_str());
//    
//    GFXTextureObject *retTexObj = _lookupTexture( pathNoExt.c_str(), profile );
//
//    if( retTexObj )
//        return retTexObj;
//    
//    GFXOpenGLTextureObject *makeTexObj;
//    
//    NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:NO]
//                                                            forKey:GLKTextureLoaderOriginBottomLeft];
//
//    NSString *npath = nil;
//    NSError * error;
//    NSString* temppath = nil;
//    
//    U32 i = 0;
//    do
//    {
//        if (!textureExt)
//            thePath.setExtension(extArray[i]);
//        
//        //Ask the bundle for the path to the file
//        NSFileManager* fMan = [[NSFileManager alloc] init];
//        NSString* temp = [[NSString alloc ] initWithUTF8String:thePath.getFullPath().c_str()];
////        Con::printf("s %s", [temppath UTF8String]);
//        
//        if ([fMan fileExistsAtPath:temp ])
//            npath = [[NSString alloc] initWithString: temp];
//       
//        [temp release];
//       [fMan release];
//        i++;
//    }
//    while (!npath && !textureExt && ( i < EXT_ARRAY_SIZE));
//   
//    if (!npath)
//    {
////       Con::printf("GFXOpenGLTextureManager::createTexture unable to find: %s - %s", temp, path.getExtension().c_str());
////       [temppath release];
//       return NULL;
//    }
////   [temppath release];
//   
//    if (npath)
//    {
//        makeTexObj = new GFXOpenGLTextureObject( GFX, profile );
//        makeTexObj->registerResourceWithDevice( GFX );
//        makeTexObj->mTextureLookupName = pathNoExt;
//        
//        GLKTextureLoaderCallback texCallback = ^(GLKTextureInfo *textureInfo, NSError *outError){
//            makeTexObj->setTexture(textureInfo);
//            Con::printf("ding fries are done: %s", [npath UTF8String]);
//        };
//        
//        Con::printf("async load %s", [npath UTF8String]);
//        [ texLoader textureWithContentsOfFile:npath options:options queue:NULL completionHandler:[[texCallback copy]autorelease]];
//    }
//    
//    
//    if ( makeTexObj )
//    {
//        // Store the path for later use.
//        makeTexObj->mPath = thePath;
//        _linkTexture( makeTexObj );
//    }
//    else
//    {
//        Con::printf("unable to make texture");
//    }
//    
//    return makeTexObj;
//}


GFXTextureObject *GFXOpenGLTextureManager::_createTextureObject(   U32 height,
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXOpenGLTextureManager::_createTexture - invalid format!");

   GFXOpenGLTextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXOpenGLTextureObject*>( inTex ), "GFXOpenGLTextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXOpenGLTextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXOpenGLTextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips);

   return retTex;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXOpenGLTextureManager::innerCreateTexture( GFXOpenGLTextureObject *retTex, 
                                               U32 height, 
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips)
{
   // No 24 bit formats.  They trigger various oddities because hardware (and Apple's drivers apparently...) don't natively support them.
   if(format == GFXFormatR8G8B8)
      format = GFXFormatR8G8B8A8;
      
   retTex->mFormat = format;
   retTex->mIsZombie = false;
   retTex->mIsNPoT2 = false;
   
//   GLenum binding = (depth == 0) ? GL_TEXTURE_2D : GL_TEXTURE_3D;
    GLenum binding = GL_TEXTURE_2D;

   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
   {
      retTex->mIsNPoT2 = true;
   }
   retTex->mBinding = binding;
   
    // Bind it
   glActiveTexture(GL_TEXTURE0);
   PRESERVE_2D_TEXTURE();
//   PRESERVE_3D_TEXTURE();
   GL_CHECK(glBindTexture(binding, retTex->getHandle()));
   
//   // Create it
//   // TODO: Reenable mipmaps on render targets when Apple fixes their drivers
//   if(forceMips && !retTex->mIsNPoT2)
//   {
//      glTexParameteri(binding, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
//      retTex->mMipLevels = 0;
//   }
//   else if(profile->testFlag(GFXTextureProfile::NoMipmap) || profile->testFlag(GFXTextureProfile::RenderTarget) || numMipLevels == 1 || retTex->mIsNPoT2)
//   {
//      retTex->mMipLevels = 1;
//   }
//   else
//   {
//      glTexParameteri(binding, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
//      retTex->mMipLevels = 0;
//   }

   if(!retTex->mIsNPoT2)
   {
      if(!isPow2(width))
         width = getNextPow2(width);
      if(!isPow2(height))
         height = getNextPow2(height);
      if(depth && !isPow2(depth))
         depth = getNextPow2(depth);
   }
   
   AssertFatal(GFXGLTextureInternalFormat[format] != GL_ZERO, "GFXOpenGLTextureManager::innerCreateTexture - invalid internal format");
   AssertFatal(GFXGLTextureFormat[format] != GL_ZERO, "GFXOpenGLTextureManager::innerCreateTexture - invalid format");
   AssertFatal(GFXGLTextureType[format] != GL_ZERO, "GFXOpenGLTextureManager::innerCreateTexture - invalid type");
   
    //   if(binding != GL_TEXTURE_3D)
      GL_CHECK(glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL));

    //   else
//      glTexImage3D(GL_TEXTURE_3D, 0, GFXGLTextureInternalFormat[format], width, height, depth, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);
   
   // Complete the texture
   glTexParameteri(binding, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(binding, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(binding, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(binding, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//   if(binding == GL_TEXTURE_3D)
//      glTexParameteri(binding, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
    
    // Get the size from GL (you never know...)
   GLint texHeight, texWidth, texDepth = 0;
   
   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_WIDTH, &texWidth);
   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_HEIGHT, &texHeight);
//   if(binding == GL_TEXTURE_3D)
//      glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_DEPTH, &texDepth);
   
    retTex->mTextureSize.set(texWidth, texHeight, texDepth);
}





//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

static void _slowTextureLoad(GFXOpenGLTextureObject* texture, GBitmap* pDL)
{
   glTexSubImage2D(texture->getBinding(), 0, 0, 0, pDL->getWidth(0), pDL->getHeight(0), GFXGLTextureFormat[pDL->getFormat()], GFXGLTextureType[pDL->getFormat()], pDL->getBits(0));
}

bool GFXOpenGLTextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   GFXOpenGLTextureObject *texture = static_cast<GFXOpenGLTextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXOpenGLTextureManager::_loadTexture(GBitmap) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   // Bind to edit
   glActiveTexture(GL_TEXTURE0);
   PRESERVE_2D_TEXTURE();
   glBindTexture(texture->getBinding(), texture->getHandle());
   
      _slowTextureLoad(texture, pDL);
   
   glBindTexture(texture->getBinding(), 0);
   
   return true;
}


bool GFXOpenGLTextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   if(aTexture->getDepth() < 1)
      return false;
   
   GFXOpenGLTextureObject* texture = static_cast<GFXOpenGLTextureObject*>(aTexture);
   
   glActiveTexture(GL_TEXTURE0);

   glBindTexture(GL_TEXTURE_2D, texture->getHandle());
   glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   glBindTexture(GL_TEXTURE_2D, 0);
   
   return true;
}

bool GFXOpenGLTextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXOpenGLTextureObject*>(texture)->zombify();
   else
      static_cast<GFXOpenGLTextureObject*>(texture)->release();
      
   return true;
}

bool GFXOpenGLTextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXOpenGLTextureObject* realTex = static_cast<GFXOpenGLTextureObject*>(texture);
      
   if(texture->mProfile->doStoreBitmap())
   {
      if(realTex->isZombie())
      {
         realTex->resurrect();
         innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      }
      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap);
      
      usedStrategies++;
   }
   
   if(texture->mProfile->isRenderTarget() || texture->mProfile->isDynamic() || texture->mProfile->isZTarget() || !usedStrategies)
   {
      realTex->release();
      realTex->resurrect();
      innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      realTex->reloadFromCache();
      usedStrategies++;
   }
   
   AssertFatal(usedStrategies < 2, "GFXOpenGLTextureManager::_refreshTexture - Inconsistent profile flags (store bitmap and dynamic/target");
   
   return true;
}
