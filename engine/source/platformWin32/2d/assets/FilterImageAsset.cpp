//
//  FilterImageAsset.mm
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-24.
//

#include "FilterImageAsset.h"
#include "console/consoleTypes.h"

//------------------------------------------------------------------------------

ConsoleType( FilterImageAssetPtr, TypeFilterImageAssetPtr, sizeof(AssetPtr<FilterImageAsset>), ASSET_ID_FIELD_PREFIX )

//-----------------------------------------------------------------------------

ConsoleGetType( TypeFilterImageAssetPtr )
{
    // Fetch asset Id.
    return (*((AssetPtr<FilterImageAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType( TypeFilterImageAssetPtr )
{
    // Was a single argument specified?
    if( argc == 1 )
    {
        // Yes, so fetch field value.
        const char* pFieldValue = argv[0];
        
        // Fetch asset pointer.
        AssetPtr<FilterImageAsset>* pAssetPtr = dynamic_cast<AssetPtr<FilterImageAsset>*>((AssetPtrBase*)(dptr));
        
        // Is the asset pointer the correct type?
        if ( pAssetPtr == NULL )
        {
            // No, so fail.
            Con::warnf( "(TypeFilterImageAssetPtr) - Failed to set asset Id '%d'.", pFieldValue );
            return;
        }
        
        // Set asset.
        pAssetPtr->setAssetId( pFieldValue );
        
        return;
    }
    
    // Warn.
    Con::warnf( "(TypeFilterImageAssetPtr) - Cannot set multiple args to a single asset." );
}

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(FilterImageAsset);

//------------------------------------------------------------------------------

FilterImageAsset::FilterImageAsset()
{

}

//------------------------------------------------------------------------------

FilterImageAsset::~FilterImageAsset()
{
}

//------------------------------------------------------------------------------

void FilterImageAsset::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();
    
    addProtectedField("Image", TypeImageAssetPtr, Offset(mImageAsset, FilterImageAsset), &setImage, &defaultProtectedGetFn, &writeImage, "");
}

//------------------------------------------------------------------------------

bool FilterImageAsset::onAdd()
{
    // Call Parent.
    if(!Parent::onAdd())
        return false;
    
    // Return Okay.
    return true;
}

//------------------------------------------------------------------------------

void FilterImageAsset::onRemove()
{
    // Call Parent.
    Parent::onRemove();
}

//-----------------------------------------------------------------------------


void FilterImageAsset::initializeAsset( void )
{
   // Ensure the image-file is expanded.
   mImageFile = expandAssetFilePath( mImageFile );

   // Call parent.
    Parent::initializeAsset();
}

//------------------------------------------------------------------------------

void FilterImageAsset::onAssetRefresh( void )
{
    // Ignore if not yet added to the sim.
    if ( !isProperlyAdded() )
        return;
    
    // Call parent.
    Parent::onAssetRefresh();
    
    // Compile image.
    calculateImage();
}

//------------------------------------------------------------------------------

void FilterImageAsset::copyTo(SimObject* object)
{
    // Call to parent.
    Parent::copyTo(object);
    
    // Cast to asset.
    FilterImageAsset* pAsset = static_cast<FilterImageAsset*>(object);
    
    // Sanity!
    AssertFatal(pAsset != NULL, "FilterImageAsset::copyTo() - Object is not the correct type.");
    
    // Copy state.
    pAsset->setImage( getImage().getAssetId() );
}

//------------------------------------------------------------------------------

void FilterImageAsset::setImage( const char* pAssetId )
{
    // Ignore no change.
    if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
        return;
    
    // Update.
    mImageAsset = pAssetId;
    
    refreshAsset();

}


void FilterImageAsset::calculateImage( void )
{
    // Clear frames.
    mFrames.clear();
   
    // Get image texture.
   mImageTextureHandle = mImageAsset->getImageTexture().getPointer();

   // Calculate according to mode.
   if ( mExplicitMode )
   {
      calculateExplicitMode();
   }
   else
   {
      calculateImplicitMode();
   }

}

//------------------------------------------------------------------------------

bool FilterImageAsset::isAssetValid( void ) const
{
    return mImageAsset.notNull() && mImageAsset->isAssetValid();
}

//------------------------------------------------------------------------------


// object->getDataField