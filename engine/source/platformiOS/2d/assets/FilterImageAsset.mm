//
//  FilterImageAsset.mm
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-24.
//

#include "FilterImageAsset.h"
#include "console/consoleTypes.h"
#include "platformiOS/graphics/ES20/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/ES20/gfxOpenGLES20iOSTextureObject.h"

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
    AssertFatal(pAsset != NULL, "FilterImageAsset::copyTo() - Object is not the correct type.");
}

//------------------------------------------------------------------------------


void FilterImageAsset::setFilterName( const char* pAssetId )
{
    // Update.
    mFilterName = pAssetId;
    
   // Refresh the asset.
    refreshAsset();
}


void FilterImageAsset::calculateImage( void )
{
    // Debug Profiling.    PROFILE_SCOPE(FilterImageAsset_CalculateImage);
    GFXOpenGLES20iOSDevice *device = dynamic_cast<GFXOpenGLES20iOSDevice*>(GFX);
    
    // Clear frames.
    mFrames.clear();

    Vector<CGImageRef> imageRefs;

    NSString *filterString = [[NSString alloc] initWithUTF8String:mFilterName];
    CIFilter *mFilter = [CIFilter filterWithName:filterString];
    [mFilter setDefaults];
    CGSize texSize;
    texSize.width = 256;
    texSize.height = 256;

    if (mFilter != nil)
    {
       [mFilter setDefaults];
        for ( NSString *string in [mFilter inputKeys])
        {
           NSDictionary* info = (mFilter.attributes)[string];
           StringTableEntry strvalue = getDataField(StringTable->insert(string.UTF8String), nullptr);
           Con::printf("%s: %s", string.UTF8String, strvalue);

           if (strvalue != nullptr)
           {
              NSObject *value = [NSClassFromString(info[kCIAttributeClass]) alloc];
              NSString *aType = info[kCIAttributeType];
              if ([value isMemberOfClass:[NSNumber class]])
              {
                 if ([aType isEqualToString:kCIAttributeTypeTime])
                 {
                    [mFilter setValue:[NSNumber numberWithFloat:mClamp(dAtof(strvalue), 0.0, 1.0)] forKey:string];
                 }
                 else
                 {
                    [mFilter setValue:@(dAtof(strvalue)) forKey:string];
                 }
              }
              else if ([value isMemberOfClass:[CIImage class]])
              {
                  const char* temp = expandAssetFilePath(strvalue);
                  CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename(temp);
                  CGImageRef img = CGImageCreateWithPNGDataProvider(dataProvider, nullptr, NO, kCGRenderingIntentDefault);
                  CIImage *image = [CIImage imageWithCGImage:img];
                  [mFilter setValue:image forKey:string];
                  texSize.height = CGImageGetHeight(img);
                  texSize.width = CGImageGetWidth(img);
                  imageRefs.push_back(img);
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
              else if ([value isMemberOfClass:[NSValue class]])
              {
                 if ([aType isEqualToString:kCIAttributeTypeTransform])
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
                 GLKVector4 temp;
                 if (StringUnit::getUnitCount(strvalue, " ") >= 3)
                 {
                    temp.r = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 0, " "));
                    temp.g = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 1, " "));
                    temp.b = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 2, " "));
                 }
                 if (StringUnit::getUnitCount(strvalue, " ") == 4)
                 {
                    temp.a = (CGFloat)dAtof(StringUnit::getUnit(strvalue, 3, " "));
                 }
                 else
                 {
                    temp.a = 1.0;
                 }
                 [mFilter setValue:[CIColor colorWithRed:temp.r green:temp.g blue:temp.b alpha:temp.a ] forKey:string];
              }
           }
        }
    }

    GFXTextureTarget *texTarget = GFX->allocRenderToTextureTarget();
    mImageTextureHandle = TEXMGR->createTexture( texSize.width, texSize.height, GFXFormatR8G8B8A8, &GFXImageAssetTextureProfile, 0, 0 );
    texTarget->attachTexture(mImageTextureHandle);

    GFXTarget *oldTarget = GFX->getActiveRenderTarget();
    device->setActiveRenderTarget(texTarget);

    CGRect rect = CGRectMake(0, 0, texSize.width, texSize.height);

    CIImage *output;
    output = [mFilter valueForKey:kCIOutputImageKey];
    // draw Image to textureTarget
    device->drawImage(output, rect, rect);

    GFX->setActiveRenderTarget(oldTarget);
    GFX->updateStates(true);

    for (CGImageRef img:imageRefs)
        CGImageRelease(img);

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


// object->getDataField
