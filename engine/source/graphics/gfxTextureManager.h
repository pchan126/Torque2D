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

#ifndef _GFXTEXTUREMANAGER_H_
#define _GFXTEXTUREMANAGER_H_

#include "graphics/gfxTextureObject.h"
#include "graphics/gBitmap.h"
#include "io/resource/resourceManager.h"
#include "collection/hashTable.h"
#include "sim/simBase.h"
#include "delegates/delegateSignal.h"
#include <memory>

class GFXCubemap;

class GFXTextureManager : public SimObject
{   
public:
    typedef SimObject Parent;
   
   enum
   {
      AA_MATCH_BACKBUFFER = -1
   };

   GFXTextureManager();
   virtual ~GFXTextureManager();

   /// Set up some global script interface stuff.
   static void init();

   /// Update width and height based on available resources.
   ///
   /// We provide a simple interface for managing texture memory usage. Specifically,
   /// if the total video memory is below a certain threshold, we scale all texture
   /// resolutions down by a specific factor (you can specify different scale factors
   /// for different types of textures).
   ///
   /// @note The base GFXTextureManager class provides all the logic to do this scaling. 
   ///       Subclasses need only implement getTotalVideoMemory().
   ///
   /// @param  type     Type of the requested texture. This is used to determine scaling factors.
   /// @param  width    Requested width - is changed to the actual width that should be used.
   /// @param  height   Requested height - is changed to the actual height that should be used.
   /// @return True if the texture request should be granted, false otherwise.
   virtual bool validateTextureQuality(GFXTextureProfile *profile, U32 &width, U32 &height);

   ///
   static U32 getTextureDownscalePower( GFXTextureProfile *profile );

   virtual GFXTexHandle createTexture(GBitmap *bmp,
           const String &resourceName,
           GFXTextureProfile *profile,
           bool deleteBmp);

   virtual GFXTexHandle createTexture(const String &path,
           GFXTextureProfile *profile);

   virtual GFXTexHandle createTexture(U32 width,
           U32 height,
           void *pixels,
           GFXFormat format,
           GFXTextureProfile *profile);

   virtual GFXTexHandle createTexture(U32 width,
           U32 height,
           U32 depth,
           void *pixels,
           GFXFormat format,
           GFXTextureProfile *profile);

   virtual GFXTexHandle createTexture(U32 width,
           U32 height,
           GFXFormat format,
           GFXTextureProfile *profile,
           U32 numMipLevels,
           S32 antialiasLevel);

   void deleteTexture(GFXTexHandle texture);
   void reloadTexture(GFXTexHandle texture);

   /// Request that the texture be deleted which will
   /// either occur immediately or delayed if its cached.
   void requestDeleteTexture(GFXTexHandle texture);

   /// @name Texture Necromancy
   /// 
   /// Texture necromancy in three easy steps:
   /// - If you want to destroy the texture manager, call kill().
   /// - If you want to switch resolutions, or otherwise reset the device, call zombify().
   /// - When you want to bring the manager back from zombie state, call resurrect().
   /// @{

   ///
   void kill();
   void zombify();
   void resurrect();

   /// This releases any pooled textures which are 
   /// currently unused freeing up video memory.
   void cleanupPool();

   ///
   void reloadTextures();

   /// This releases cached textures that have not
   /// been referenced for a period of time.
   void cleanupCache( U32 secondsToLive = 0 );

   /// Registers a callback for texture zombify and resurrect events.
   /// @see GFXTexCallbackCode
   /// @see removeEventDelegate
   template <class T,class U>
   static void addEventDelegate( T obj, U func );

   /// Unregisteres a texture event callback.
   /// @see addEventDelegate
   template <class T,class U>
   static void removeEventDelegate( T obj, U func ) { smEventSignal.remove( obj, func ); }

   /// @}

   /// Load a cubemap from a texture file.
   GFXCubemap* createCubemap( const String &path );

   /// Used to remove a cubemap from the cache.
   void releaseCubemap( GFXCubemap *cubemap );

protected:

   /// The amount of texture mipmaps to skip when loading a
   /// texture that allows downscaling.
   ///
   /// Exposed to script via $pref::Video::textureReductionLevel.
   ///
   /// @see GFXTextureProfile::PreserveSize
   /// 
   static S32 smTextureReductionLevel;

//   std::unordered_map<std::string, GFXTexHandle> mHashTable;
    std::unordered_map<std::string, std::weak_ptr<GFXTextureObject>> mHashTable;

//   GFXTexHandle mListHead;
//   GFXTexHandle mListTail;
//
//   // We have a hash table for fast texture lookups
//   GFXTexHandle *mHashTable;
//   U32                mHashCount;

   GFXTexHandle      find(std::string name);
   void              hashInsert(GFXTexHandle &object);
   void              hashRemove(GFXTexHandle &object);

   // The cache of loaded cubemap textures.
//   typedef HashTable<String,GFXCubemap*> CubemapTable;
//   CubemapTable mCubemapTable;

   /// The textures waiting to be deleted.
   Vector<GFXTexHandle> mToDelete;

   enum TextureManagerState
   {
       NotInitialized = 0,
       Alive,
       Dead,
       Resurrecting,
//      Living,
//      Zombie,
//      Dead

   } mTextureManagerState;

   /// The texture pool collection type.
   typedef HashTable<GFXTextureProfile*, GFXTexHandle > TexturePoolMap;

   /// All the allocated texture pool textures.
   TexturePoolMap mTexturePool;

   //-----------------------------------------------------------------------
   // Protected methods
   //-----------------------------------------------------------------------

   /// Returns a free texture of the requested attributes from
   /// from the shared texture pool.  It returns NULL if no match
   /// is found.
   GFXTexHandle _findPooledTexture(U32 width,
           U32 height,
           GFXFormat format,
           GFXTextureProfile *profile,
           U32 numMipLevels,
           S32 antialiasLevel);

    GFXTexHandle _createTexture(GBitmap *bmp,
            const String &resourceName,
            GFXTextureProfile *profile,
            bool deleteBmp,
            GFXTexHandle inObj);

   /// Frees the API handles to the texture, for D3D this is a release call
   ///
   /// @note freeTexture MUST NOT DELETE THE TEXTURE OBJECT
   virtual void freeTexture(GFXTexHandle &texture, bool zombify = false);

   virtual void refreshTexture(GFXTexHandle &texture);

   /// @group Internal Texture Manager Interface
   ///
   /// These pure virtual functions are overloaded by each API-specific
   /// subclass.
   ///
   /// The order of calls is:
   /// @code
   /// _createTexture()
   /// _loadTexture
   /// _refreshTexture()
   /// _refreshTexture()
   /// _refreshTexture()
   /// ...
   /// _freeTexture()
   /// @endcode
   ///
   /// @{

   /// Allocate a texture with the internal API.
   ///
   /// @param  height   Height of the texture.
   /// @param  width    Width of the texture.
   /// @param  depth    Depth of the texture. (Will normally be 1 unless
   ///                  we are doing a cubemap or volumetexture.)
   /// @param  format   Pixel format of the texture.
   /// @param  profile  Profile for the texture.
   /// @param  numMipLevels   If not-NULL, then use that many mips.
   ///                        If NULL create the full mip chain
   /// @param  antialiasLevel, Use GFXTextureManager::AA_MATCH_BACKBUFFER to match the backbuffer settings (for render targets that want to share
   ///                         the backbuffer z buffer.  0 for no antialiasing, > 0 for levels that match the GFXVideoMode struct.
   virtual GFXTexHandle _createTextureObject(U32 height,
           U32 width,
           U32 depth,
           GFXFormat format,
           GFXTextureProfile *profile,
           U32 numMipLevels,
           bool forceMips = false,
           S32 antialiasLevel = 0,
           GFXTexHandle inTex = nullptr,
           void *data = nullptr) = 0;

   /// Load data into a texture from a GBitmap using the internal API.
   virtual bool _loadTexture(GFXTexHandle &texture, GBitmap *bmp)=0;

   /// Load data into a texture from a raw buffer using the internal API.
   ///
   /// Note that the size of the buffer is assumed from the parameters used
   /// for this GFXTextureObject's _createTexture call.
   virtual bool _loadTexture(GFXTexHandle &texture, void *raw)=0;

   /// Refresh a texture using the internal API.
   virtual bool _refreshTexture(GFXTexHandle &texture)=0;

   /// Free a texture (but do not delete the GFXTextureObject) using the internal
   /// API.
   ///
   /// This is only called during zombification for textures which need it, so you
   /// don't need to do any internal safety checks.
   virtual bool _freeTexture(GFXTexHandle &texture, bool zombify = false)=0;

   /// @}

   /// Store texture into the hash table cache and linked list.
   void _linkTexture( GFXTexHandle &obj );

   /// Validate the parameters for creating a texture.
   void _validateTexParams( const U32 width, const U32 height, const GFXTextureProfile *profile, 
      U32 &inOutNumMips, GFXFormat &inOutFormat );

   // New texture manager methods for the cleanup work:
   GFXTexHandle _lookupTexture(const char *filename, const GFXTextureProfile *profile);

   /// The texture event signal type.
   typedef Signal<void(GFXTexCallbackCode code)> EventSignal;

   /// The texture event signal.
   static EventSignal smEventSignal;


    // Merge with T2D texture manager
    friend class GFXTextureObject;
        
    public:
        /// Texture manager event codes.
        enum TextureEventCode
        {
            BeginZombification,
            BeginResurrection,
            EndResurrection
        };
        
        typedef void (*TextureEventCallback)(const TextureEventCode eventCode, void *userData);
        
    protected:
        S32 mMasterTextureKeyIndex;
        S32 mTextureResidentWasteSize;
        S32 mTextureResidentSize;
        S32 mTextureResidentCount;
        S32 mBitmapResidentSize;
        bool mForce16BitTexture;
        bool mAllowTextureCompression;
        bool mDisableTextureSubImageUpdates;
            
    public:
        void create();
        void destroy();
        TextureManagerState getManagerState( void ) { return mTextureManagerState; }
        
        void killManager();
        void resurrectManager();
        void flush();
        S32 getBitmapResidentSize( void ) { return mBitmapResidentSize; }
        S32 getTextureResidentSize( void ) { return mTextureResidentSize; }
        S32 getTextureResidentWasteSize( void ) { return mTextureResidentWasteSize; }
        S32 getTextureResidentCount( void ) { return mTextureResidentCount; }
        
        U32  registerEventCallback(TextureEventCallback, void *userData);
        void unregisterEventCallback(const U32 callbackKey);
        
        StringTableEntry getUniqueTextureKey( void );
        
        void dumpMetrics( void );
        
    private:
        void postTextureEvent(const TextureEventCode eventCode);
        void freeTexture( GFXTextureObject* pTextureObject );
        void refresh(GFXTextureObject* pTextureObject);
   
        static F32 getResidentFraction( void );
};


template <class T,class U>
inline void GFXTextureManager::addEventDelegate( T obj, U func ) 
{
   EventSignal::DelegateSig d( obj, func );
   
   AssertFatal( !smEventSignal.contains( d ), 
      "GFXTextureManager::addEventDelegate() - This is already registered!" );

   smEventSignal.notify( d ); 
}

inline void GFXTextureManager::reloadTexture(GFXTexHandle texture)
{
   refreshTexture( texture );
}

/// Returns the GFXTextureManager singleton.  Should only be
/// called after the GFX device has been initialized.
#define TEXMGR GFXDevice::get()->getTextureManager()

#endif // _GFXTEXTUREMANAGER_H_
