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

#include "./gfxOpenGL32CardProfiler.h"
#include "./GFXOpenGL32Device.h"
#include "./gfxOpenGL32EnumTranslate.h"
#include "platform/platformGL.h"

BOOL CheckForExtension(NSString *searchName)
{
   // For performance, the array can be created once and cached.
   static NSString *extensionsString = nil;
   if (extensionsString == nil)
      extensionsString = [[NSString alloc ]initWithUTF8String:(const char*)glGetString(GL_EXTENSIONS)];
   NSArray *extensionsNames = [extensionsString componentsSeparatedByString:@" "];
   return [extensionsNames containsObject: searchName];
}


//-----------------------------------------------------------------------------
// helper function for getGLCapabilities.
//  returns a new CGL context for platState.hDisplay
NSOpenGLContext* getContextForCapsCheck()
{
    // silently create an opengl context on the current display, so that
    // we can get valid renderer and capability info
    // some of the following code is from:
    //  http://developer.apple.com/technotes/tn2002/tn2080.html#TAN55
    
    // From the CG display id, we can create a pixel format & context
    // and with that context we can check opengl capabilities
//    CGOpenGLDisplayMask cglDisplayMask = CGDisplayIDToOpenGLDisplayMask(display);

    NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion3_2Core,
		0
	};

    NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
    NSOpenGLContext* context = [[[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil] autorelease];
    
    return context;
   
    // if we can't get a good context, we can't check caps... this won't be good.
    Con::errorf("getContextForCapsCheck could not create a cgl context on the display for gl capabilities checking!");
    return NULL;
}



void GFXOpenGL32OSXCardProfiler::init()
{
   Parent::init();
}


void GFXOpenGL32OSXCardProfiler::setupCardCapabilities()
{
//   NSOpenGLContext* tempCtx = getContextForCapsCheck();
//   [tempCtx makeCurrentContext];
   
   
   CGDirectDisplayID display = CGMainDisplayID ();
   CGOpenGLDisplayMask cglDisplayMask = CGDisplayIDToOpenGLDisplayMask (display);
   { // check capabilities of display represented by display mask
      CGLPixelFormatAttribute attribs[] = {kCGLPFADisplayMask,
         (CGLPixelFormatAttribute)cglDisplayMask, (CGLPixelFormatAttribute)0};
      CGLPixelFormatObj pixelFormat = NULL;
      GLint numPixelFormats = 0;
      CGLContextObj cglContext = 0;
      CGLContextObj curr_ctx = CGLGetCurrentContext ();
      
      CGLChoosePixelFormat (attribs, &pixelFormat, &numPixelFormats);
      if (pixelFormat) {
         CGLCreateContext (pixelFormat, NULL, &cglContext);
         CGLDestroyPixelFormat (pixelFormat);
         if (cglContext) {
            CGLSetCurrentContext (cglContext);
            Parent::setupCardCapabilities();
            
            GLint maxShaderTextures;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&maxShaderTextures);
            setCapability("maxTextureImageUnits", maxShaderTextures);
            
//            setCapability("GL::EXT_depth_bounds_test", CheckForExtension(@"GL_EXT_depth_bounds_test"));
//            setCapability("GL::EXT_framebuffer_multisample_blit_scaled", CheckForExtension(@"GL_EXT_framebuffer_multisample_blit_scaled"));
//            
//            setCapability("GL::EXT_texture_compression_s3tc", CheckForExtension(@"GL_EXT_texture_compression_s3tc"));
//            setCapability("GL::EXT_texture_filter_anisotropic", CheckForExtension(@"GL_EXT_texture_filter_anisotropic"));
//            setCapability("GL::EXT_texture_mirror_clamp", CheckForExtension(@"GL_EXT_texture_mirror_clamp"));
//            setCapability("GL::EXT_texture_sRGB_decode", CheckForExtension(@"GL_EXT_texture_sRGB_decode"));
//            
//            setCapability("GL::APPLE_client_storage", CheckForExtension(@"GL_APPLE_client_storage"));
//            setCapability("GL::APPLE_container_object_shareable", CheckForExtension(@"GL_APPLE_container_object_shareable"));
//            setCapability("GL::APPLE_object_purgeable", CheckForExtension(@"GL_APPLE_object_purgeable"));
//            setCapability("GL::APPLE_rgb_422", CheckForExtension(@"GL_APPLE_rgb_422"));
//            setCapability("GL::APPLE_row_bytes", CheckForExtension(@"GL_APPLE_row_bytes"));
//            setCapability("GL::APPLE_texture_range", CheckForExtension(@"GL_APPLE_texture_range"));
            CGLDestroyContext (cglContext);
         }
      }
      CGLSetCurrentContext (curr_ctx); // reset current CGL context
   }
   
   
//   [tempCtx release];
}

bool GFXOpenGL32OSXCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   return glfwExtensionSupported(query.c_str());
}

bool GFXOpenGL32OSXCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
	// We assume if the format is valid that we can use it for any purpose.
   // This may not be the case, but we have no way to check short of in depth 
   // testing of every format for every purpose.  And by testing, I mean sitting
   // down and doing it by hand, because there is no OpenGL API to check these
   // things.
   return GFXGLTextureInternalFormat[fmt] != GL_ZERO;
}
