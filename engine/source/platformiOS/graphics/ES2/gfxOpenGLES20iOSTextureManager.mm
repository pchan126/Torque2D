//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./GFXOpenGLES20iOSTextureManager.h"
#include "./gfxOpenGLES20iOSEnumTranslate.h"
#include "graphics/gfxCardProfile.h"
#include "memory/safeDelete.h"
#include "./gfxOpenGLES20iOSUtils.h"
#include "./gfxOpenGLES20iOSDevice.h"
#import <GLKit/GLKit.h>


//#define EXT_ARRAY_SIZE 4
//static const char* extArray[EXT_ARRAY_SIZE] = { "png", "pvr", "jpg", ""};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXOpenGLES20iOSTextureManager::GFXOpenGLES20iOSTextureManager()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXOpenGLES20iOSTextureManager::~GFXOpenGLES20iOSTextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}


GFXTextureObject *GFXOpenGLES20iOSTextureManager::createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp )
{
    AssertFatal(bmp, "GFXTextureManager::createTexture() - Got NULL bitmap!");
    
    GFXTextureObject *cacheHit = _lookupTexture( resourceName, profile );
    if( cacheHit != NULL)
    {
        // Con::errorf("Cached texture '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
        if (deleteBmp)
            delete bmp;
        return cacheHit;
    }
    
    return _createTexture( bmp, resourceName, profile, deleteBmp, NULL );
}


GFXTextureObject *GFXOpenGLES20iOSTextureManager::_createTexture(  GBitmap *bmp,
                                                    const String &resourceName,
                                                    GFXTextureProfile *profile,
                                                    bool deleteBmp,
                                                    GFXTextureObject *inObj )
{
    PROFILE_SCOPE( GFXOpenGLES20iOSTextureManager_CreateTexture_Bitmap );
    
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
    
    GFXTextureObject *ret;
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
        ret = _createTextureObject(realHeight, realWidth, 0, realFmt, profile, numMips, false, 0, NULL, (void*)bmp->getBits());
   
    if(!ret)
    {
        Con::errorf("GFXTextureManager - failed to create texture (1) for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
        return nullptr;
    }

   GFXOpenGLES20iOSTextureObject* retTex = dynamic_cast<GFXOpenGLES20iOSTextureObject*>(ret);

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
        return nullptr;
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
        SAFE_DELETE( ret->mBitmap );
        ret->mBitmap = new GBitmap( *realBmp );
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

////-----------------------------------------------------------------------------
//// createTexture
////-----------------------------------------------------------------------------
GFXTextureObject *GFXOpenGLES20iOSTextureManager::createTexture( const String &fullPath, GFXTextureProfile *profile )
{
    GLKTextureInfo *texture = nil;

    StringTableEntry path, fileName;
    
    static char buf[1024];
    char *ptr = (char *) dStrrchr (fullPath.c_str(), '/');
    if (!ptr)
    {
        path = nullptr;
        fileName = StringTable->insert (fullPath.c_str());
    }
    else
    {
        size_t len = ptr - fullPath.c_str();
        dStrncpy (buf, fullPath.c_str(), len);
        buf[len] = 0;
        fileName = StringTable->insert (ptr + 1);
        path = StringTable->insert (buf);
    }
    if(path)
    {
        char fullPath[1024];
        Platform::makeFullPathName(path, fullPath, sizeof(fullPath));
        path = StringTable->insert(fullPath);
    }
    
    GFXTextureObject *retTexObj = _lookupTexture( fileName, profile );

    if( retTexObj )
        return retTexObj;

    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft: @NO};
    NSString *npath = [[NSString alloc] initWithUTF8String:fullPath.c_str()];
    NSError * error;
    texture = [GLKTextureLoader textureWithContentsOfFile:npath
                                                      options:options error:&error];

    if (!texture) // error loading - use old loading
    {
        NSLog(@"Error loading texture: %@ %@", npath, [error localizedDescription]);
        GBitmap* bmp = GBitmap::load(fullPath);
        return createTexture( bmp, path, profile, true );
    }

    retTexObj = new GFXOpenGLES20iOSTextureObject( GFX, profile, texture );
    retTexObj->registerResourceWithDevice( GFX );

    if ( retTexObj )
    {
        // Store the path for later use.
        retTexObj->mPath = fullPath;
        _linkTexture( retTexObj );
    }
    else
    {
        Con::printf("unable to make texture");
    }
    
    return retTexObj;
}


GFXTextureObject *GFXOpenGLES20iOSTextureManager::_createTextureObject(   U32 height,
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex,
                                                               void* data)
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXOpenGLES20iOSTextureManager::_createTexture - invalid format!");

   GFXOpenGLES20iOSTextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXOpenGLES20iOSTextureObject*>( inTex ), "GFXOpenGLES20iOSTextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXOpenGLES20iOSTextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXOpenGLES20iOSTextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips, data);

   return retTex;
}

//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

static void _slowTextureLoad(GFXOpenGLES20iOSTextureObject* texture, GBitmap* pDL)
{
   glTexSubImage2D(texture->getBinding(), 0, 0, 0, pDL->getWidth(0), pDL->getHeight(0), GFXGLTextureFormat[pDL->getFormat()], GFXGLTextureType[pDL->getFormat()], pDL->getBits(0));
}

bool GFXOpenGLES20iOSTextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   GFXOpenGLES20iOSTextureObject *texture = static_cast<GFXOpenGLES20iOSTextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXOpenGLES20iOSTextureManager::_loadTexture(GBitmap) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   // Bind to edit
   device->setTextureUnit(0);
   glBindTexture(texture->getBinding(), texture->getHandle());
   
   texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    _slowTextureLoad(texture, pDL);
   
   glBindTexture(texture->getBinding(), 0);
   
   return true;
}


bool GFXOpenGLES20iOSTextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   if(aTexture->getDepth() < 1)
      return false;
   
   GFXOpenGLDevice *device = dynamic_cast<GFXOpenGLDevice*>(GFX);
   GFXOpenGLES20iOSTextureObject* texture = static_cast<GFXOpenGLES20iOSTextureObject*>(aTexture);
   
   device->setTextureUnit(0);

   glBindTexture(GL_TEXTURE_2D, texture->getHandle());
   glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   glBindTexture(GL_TEXTURE_2D, 0);
   
   return true;
}

bool GFXOpenGLES20iOSTextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXOpenGLES20iOSTextureObject*>(texture)->zombify();
   else
      static_cast<GFXOpenGLES20iOSTextureObject*>(texture)->release();
      
   return true;
}

bool GFXOpenGLES20iOSTextureManager::_refreshTexture(GFXTextureObject *texture)
{
    GFXOpenGLES20iOSTextureObject * pTextureObject = dynamic_cast<GFXOpenGLES20iOSTextureObject*>(texture);

    if (pTextureObject->mBitmap == nullptr)
    {
        NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft: @NO};
        NSString *npath = [[NSString alloc] initWithUTF8String:pTextureObject->mPath.c_str()];
        NSError * error;
        GLKTextureInfo *textureInfo = nil;
        textureInfo = [GLKTextureLoader textureWithContentsOfFile:npath
                                                      options:options error:&error];
        
        if (textureInfo)
        {
            pTextureObject->mTextureSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
            pTextureObject->mBitmapSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
            pTextureObject->mBinding = [ textureInfo target];
            pTextureObject->mHandle = [ textureInfo name ];
            return true;
        }
    }
    
    // Fetch bitmaps.
    GBitmap* pSourceBitmap = texture->mBitmap;
    GBitmap* pNewBitmap = pSourceBitmap->createPowerOfTwoBitmap();
    
    bool isCompressed = (pNewBitmap->getFormat() >= GFXFormat_PVR2) && (pNewBitmap->getFormat() <= GFXFormat_PVR4);
    
    if (isCompressed)
    {
        switch (pNewBitmap->getFormat()) {
            case GFXFormat_PVR2:
            case GFXFormat_PVR2A:
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GFXGLTextureType[pNewBitmap->getFormat()],
                                       pNewBitmap->getWidth(), pNewBitmap->getHeight(), 0, (std::max((int)pNewBitmap->getWidth(),16) * std::max((int)pNewBitmap->getHeight(), 8) * 2 + 7) / 8, pNewBitmap->getBits() );
                break;
            case GFXFormat_PVR4:
            case GFXFormat_PVR4A:
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GFXGLTextureType[pNewBitmap->getFormat()],
                                       pNewBitmap->getWidth(), pNewBitmap->getHeight(), 0, (std::max((int)pNewBitmap->getWidth(),8) * std::max((int)pNewBitmap->getHeight(), 8) * 4 + 7) / 8, pNewBitmap->getBits() );
                break;
            default:
                // already tested for range of values, so default is just to keep the compiler happy!
                break;
        }
    }
    else
    {
        // Bind texture.
        glBindTexture( GL_TEXTURE_2D, pTextureObject->getBinding() );
    }
    
    // Are we forcing to 16-bit?
    if( pSourceBitmap->mForce16Bit )
    {
        // Yes, so generate a 16-bit texture.
        GFXFormat GLformat;
        
        U16* pBitmap16 = pSourceBitmap->create16BitBitmap( &GLformat );
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GLformat,
                     pNewBitmap->getWidth(), pNewBitmap->getHeight(),
                     0,
                     GFXGLTextureFormat[GLformat],
                     GFXGLTextureType[GLformat],
                     pBitmap16
                     );
        
        //copy new texture_data into pBits
        delete [] pBitmap16;
    }
    else
    {
        // No, so upload as-is.
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GFXGLTextureFormat[pNewBitmap->getFormat()],
                     pNewBitmap->getWidth(), pNewBitmap->getHeight(),
                     0,
                     GFXGLTextureInternalFormat[pNewBitmap->getFormat()],
                     GFXGLTextureType[pNewBitmap->getFormat()],
                     pNewBitmap->getBits());
    }
    
   pTextureObject->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   pTextureObject->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
   GLenum glClamp;
   if ( pTextureObject->getClamp() )
      glClamp = GL_CLAMP_TO_EDGE;
   else
      glClamp = GL_REPEAT;
   
   pTextureObject->setParameter(GL_TEXTURE_WRAP_S, glClamp );
   pTextureObject->setParameter(GL_TEXTURE_WRAP_T, glClamp );
   
    if(pNewBitmap != pSourceBitmap)
    {
        delete pNewBitmap;
    }
   
   return true;
}
