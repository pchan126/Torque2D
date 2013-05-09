//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <OpenGLES/ES2/glext.h>

#include "console/console.h"
#include "math/mRect.h"
#include "platformiOS/graphics/gfxOpenGLESTextureObject.h"
#include "platformiOS/graphics/gfxOpenGLESDevice.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"
#include "platformiOS/graphics/gfxOpenGLESTextureManager.h"
#include "platformiOS/graphics/gfxOpenGLESUtils.h"
#include "graphics/gfxCardProfile.h"
#import <GLKit/GLKit.h>

GFXOpenGLESTextureObject::GFXOpenGLESTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile, GLKTextureInfo* textureInfo) :
    GFXTextureObject(aDevice, profile),
    mBytesPerTexel(4),
    mLockedRectRect(0, 0, 0, 0),
    mGLDevice(static_cast<GFXOpenGLESDevice*>(mDevice)),
    mZombieCache(NULL),
    mFilter( GL_NEAREST )
{
    mTextureSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBitmapSize.set([ textureInfo width ], [ textureInfo height ], 0.0);
    mBinding = [ textureInfo target];
    mHandle = [ textureInfo name ];
}


GFXOpenGLESTextureObject::GFXOpenGLESTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXTextureObject(aDevice, profile),
   mBinding(GL_TEXTURE_2D),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mGLDevice(static_cast<GFXOpenGLESDevice*>(mDevice)),
   mZombieCache(NULL)
{
   AssertFatal(dynamic_cast<GFXOpenGLESDevice*>(mDevice), "GFXOpenGLESTextureObject::GFXOpenGLESTextureObject - Invalid device type, expected GFXOpenGLESDevice!");
   glGenTextures(1, &mHandle);
}

GFXOpenGLESTextureObject::~GFXOpenGLESTextureObject() 
{
//   glDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}

GFXLockedRect* GFXOpenGLESTextureObject::lock(U32 mipLevel, RectI *inRect)
{
//   AssertFatal(mBinding != GL_TEXTURE_3D, "GFXOpenGLESTextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;

   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXOpenGLESTextureObject::lock - Rectangle too big!");

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

void GFXOpenGLESTextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;

   glActiveTexture(GL_TEXTURE0);
   U32 boundTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
   mLockedRect.bits = NULL;

   glBindTexture(GL_TEXTURE_2D, boundTexture);
}

void GFXOpenGLESTextureObject::release()
{
   glDeleteTextures(1, &mHandle);
   mHandle = 0;
}

GBitmap* GFXOpenGLESTextureObject::getBitmap()
{
    if (mBitmap)
        return mBitmap;
    
    if (mPath.isEmpty())
        return NULL;
    
//    GBitmap
//
//    Stream& io_rStream;
//    int filesize = io_rStream.getStreamSize();
//    U8 *buff = new U8[filesize+1024];
//    // UNUSED: JOSEPH THOMAS -> unsigned long size = io_rStream.read(filesize,buff);
//    
//    CGDataProviderRef data_provider = CGDataProviderCreateWithData(nil, buff, filesize, nil);
//    CGImageRef apple_image = CGImageCreateWithPNGDataProvider(data_provider, nil, false, kCGRenderingIntentDefault);
//    
//    // Choose alpha strategy based on whether the source image has alpha or not.
//    // UNUSED: JOSEPH THOMAS -> CGImageAlphaInfo alpha_info = CGImageGetAlphaInfo(apple_image);
//    int width = CGImageGetWidth(apple_image);
//    int height = CGImageGetHeight(apple_image);
//    U32 rowBytes = width * 4;
//    
//    // Set up the row pointers...
//    AssertISV(width <= 1024, "Error, cannot load images wider than 1024 pixels!");
//    AssertISV(height <= 1024, "Error, cannot load images taller than 1024 pixels!");
//    
//    BitmapFormat format = RGBA;
//    // actually allocate the bitmap space...
//    allocateBitmap(width, height,
//                   false,            // don't extrude miplevels...
//                   format);          // use determined format...
//    
//    U8 *pBase = (U8*)getBits();
//    
//    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
//    CGContextRef texture_context = CGBitmapContextCreate(pBase, width, height, 8, rowBytes, color_space, kCGImageAlphaPremultipliedLast);
//    
//    CGContextDrawImage(texture_context, CGRectMake(0.0, 0.0, width, height), apple_image);
//    
//    
//    CGImageRelease(apple_image);
//    CGDataProviderRelease(data_provider);
    
//    delete [] buff;
    return NULL;
}


bool GFXOpenGLESTextureObject::copyToBmp(GBitmap * bmp)
{
    // not supported in opengl es
   return false;
}

void GFXOpenGLESTextureObject::bind(U32 textureUnit) const
{
//    AssertFatal(mBinding == GL_TEXTURE_2D, "GFXOpenGLESTextureObject::bind - only GL_TEXTURE_2D supported");
//   glActiveTexture(GL_TEXTURE0 + textureUnit);
//
//    GLuint han = mHandle;
//   glBindTexture(mBinding, han);
//    
//   GFXOpenGLESStateBlockRef sb = mGLDevice->getCurrentStateBlock();
//   AssertFatal(sb, "GFXOpenGLESTextureObject::bind - No active stateblock!");
//   if (!sb)
//      return;
//         
//   const GFXSamplerStateDesc ssd = sb->getDesc().samplers[textureUnit];
//   glTexParameteri(mBinding, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));   
//   glTexParameteri(mBinding, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);
//    
//   glTexParameteri(mBinding, GL_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeU] : GL_CLAMP_TO_EDGE);
//   glTexParameteri(mBinding, GL_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXGLTextureAddress[ssd.addressModeV] : GL_CLAMP_TO_EDGE);
}

U8* GFXOpenGLESTextureObject::getTextureData()
{
   U8* data = new U8[mTextureSize.x * mTextureSize.y * mBytesPerTexel];
   glBindTexture(GL_TEXTURE_2D, mHandle);
   return data;
}

void GFXOpenGLESTextureObject::copyIntoCache()
{
   glBindTexture(mBinding, mHandle);
   U32 cacheSize = mTextureSize.x * mTextureSize.y;
    
   cacheSize *= mBytesPerTexel;
   mZombieCache = new U8[cacheSize];
   
   glBindTexture(mBinding, 0);
}

void GFXOpenGLESTextureObject::reloadFromCache()
{
   if(!mZombieCache)
      return;
      
   glBindTexture(mBinding, mHandle);
   glTexSubImage2D(mBinding, 0, 0, 0, mTextureSize.x, mTextureSize.y, GFXGLTextureFormat[mFormat], GFXGLTextureType[mFormat], mZombieCache);
   
//   if(GFX->getCardProfiler()->queryProfile("GL::Workaround::needsExplicitGenerateMipmap") && mMipLevels != 1)
      glGenerateMipmap(mBinding);
      
   delete[] mZombieCache;
   mZombieCache = NULL;
   mIsZombie = false;
}

void GFXOpenGLESTextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXOpenGLESTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   glGenTextures(1, &mHandle);
}

F32 GFXOpenGLESTextureObject::getMaxUCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getWidth();
}

F32 GFXOpenGLESTextureObject::getMaxVCoord() const
{
   return mBinding == GL_TEXTURE_2D ? 1.0f : (F32)getHeight();
}

const String GFXOpenGLESTextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   ret += String::ToString("   GL Handle: %i", mHandle);
   
   return ret;
}

void GFXOpenGLESTextureObject::setFilter(const GFXTextureFilterType filter)
{
//    // Set filter.
//    mFilter = GFXGLTextureFilter[filter];
//    
//    // Finish if no GL texture name.
//    if ( mHandle == 0 )
//        return;
//    
//    // Set texture state.
//    glBindTexture( GL_TEXTURE_2D, mHandle );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
}


