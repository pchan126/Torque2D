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
#include "graphics/gfxTextureManager.h"

#include "graphics/gfxDevice.h"
#include "graphics/gfxCardProfile.h"
#include "graphics/gfxStringEnumTranslate.h"
#include "platform/platformString.h"
#include "memory/safeDelete.h"
#include "io/resource/resourceManager.h"
#include "console/consoleTypes.h"

//#define DEBUG_SPEW

S32 GFXTextureManager::smTextureReductionLevel = 0;

String GFXTextureManager::smMissingTexturePath("core/art/missingTexture");
String GFXTextureManager::smUnavailableTexturePath("core/art/unavailable");
String GFXTextureManager::smWarningTexturePath("core/art/warnmat");

GFXTextureManager::EventSignal GFXTextureManager::smEventSignal;

void GFXTextureManager::init()
{
    Con::addVariable( "$pref::Video::textureReductionLevel", TypeS32, &smTextureReductionLevel);
//,
//      "The number of mipmap levels to drop on loaded textures to reduce "
//      "video memory usage.  It will skip any textures that have been defined "
//      "as not allowing down scaling.\n"
//      "@ingroup GFX\n" );
}

GFXTextureManager::GFXTextureManager()
{
    mMasterTextureKeyIndex = 0;
    mForce16BitTexture = false;
    mAllowTextureCompression = false;
    mDisableTextureSubImageUpdates = false;
    mBitmapResidentSize = 0;
    mTextureResidentSize = 0;
    mTextureResidentWasteSize = 0;
    mTextureResidentCount = 0;

    mListHead = mListTail = NULL;
   mTextureManagerState = GFXTextureManager::NotInitialized;

   // Set up the hash table
   mHashCount = 1023;
   mHashTable = new GFXTextureObject *[mHashCount];
   for(U32 i = 0; i < mHashCount; i++)
      mHashTable[i] = NULL;
}

GFXTextureManager::~GFXTextureManager()
{
   if( mHashTable )
      SAFE_DELETE_ARRAY( mHashTable );

//   mCubemapTable.clear();
}

U32 GFXTextureManager::getTextureDownscalePower( GFXTextureProfile *profile )
{
   if ( !profile || profile->canDownscale() )
      return smTextureReductionLevel;

   return 0;
}

bool GFXTextureManager::validateTextureQuality( GFXTextureProfile *profile, U32 &width, U32 &height )
{
   U32 scaleFactor = getTextureDownscalePower( profile );
   if ( scaleFactor == 0 )
      return true;

   // Otherwise apply the appropriate scale...
   width  >>= scaleFactor;
   height >>= scaleFactor;

   return true;
}

void GFXTextureManager::kill()
{
   AssertFatal( mTextureManagerState != GFXTextureManager::Dead, "Texture Manager already killed!" );

   // Release everything in the cache we can
   // so we don't leak any textures.
   cleanupCache();

   GFXTextureObject *curr = mListHead;
   GFXTextureObject *temp;

   // Actually delete all the textures we know about.
   while( curr != NULL ) 
   {
      temp = curr->mNext;
      curr->kill();
      curr = temp;
   }

//   mCubemapTable.clear();

   mTextureManagerState = GFXTextureManager::Dead;
}

void GFXTextureManager::zombify()
{
   AssertFatal( mTextureManagerState != GFXTextureManager::Dead, "Texture Manager already a zombie!" );

   // Notify everyone that cares about the zombification!
   smEventSignal.trigger( GFXZombify );

   // Release unused pool textures.
   cleanupPool();

   // Release everything in the cache we can.
   cleanupCache();

   // Free all the device copies of the textures.
   GFXTextureObject *temp = mListHead;
   while( temp != NULL ) 
   {
      freeTexture( temp, true );
      temp = temp->mNext;
   }

   // Finally, note our state.
   mTextureManagerState = GFXTextureManager::Dead;
}

void GFXTextureManager::resurrect()
{
   // Reupload all the device copies of the textures.
   GFXTextureObject *temp = mListHead;

   while( temp != NULL ) 
   {
      refreshTexture( temp );
      temp = temp->mNext;
   }

   // Notify callback registries.
   smEventSignal.trigger( GFXResurrect );
   
   // Update our state.
   mTextureManagerState = GFXTextureManager::Alive;
}

void GFXTextureManager::cleanupPool()
{
   PROFILE_SCOPE( GFXTextureManager_CleanupPool );

   TexturePoolMap::iterator iter = mTexturePool.begin();
   for ( ; iter != mTexturePool.end(); )
   {
      if ( iter->second->getRefCount() == 1 )
      {
         // This texture is unreferenced, so take the time
         // now to completely remove it from the pool.
         TexturePoolMap::iterator unref = iter;
         iter++;
         unref->second = NULL;
         mTexturePool.erase( unref );
         continue;
      }

      iter++;
   }
}

void GFXTextureManager::requestDeleteTexture( GFXTextureObject *texture )
{
   assert(texture != nullptr); 

   // If this is a non-cached texture then just really delete it.
   if ( texture->mTextureLookupName.isEmpty() )
   {
      delete texture;
      return;
   }

   // Set the time and store it.
   texture->mDeleteTime = Platform::getTime();
   mToDelete.push_back_unique( texture );
}

void GFXTextureManager::cleanupCache( U32 secondsToLive )
{
   PROFILE_SCOPE( GFXTextureManager_CleanupCache );

   U32 killTime = Platform::getTime() - secondsToLive;

   for ( U32 i=0; i < mToDelete.size(); )
   {
      GFXTextureObject *tex = mToDelete[i];

      // If the texture was picked back up by a user
      // then just remove it from the list.
      if ( tex->getRefCount() != 0 )
      {
         mToDelete.erase( i );
         continue;
      }

      // If its time has expired delete it for real.
      if ( tex->mDeleteTime <= killTime )
      {
         //Con::errorf( "Killed texture: %s", tex->mTextureLookupName.c_str() );
         delete tex;
         mToDelete.erase( i );
         continue;
      }

      i++;
   }
}

GFXTextureObject *GFXTextureManager::_lookupTexture( const char *hashName, const GFXTextureProfile *profile  )
{
   GFXTextureObject *ret = find( hashName );

   // TODO: Profile checking HERE

   return ret;
}


GFXTextureObject *GFXTextureManager::createTexture( GBitmap *bmp, const String &resourceName, GFXTextureProfile *profile, bool deleteBmp )
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

GFXTextureObject *GFXTextureManager::_createTexture(  GBitmap *bmp, 
                                                      const String &resourceName, 
                                                      GFXTextureProfile *profile, 
                                                      bool deleteBmp,
                                                      GFXTextureObject *inObj )
{
   PROFILE_SCOPE( GFXTextureManager_CreateTexture_Bitmap );
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] _createTexture (GBitmap) '%s'",
      resourceName.c_str()
   );
   #endif

   // Massage the bitmap based on any resize rules.
   U32 scalePower = getTextureDownscalePower( profile );

   GBitmap *realBmp = bmp;
   U32 realWidth = bmp->getWidth();
   U32 realHeight = bmp->getHeight();

   if (  scalePower && 
         isPow2(bmp->getWidth()) && 
         isPow2(bmp->getHeight()) && 
         profile->canDownscale() )
   {
      // We only work with power of 2 textures for now, so we 
      // don't have to worry about padding.

      // We downscale the bitmap on the CPU... this is the reason
      // you should be using DDS which already has good looking mips.
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
      ret = _createTextureObject(realHeight, realWidth, 0, realFmt, profile, numMips );

   if(!ret)
   {
      Con::errorf("GFXTextureManager - failed to create texture (1) for '%s'", (resourceName.isNotEmpty() ? resourceName.c_str() : "unknown"));
      return NULL;
   }

   // Extrude mip levels
   // Don't do this for fonts!
   if( ret->mMipLevels > 1 && ( realBmp->getNumMipLevels() == 1 ) && ( realBmp->getFormat() != GFXFormatA8 ) &&
      isPow2( realBmp->getHeight() ) && isPow2( realBmp->getWidth() ) && !profile->noMip() )
   {
      realBmp->extrudeMipLevels(false);
   }

   // If _validateTexParams kicked back a different format, than there needs to be
   // a conversion
   if( realBmp->getFormat() != realFmt )
   {
//      const GFXFormat oldFmt = realBmp->getFormat();

      if( !realBmp->setFormat( realFmt ) )
      {
      }
#ifdef TORQUE_DEBUG
      else
      {
         //Con::warnf( "[GFXTextureManager]: Changed bitmap format from %s to %s.", 
         //   GFXStringTextureFormat[oldFmt], GFXStringTextureFormat[realFmt] );
      }
#endif
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


GFXTextureObject *GFXTextureManager::createTexture( const String &path, GFXTextureProfile *profile )
{
    GBitmap *bitmap;
    GFXTextureObject *retTexObj = NULL;

    bitmap = GBitmap::load( path );
    if( bitmap != NULL )
    {
        retTexObj = createTexture( bitmap, path, profile, false );
        retTexObj->mPath = path;
    }
    
    return retTexObj;
}



GFXTextureObject *GFXTextureManager::createTexture(  U32 width, U32 height, void *pixels, GFXFormat format, GFXTextureProfile *profile )
{
    // For now, stuff everything into a GBitmap and pass it off... This may need to be revisited -- BJG
    GBitmap *bmp = new GBitmap(width, height, 0, format);
    dMemcpy(bmp->getWritableBits(), pixels, width * height * bmp->mBytesPerPixel);
    
    return createTexture( bmp, String::EmptyString, profile, true );
}

GFXTextureObject *GFXTextureManager::createTexture( U32 width, U32 height, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, S32 antialiasLevel )
{
    // Deal with sizing issues...
    U32 localWidth = width;
    U32 localHeight = height;
    
    // TODO: Format check HERE! -patw
    
    validateTextureQuality(profile, localWidth, localHeight);
    
    U32 numMips = numMipLevels;
    GFXFormat checkFmt = format;
    _validateTexParams( localWidth, localHeight, profile, numMips, checkFmt );
    
    AssertFatal( checkFmt == format, "Anonymous texture didn't get the format it wanted." );
    
    GFXTextureObject *outTex = NULL;
    
    // If this is a pooled profile then look there first.
    if ( profile->isPooled() )
    {
        outTex = _findPooledTexure(   localWidth, localHeight, checkFmt,
                                   profile, numMips, antialiasLevel );
        
        // If we got a pooled texture then its
        // already setup... just return it.
        if ( outTex )
            return outTex;
    }
    
    // Create the texture if we didn't get one from the pool.
    if ( !outTex )
    {
        outTex = _createTextureObject( localHeight, localWidth, 0, format, profile, numMips, false, antialiasLevel );
        
        // Make sure we add it to the pool.
        if ( outTex && profile->isPooled() )
            mTexturePool.insertEqual( profile, outTex );
    }
    
    if ( !outTex )
    {
        Con::errorf("GFXTextureManager - failed to create anonymous texture.");
        return NULL;
    }
    
    // And do book-keeping...
    //    - texture info
    outTex->mBitmapSize.set(localWidth, localHeight, 0);
    outTex->mAntialiasLevel = antialiasLevel;
    
    // PWTODO: Need to assign this a lookup name before _linkTexture() is called
    // otherwise it won't get a hash insert call
    
    _linkTexture( outTex );
    
    return outTex;
}

GFXTextureObject *GFXTextureManager::createTexture(   U32 width,
                                                   U32 height,
                                                   U32 depth,
                                                   void *pixels,
                                                   GFXFormat format,
                                                   GFXTextureProfile *profile )
{
    PROFILE_SCOPE( GFXTextureManager_CreateTexture_3D );
    
    // Create texture...
    GFXTextureObject *ret = _createTextureObject( height, width, depth, format, profile, 1 );
    
    if(!ret)
    {
        Con::errorf("GFXTextureManager - failed to create anonymous texture.");
        return NULL;
    }
    
    // Call the internal load...
    if( !_loadTexture( ret, pixels ) )
    {
        Con::errorf("GFXTextureManager - failed to load volume texture" );
        return NULL;
    }
    
    
    // And do book-keeping...
    //    - texture info
    ret->mBitmapSize.set( width, height, depth );
    
    _linkTexture( ret );
    
    
    // Return the new texture!
    return ret;
}

GFXTextureObject* GFXTextureManager::_findPooledTexure(  U32 width,
                                                         U32 height, 
                                                         GFXFormat format, 
                                                         GFXTextureProfile *profile,
                                                         U32 numMipLevels,
                                                         S32 antialiasLevel )
{
   PROFILE_SCOPE( GFXTextureManager_FindPooledTexure );

   GFXTextureObject *outTex;

   // First see if we have a free one in the pool.
   TexturePoolMap::iterator iter = mTexturePool.find( profile );
   for ( ; iter != mTexturePool.end() && iter->first == profile; iter++ )
   {
      outTex = iter->second;

      // If the reference count is 1 then we're the only
      // ones holding on to this texture and we can hand
      // it out if the size matches... else its in use.
      if ( outTex->getRefCount() != 1 )
         continue;

      // Check for a match... if so return it.  The assignment
      // to a GFXTexHandle will take care of incrementing the
      // reference count and keeping it from being handed out
      // to anyone else.
      if (  outTex->getFormat() == format &&
            outTex->getWidth() == width &&
            outTex->getHeight() == height &&            
            outTex->getMipLevels() == numMipLevels &&
            outTex->mAntialiasLevel == antialiasLevel )
         return outTex;
   }

   return NULL;
}

void GFXTextureManager::hashInsert( GFXTextureObject *object )
{
   if ( object->mTextureLookupName.isEmpty() )
      return;
      
   U32 key = object->mTextureLookupName.getHashCaseInsensitive() % mHashCount;
   object->mHashNext = mHashTable[key];
   mHashTable[key] = object;
}

void GFXTextureManager::hashRemove( GFXTextureObject *object )
{
   if ( object->mTextureLookupName.isEmpty() )
      return;

   U32 key = object->mTextureLookupName.getHashCaseInsensitive() % mHashCount;
   GFXTextureObject **walk = &mHashTable[key];
   while(*walk)
   {
      if(*walk == object)
      {
         *walk = object->mHashNext;
         break;
      }
      walk = &((*walk)->mHashNext);
   }
}

GFXTextureObject* GFXTextureManager::find( const String &name )
{
   if ( name.isEmpty() )
      return NULL;

   U32 key = name.getHashCaseInsensitive() % mHashCount;
   GFXTextureObject *walk = mHashTable[key];
   for(; walk; walk = walk->mHashNext)
   {
      if( walk->mTextureLookupName.equal( name, String::NoCase ) )
         break;
   }

   return walk;
}

void GFXTextureManager::freeTexture(GFXTextureObject *texture, bool zombify)
{
   // Ok, let the backend deal with it.
   _freeTexture(texture, zombify);
}

void GFXTextureManager::refreshTexture(GFXTextureObject *texture)
{
   _refreshTexture(texture);
}

void GFXTextureManager::_linkTexture( GFXTextureObject *obj )
{
   // info for the profile
   GFXTextureProfile::updateStatsForCreation(obj);

   // info for the cache
   hashInsert(obj);

   // info for the master list
   if( mListHead == NULL )
      mListHead = obj;

   if( mListTail != NULL ) 
      mListTail->mNext = obj;

   obj->mPrev = mListTail;
   mListTail = obj;
}

void GFXTextureManager::deleteTexture( GFXTextureObject *texture )
{
   if ( mTextureManagerState == GFXTextureManager::Dead )
      return;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GFXTextureManager] deleteTexture '%s'",
      texture->mTextureLookupName.c_str()
   );
   #endif

   if( mListHead == texture )
      mListHead = texture->mNext;
   if( mListTail == texture )
      mListTail = texture->mPrev;

   hashRemove( texture );

   GFXTextureProfile::updateStatsForDeletion(texture);

//   freeTexture( texture );
}

void GFXTextureManager::_validateTexParams( const U32 width, const U32 height, 
                                          const GFXTextureProfile *profile, 
                                          U32 &inOutNumMips,
                                          GFXFormat &inOutFormat  )
{
   // Validate mipmap parameter. If this profile requests no mips, set mips to 1.
   if( profile->noMip() )
   {
      inOutNumMips = 0;
   }
   else if( !isPow2( width ) || !isPow2( height ) )
   {
      // If a texture is not power-of-2 in size for both dimensions, it must
      // have only 1 mip level.
      inOutNumMips = 0;
   }
   
   // Check format, and compatibility with texture profile requirements
   bool autoGenSupp = ( inOutNumMips == 0 );

   // If the format is non-compressed, and the profile requests a compressed format
   // than change the format.
   GFXFormat testingFormat = inOutFormat;
   if( profile->getCompression() != GFXTextureProfile::None )
   {
      const int offset = profile->getCompression() - GFXTextureProfile::DXT1;
      testingFormat = GFXFormat( GFXFormatDXT1 + offset );

      // No auto-gen mips on compressed textures
      autoGenSupp = false;
   }

   // inOutFormat is not modified by this method
   bool chekFmt = GFX->getCardProfiler()->checkFormat( testingFormat, profile, autoGenSupp ); 
   
   if( !chekFmt )
   {
      // It tested for a compressed format, and didn't like it
      if( testingFormat != inOutFormat && profile->getCompression() )
         testingFormat = inOutFormat; // Reset to requested format, and try again

      // Trying again here, so reset autogen mip
      autoGenSupp = ( inOutNumMips == 0 );

      // Wow more weak sauce. There should be a better way to do this.
      switch( inOutFormat )
      {
         case GFXFormatR8G8B8:
            testingFormat = GFXFormatR8G8B8X8;
            chekFmt = GFX->getCardProfiler()->checkFormat( testingFormat, profile, autoGenSupp );
            break;

         case GFXFormatA8:
            testingFormat = GFXFormatR8G8B8A8;
            chekFmt = GFX->getCardProfiler()->checkFormat( testingFormat, profile, autoGenSupp );
            break;
         
         default:
            chekFmt = GFX->getCardProfiler()->checkFormat( testingFormat, profile, autoGenSupp );
            break;
      }
   }

   // Write back num mips that need to be generated by GBitmap
   if( !chekFmt )
      Con::errorf( "Format %s not supported with specified profile.", GFXStringTextureFormat[inOutFormat] );
   else
   {
      inOutFormat = testingFormat;

      // If auto gen mipmaps were requested, and they aren't supported for whatever
      // reason, than write out the number of mips that need to be generated.
      //
      // NOTE: Does this belong here?
      if( inOutNumMips == 0 && !autoGenSupp )
      {
         U32 currWidth  = width;
         U32 currHeight = height;

         inOutNumMips = 1;
         do 
         {
            currWidth  >>= 1;
            currHeight >>= 1;
            if( currWidth == 0 )
               currWidth  = 1;
            if( currHeight == 0 ) 
               currHeight = 1;

            inOutNumMips++;
         } while ( currWidth != 1 || currHeight != 1 );
      }
   }
}

GFXCubemap* GFXTextureManager::createCubemap( const String &path )
{
//   // Very first thing... check the cache.
//   CubemapTable::iterator iter = mCubemapTable.find( path.getFullPath() );
//   if ( iter != mCubemapTable.end() )
//      return iter->value;
//
//   // Not in the cache... we have to load it ourselves.
//
//   const U32 scalePower = getTextureDownscalePower( NULL );
//
   GFXCubemap *cubemap = GFX->createCubemap();
//   cubemap->initStatic( dds );
//   cubemap->_setPath( path.getFullPath() );
//
//   // Store the cubemap into the cache.
//   mCubemapTable.insertUnique( path.getFullPath(), cubemap );

   return cubemap;
}

void GFXTextureManager::releaseCubemap( GFXCubemap *cubemap )
{
   if ( mTextureManagerState == GFXTextureManager::Dead )
      return;
//
//   const String &path = cubemap->getPath();
//
//   CubemapTable::iterator iter = mCubemapTable.find( path );
//   if ( iter != mCubemapTable.end() && iter->value == cubemap )
//      mCubemapTable.erase( iter );
//
//   // If we have a path for the texture then
//   // remove change notifications for it.
//   //Path texPath = texture->getPath();
//   //if ( !texPath.empty() )
//      //FS::RemoveChangeNotification( texPath, this, &GFXTextureManager::_onFileChanged );
}


void GFXTextureManager::reloadTextures()
{
   GFXTextureObject *tex = mListHead;

   while ( tex != NULL ) 
   {
      const String path( tex->mPath );
      if ( !path.isEmpty() )
      {
//         const U32 scalePower = getTextureDownscalePower( tex->mProfile );

        GBitmap *bmp = GBitmap::load( path );
        if( bmp )
           _createTexture( bmp, tex->mTextureLookupName, tex->mProfile, false, tex );
      }

      tex = tex->mNext;
   }
}

ConsoleFunction( flushTextureCache, void, 2, 2,
   "Releases all textures and resurrects the texture manager.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->zombify();
   TEXMGR->resurrect();
}

ConsoleFunction( cleanupTexturePool, void, 2, 2,
   "Release the unused pooled textures in texture manager freeing up video memory.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->cleanupPool();
}

ConsoleFunction( reloadTextures, void, 2, 2,
   "Reload all the textures from disk.\n"
   "@ingroup GFX\n" )
{
   if ( !GFX || !TEXMGR )
      return;

   TEXMGR->reloadTextures();
}
