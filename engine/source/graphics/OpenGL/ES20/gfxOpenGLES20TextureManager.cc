//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformGL.h"
#include "./GFXOpenGLES20TextureManager.h"
#include "./GFXOpenGLES20EnumTranslate.h"
#include "graphics/gfxCardProfile.h"
#include "memory/safeDelete.h"
#include "./GFXOpenGLES20Utils.h"


//#define EXT_ARRAY_SIZE 4
//static const char* extArray[EXT_ARRAY_SIZE] = { "png", "pvr", "jpg", ""};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXOpenGLES20TextureManager::GFXOpenGLES20TextureManager()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXOpenGLES20TextureManager::~GFXOpenGLES20TextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}


GFXTextureObject *GFXOpenGLES20TextureManager::createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp )
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


GFXTextureObject *GFXOpenGLES20TextureManager::_createTexture(  GBitmap *bmp,
                                                    const String &resourceName,
                                                    GFXTextureProfile *profile,
                                                    bool deleteBmp,
                                                    GFXTextureObject *inObj )
{
    PROFILE_SCOPE( GFXOpenGLES20TextureManager_CreateTexture_Bitmap );
    
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
        scalePower = getMin( scalePower, padBmp->getNumMipLevels() - 1 );
        
        realWidth  = getMax( (U32)1, padBmp->getWidth() >> scalePower );
        realHeight = getMax( (U32)1, padBmp->getHeight() >> scalePower );
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
        ret = _createTextureObject(realHeight, realWidth, 0, realFmt, profile, numMips );
   
    if(!ret)
    {
        Con::errorf("GFXTextureManager - failed to create texture (1) for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
        return NULL;
    }

   GFXOpenGLES20TextureObject* retTex = dynamic_cast<GFXOpenGLES20TextureObject*>(ret);

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
        return NULL;
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



GFXTextureObject *GFXOpenGLES20TextureManager::_createTextureObject(   U32 height,
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXOpenGLES20TextureManager::_createTexture - invalid format!");

   GFXOpenGLES20TextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXOpenGLES20TextureObject*>( inTex ), "GFXOpenGLES20TextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXOpenGLES20TextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXOpenGLES20TextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips);

   return retTex;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXOpenGLES20TextureManager::innerCreateTexture( GFXOpenGLES20TextureObject *retTex, 
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
   
    GLenum binding = GL_TEXTURE_2D;   // No 3d texture in openGL es 2.0
   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
      retTex->mIsNPoT2 = true;
   retTex->mBinding = binding;
   
   // Bind it
   glActiveTexture(GL_TEXTURE0);
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
   
   AssertFatal(GFXGLTextureInternalFormat[format] != GL_ZERO, "GFXOpenGLES20TextureManager::innerCreateTexture - invalid internal format");
   AssertFatal(GFXGLTextureFormat[format] != GL_ZERO, "GFXOpenGLES20TextureManager::innerCreateTexture - invalid format");
   AssertFatal(GFXGLTextureType[format] != GL_ZERO, "GFXOpenGLES20TextureManager::innerCreateTexture - invalid type");
   
    glTexImage2D(binding, 0, GFXGLTextureInternalFormat[format], width, height, 0, GFXGLTextureFormat[format], GFXGLTextureType[format], NULL);
   
   retTex->mTextureSize.set(width, height, 0);
}


//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

static void _slowTextureLoad(GFXOpenGLES20TextureObject* texture, GBitmap* pDL)
{
   glTexSubImage2D(texture->getBinding(), 0, 0, 0, pDL->getWidth(0), pDL->getHeight(0), GFXGLTextureFormat[pDL->getFormat()], GFXGLTextureType[pDL->getFormat()], pDL->getBits(0));
}

bool GFXOpenGLES20TextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   GFXOpenGLES20TextureObject *texture = static_cast<GFXOpenGLES20TextureObject*>(aTexture);
   
   AssertFatal(texture->getBinding() == GL_TEXTURE_2D, 
      "GFXOpenGLES20TextureManager::_loadTexture(GBitmap) - This method can only be used with 2D textures");
      
   if(texture->getBinding() != GL_TEXTURE_2D)
      return false;
         
   // No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   // Bind to edit
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(texture->getBinding(), texture->getHandle());
   
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    _slowTextureLoad(texture, pDL);
   
   glBindTexture(texture->getBinding(), 0);
   
   return true;
}


bool GFXOpenGLES20TextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   if(aTexture->getDepth() < 1)
      return false;
   
   GFXOpenGLES20TextureObject* texture = static_cast<GFXOpenGLES20TextureObject*>(aTexture);
   
   glActiveTexture(GL_TEXTURE0);

   glBindTexture(GL_TEXTURE_2D, texture->getHandle());
   glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->getWidth(), texture->getHeight(), GFXGLTextureFormat[texture->mFormat], GFXGLTextureType[texture->mFormat], raw);
   glBindTexture(GL_TEXTURE_2D, 0);
   
   return true;
}

bool GFXOpenGLES20TextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXOpenGLES20TextureObject*>(texture)->zombify();
   else
      static_cast<GFXOpenGLES20TextureObject*>(texture)->release();
      
   return true;
}

bool GFXOpenGLES20TextureManager::_refreshTexture(GFXTextureObject *texture)
{
    GFXOpenGLES20TextureObject * pTextureObject = dynamic_cast<GFXOpenGLES20TextureObject*>(texture);

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
                                       pNewBitmap->getWidth(), pNewBitmap->getHeight(), 0, (getMax((int)pNewBitmap->getWidth(),16) * getMax((int)pNewBitmap->getHeight(), 8) * 2 + 7) / 8, pNewBitmap->getBits() );
                break;
            case GFXFormat_PVR4:
            case GFXFormat_PVR4A:
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GFXGLTextureType[pNewBitmap->getFormat()],
                                       pNewBitmap->getWidth(), pNewBitmap->getHeight(), 0, (getMax((int)pNewBitmap->getWidth(),8) * getMax((int)pNewBitmap->getHeight(), 8) * 4 + 7) / 8, pNewBitmap->getBits() );
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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
    GLenum glClamp;
    if ( pTextureObject->getClamp() )
        glClamp = GL_CLAMP_TO_EDGE;
    else
        glClamp = GL_REPEAT;
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glClamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glClamp );
    
    if(pNewBitmap != pSourceBitmap)
    {
        delete pNewBitmap;
    }
   
   return true;
}
