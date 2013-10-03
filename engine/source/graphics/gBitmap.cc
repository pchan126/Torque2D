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

#include "io/stream.h"
#include "io/fileStream.h"
#include "graphics/gBitmap.h"
#include "graphics/gPalette.h"
#include "io/resource/resourceManager.h"
#include "platform/platform.h"
#include "memory/safeDelete.h"
#include "math/mRect.h"
#include "console/console.h"
#include "graphics/bitmapUtils.h"

#ifndef _TORQUECONFIG_H_
#include "torqueConfig.h"//for PNG loading setting
#endif



const U32 GBitmap::csFileVersion   = 3;
U32       GBitmap::sBitmapIdSource = 0;


GBitmap::GBitmap()
 : mInternalFormat(GFXFormatR8G8B8),
   mBits(nullptr),
   mByteSize(0),
   mWidth(0),
   mHeight(0),
   mNumMipLevels(0),
   mBytesPerPixel(0),
   pPalette(nullptr),
   mForce16Bit(false)
{
   for (U32 i = 0; i < c_maxMipLevels; i++)
      mMipLevelOffsets[i] = 0xffffffff;
}

GBitmap::GBitmap(const GBitmap& rCopy)
{

   if (rCopy.pPalette)
   {
      pPalette = new GPalette;
      pPalette->setPaletteType(rCopy.pPalette->getPaletteType());
      dMemcpy(rCopy.pPalette->getColors(), pPalette->getColors(), sizeof(ColorI)*256);
   }
   else
      pPalette = nullptr;

   mInternalFormat = rCopy.mInternalFormat;

   
   mForce16Bit = rCopy.mForce16Bit;

   mByteSize = rCopy.mByteSize;
   mBits    = new U8[mByteSize];
   dMemcpy(mBits, rCopy.mBits, mByteSize);

   mWidth        = rCopy.mWidth;
   mHeight       = rCopy.mHeight;
   mBytesPerPixel = rCopy.mBytesPerPixel;
   mNumMipLevels = rCopy.mNumMipLevels;
   dMemcpy(mMipLevelOffsets, rCopy.mMipLevelOffsets, sizeof(mMipLevelOffsets));
}


GBitmap::GBitmap(const U32  in_width,
                 const U32  in_height,
                 const bool in_extrudeMipLevels,
                 const GFXFormat in_format)
 : mBits(nullptr),
   mByteSize(0),
   pPalette(nullptr),
   mForce16Bit(false)
{
   for (U32 i = 0; i < c_maxMipLevels; i++)
      mMipLevelOffsets[i] = 0xffffffff;

   allocateBitmap(in_width, in_height, in_extrudeMipLevels, in_format);
}


//--------------------------------------------------------------------------
GBitmap::~GBitmap()
{
   deleteImage();
}


//--------------------------------------------------------------------------
void GBitmap::deleteImage()
{
   delete [] mBits;
   mBits    = nullptr;
   mByteSize = 0;

   mWidth        = 0;
   mHeight       = 0;
   mNumMipLevels = 0;

   SAFE_DELETE(pPalette);
}


//--------------------------------------------------------------------------
//void GBitmap::setPalette(GPalette* in_pPalette)
//{
//   SAFE_DELETE(pPalette);
//   pPalette = in_pPalette;
//}

void GBitmap::copyRect(const GBitmap *src, const RectI &srcRect, const Point2I &dstPt)
{
   if(src->getFormat() != getFormat())
      return;
   if(srcRect.extent.x + srcRect.point.x > (S32)src->getWidth() || srcRect.extent.y + srcRect.point.y > (S32)src->getHeight())
      return;
   if(srcRect.extent.x + dstPt.x > (S32)getWidth() || srcRect.extent.y + dstPt.y > (S32)getHeight())
      return;

   for(U32 i = 0; i < (U32)srcRect.extent.y; i++)
   {
      dMemcpy(getAddress(dstPt.x, dstPt.y + i),
              src->getAddress(srcRect.point.x, srcRect.point.y + i),
              mBytesPerPixel * srcRect.extent.x);
   }
}

//--------------------------------------------------------------------------
void GBitmap::allocateBitmap(const U32 in_width, const U32 in_height, const bool in_extrudeMipLevels, const GFXFormat in_format)
{
   //-------------------------------------- Some debug checks...
   U32 svByteSize = mByteSize;
   U8 *svBits = mBits;

   AssertFatal(in_width != 0 && in_height != 0, "GBitmap::allocateBitmap: mWidth or mHeight is 0");

   if (in_extrudeMipLevels == true) {
      //AssertFatal(in_width <= 256 && in_height <= 256, "GBitmap::allocateBitmap: mWidth or mHeight is too large");
      AssertFatal(isPow2(in_width) == true && isPow2(in_height) == true, "GBitmap::GBitmap: in order to extrude miplevels, bitmap w/h must be pow2");
   }

   mInternalFormat = in_format;
   mWidth          = in_width;
   mHeight         = in_height;

   mBytesPerPixel = 1;
    switch (mInternalFormat)
    {
        case GFXFormatA8:
        case GFXFormatL8:           mBytesPerPixel = 1;
            break;
        case GFXFormatR8G8B8:       mBytesPerPixel = 3;
            break;
        case GFXFormatR8G8B8X8:
        case GFXFormatR8G8B8A8:     mBytesPerPixel = 4;
            break;
        case GFXFormatR5G6B5:
        case GFXFormatR5G5B5A1:     mBytesPerPixel = 2;
            break;
       case GFXFormatR32G32B32A32F:
          mBytesPerPixel = 16;
          break;
        case GFXFormat_PVR2:
        case GFXFormat_PVR2A:
        case GFXFormat_PVR4:
        case GFXFormat_PVR4A:
            // compressed textures can't be allocated
            return;
        default:
            AssertFatal(false, "GBitmap::GBitmap: misunderstood format specifier");
            break;
    }
    

    // Set up the mip levels, if necessary...
   mNumMipLevels       = 1;
   U32 allocPixels = in_width * in_height * mBytesPerPixel;
   mMipLevelOffsets[0] = 0;


   if (in_extrudeMipLevels == true) 
   {
      U32 currWidth  = in_width;
      U32 currHeight = in_height;

      do 
      {
         mMipLevelOffsets[mNumMipLevels] = mMipLevelOffsets[mNumMipLevels - 1] +
                                         (currWidth * currHeight * mBytesPerPixel);
         currWidth  >>= 1;
         currHeight >>= 1;
         if (currWidth  == 0) currWidth  = 1;
         if (currHeight == 0) currHeight = 1;

         mNumMipLevels++;
         allocPixels += currWidth * currHeight * mBytesPerPixel;
      } while (currWidth != 1 || currHeight != 1);
   }
   AssertFatal(mNumMipLevels <= c_maxMipLevels, "GBitmap::allocateBitmap: too many miplevels");

   // Set up the memory...
   mByteSize = allocPixels;
   mBits    = new U8[mByteSize];
    dMemset(mBits, 0xFF, mByteSize);
    
   if(svBits != nullptr)
   {
      dMemcpy(mBits, svBits, std::min(mByteSize, svByteSize));
      delete[] svBits;
   }
}


//--------------------------------------------------------------------------
void GBitmap::extrudeMipLevels(bool clearBorders)
{
    if(mNumMipLevels == 1)
        allocateBitmap(getWidth(), getHeight(), true, getFormat());
    
    switch (getFormat())
    {
        case GFXFormatR5G5B5A1:
        {
            for(U32 i = 1; i < mNumMipLevels; i++)
                bitmapExtrude5551(getBits(i - 1), getWritableBits(i), getHeight(i), getWidth(i));
            break;
        }
            
        case GFXFormatR8G8B8:
        {
            for(U32 i = 1; i < mNumMipLevels; i++)
                bitmapExtrudeRGB(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
            break;
        }
            
        case GFXFormatR8G8B8A8:
        case GFXFormatR8G8B8X8:
        {
            for(U32 i = 1; i < mNumMipLevels; i++)
                bitmapExtrudeRGBA(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
            break;
        }
            
        default:
            break;
    }
    if (clearBorders)
    {
        for (U32 i = 1; i<mNumMipLevels; i++)
        {
            U32 mWidth = getWidth(i);
            U32 mHeight = getHeight(i);
            if (mHeight<3 || mWidth<3)
                // bmp is all borders at this mip level
                dMemset(getWritableBits(i),0,mWidth*mHeight*mBytesPerPixel);
            else
            {
                mWidth *= mBytesPerPixel;
                U8 * bytes = getWritableBits(i);
                U8 * end = bytes + (mHeight-1)*mWidth - mBytesPerPixel; // end = last row, 2nd column
                // clear first row sans the last pixel
                dMemset(bytes,0,mWidth-mBytesPerPixel);
                bytes -= mBytesPerPixel;
                while (bytes<end)
                {
                    // clear last pixel of row N-1 and first pixel of row N
                    bytes += mWidth;
                    dMemset(bytes,0,mBytesPerPixel*2);
                }
                // clear last row sans the first pixel
                dMemset(bytes+2*mBytesPerPixel,0,mWidth-mBytesPerPixel);
            }
        }
    }
}

//--------------------------------------------------------------------------
void GBitmap::extrudeMipLevelsDetail()
{
   AssertFatal(getFormat() == GFXFormatR8G8B8, "Error, only handles GFXFormatR8G8B8 for now...");
   U32 i,j;

   if(mNumMipLevels == 1)
      allocateBitmap(getWidth(), getHeight(), true, getFormat());

   for (i = 1; i < mNumMipLevels; i++) {
      bitmapExtrudeRGB(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
   }

   // Ok, now that we have the levels extruded, we need to move the lower miplevels
   //  closer to 0.5.
   for (i = 1; i < mNumMipLevels - 1; i++) {
      U8* pMipBits = (U8*)getWritableBits(i);
      U32 numBytes = getWidth(i) * getHeight(i) * 3;

      U32 shift    = i;
      U32 start    = ((1 << i) - 1) * 0x80;

      for (j = 0; j < numBytes; j++) {
         U32 newVal = (start + pMipBits[j]) >> shift;
         AssertFatal(newVal <= 255, "Error, oob");
         pMipBits[j] = U8(newVal);
      }
   }
   AssertFatal(getWidth(mNumMipLevels - 1) == 1 && getHeight(mNumMipLevels - 1) == 1,
               "Error, last miplevel should be 1x1!");
   ((U8*)getWritableBits(mNumMipLevels - 1))[0] = 0x80;
   ((U8*)getWritableBits(mNumMipLevels - 1))[1] = 0x80;
   ((U8*)getWritableBits(mNumMipLevels - 1))[2] = 0x80;
}


//--------------------------------------------------------------------------
bool GBitmap::setFormat(GFXFormat fmt)
{
    if (getFormat() == fmt)
        return true;
    
    PROFILE_SCOPE(GBitmap_setFormat);
    
    // this is a nasty pointer math hack
    // is there a quick way to calc pixels of a fully mipped bitmap?
    U32 pixels = 0;
    for (U32 i=0; i < mNumMipLevels; i++)
        pixels += getHeight(i) * getWidth(i);
    
    switch( getFormat() )
    {
        case GFXFormatR8G8B8:
            switch ( fmt )
        {
            case GFXFormatR5G5B5A1:
                bitmapConvertRGB_to_5551(mBits, pixels);
                mInternalFormat = GFXFormatR5G5B5A1;
                mBytesPerPixel  = 2;
                break;
                
            case GFXFormatR8G8B8A8:
            case GFXFormatR8G8B8X8:
                // Took this out, it may crash -patw
                //AssertFatal( mNumMipLevels == 1, "Do the mip-mapping in hardware." );
                
                bitmapConvertRGB_to_RGBX( &mBits, pixels );
                mInternalFormat = fmt;
                mBytesPerPixel = 4;
                mByteSize = pixels * 4;
                break;
                
            default:
                AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
                return false;
        }
            break;
            
        case GFXFormatR8G8B8X8:
            switch( fmt )
        {
                // No change needed for this
            case GFXFormatR8G8B8A8:
                mInternalFormat = GFXFormatR8G8B8A8;
                break;
                
            case GFXFormatR8G8B8:
                bitmapConvertRGBX_to_RGB( &mBits, pixels );
                mInternalFormat = GFXFormatR8G8B8;
                mBytesPerPixel = 3;
                mByteSize = pixels * 3;
                break;
                
            default:
                AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
                return false;
        }
            break;
            
        case GFXFormatR8G8B8A8:
            switch( fmt )
        {
                // No change needed for this
            case GFXFormatR8G8B8X8:
                mInternalFormat = GFXFormatR8G8B8X8;
                break;
                
            case GFXFormatR8G8B8:
                bitmapConvertRGBX_to_RGB( &mBits, pixels );
                mInternalFormat = GFXFormatR8G8B8;
                mBytesPerPixel = 3;
                mByteSize = pixels * 3;
                break;
                
            default:
                AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
                return false;
        }
            break;
            
        case GFXFormatA8:
            switch( fmt )
        {
            case GFXFormatR8G8B8A8:
                mInternalFormat = GFXFormatR8G8B8A8;
                bitmapConvertA8_to_RGBA( &mBits, pixels );
                mBytesPerPixel = 4;
                mByteSize = pixels * 4;
                break;
                
            default:
                AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
                return false;
        }
            break;
            
        default:
            AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
            return false;
    }
    
    U32 offset = 0;
    for (U32 j=0; j < mNumMipLevels; j++)
    {
        mMipLevelOffsets[j] = offset;
        offset += getHeight(j) * getWidth(j) * mBytesPerPixel;
    }
    
    return true;
}


//--------------------------------------------------------------------------
bool GBitmap::getColorBGRA(const U32 x, const U32 y, ColorI& rColor) const
{
   if(!getColor(x, y, rColor))
      return false;
   //jk - swap red and blue...
   U8 r = rColor.red;
   rColor.red = rColor.blue;
   rColor.blue = r;
   return true;
}

bool GBitmap::setColorBGRA(const U32 x, const U32 y, ColorI& rColor)
{
   //jk - copy then swap red and blue...
   //jk - using a copy so the color object provided by the caller isn't swapped...
   ColorI temp = rColor;

   U8 r = temp.red;
   temp.red = temp.blue;
   temp.blue = r;

   return setColor(x, y, temp);
}


bool GBitmap::getColor(const U32 x, const U32 y, ColorI& rColor) const
{
    if (x >= mWidth || y >= mHeight)
        return false;
    
    const U8* pLoc = getAddress(x, y);
    
    switch (mInternalFormat) {
        case GFXFormatA8:
        case GFXFormatL8:
            rColor.set( *pLoc, *pLoc, *pLoc, *pLoc );
            break;
            
        case GFXFormatR8G8B8:
        case GFXFormatR8G8B8X8:
            rColor.set( pLoc[0], pLoc[1], pLoc[2], 255 );
            break;
            
        case GFXFormatR8G8B8A8:
            rColor.set( pLoc[0], pLoc[1], pLoc[2], pLoc[3] );
            break;
            
        case GFXFormatR5G5B5A1:
#if defined(TORQUE_OS_MAC)
            rColor.set( (*((U16*)pLoc) >> 0) & 0x1F,
                       (*((U16*)pLoc) >> 5) & 0x1F,
                       (*((U16*)pLoc) >> 10) & 0x1F,
                       ((*((U16*)pLoc) >> 15) & 0x01) ? 255 : 0 );
#else
            rColor.set( *((U16*)pLoc) >> 11,
                       (*((U16*)pLoc) >> 6) & 0x1f,
                       (*((U16*)pLoc) >> 1) & 0x1f,
                       (*((U16*)pLoc) & 1) ? 255 : 0 );
#endif
            break;
            
        default:
            AssertFatal(false, "Bad internal format");
            return false;
    }
    
    return true;
}


//--------------------------------------------------------------------------
bool GBitmap::setColor(const U32 x, const U32 y, ColorI& rColor)
{
    if (x >= mWidth || y >= mHeight)
        return false;
    
    U8* pLoc = getAddress(x, y);
    
    switch (mInternalFormat) {
        case GFXFormatA8:
        case GFXFormatL8:
            *pLoc = rColor.alpha;
            break;
            
        case GFXFormatR8G8B8:
            dMemcpy( pLoc, &rColor, 3 * sizeof( U8 ) );
            break;
            
        case GFXFormatR8G8B8A8:
        case GFXFormatR8G8B8X8:
            dMemcpy( pLoc, &rColor, 4 * sizeof( U8 ) );
            break;
            
        case GFXFormatR5G6B5:
#ifdef TORQUE_OS_MAC
            *((U16*)pLoc) = (rColor.red << 11) | (rColor.green << 5) | (rColor.blue << 0) ;
#else
            *((U16*)pLoc) = (rColor.blue << 0) | (rColor.green << 5) | (rColor.red << 11);
#endif
            break;
            
        case GFXFormatR5G5B5A1:
#ifdef TORQUE_OS_MAC
            *((U16*)pLoc) = (((rColor.alpha>0) ? 1 : 0)<<15) | (rColor.blue << 10) | (rColor.green << 5) | (rColor.red << 0);
#else
            *((U16*)pLoc) = (rColor.blue << 1) | (rColor.green << 6) | (rColor.red << 11) | ((rColor.alpha>0) ? 1 : 0);
#endif
            break;
            
        default:
            AssertFatal(false, "Bad internal format");
            return false;
    }
    
    return true;
}

//-----------------------------------------------------------------------------

GBitmap* GBitmap::createPowerOfTwoBitmap()
{
   if (isPow2(getWidth()) && isPow2(getHeight()))
      return this;

   AssertFatal(getNumMipLevels() == 1,
      "Cannot have non-pow2 bitmap with miplevels");

   U32 mWidth = getWidth();
   U32 mHeight = getHeight();

   U32 newWidth  = getNextPow2(getWidth());
   U32 newHeight = getNextPow2(getHeight());

   GBitmap* pReturn = new GBitmap(newWidth, newHeight, false, getFormat());

   for (U32 i = 0; i < mHeight; i++) 
   {
      U8*       pDest = (U8*)pReturn->getAddress(0, i);
      const U8* pSrc  = (const U8*)getAddress(0, i);

      dMemcpy(pDest, pSrc, mWidth * mBytesPerPixel);

      pDest += mWidth * mBytesPerPixel;
      // set the src pixel to the last pixel in the row
      const U8 *pSrcPixel = pDest - mBytesPerPixel; 

      for(U32 j = mWidth; j < newWidth; j++)
         for(U32 k = 0; k < mBytesPerPixel; k++)
            *pDest++ = pSrcPixel[k];
   }

   for(U32 i = mHeight; i < newHeight; i++)
   {
      U8* pDest = (U8*)pReturn->getAddress(0, i);
      U8* pSrc = (U8*)pReturn->getAddress(0, mHeight-1);
      dMemcpy(pDest, pSrc, newWidth * mBytesPerPixel);
   }

   return pReturn;
}

U16* GBitmap::create16BitBitmap( GFXFormat *GLformat )
{
    //PUAP -Mat make 16 bit
    U16 *texture_data = new U16[mWidth * mHeight];
    U16 *dest = texture_data;
    U32 *source = (U32*)getWritableBits();
    //since the pointer is 4 bytes, multiply by the number of bytes per pixel over 4
    U32 spanInBytes = (U32)((mWidth * mHeight) * (mBytesPerPixel / 4.0f));
    U32 *source_end = source + spanInBytes;
    
    switch (getFormat()) {
        case GFXFormatR32F: //ALPHA_TRANSPARENT:
            while (source != source_end) {
                U32 color = *source++;
                *dest++ = ((color & 0xF8) << 8) | ((color & 0xF800) >> 5) | ((color & 0xF80000) >> 18) | (color >> 31);
            }
            *GLformat = GFXFormatR5G5B5A1;
            break;
        case GFXFormatR8G8B8A8://ALPHA_BLEND
            while (source != source_end) {
                U32 color = *source++;
                *dest++ = ((color & 0xF0) << 8) | ((color & 0xF000) >> 4) | ((color & 0xF00000) >> 16) | ((color & 0xF0000000) >> 28);
            }
            *GLformat = GFXFormatR4G4B4A4;
            break;
            
        default://ALPHA_NONE
            U8 *source8 = (U8*)source;
            //32 bytes per address, snce we are casting to U8 we need 4 times as many
            U8 *end8 = source8 + (U32)(spanInBytes*4);
            while (source8 < end8) {
                U16 red = (U16)*source8;
                source8++;
                U16 green = (U16)*source8;
                source8++;
                U16 blue = (U16)*source8;
                source8++;
                //now color should be == RR GG BB 00
                *dest = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
                dest++;
            }
            *GLformat = GFXFormatR5G6B5;
            break;
    }
    return texture_data;
}

//------------------------------------------------------------------------------
//-------------------------------------- Persistent I/O
//

#ifdef TORQUE_OS_IOS
#define EXT_ARRAY_SIZE 4
static const char* extArray[EXT_ARRAY_SIZE] = { "", ".pvr", ".jpg", ".png"};
#else
#define EXT_ARRAY_SIZE 3
static const char* extArray[EXT_ARRAY_SIZE] = { "", ".jpg", ".png"};
#endif

ResourceObject * GBitmap::findBmpResource(const char * path)
{
   char fileNameBuffer[512];
   dStrcpy( fileNameBuffer, path );

   // Try some different possible filenames.
   U32 len = dStrlen( fileNameBuffer );
   for( U32 i = 0; i < EXT_ARRAY_SIZE; i++ ) 
   {
      dStrcpy( fileNameBuffer + len, extArray[i] );
      ResourceObject * ret = ResourceManager->find( fileNameBuffer );
      if (ret)
         return ret;
   }
   return nullptr;
}

GBitmap *GBitmap::load(const char *path)
{
   ResourceObject * ro = findBmpResource(path);
   if (ro)
   {
      GBitmap *bmp = (GBitmap*)ResourceManager->loadInstance(ro);
      return bmp;
   }

   // If unable to load texture in current directory
   // look in the parent directory.  But never look in the root.
   char fileNameBuffer[512];
   dStrcpy( fileNameBuffer, path );
   char *name = dStrrchr( fileNameBuffer, '/' );

   if( name ) 
   {
      *name++ = 0;
      char *parent = dStrrchr( fileNameBuffer, '/' );

      if( parent ) 
      {
         parent[1] = 0;
         dStrcat( fileNameBuffer, name );
         return load( fileNameBuffer );
      }
   }

   return nullptr;
}

bool GBitmap::read(Stream& io_rStream)
{
   // Handle versioning
   U32 version;
   io_rStream.read(&version);
   AssertFatal(version == csFileVersion, "Bitmap::read: incorrect file version");

   //-------------------------------------- Read the object
   U32 fmt;
   io_rStream.read(&fmt);
   mInternalFormat = GFXFormat(fmt);
   mBytesPerPixel = 1;
    switch (mInternalFormat) {
        case GFXFormatA8:
        case GFXFormatL8:  mBytesPerPixel = 1;
            break;
        case GFXFormatR8G8B8:        mBytesPerPixel = 3;
            break;
        case GFXFormatR8G8B8A8:       mBytesPerPixel = 4;
            break;
        case GFXFormatR5G6B5:
        case GFXFormatR5G5B5A1:    mBytesPerPixel = 2;
            break;
        default:
            AssertFatal(false, "GBitmap::read: misunderstood format specifier");
            break;
    }

   io_rStream.read(&mByteSize);

   mBits = new U8[mByteSize];
   io_rStream.read(mByteSize, mBits);

   io_rStream.read(&mWidth);
   io_rStream.read(&mHeight);

   io_rStream.read(&mNumMipLevels);
   for (U32 i = 0; i < c_maxMipLevels; i++)
      io_rStream.read(&mMipLevelOffsets[i]);

//   if (mInternalFormat == Palettized) {
//      pPalette = new GPalette;
//      pPalette->read(io_rStream);
//   }

   return (io_rStream.getStatus() == Stream::Ok);
}

bool GBitmap::write(Stream& io_rStream) const
{
   // Handle versioning
   io_rStream.write(csFileVersion);

   //-------------------------------------- Write the object
   io_rStream.write(U32(mInternalFormat));

   io_rStream.write(mByteSize);
   io_rStream.write(mByteSize, mBits);

   io_rStream.write(mWidth);
   io_rStream.write(mHeight);

   io_rStream.write(mNumMipLevels);
   for (U32 i = 0; i < c_maxMipLevels; i++)
      io_rStream.write(mMipLevelOffsets[i]);

//   if (mInternalFormat == Palettized) {
//      AssertFatal(pPalette != nullptr,
//                  "GBitmap::write: cannot write a palettized bitmap wo/ a palette");
//      pPalette->write(io_rStream);
//   }

    return true;
}


//-------------------------------------- GFXBitmap
ResourceInstance* constructBitmapJPEG(std::iostream &stream)
{
   GBitmap* bmp = new GBitmap;
   if (bmp->readJPEG(stream))
      return bmp;
    else
    {
      delete bmp;
      return nullptr;
   }
}

ResourceInstance* constructBitmapPNG(std::iostream &stream)
{
   GBitmap* bmp = new GBitmap;

   //PUAP -Mat uless you compile with a custom build step 'IPHONE_OPTIMIZE_OPTIONS' set to '-skip-PNGs', you're PNGs will be altered(optimized)
    //So either deal with that youself or define this so that we load it using apple iPhone functions to get the PNG data to Torque	
#ifdef USE_APPLE_OPTIMIZED_PNGS
    if (bmp->readPNGiPhone(stream)){
#else
    if (bmp->readPNG(stream)){
#endif
      return bmp;
    }
    else
    {
      delete bmp;
      return nullptr;
    }
}

ResourceInstance* constructBitmapBMP(std::iostream &stream)
{
   GBitmap *bmp = new GBitmap;
   if(bmp->readMSBmp(stream))
      return bmp;
   else
   {
      delete bmp;
      return nullptr;
   }
}

#ifdef TORQUE_OS_IOS
ResourceInstance* constructBitmapPVR(std::iostream &stream)
{
    GBitmap *bmp = new GBitmap;
    if(bmp->readPvr(stream))
        return bmp;
    else
    {
        delete bmp;
        return nullptr;
    }
}
#endif



