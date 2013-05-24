//
//  FilterImageAsset.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-24.
//

#ifndef __Torque2D__FilterImageAsset__
#define __Torque2D__FilterImageAsset__

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

#include "2d/assets/ImageAsset.h"

//-----------------------------------------------------------------------------

DefineConsoleType( TypeAnimationAssetPtr )

//-----------------------------------------------------------------------------

class FilterImageAsset : public ImageAsset
{
private:
    typedef ImageAsset Parent;
    
public:
    FilterImageAsset();
    virtual ~FilterImageAsset();

    AssetPtr<ImageAsset>    mImageAsset;
    StringTableEntry        mFilterName;

    // Asset validation.
    virtual bool    isAssetValid( void ) const;
    
    static void initPersistFields();
    virtual bool onAdd();
    virtual void onRemove();
    virtual void copyTo(SimObject* object);
    
    void            setImage( const char* pAssetId );
    void            setFilterName( const char* pAssetId );
    inline const AssetPtr<ImageAsset>& getImage( void ) const           { return mImageAsset; }
    
    /// Declare Console Object.
    DECLARE_CONOBJECT(FilterImageAsset);

protected:
    virtual void initializeAsset( void );
    virtual void onAssetRefresh( void );

private:
    virtual void calculateImage( void );
    
protected:
    static bool setImage( void* obj, const char* data )                         { static_cast<FilterImageAsset*>(obj)->setImage( data ); return false; }
    static bool writeImage( void* obj, StringTableEntry pFieldName )            { return static_cast<FilterImageAsset*>(obj)->mImageAsset.notNull(); }
    static bool setFilterName( void* obj, const char* data )                         { static_cast<FilterImageAsset*>(obj)->setImage( data ); return false; }
    static bool writeFilterName( void* obj, StringTableEntry pFieldName )            { return static_cast<FilterImageAsset*>(obj)->mFilterName; }

};

Vector<StringTableEntry> gFilterIndexNames;

#endif /* defined(__Torque2D__FilterImageAsset__) */
