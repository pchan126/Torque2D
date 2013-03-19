//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platformiOS/graphics/gfxOpenGLESTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"
#include "graphics/gfxCardProfile.h"
#include "memory/safeDelete.h"
#include "platformiOS/graphics/gfxOpenGLESUtils.h"
#import <GLKit/GLKit.h>

#define EXT_ARRAY_SIZE 4
static const char* extArray[EXT_ARRAY_SIZE] = { "png", "pvr", "jpg", ""};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXOpenGLESTextureManager::GFXOpenGLESTextureManager()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXOpenGLESTextureManager::~GFXOpenGLESTextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}

////-----------------------------------------------------------------------------
//// createTexture
////-----------------------------------------------------------------------------
//GFXTextureObject *GFXOpenGLESTextureManager::createTexture( const StringTableEntry &path, GFXTextureProfile *profile )
//{
//    PROFILE_SCOPE( GFXTextureManager_createTexture );
//    
//    StringTableEntry thePath = path;
//    bool textureExt = thePath.getExtension().isNotEmpty();
//    GLKTextureInfo *texture = nil;
//    
//    String fullPath = path.getFullPath();
//    
//    // Check the cache first...
//    String pathNoExt;
//    pathNoExt = StringTableEntry::Join( thePath.getPath(), '/', thePath.getFileName() );
//    Con::printf("GFXOpenGLTextureManager::createTexture %s", pathNoExt.c_str());
//    
//    GFXTextureObject *retTexObj = _lookupTexture( pathNoExt, profile );
//    
//    if( retTexObj )
//        return retTexObj;
//    
//    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft: @NO};
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
//        temppath = [[NSString alloc] initWithUTF8String:thePath];
//        Con::printf("s %s", [temppath UTF8String]);
//        
//        if ([fMan fileExistsAtPath:temppath ])
//            npath = temppath;
//        i++;
//    }
//    while (!npath && !textureExt && ( i < EXT_ARRAY_SIZE));
//    
//    if (!npath)
//    {
//        Con::printf("GFXOpenGLESTextureManager::createTexture unable to find: %s - %s", [temppath UTF8String], path.getExtension().c_str());
//        return NULL;
//    }
//    
//    if (npath)
//    {
//        Con::printf("loading %s", [npath UTF8String] );
//        
//        texture = [GLKTextureLoader textureWithContentsOfFile:npath
//                                                      options:options error:&error];
//        
//        if (!texture) // error loading - use old loading
//        {
//            NSLog(@"Error loading texture: %@ %@", npath, [error localizedDescription]);
//            return Parent::createTexture( path, profile );
//        }
//        
//        retTexObj = new GFXOpenGLESTextureObject( GFX, profile, (__bridge void*)texture );
//        retTexObj->registerResourceWithDevice( GFX );
//        
//        
//        //            GLKTextureLoaderCallback texCallback = ^(GLKTextureInfo *textureInfo, NSError *outError){
//        //                this->parseTextureInfo(textureInfo);
//        //            };
//        //
//        //            [ platState.textureLoader textureWithContentsOfFile:path options:options queue:NULL completionHandler:
//        //             ^(GLKTextureInfo *textureInfo, NSError *outError)
//        //             {
//        //             if (outError) {     Con::printf("imageloadError"); };     //[ISDebugger logError:outError inMethod:_cmd];
//        //             GLuint textureName = [textureInfo name];
//        //             this->parseTextureInfo(textureInfo);
//        //             } ];
//    }
//    
//    
//    if ( retTexObj )
//    {
//        // Store the path for later use.
//        retTexObj->mPath = thePath;
//        _linkTexture( retTexObj );
//    }
//    else
//    {
//        Con::printf("unable to make texture");
//    }
//    
//    return retTexObj;
//}
//

GFXTextureObject *GFXOpenGLESTextureManager::_createTextureObject(   U32 height,
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXOpenGLESTextureManager::_createTexture - invalid format!");

   GFXOpenGLESTextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXOpenGLESTextureObject*>( inTex ), "GFXOpenGLESTextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXOpenGLESTextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXOpenGLESTextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips);

   return retTex;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXOpenGLESTextureManager::innerCreateTexture( GFXOpenGLESTextureObject *retTex, 
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
    GLenum binding = GL_TEXTURE_2D;   // No 3d texture in openGL es 2.0
   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
      retTex->mIsNPoT2 = true;
   retTex->mBinding = binding;
   
   // Bind it
   glActiveTexture(GL_TEXTURE0);
//   PRESERVE_2D_TEXTURE();
//   PRESERVE_3D_TEXTURE();
   glBindTexture(binding, retTex->getHandle());
   
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
   
   AssertFatal(GFXGLTextureInternalFormat[format] != GL_ZERO, "GFXOpenGLESTextureManager::innerCreateTexture - invalid internal format");
   AssertFatal(GFXGLTextureFormat[format] != GL_ZERO, "GFXOpenGLESTextureManager::innerCreateTexture - invalid format");
   AssertFatal(GFXGLTextureType[format] != GL_ZERO, "GFXOpenGLESTextureManager::innerCreateTexture - invalid type");
   
//   if(binding != GL_TEXTURE_3D)
      glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);
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
//   GLint texHeight, texWidth, texDepth = 0;
   
//   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_WIDTH, &texWidth);
//   glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_HEIGHT, &texHeight);
//   if(binding == GL_TEXTURE_3D)
//      glGetTexLevelParameteriv(binding, 0, GL_TEXTURE_DEPTH, &texDepth);
   
//   retTex->mTextureSize.set(texWidth, texHeight, texDepth);
}





//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

static void _slowTextureLoad(GFXOpenGLESTextureObject* texture, GBitmap* pDL)
{
   glTexSubImage2D(texture->getBinding(), 0, 0, 0, pDL->getWidth(0), pDL->getHeight(0), GFXGLTextureFormat[pDL->getFormat()], GFXGLTextureType[pDL->getFormat()], pDL->getBits(0));
}

bool GFXOpenGLESTextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   GFXOpenGLESTextureObject *texture = static_cast<GFXOpenGLESTextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXOpenGLESTextureManager::_loadTexture(GBitmap) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   // Bind to edit
   glActiveTexture(GL_TEXTURE0);
//   PRESERVE_2D_TEXTURE();
   glBindTexture(texture->getBinding(), texture->getHandle());
   
      _slowTextureLoad(texture, pDL);
   
   glBindTexture(texture->getBinding(), 0);
   
   return true;
}


bool GFXOpenGLESTextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   if(aTexture->getDepth() < 1)
      return false;
   
   GFXOpenGLESTextureObject* texture = static_cast<GFXOpenGLESTextureObject*>(aTexture);
   
   glActiveTexture(GL_TEXTURE0);

   glBindTexture(GL_TEXTURE_2D, texture->getHandle());
   glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   glBindTexture(GL_TEXTURE_2D, 0);
   
   return true;
}

bool GFXOpenGLESTextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXOpenGLESTextureObject*>(texture)->zombify();
   else
      static_cast<GFXOpenGLESTextureObject*>(texture)->release();
      
   return true;
}

bool GFXOpenGLESTextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXOpenGLESTextureObject* realTex = static_cast<GFXOpenGLESTextureObject*>(texture);
      
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
   
   AssertFatal(usedStrategies < 2, "GFXOpenGLESTextureManager::_refreshTexture - Inconsistent profile flags (store bitmap and dynamic/target");
   
   return true;
}
