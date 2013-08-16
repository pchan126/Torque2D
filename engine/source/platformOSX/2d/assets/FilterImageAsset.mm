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
#import <QuartzCore/CoreImage.h>
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
        if ( pAssetPtr == nullptr )
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
    AssertFatal(pAsset != nullptr, "FilterImageAsset::copyTo() - Object is not the correct type.");
}

//------------------------------------------------------------------------------


void FilterImageAsset::setFilterName( const char* pAssetId )
{
//    // Ignore no change.
//    if ( mImageAsset.getAssetId() == StringTable->insert( pAssetId ) )
//        return;
//    
    // Update.
//   NSString *filterString = [[NSString alloc] initWithUTF8String:pAssetId];
//   mFilter = [CIFilter filterWithName:filterString];
//   [filterString release];
   // Update.
   mFilterName = pAssetId;

   // Refresh the asset.
    refreshAsset();
}


void FilterImageAsset::calculateImage( void )
{
    // Debug Profiling.    PROFILE_SCOPE(FilterImageAsset_CalculateImage);
    GFXOpenGL32Device *device = dynamic_cast<GFXOpenGL32Device*>(GFX);
    
    // Clear frames.
    mFrames.clear();
   

   NSString *filterString = [[NSString alloc] initWithUTF8String:mFilterName];
   CIFilter *mFilter = [CIFilter filterWithName:filterString];
   [mFilter setDefaults];
   CGSize texSize;
   texSize.width = 256;
   texSize.height = 256;

    if (mFilter != nil)
    {
       [mFilter setDefaults];
//       [mFilter setValue:input forKey:@"inputImage"];
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
              else if ([value isMemberOfClass:[CIImage class]])
              {
                  const char* temp = expandAssetFilePath(strvalue);
                  CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename(temp);
                  CGImageRef img = CGImageCreateWithPNGDataProvider(dataProvider, NULL, NO, kCGRenderingIntentDefault);
                  CIImage *image = [CIImage imageWithCGImage:img];
                  [mFilter setValue:image forKey:string];
                  texSize.height = CGImageGetHeight(img);
                  texSize.width = CGImageGetWidth(img);
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
    GBitmap *bmap = new GBitmap(texSize.width, texSize.height, false, GFXFormatR32G32B32A32F);
    CGRect rect = CGRectMake(0, 0, texSize.width, texSize.height);

    CIImage *output = [mFilter valueForKey:kCIOutputImageKey];
   CIContext* temp = [[NSGraphicsContext currentContext] CIContext];
//    CIContext *temp = (CIContext*)[[NSOpenGLContext currentContext] CGLContextObj];
   [temp render:output toBitmap:(void*)bmap->getWritableBits() rowBytes:bmap->getWidth()*bmap->mBytesPerPixel bounds:rect format:kCIFormatRGBAf colorSpace:NULL];
//   CGImageRelease(img);
   
   mImageTextureHandle = TEXMGR->createTexture(bmap, getAssetName(), &GFXImageAssetTextureProfile, true );

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
    return true;
}

//------------------------------------------------------------------------------
