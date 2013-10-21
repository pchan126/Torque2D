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

#include "./gfxOpenGL32TextureManager.h"
#include "./gfxOpenGL32EnumTranslate.h"
#include "graphics/gfxCardProfile.h"
#include "memory/safeDelete.h"
#include "./gfxOpenGL32Utils.h"
#include "platform/platformGL.h"
#include <memory>
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
GFXOpenGL32TextureManager::GFXOpenGL32TextureManager(NSOpenGLContext* mContext)
{
    mContext = mContext;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXOpenGL32TextureManager::~GFXOpenGL32TextureManager()
{
}

// build texture from GBitmap
GFXTexHandle GFXOpenGL32TextureManager::createTexture(GBitmap *bmp,
        const String &resourceName,
        GFXTextureProfile *profile,
        bool deleteBmp)
{
    AssertFatal(bmp, "GFXTextureManager::createTexture() - Got nullptr bitmap!");
    
    GFXTexHandle cacheHit = _lookupTexture( resourceName, profile );
    if( cacheHit != nullptr)
    {
        // Con::errorf("Cached texture '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
        if (deleteBmp)
            delete bmp;
        return cacheHit;
    }

    return _createTexture( bmp, resourceName, profile, deleteBmp, nullptr );
}

//GFXTextureObject *GFXOpenGL32TextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        void *pixels,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile)
//{
//    
//}
//
//GFXTextureObject *GFXOpenGL32TextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        U32 depth,
//                                        void *pixels,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile )
//{
//    
//}

//GFXTextureObject *GFXOpenGL32TextureManager::createTexture(  U32 width,
//                                        U32 height,
//                                        GFXFormat format,
//                                        GFXTextureProfile *profile,
//                                        U32 numMipLevels,
//                                        S32 antialiasLevel)
//{
//    
//}

GFXTexHandle GFXOpenGL32TextureManager::_createTexture(GBitmap *bmp,
        const String &resourceName,
        GFXTextureProfile *profile,
        bool deleteBmp,
        GFXTexHandle inObj)
{
   PROFILE_SCOPE( GFXOpenGLESTextureManager_CreateTexture_Bitmap );
   
#ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] _createTexture (GBitmap) '%s'",
                               resourceName.c_str()
                               );
#endif
   
   // Massage the bitmap based on any resize rules.
   U32 scalePower = getTextureDownscalePower( profile );
   
   GBitmap *realBmp = bmp->createPowerOfTwoBitmap();
   U32 realWidth = bmp->getWidth();
   U32 realHeight = bmp->getHeight();
   
   if (  scalePower &&
       isPow2(bmp->getWidth()) &&
       isPow2(bmp->getHeight()) &&
       profile->canDownscale() )
   {
      // We only work with power of 2 textures for now, so we
      // don't have to worry about padding.
      
      GBitmap *padBmp = bmp;
      padBmp->extrudeMipLevels();
      scalePower = std::min( scalePower, padBmp->getNumMipLevels() - 1 );
      
      realWidth  = std::max( (U32)1, padBmp->getWidth() >> scalePower );
      realHeight = std::max( (U32)1, padBmp->getHeight() >> scalePower );
      realBmp = new GBitmap( realWidth, realHeight, false, bmp->getFormat() );
      
      // Copy to the new bitmap...
      dMemcpy( realBmp->getWritableBits(),
              padBmp->getBits(scalePower),
              padBmp->mBytesPerPixel * realWidth * realHeight );
      
      // This line is commented out because createPaddedBitmap is commented out.
      // If that line is added back in, this line should be added back in.
      // delete padBmp;
   }
   
   // Call the internal create... (use the real* variables now, as they
   // reflect the reality of the texture we are creating.)
   U32 numMips = 0;
   GFXFormat realFmt = realBmp->getFormat();
   _validateTexParams( realWidth, realHeight, profile, numMips, realFmt );
   
   GFXTexHandle ret = GFXTexHandle(nullptr);
   if ( inObj )
   {
      // If the texture has changed in dimensions
      // then we need to recreate it.
      if (  inObj->getWidth() != realWidth ||
          inObj->getHeight() != realHeight ||
          inObj->getFormat() != realFmt )
         ret = _createTextureObject( realHeight, realWidth, 0, realFmt, profile, numMips, false, 0, inObj );
      else
         ret = inObj;
   }
   else
      ret = _createTextureObject(realHeight, realWidth, 0, realFmt, profile, numMips, false, 0, nullptr, (void*)bmp->getBits() );
   
   if(!ret)
   {
      Con::errorf("GFXTextureManager - failed to create texture (1) for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      return ret;
   }
   
   auto retTex = std::dynamic_pointer_cast<GFXOpenGL32TextureObject>(ret);
   
   if (realBmp != bmp && retTex)
      retTex->mIsNPoT2 = true;
   
   
   // Extrude mip levels
   // Don't do this for fonts!
   if( ret->mMipLevels > 1 && ( realBmp->getNumMipLevels() == 1 ) && ( realBmp->getFormat() != GFXFormatA8 ) &&
      isPow2( realBmp->getHeight() ) && isPow2( realBmp->getWidth() ) && !profile->noMip() )
   {
      // NOTE: This should really be done by extruding mips INTO a DDS file instead
      // of modifying the gbitmap
      realBmp->extrudeMipLevels(false);
   }
   
   
   if (!_loadTexture( ret, realBmp ))
   {
      Con::errorf("GFXTextureManager - failed to load GBitmap for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      return ret;
   }
   
   // Do statistics and book-keeping...
   
   //    - info for the texture...
   ret->mTextureLookupName = resourceName;
   ret->mBitmapSize.set(realWidth, realHeight,0);
   
#ifdef TORQUE_DEBUG
   if (resourceName.isNotEmpty())
      ret->mDebugDescription = resourceName;
   else
      ret->mDebugDescription = "Anonymous Texture Object";
   
#endif
   
   if(profile->doStoreBitmap())
   {
      // NOTE: may store a downscaled copy!
      ret->mBitmap = GBitmapPtr(new GBitmap( *realBmp ));
   }
   
   if ( !inObj )
      _linkTexture( ret );
   
   //    - output debug info?
   // Save texture for debug purpose
   //   static int texId = 0;
   //   char buff[256];
   //   dSprintf(buff, sizeof(buff), "tex_%d", texId++);
   //   bmp->writePNGDebug(buff);
   //   texId++;
   
   // Before we delete the bitmap save our transparency flag
   //   ret->mHasTransparency = realBmp->getHasTransparency();
   
   // Some final cleanup...
   if(realBmp != bmp)
      SAFE_DELETE(realBmp);
   if (deleteBmp)
      SAFE_DELETE(bmp);
   
   // Return the new texture!
   return ret;
}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
//GFXTextureObject *GFXOpenGL32TextureManager::createTexture( const String &path, GFXTextureProfile *profile )
//{
//    PROFILE_SCOPE( GFXTextureManager_createTexture );
//    GFXOpenGL32Device *device = (GFXOpenGL32Device*)GFX;
//    GLKTextureLoader *texLoader = (GLKTextureLoader*)device->getTextureLoader();
//    
//    AssertFatal((texLoader != nullptr), "texture loader not initalized");
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
////    Con::printf("GFXOpenGL32TextureManager::createTexture %s", pathNoExt.c_str());
//    
//    GFXTextureObject *retTexObj = _lookupTexture( pathNoExt.c_str(), profile );
//
//    if( retTexObj )
//        return retTexObj;
//    
//    GFXOpenGL32TextureObject *makeTexObj;
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
////       Con::printf("GFXOpenGL32TextureManager::createTexture unable to find: %s - %s", temp, path.getExtension().c_str());
////       [temppath release];
//       return nullptr;
//    }
////   [temppath release];
//   
//    if (npath)
//    {
//        makeTexObj = new GFXOpenGL32TextureObject( GFX, profile );
//        makeTexObj->registerResourceWithDevice( GFX );
//        makeTexObj->mTextureLookupName = pathNoExt;
//        
//        GLKTextureLoaderCallback texCallback = ^(GLKTextureInfo *textureInfo, NSError *outError){
//            makeTexObj->setTexture(textureInfo);
//            Con::printf("ding fries are done: %s", [npath UTF8String]);
//        };
//        
//        Con::printf("async load %s", [npath UTF8String]);
//        [ texLoader textureWithContentsOfFile:npath options:options queue:nullptr completionHandler:[[texCallback copy]autorelease]];
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


GFXTexHandle GFXOpenGL32TextureManager::_createTextureObject(U32 height,
        U32 width,
        U32 depth,
        GFXFormat format,
        GFXTextureProfile *profile,
        U32 numMipLevels,
        bool forceMips,
        S32 antialiasLevel,
        GFXTexHandle inTex,
        void *data)
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXOpenGL32TextureManager::_createTexture - invalid format!");

    std::shared_ptr<GFXOpenGL32TextureObject> retTex;
   if ( inTex )
   {
      AssertFatal( std::dynamic_pointer_cast<GFXOpenGL32TextureObject>( inTex ), "GFXOpenGL32TextureManager::_createTexture() - Bad inTex type!" );
      retTex = std::static_pointer_cast<GFXOpenGL32TextureObject>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = std::make_shared<GFXOpenGL32TextureObject>( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips, data);

   GFXTexHandle ret = std::dynamic_pointer_cast<GFXTextureObject>(retTex);
   return ret;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXOpenGL32TextureManager::innerCreateTexture( std::shared_ptr<GFXOpenGL32TextureObject>& retTex,
                                               U32 height,
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips, void* data)
{
    GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   // No 24 bit formats.  They trigger various oddities because hardware (and Apple's drivers apparently...) don't natively support them.
   if(format == GFXFormatR8G8B8)
      format = GFXFormatR8G8B8A8;
      
   retTex->mFormat = format;
   retTex->mIsZombie = false;
   retTex->mIsNPoT2 = false;
   
   GLenum binding = (depth == 0) ? GL_TEXTURE_2D : GL_TEXTURE_3D;

   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
      retTex->mIsNPoT2 = true;

   retTex->mBinding = binding;
   
    // Bind it
   device->setTextureUnit(0);
   glBindTexture(binding, retTex->getHandle());

   if(!retTex->mIsNPoT2)
   {
      if(!isPow2(width))
         width = getNextPow2(width);
      if(!isPow2(height))
         height = getNextPow2(height);
      if(depth && !isPow2(depth))
         depth = getNextPow2(depth);
   }
   
   AssertFatal(GFXGLTextureInternalFormat[format] != GL_ZERO, "GFXOpenGL32TextureManager::innerCreateTexture - invalid internal format");
   AssertFatal(GFXGLTextureFormat[format] != GL_ZERO, "GFXOpenGL32TextureManager::innerCreateTexture - invalid format");
   AssertFatal(GFXGLTextureType[format] != GL_ZERO, "GFXOpenGL32TextureManager::innerCreateTexture - invalid type");

   if(binding == GL_TEXTURE_RECTANGLE)
      glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], nullptr);
   else if(binding == GL_TEXTURE_2D)
       glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], nullptr);
   else
      glTexImage3D(GL_TEXTURE_3D, 0, GFXGLTextureInternalFormat[format], width, height, depth, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], nullptr);

//   switch (binding) {
//      case GL_TEXTURE_1D:
//      case GL_PROXY_TEXTURE_1D:
//         glTexImage1D(binding, 0, GFXGLTextureInternalFormat[format], width, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], data);
//         break;
//         
//      case GL_TEXTURE_2D:
//         glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], data);
//         break;
//
//      case GL_PROXY_TEXTURE_2D:
//      case GL_TEXTURE_1D_ARRAY:
//      case GL_PROXY_TEXTURE_1D_ARRAY:
//      case GL_TEXTURE_RECTANGLE:
//      case GL_PROXY_TEXTURE_RECTANGLE:
//      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
//      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
//      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
//      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
//      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
//      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
//      case GL_PROXY_TEXTURE_CUBE_MAP:
//         glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], data);
//         break;
//         
//      case GL_TEXTURE_3D:
//      case GL_PROXY_TEXTURE_3D:
//      case GL_TEXTURE_2D_ARRAY:
//      case GL_PROXY_TEXTURE_2D_ARRAY:
//         glTexImage3D(binding, 0, GFXGLTextureInternalFormat[format], width, height, depth, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], data);
//         break;
//
//      default:
//         break;
//   }
   
    retTex->mTextureSize.set(width, height, 0);
}





//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

bool GFXOpenGL32TextureManager::_loadTexture(GFXTexHandle &aTexture, GBitmap *pDL)
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   auto texture = std::static_pointer_cast<GFXOpenGL32TextureObject>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXOpenGL32TextureManager::_loadTexture(GBitmap) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   // Bind to edit
   device->setTextureUnit(0);
   glBindTexture(texture->getBinding(), texture->getHandle());
   glTexSubImage2D(texture->getBinding(), 0, 0, 0, pDL->getWidth(0), pDL->getHeight(0), GFXGLTextureFormat[pDL->getFormat()], GFXGLTextureType[pDL->getFormat()], pDL->getBits(0));
   glBindTexture(texture->getBinding(), 0);
   
   return true;
}


bool GFXOpenGL32TextureManager::_loadTexture(GFXTexHandle &aTexture, void *raw)
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   if(aTexture->getDepth() < 1)
      return false;
   
   auto texture = std::static_pointer_cast<GFXOpenGL32TextureObject>(aTexture);

   device->setTextureUnit(0);

   glBindTexture(GL_TEXTURE_2D, texture->getHandle());
   glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   glBindTexture(GL_TEXTURE_2D, 0);
   
   return true;
}

bool GFXOpenGL32TextureManager::_freeTexture(GFXTexHandle &texture, bool zombify /*= false*/)
{
   auto realTex = std::static_pointer_cast<GFXOpenGL32TextureObject>(texture);
   if(zombify)
       realTex->zombify();
   else
       realTex->release();
      
   return true;
}

bool GFXOpenGL32TextureManager::_refreshTexture(GFXTexHandle &texture)
{
   U32 usedStrategies = 0;
   auto realTex = std::static_pointer_cast<GFXOpenGL32TextureObject>(texture);
      
   if(texture->mProfile->doStoreBitmap())
   {
      if(realTex->isZombie())
      {
         realTex->resurrect();
         innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      }
      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap.get());
      
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
   
   AssertFatal(usedStrategies < 2, "GFXOpenGL32TextureManager::_refreshTexture - Inconsistent profile flags (store bitmap and dynamic/target");
   
   return true;
}
