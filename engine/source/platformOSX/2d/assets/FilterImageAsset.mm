//
//  FilterImageAsset.mm
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-24.
//

#include "FilterImageAsset.h"
#include "console/consoleTypes.h"
#include "platformOSX/graphics/gfxOpenGL32Device.h"
#include "platformOSX/graphics/gfxOpenGL32TextureObject.h"
#import <QuartzCore/QuartzCore.h>

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
   mFilter = nil;
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
    
//   // Finish if we don't have a valid image asset.
//   if ( mImageAsset.isNull() )
//      return;

   // Refresh the asset.
    refreshAsset();

//   // Ignore no change.
//   if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
//      return;
//   
//   // Update.
//   mImageAsset = pAssetId;
//   
//   // Validate frames.
//   validateFrames();
//   
//   // Refresh the asset.
//   refreshAsset();

}


void FilterImageAsset::setFilterName( const char* pAssetId )
{
//    // Ignore no change.
//    if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
//        return;
//    
    // Update.
    mFilterName = pAssetId;
    
   NSString *filterString = [[NSString alloc] initWithUTF8String:mFilterName];
   mFilter = [CIFilter filterWithName:filterString];

   // Refresh the asset.
    refreshAsset();
}


void FilterImageAsset::calculateImage( void )
{
    // Debug Profiling.    PROFILE_SCOPE(FilterImageAsset_CalculateImage);
    GFXOpenGL32Device *device = dynamic_cast<GFXOpenGL32Device*>(GFX);
    
    // Clear frames.
    mFrames.clear();
   
    // Get image texture.
    
    GFXOpenGLTextureObject* texture = dynamic_cast<GFXOpenGLTextureObject*>(mImageAsset->getImageTexture().getPointer());
   
   // Is the texture valid?
   if ( texture == NULL )
   {
      // No, so warn.
      Con::warnf( "Image '%s' could not load texture '%s'.", getAssetId(), mImageFile );
      return;
   }

    GFXTextureTarget *texTarget = GFX->allocRenderToTextureTarget();
    
    mImageTextureHandle = TEXMGR->createTexture( texture->getWidth(), texture->getHeight(), GFXFormatR8G8B8A8, &GFXImageAssetTextureProfile, 0, 0 );
    GFXOpenGLTextureObject* outTexture = dynamic_cast<GFXOpenGLTextureObject*>(mImageTextureHandle.getPointer());
   
    texTarget->attachTexture(mImageTextureHandle);
//   texTarget->attachTexture(texture);
   
    GFXTarget *oldTarget = GFX->getActiveRenderTarget();
    device->setActiveRenderTarget(texTarget);
    GFX->updateStates(true);
   
    CGSize texSize;
    texSize.height = texture->getHeight();
    texSize.width = texture->getWidth();
    CIImage *input = [CIImage imageWithTexture:texture->getHandle() size:texSize flipped:FALSE colorSpace:nil];
    CGRect rect = CGRectMake(0, 0, texSize.width, texSize.height);
    
    if (mFilter != nil)
    {
       [mFilter setDefaults];
       [mFilter setValue:input forKey:@"inputImage"];
        for ( NSString *string in [mFilter inputKeys])
        {
           NSDictionary* info = [mFilter.attributes objectForKey:string];
           StringTableEntry strvalue = getDataField(StringTable->insert(string.UTF8String), NULL);
           Con::printf("%s: %s", string.UTF8String, strvalue);
           
           if (strvalue != NULL)
           {
              NSObject *value = [NSClassFromString([info objectForKey:kCIAttributeClass]) alloc];
              NSString *aType = [info objectForKey:kCIAttributeType];
              if ([value isMemberOfClass:[NSNumber class]])
              {
                 if ([aType isEqualToString:kCIAttributeTypeTime])
                 {
                    [mFilter setValue:[NSNumber numberWithFloat:mClamp(dAtof(strvalue), 0.0, 1.0)] forKey:string];
                 }
                 else
                 {
                    [mFilter setValue:[NSNumber numberWithFloat:dAtof(strvalue)] forKey:string];
                 }
              }
              else if ([value isMemberOfClass:[CIVector class]])
              {
                 if ([aType isEqualToString:kCIAttributeTypePosition] || [aType isEqualToString:kCIAttributeTypeOffset])
                 {
                    if (StringUnit::getUnitCount(strvalue, " ") == 2)
                    {
                       char buffer[128];
                       dSprintf( buffer, 127, "[%s]", strvalue);
                       [mFilter setValue:[CIVector vectorWithString:[[NSString alloc] initWithUTF8String:buffer ]] forKey:string];
                    }
                 }
                 else if ([aType isEqualToString:kCIAttributeTypePosition3])
                 {
                    if (StringUnit::getUnitCount(strvalue, " ") == 3)
                    {
                       char buffer[128];
                       dSprintf( buffer, 127, "[%s]", strvalue);
                       [mFilter setValue:[CIVector vectorWithString:[[NSString alloc] initWithUTF8String:buffer ]] forKey:string];
                    }
                 }
                 else if ([aType isEqualToString:kCIAttributeTypeRectangle])
                 {
                    if (StringUnit::getUnitCount(strvalue, " ") == 4)
                    {
                       char buffer[128];
                       dSprintf( buffer, 127, "[%s]", strvalue);
                       [mFilter setValue:[CIVector vectorWithString:[[NSString alloc] initWithUTF8String:buffer ]] forKey:string];
                    }
                 }
              }
              else if ([value isMemberOfClass:[NSAffineTransform class]])
              {
                 if ([aType isEqualToString:kCIInputTransformKey])
                 {
                    if (StringUnit::getUnitCount(strvalue, " ") == 6)
                     {
                        CGAffineTransform xform;
                        xform.a = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 0, " "));
                        xform.b = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 1, " "));
                        xform.c = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 2, " "));
                        xform.d = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 3, " "));
                        xform.tx = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 4, " "));
                        xform.ty = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 5, " "));
                        [mFilter setValue:[NSValue valueWithBytes:&xform objCType:@encode(CGAffineTransform)] forKey:string];
                     }
                 }
              }
              else if ([value isMemberOfClass:[CIColor class]])
              {
                 CGFloat r = 1.0;
                 CGFloat g = 1.0;
                 CGFloat b = 1.0;
                 CGFloat a = 1.0;
                 if (StringUnit::getUnitCount(strvalue, " ") >= 3)
                 {
                    r = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 0, " "));
                    g = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 1, " "));
                    b = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 2, " "));
                 }
                 if (StringUnit::getUnitCount(strvalue, " ") == 4)
                 {
                    a = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 3, " "));
                 }
                 [mFilter setValue:[CIColor colorWithRed:r green:g blue:b alpha:a ] forKey:string];
              }
           }
        }
    }
    
    CIImage *output = [mFilter valueForKey:kCIOutputImageKey];
    // draw Image to textureTarget
    device->drawImage(output, rect, rect);

    GFX->setActiveRenderTarget(oldTarget);
    GFX->updateStates(true);


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