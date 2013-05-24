//
//  FilterImageAsset.mm
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-24.
//

#include "FilterImageAsset.h"
#include "console/consoleTypes.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureObject.h"

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
    addProtectedField("Filter", TypeString, Offset(mFilterName, FilterImageAsset), &setFilterName, &defaultProtectedGetFn, &writeFilterName, "");
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
    // Call parent.
    Parent::initializeAsset();
    
    // Ensure the image-file is expanded.
    mImageFile = expandAssetFilePath( mImageFile );
    
    // Calculate the image.
    calculateImage();
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
//    pAsset->setAnimationFrames( Con::getData( TypeS32Vector, (void*)&getSpecifiedAnimationFrames(), 0 ) );
//    pAsset->setAnimationTime( getAnimationTime() );
//    pAsset->setAnimationCycle( getAnimationCycle() );
//    pAsset->setRandomStart( getRandomStart() );
}

//------------------------------------------------------------------------------

void FilterImageAsset::setImage( const char* pAssetId )
{
    // Ignore no change.
    if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
        return;
    
    // Update.
    mImageAsset = pAssetId;
    
    // Refresh the asset.
    refreshAsset();
}


void FilterImageAsset::setFilterName( const char* pAssetId )
{
    // Ignore no change.
    if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
        return;
    
    // Update.
    mFilterName = pAssetId;
    
    // Refresh the asset.
    refreshAsset();
}


void FilterImageAsset::calculateImage( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(FilterImageAsset_CalculateImage);
    GFXOpenGLES20iOSDevice *device = dynamic_cast<GFXOpenGLES20iOSDevice*>(GFX);
    
    // Clear frames.
    mFrames.clear();
    
//    // If we have an existing texture and we're setting to the same bitmap then force the texture manager
//    // to refresh the texture itself.
//    if ( !mImageTextureHandle.IsNull())
//        if (dStricmp(mImageTextureHandle->getTextureKey(), mImageFile) == 0 )
//            mImageTextureHandle.refresh();
    // GFXTextureObject *GFXTextureManager::createTexture(  U32 width, U32 height, void *pixels, GFXFormat format, GFXTextureProfile *profile )
//    {
//        // For now, stuff everything into a GBitmap and pass it off... This may need to be revisited -- BJG
//        GBitmap *bmp = new GBitmap(width, height, 0, format);
//        dMemcpy(bmp->getWritableBits(), pixels, width * height * bmp->mBytesPerPixel);
//        
//        return createTexture( bmp, String::EmptyString, profile, true );
//    }
    
    // Get image texture.
    GFXOpenGLES20iOSTextureObject* texture = dynamic_cast<GFXOpenGLES20iOSTextureObject*>(TEXMGR->createTexture( mImageFile, &GFXImageAssetTextureProfile ));
    mImageTextureHandle = GFXTexHandle( texture );
    
    GFXTextureTarget *texTarget = GFX->allocRenderToTextureTarget();
    
    GFXOpenGLES20iOSTextureObject* intTexture = dynamic_cast<GFXOpenGLES20iOSTextureObject*>(TEXMGR->createTexture( texture->getWidth(), texture->getHeight(), NULL, GFXFormatR8G8B8A8, &GFXImageAssetTextureProfile  ));
    
    texTarget->attachTexture(intTexture);
    
    GFXTarget *oldTarget = GFX->getActiveRenderTarget();
    device->setActiveRenderTarget(texTarget);

    // Is the texture valid?
    if ( mImageTextureHandle.IsNull() )
    {
        // No, so warn.
        Con::warnf( "Image '%s' could not load texture '%s'.", getAssetId(), mImageFile );
        return;
    }
    
    CGSize texSize;
    texSize.height = texture->getHeight();
    texSize.width = texture->getWidth();
    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CIImage *image = [CIImage imageWithTexture:texture->getHandle() size:texSize flipped:FALSE colorSpace:color_space];

    NSString *filterString = [[NSString alloc] initWithUTF8String:mFilterName];
    CIFilter *filter = [CIFilter filterWithName:filterString];
    CGRect rect = CGRectMake(0, 0, texSize.width, texSize.height);
    
    if (filter)
    {
        [filter setValue:image forKey:@"inputImage"];
        NSArray *attrKey = [filter inputKeys];
        for ( NSString *string in attrKey)
        {
            StringTableEntry entry = StringTable->insert(string.UTF8String);
            Con::printf("%s", entry);
            StringTableEntry value = getDataField(entry, NULL);
//            getDataField(<#StringTableEntry slotName#>, <#const char *array#>)
//            [filter setValue: forKey:string];
        }

        @try {
            image = filter.outputImage;
        }
        @catch (NSException *exception) {
            Con::printf("error ");
            return;
        }
    }
    
    // draw Image to textureTarget
    device->drawImage([filter valueForKey:kCIOutputImageKey ], rect, rect);

    GFX->setActiveRenderTarget(oldTarget);
}

//------------------------------------------------------------------------------

bool FilterImageAsset::isAssetValid( void ) const
{
    return mImageAsset.notNull() && mImageAsset->isAssetValid();
}

//------------------------------------------------------------------------------


// object->getDataField