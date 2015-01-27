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

#ifndef _GBITMAP_H_
#define _GBITMAP_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _RESMANAGER_H_
#include "io/resource/resourceManager.h"
#endif
#ifndef _COLOR_H_
#include "graphics/color.h"
#endif

#include "graphics/gfxEnums.h" // For the format

//-------------------------------------- Forward decls.
class Stream;
class GPalette;
class RectI;

extern ResourceInstance* constructBitmapBMP(std::iostream &stream);
extern ResourceInstance* constructBitmapPNG(std::iostream &stream);
extern ResourceInstance* constructBitmapJPEG(std::iostream &stream);

#ifdef TORQUE_OS_IOS
extern ResourceInstance* constructBitmapPVR(std::iostream &stream);
#endif

//------------------------------------------------------------------------------
//-------------------------------------- GBitmap
//
class GBitmap: public ResourceInstance
{
   //-------------------------------------- public enumerants and structures
  public:
   /// GFXFormat and UsageHint are
   ///  written to the stream in write(...),
   ///  be sure to maintain compatability
   ///  if they are changed.
//   enum GFXFormat {
//      Palettized = 0,
//      Intensity  = 1,
//      RGB        = 2,
//      RGBA       = 3,
//      Alpha      = 4,
//      RGB565     = 5,
//      RGB5551    = 6,
//      Luminance  = 7,
//      LuminanceAlpha = 8
//#ifdef TORQUE_OS_IOS
//       , PVR2 = 8,
//       PVR2A = 9,
//       PVR4 = 10,
//       PVR4A = 11
//#endif
//   };

   enum Constants {
      c_maxMipLevels = 12 //(2^(12 + 1) = 2048)
   };

  public:

   static GBitmap *load(const char *path);
   static ResourceObject * findBmpResource(const char * path);

   GBitmap();
   GBitmap(const GBitmap&);
   GBitmap(const U32  in_width,
           const U32  in_height,
           const bool in_extrudeMipLevels = false,
           const GFXFormat in_format = GFXFormatR8G8B8);
   virtual ~GBitmap();

   void allocateBitmap(const size_t  in_width,
                       const size_t  in_height,
                       const bool in_extrudeMipLevels = false,
                       const GFXFormat in_format = GFXFormatR8G8B8);

   void extrudeMipLevels(bool clearBorders = false);
   void extrudeMipLevelsDetail();

   GBitmap *createPowerOfTwoBitmap();
   U16* create16BitBitmap( GFXFormat *GLformat );

   void copyRect(const GBitmap *src, const RectI &srcRect, const Point2I &dstPoint);

   GFXFormat getFormat()       const;
   bool         setFormat(GFXFormat fmt);
   U32          getNumMipLevels() const;
   U32          getWidth(const U32 in_mipLevel  = 0) const;
   U32          getHeight(const U32 in_mipLevel = 0) const;

   U8*         getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel = U32(0));
   const U8*   getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel = U32(0)) const;

   const U8*   getBits(const U32 in_mipLevel = 0) const;
   U8*         getWritableBits(const U32 in_mipLevel = 0);

   bool        getColorBGRA(const U32 x, const U32 y, ColorI& rColor) const;
   bool        setColorBGRA(const U32 x, const U32 y, ColorI& rColor);
   bool        getColor(const U32 x, const U32 y, ColorI& rColor) const;
   bool        setColor(const U32 x, const U32 y, ColorI& rColor);

    
    
   /// Note that on set palette, the bitmap deletes its palette.
//   GPalette const* getPalette() const;
//   void            setPalette(GPalette* in_pPalette);

   //-------------------------------------- Internal data/operators
   static U32 sBitmapIdSource;

   void deleteImage();

  public:

    GFXFormat mInternalFormat;
    
    U8* mBits; // Master bytes
    U32 mByteSize;
    U32 mWidth;
    U32 mHeight;
    U32 mBytesPerPixel;
    
    U32 mNumMipLevels;
    U32 mMipLevelOffsets[c_maxMipLevels];

   bool mForce16Bit;//-Mat some paletted images will always be 16bit
   GPalette* pPalette;      ///< Note that this palette pointer is ALWAYS
                            ///  owned by the bitmap, and will be
                            ///  deleted on exit, or written out on a
                            ///  write.

   //-------------------------------------- Input/Output interface
  public:
   bool readJPEG(std::iostream &io_rStream);              // located in bitmapJpeg.cc
   bool writeJPEG(std::iostream &io_rStream) const;

   bool readPNG(std::iostream &io_rStream);               // located in bitmapPng.cc
   bool writePNG(std::iostream &io_rStream, bool compressHard = false) const;
   bool writePNGUncompressed(std::iostream &io_rStream) const;

   bool readMSBmp(std::iostream &io_rStream);             // located in bitmapMS.cc
   bool writeMSBmp(std::iostream &io_rStream) const;      // located in bitmapMS.cc

#ifdef TORQUE_OS_IOS
    bool readPNGiPhone(std::istream &io_rStream);               // located in iPhoneUtil.mm
    bool readPvr(std::iostream &io_rStream);		// located in bitmapPvr.cc for IPHONE
    bool writePvr(Stream& io_rStream) const;
#endif
    
   bool read(std::iostream &io_rStream);
   bool write(std::iostream &io_rStream) const;

  private:
   bool _writePNG(std::iostream   &stream, U32 const, U32 const, U32 const) const;

   static const U32 csFileVersion;
};

//------------------------------------------------------------------------------
//-------------------------------------- Inlines
//

inline GFXFormat GBitmap::getFormat() const
{
   return mInternalFormat;
}

inline U32 GBitmap::getNumMipLevels() const
{
   return mNumMipLevels;
}

inline U32 GBitmap::getWidth(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getWidth: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   U32 retVal = mWidth >> in_mipLevel;

   return (retVal != 0) ? retVal : 1;
}

inline U32 GBitmap::getHeight(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("Bitmap::getHeight: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   U32 retVal = mHeight >> in_mipLevel;

   return (retVal != 0) ? retVal : 1;
}

//inline const GPalette* GBitmap::getPalette() const
//{
//   AssertFatal(getFormat() == Palettized,
//               "Error, incorrect internal format to return a palette");
//
//   return pPalette;
//}

inline const U8* GBitmap::getBits(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getBits: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   return &mBits[mMipLevelOffsets[in_mipLevel]];
}

inline U8* GBitmap::getWritableBits(const U32 in_mipLevel)
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getWritableBits: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   return &mBits[mMipLevelOffsets[in_mipLevel]];
}

inline U8* GBitmap::getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel)
{
   return (getWritableBits(mipLevel) + ((in_y * getWidth(mipLevel)) + in_x) * mBytesPerPixel);
}

inline const U8* GBitmap::getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel) const
{
   return (getBits(mipLevel) + ((in_y * getWidth(mipLevel)) + in_x) * mBytesPerPixel);
}


extern void (*bitmapExtrude5551)(const void *srcMip, void *mip, U32 height, U32 width);
extern void (*bitmapExtrudeRGB)(const void *srcMip, void *mip, U32 height, U32 width);
extern void (*bitmapConvertRGB_to_5551)(U8 *src, U32 pixels);
extern void (*bitmapExtrudePaletted)(const void *srcMip, void *mip, U32 height, U32 width);

void bitmapExtrudeRGB_c(const void *srcMip, void *mip, U32 height, U32 width);

#endif //_GBITMAP_H_
