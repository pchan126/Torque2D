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

#ifndef _MACGLUTILS_H_
#define _MACGLUTILS_H_

static Vector<NSOpenGLPixelFormatAttribute> _beginPixelFormatAttributesForDisplay(CGDirectDisplayID display)
{
   Vector<NSOpenGLPixelFormatAttribute> attributes;
   attributes.reserve(16); // Most attribute lists won't exceed this
   
//    attributes.push_back(NSOpenGLPFAScreenMask);
//   attributes.push_back((NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(display));
//   attributes.push_back(NSOpenGLPFANoRecovery);
   attributes.push_back(NSOpenGLPFADoubleBuffer);
   attributes.push_back(NSOpenGLPFAAccelerated);
//   attributes.push_back(NSOpenGLPFAAuxBuffers);
   attributes.push_back(NSOpenGLPFAOpenGLProfile);
   attributes.push_back(NSOpenGLProfileVersion3_2Core);
//   attributes.push_back((NSOpenGLPixelFormatAttribute)1);
   return attributes;
}

static void _addColorAlphaDepthStencilAttributes(Vector<NSOpenGLPixelFormatAttribute>& attributes, U32 color, U32 alpha, U32 depth, U32 stencil)
{
   attributes.push_back(NSOpenGLPFAColorSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)color);
   attributes.push_back(NSOpenGLPFAAlphaSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)alpha);
   attributes.push_back(NSOpenGLPFADepthSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)depth);
   attributes.push_back(NSOpenGLPFAStencilSize); attributes.push_back((NSOpenGLPixelFormatAttribute)stencil);
}

static void _addFullscreenAttributes(Vector<NSOpenGLPixelFormatAttribute>& attributes)
{
   attributes.push_back(NSOpenGLPFAFullScreen);
}

static void _endAttributeList(Vector<NSOpenGLPixelFormatAttribute>& attributes)
{
   attributes.push_back((NSOpenGLPixelFormatAttribute)0);
}

static Vector<NSOpenGLPixelFormatAttribute> _createStandardPixelFormatAttributesForDisplay(CGDirectDisplayID display)
{
   Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(display);
   _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
   _endAttributeList(attributes);
   
   return attributes;
}


static NSOpenGLPixelFormat* generateValidPixelFormat(bool fullscreen, U32 bpp, U32 samples)
{
   AssertWarn(bpp==16 || bpp==32 || bpp==0, "An unusual bit depth was requested in findValidPixelFormat(). clamping to 16|32");
   
   if (bpp)
      bpp = bpp > 16 ? 32 : 16;
   
   AssertWarn(samples <= 6, "An unusual multisample depth was requested in findValidPixelFormat(). clamping to 0...6");
   
   samples = samples > 6 ? 6 : samples;
   
   int i = 0;
   NSOpenGLPixelFormatAttribute attr[64];
   
   attr[i++] = NSOpenGLPFADoubleBuffer;
   attr[i++] = NSOpenGLPFANoRecovery;
   attr[i++] = NSOpenGLPFAAccelerated;
   attr[i++] = NSOpenGLPFAOpenGLProfile;
   attr[i++] = NSOpenGLProfileVersion3_2Core;
   
   if (fullscreen)
      attr[i++] = NSOpenGLPFAFullScreen;
   
   if(bpp != 0)
   {
      // native pixel formats are argb 1555 & argb 8888.
      U32 colorbits = 0;
      U32 alphabits = 0;
      
      if(bpp == 16)
      {
         colorbits = 5;             // ARGB 1555
         alphabits = 1;
      }
      else if(bpp == 32)
         colorbits = alphabits = 8; // ARGB 8888
      
      attr[i++] = NSOpenGLPFADepthSize;
      attr[i++] = (NSOpenGLPixelFormatAttribute)bpp;
      attr[i++] = NSOpenGLPFAColorSize;
      attr[i++] = (NSOpenGLPixelFormatAttribute)colorbits;
      attr[i++] = NSOpenGLPFAAlphaSize;
      attr[i++] = (NSOpenGLPixelFormatAttribute)alphabits;
   }
   
   if (samples != 0)
   {
      attr[i++] = NSOpenGLPFAMultisample;
      attr[i++] = (NSOpenGLPixelFormatAttribute)1;
      attr[i++] = NSOpenGLPFASamples;
      attr[i++] = (NSOpenGLPixelFormatAttribute)samples;
   }
   
   attr[i++] = 0;
   
   NSOpenGLPixelFormat* format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attr] autorelease];
   
   return format;
}

#endif