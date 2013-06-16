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

#include "console/console.h"

#include "math/mRect.h"
#include "./gfxOpenGL33WinTextureObject.h"
#include "./gfxOpenGL33WinDevice.h"
#include "./gfxOpenGL33WinEnumTranslate.h"
#include "./gfxOpenGL33WinTextureManager.h"
#include "./gfxOpenGL33WinUtils.h"
#include "graphics/gfxCardProfile.h"
#include "platform/platformGL.h"

//#import <GLKit/GLKit.h>
//#import <ImageIO/ImageIO.h>

GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, void* texInfo) :
    GFXTextureObject(aDevice, profile),
    mBytesPerTexel(4),
    mLockedRectRect(0, 0, 0, 0),
    mGLDevice(static_cast<GFXOpenGL33WinDevice*>(mDevice)),
    mZombieCache(NULL),
    mFilter( GL_NEAREST )
{
    mBitmap = NULL;
//    setTexture(texInfo);
}


GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mGLDevice(static_cast<GFXOpenGL33WinDevice*>(mDevice)),
   mZombieCache(NULL),
   mFilter( GL_NEAREST )
{
   AssertFatal(dynamic_cast<GFXOpenGL33WinDevice*>(mDevice), "GFXOpenGL33WinTextureObject::GFXOpenGL33WinTextureObject - Invalid device type, expected GFXOpenGL33WinDevice!");
   glGenTextures(1, &mHandle);
}

GFXOpenGL33WinTextureObject::~GFXOpenGL33WinTextureObject() 
{
   delete[] mZombieCache;
   kill();
}

//void GFXOpenGL33WinTextureObject::setTexture(void* texInfo)
//{
//    if (texInfo != NULL)
//    {
//        GLKTextureInfo *textureInfo = (GLKTextureInfo*) texInfo;
//        mBitmapSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
//        mTextureSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
//        mBinding = [ textureInfo target];
//        mHandle = [ textureInfo name ];
//        mLoaded = true;
//        smEventSignal.trigger(GFXLoadNotify);
//    }
//    else
//    {
//        mLoaded = false;
//    }
//}


GFXLockedRect* GFXOpenGL33WinTextureObject::lock(U32 mipLevel, RectI *inRect)
{
//   AssertFatal(mBinding != GL_TEXTURE_3D, "GFXOpenGL33WinTextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;

   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXOpenGL33WinTextureObject::lock - Rectangle too big!");

      mLockedRectRect = *inRect;
   }
   else
   {
      mLockedRectRect = RectI(0, 0, width, height);
   }
   
   mLockedRect.pitch = mLockedRectRect.extent.x * mBytesPerTexel;
   
   if( !mLockedRect.bits )
      return NULL;

   return &mLockedRect;
}

void GFXOpenGL33WinTextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;

   glActiveTexture(GL_TEXTURE0);
   U32 boundTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
   mLockedRect.bits = NULL;

   glBindTexture(GL_TEXTURE_2D, boundTexture);
}

void GFXOpenGL33WinTextureObject::release()
{
   glDeleteTextures(1, &mHandle);
   mHandle = 0;
}

#if (defined TORQUE_OS_IPHONE || defined TORQUE_OS_MAC)
#define EXT_ARRAY_SIZE 4
static const char* extArray[EXT_ARRAY_SIZE] = { "", ".pvr", ".jpg", ".png"};
#else
#define EXT_ARRAY_SIZE 3
static const char* extArray[EXT_ARRAY_SIZE] = { "", ".jpg", ".png"};
#endif


//GBitmap* GFXOpenGL33WinTextureObject::getBitmap()
//{
//    if (mPath.isEmpty())
//        return NULL;
//
//    if (mLoaded == false)
//        return NULL;
//    
//    if (mBitmap)
//        return mBitmap;
//    
//    MutexHandle mutex;
//    mutex.lock( &mMutex, true );
//
//    bool textureExt = mPath.getExtension().isNotEmpty();
//
//    // Check the cache first...
//    String pathNoExt;
//    pathNoExt = String::Join( mPath.getPath(), '/', mPath.getFileName() );
//    
//    NSDictionary *options = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:NO]
//                                                        forKey:GLKTextureLoaderOriginBottomLeft];
//    
//    NSString *npath = nil;
//    
//    U32 i = 0;
//    NSString *typeName;
//    do
//    {
//        if (!textureExt)
//            mPath.setExtension(extArray[i]);
//        
//        //Ask the bundle for the path to the file
//        NSFileManager* fMan = [[NSFileManager alloc] init];
//        NSString* temppath = [[NSString alloc] initWithUTF8String:mPath.getFullPath().c_str()];
//        
//        if ([fMan fileExistsAtPath:temppath ])
//            npath = [[NSString alloc] initWithString: temppath];
//        i++;
//       [ fMan release] ;
//       [temppath release];
//    }
//    while ((npath == nil ) && !textureExt && ( i < EXT_ARRAY_SIZE));
//
//    
//    //
////    
////    do
////    {
////        if (mPath.getExtension().isEmpty())
////            typeName = [[NSString alloc] initWithUTF8String:extArray[i]];
////        else
////            typeName = [[NSString alloc] initWithUTF8String:mPath.getExtension().c_str()];
////        
////        NSString* fnameOnly =[[NSString alloc] initWithUTF8String:mPath.getFileName().c_str()];
////        NSString* pathOnly = [[NSString alloc] initWithUTF8String:collaspedBitmapName];
////        
////        //Ask the bundle for the path to the file
////        npath = [bundle pathForResource:fnameOnly ofType:typeName inDirectory:pathOnly];
////        i++;
////    }
////    while (!npath && mPath.getExtension().isEmpty() && ( i < EXT_ARRAY_SIZE));
//
//    if (!npath)
//    {
//        Con::printf("GFXOpenGL33WinTextureObject::createTexture unable to find: %s.%s", mPath.getFullPath().c_str(), mPath.getExtension().c_str());
//        return NULL;
//    }
//
//    Con::printf("loading %s", [npath UTF8String] );
//    NSURL *url = [[NSURL alloc ] initFileURLWithPath: npath];
//    CGDataProviderRef data_provider = CGDataProviderCreateWithURL((CFURLRef)url);
//   
//    CGImageRef apple_image;
//   
//    typeName = [[NSString alloc] initWithUTF8String:mPath.getExtension().c_str()];
//    if ([typeName isEqualToString:@"png"]) {
//        apple_image = CGImageCreateWithPNGDataProvider(data_provider, nil, false, kCGRenderingIntentDefault);
//    } else if ([typeName isEqualToString:@"jpg"]) {
//        apple_image = CGImageCreateWithJPEGDataProvider(data_provider, nil, false, kCGRenderingIntentDefault);
//    } else
//        return NULL;
//    
//   [typeName release];
//
//    // Choose alpha strategy based on whether the source image has alpha or not.
//    // UNUSED: JOSEPH THOMAS -> CGImageAlphaInfo alpha_info = CGImageGetAlphaInfo(apple_image);
//	int width = CGImageGetWidth(apple_image);
//	int height = CGImageGetHeight(apple_image);
//    U32 rowBytes = width * 4;
//    
//    // Set up the row pointers...
//    AssertISV(width <= 1024, "Error, cannot load images wider than 1024 pixels!");
//    AssertISV(height <= 1024, "Error, cannot load images taller than 1024 pixels!");
//    
//    mBitmap = new GBitmap( mTextureSize.x, mTextureSize.y, false, GFXFormatR8G8B8A8);
//
//    U8 *pBase = (U8*)mBitmap->getBits();
//    
//    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
//    CGContextRef texture_context = CGBitmapContextCreate(pBase, width, height, 8, rowBytes, color_space, kCGImageAlphaPremultipliedLast);
//    
//    CGContextDrawImage(texture_context, CGRectMake(0.0, 0.0, width, height), apple_image);
//    
//    CGImageRelease(apple_image);
//    CGDataProviderRelease(data_provider);
//   CFRelease(color_space);
//    CFRelease(texture_context);
//    return mBitmap;
//}


bool GFXOpenGL33WinTextureObject::copyToBmp(GBitmap * bmp)
{
    // not supported in opengl es
   return false;
}

void GFXOpenGL33WinTextureObject::bind(U32 textureUnit) const
{
    AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGL33WinTextureObject::bind - only GL_TEXTURE_2D supported");
   glActiveTexture(GL_TEXTURE0 + textureUnit);
    
    GLuint han = mHandle;
    if (!mHandle)
        return;
    
    
//   if (glIsTexture(mHandle) != GL_TRUE)
//   {
//       Con::printf("bad texture bind");
//   }
   glBindTexture(mBinding, han);
//    Con::printf("texture bind %i", han);
//   glEnable(mBinding);
  
   GFXOpenGLStateBlockRef sb = mGLDevice->getCurrentStateBlock();
   AssertFatal(sb, "GFXOpenGL33WinTextureObject::bind - No active stateblock!");
   if (!sb)
      return;
         
   const GFXSamplerStateDesc ssd = sb->getDesc().samplers[textureUnit];
   glTexParameteri(mBinding, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));   
   glTexParameteri(mBinding, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);
//    glTexParameteri(mBinding, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(mBinding, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
   glTexParameteri(mBinding, GL_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeU] : GL_CLAMP_TO_EDGE);
   glTexParameteri(mBinding, GL_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeV] : GL_CLAMP_TO_EDGE);
}

U8* GFXOpenGL33WinTextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
   return data;
}

void GFXOpenGL33WinTextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
//   if(mBinding == GL_TEXTURE_3D)
//      cacheSize *= mTextureSize.z;
    
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
//   glGetTexImage(mBinding, 0, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   glBindTexture(mBinding, 0);
}

void GFXOpenGL33WinTextureObject::reloadFromCache()
{
   if(!mZombieCache)
      return;
      
//   if(mBinding == GL_TEXTURE_3D)
//   {
//      static_cast<GFXGLTextureManager*>(TEXMGR)->_loadTexture(this, mZombieCache);
//      delete[] mZombieCache;
//      mZombieCache = NULL;
//      return;
//   }
   
   glBindTexture(mBinding, mHandle);
   glTexSubImage2D(mBinding, 0, 0, 0, mTextureSize.x, mTextureSize.y, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   
//   if(GFX->getCardProfiler()->queryProfile("GL::Workaround::needsExplicitGenerateMipmap") && mMipLevels != 1)
//      glGenerateMipmap(mBinding);
    
   delete[] mZombieCache;
   mZombieCache = NULL;
   mIsZombie = false;
}

void GFXOpenGL33WinTextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXOpenGL33WinTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   glGenTextures(1, &mHandle);
}

F32 GFXOpenGL33WinTextureObject::getMaxUCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getWidth();
}

F32 GFXOpenGL33WinTextureObject::getMaxVCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getHeight();
}

const String GFXOpenGL33WinTextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   ret += String::ToString("   GL Handle: %i", mHandle);
   
   return ret;
}

void GFXOpenGL33WinTextureObject::setFilter(const GFXTextureFilterType filter)
{
    // Set filter.
    mFilter = GFXGLTextureFilter[filter];
    
    // Finish if no GL texture name.
    if ( mHandle == 0 )
        return;
    
    // Set texture state.
    glBindTexture( GL_TEXTURE_2D, mHandle );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter );
}


