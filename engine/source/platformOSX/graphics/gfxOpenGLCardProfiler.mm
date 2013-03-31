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

#include "./gfxOpenGLCardProfiler.h"
#include "./GFXOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./osxGLUtils.h"
#include <OpenGL/gl3.h>

BOOL CheckForExtension(NSString *searchName)
{
    int num;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num);
    // For performance, the array can be created once and cached.
    for (int count = 0; count < num; count++)
    {
        NSString *extensionsString = [[NSString alloc ]initWithUTF8String:(const char*)glGetStringi(GL_EXTENSIONS, count)];
        NSComparisonResult result = [extensionsString compare: searchName];
        [extensionsString release];
        if (result == NSOrderedSame)
            return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// helper function for getGLCapabilities.
//  returns a new CGL context for platState.hDisplay
CGLContextObj getContextForCapsCheck(CGDirectDisplayID display)
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
    
    return (CGLContextObj)[context CGLContextObj];
   
    // if we can't get a good context, we can't check caps... this won't be good.
    Con::errorf("getContextForCapsCheck could not create a cgl context on the display for gl capabilities checking!");
    return NULL;
}



void GFXOpenGLCardProfiler::init()
{
//    CGLContextObj curr_ctx = CGLGetCurrentContext();
//    CGLContextObj temp_ctx =  getContextForCapsCheck(CGMainDisplayID());
//    
//    if (!temp_ctx)
//    {
//        Con::errorf("OpenGL may not be set up correctly!");
//        return;
//    }
    
//    CGLSetCurrentContext(temp_ctx);
    
//    AssertFatal(CGMainDisplayID(), "GFXOpenGLCardProfiler was called before a monitor was chosen!");

    mChipSet = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

   // get the major and minor parts of the GL version. These are defined to be
   // in the order "[major].[minor] [other]|[major].[minor].[release] [other] in the spec
   const char *versionStart = reinterpret_cast<const char*>(glGetString(GL_VERSION));
   const char *versionEnd = versionStart;
   // get the text for the version "x.x.xxxx "
   for( S32 tok = 0; tok < 2; ++tok )
   {
      char *text = dStrdup( versionEnd );
      dStrtok(text, ". ");
      versionEnd += dStrlen( text ) + 1;
      dFree( text );
   }

   mRendererString = "GL";
   mRendererString += String::SpanToString(versionStart, versionEnd - 1);

   mCardDescription = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
   mVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
   
//   mVideoMemory = static_cast<GFXOpenGLDevice*>(GFX)->getTotalVideoMemory();

   Parent::init();
   
//    if(queryProfile("GL::suppBlendMinMax"))
    // done. silently restore the old cgl context.
//    CGLSetCurrentContext(curr_ctx);
//    CGLDestroyContext(temp_ctx);
}

void GFXOpenGLCardProfiler::setupCardCapabilities()
{
    GLint maxTexSize;
    GLint maxDepthBits;
    GLint maxStencilBits;
    GLint numCompressedTexFormats;
    GLint maxShaderTextures;
    
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&maxShaderTextures);
    
//    glGetIntegerv(GL_DEPTH_BITS, &maxDepthBits);
//    glGetIntegerv(GL_STENCIL_BITS, &maxStencilBits);

    const char* versionString = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    Con::printf("OpenGl Shading Language Version: %s", versionString);
   
    // OpenGL doesn't have separate maximum width/height.
    setCapability("maxTextureWidth", maxTexSize);
    setCapability("maxTextureHeight", maxTexSize);
    setCapability("maxTextureSize", maxTexSize);
    
    setCapability("maxTextureImageUnits", maxShaderTextures);
 
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTexFormats);
    setCapability("numCompressedTextureFormats", numCompressedTexFormats);
    
//    setCapability("maxDepthBits", maxDepthBits);
//    setCapability("maxStencilBits", maxStencilBits);

    setCapability("GL::EXT_depth_bounds_test", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_depth_bounds_test"]));
    setCapability("GL::EXT_framebuffer_multisample_blit_scaled", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_framebuffer_multisample_blit_scaled"]));

    setCapability("GL::EXT_texture_compression_s3tc", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_texture_compression_s3tc"]));
    setCapability("GL::EXT_texture_filter_anisotropic", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_texture_filter_anisotropic"]));
    setCapability("GL::EXT_texture_mirror_clamp", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_texture_mirror_clamp"]));
    setCapability("GL::EXT_texture_sRGB_decode", CheckForExtension([NSString stringWithUTF8String:"GL_EXT_texture_sRGB_decode"]));
    
    setCapability("GL::APPLE_client_storage", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_client_storage"]));
    setCapability("GL::APPLE_container_object_shareable", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_container_object_shareable"]));
    setCapability("GL::APPLE_object_purgeable", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_object_purgeable"]));
    setCapability("GL::APPLE_rgb_422", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_rgb_422"]));
    setCapability("GL::APPLE_row_bytes", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_row_bytes"]));
    setCapability("GL::APPLE_texture_range", CheckForExtension([NSString stringWithUTF8String:"GL_APPLE_texture_range"]));
}

bool GFXOpenGLCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   // Just doing what the D3D9 layer does
   return 0;
}

bool GFXOpenGLCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
	// We assume if the format is valid that we can use it for any purpose.
   // This may not be the case, but we have no way to check short of in depth 
   // testing of every format for every purpose.  And by testing, I mean sitting
   // down and doing it by hand, because there is no OpenGL API to check these
   // things.
   return GFXGLTextureInternalFormat[fmt] != GL_ZERO;
}
