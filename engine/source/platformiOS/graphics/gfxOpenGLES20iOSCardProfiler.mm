//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GFXOpenGLES20iOSCardProfiler.h"
#include "./GFXOpenGLES20iOSDevice.h"
#include "./gfxOpenGLES20iOSEnumTranslate.h"
#import <OpenGLES/ES2/glext.h>

BOOL CheckForExtension(NSString *searchName)
{
    // For performance, the array can be created once and cached.
    static NSString *extensionsString = nil;
    if (extensionsString == nil)
        extensionsString = [[NSString alloc ]initWithUTF8String:(const char*)glGetString(GL_EXTENSIONS)];
    NSArray *extensionsNames = [extensionsString componentsSeparatedByString:@" "];
    return [extensionsNames containsObject: searchName];
}

void GFXOpenGLES20iOSCardProfiler::init()
{
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
   
//   mVideoMemory = static_cast<GFXOpenGLES20iOSDevice*>(GFX)->getTotalVideoMemory();

   Parent::init();
   
   // Set new enums here so if our profile script forces this to be false we keep the GL_ZEROs.
   if(queryProfile("GL::suppFloatTexture"))
   {
       GFXGLTextureType[GFXFormatR32F] = GL_FLOAT;
       GFXGLTextureType[GFXFormatR32G32B32A32F] = GL_FLOAT;
       GFXGLTextureFormat[GFXFormatR32G32B32A32F] = GL_RGBA;
   }
    
    if (queryProfile("GL::GL_OES_texture_half_float"))
    {
        GFXGLTextureType[GFXFormatR16F] = GL_HALF_FLOAT_OES;
        GFXGLTextureType[GFXFormatR16G16F] = GL_HALF_FLOAT_OES;
        GFXGLTextureType[GFXFormatR16G16B16A16F] = GL_HALF_FLOAT_OES;
        GFXGLTextureFormat[GFXFormatR16G16B16A16F] = GL_RGBA;
    }
    
    if(queryProfile("GL::suppBlendMinMax"))
    {
        GFXGLBlendOp[GFXBlendOpMin] = GL_MIN_EXT;
        GFXGLBlendOp[GFXBlendOpMax] = GL_MAX_EXT;
    }
    
    if(queryProfile("GL::GL_EXT_texture_rg"))
    {
        if (queryProfile("GL::GL_OES_texture_half_float"))
        {
            GFXGLTextureInternalFormat[GFXFormatR16F] = GL_RED_EXT;
            GFXGLTextureInternalFormat[GFXFormatR16G16F] = GL_RG_EXT;
            GFXGLTextureFormat[GFXFormatR16F] = GL_RED_EXT;
            GFXGLTextureFormat[GFXFormatR16G16F] = GL_RG_EXT;
            GFXGLTextureType[GFXFormatR16F] = GL_HALF_FLOAT_OES;
            GFXGLTextureType[GFXFormatR16G16F] = GL_HALF_FLOAT_OES;
        }
        if (queryProfile("GL::suppFloatTexture"))
        {
            GFXGLTextureInternalFormat[GFXFormatR32F] = GL_RED_EXT;
            GFXGLTextureInternalFormat[GFXFormatR32G32F] = GL_RG_EXT;
            GFXGLTextureFormat[GFXFormatR32F] = GL_RED_EXT;
            GFXGLTextureFormat[GFXFormatR32G32F] = GL_RG_EXT;
            GFXGLTextureType[GFXFormatR32F] = GL_FLOAT;
            GFXGLTextureType[GFXFormatR32G32F] = GL_FLOAT;
        }
        GFXGLTextureInternalFormat[GFXFormatR8] = GL_RED_EXT;
        GFXGLTextureInternalFormat[GFXFormatR8G8] = GL_RG_EXT;
        GFXGLTextureFormat[GFXFormatR8] = GL_RED_EXT;
        GFXGLTextureFormat[GFXFormatR8G8] = GL_RG_EXT;
        GFXGLTextureType[GFXFormatR8] = GL_UNSIGNED_BYTE;
        GFXGLTextureType[GFXFormatR8G8] = GL_UNSIGNED_BYTE;
    }
    
    if(queryProfile("GL::GL_APPLE_texture_format_BGRA8888"))
    {
        
    }
    
}

void GFXOpenGLES20iOSCardProfiler::setupCardCapabilities()
{
    GLint maxTexSize;
    GLint maxDepthBits;
    GLint maxStencilBits;
    GLint numCompressedTexFormats;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    glGetIntegerv(GL_DEPTH_BITS, &maxDepthBits);
    glGetIntegerv(GL_STENCIL_BITS, &maxStencilBits);

    const char* versionString = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    Con::printf("OpenGl Shading Language Version: %s", versionString);
   
    // OpenGL doesn't have separate maximum width/height.
    setCapability("maxTextureWidth", maxTexSize);
    setCapability("maxTextureHeight", maxTexSize);
    setCapability("maxTextureSize", maxTexSize);
 
    setCapability("maxDepthBits", maxDepthBits);
    setCapability("maxStencilBits", maxStencilBits);

    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTexFormats);
    setCapability("numCompressedTextureFormats", numCompressedTexFormats);

    // The GL_APPLE_copy_texture_levels extension builds on top of the functionality of the GL_EXT_texture_storage extension and allows a set of texture mipmaps to be copied from one texture to another. (iOS 6.0)
    setCapability("GL::GL_APPLE_copy_texture_levels", CheckForExtension(@"GL_APPLE_copy_texture_levels"));

    // The APPLE_framebuffer_multisample extension enables full-scene anti-aliasing. (iOS 4.0)
    setCapability("GL::GL_APPLE_framebuffer_multisample", CheckForExtension(@"GL_APPLE_framebuffer_multisample"));
    
    // The APPLE_rgb_422 (http://www.opengl.org/registry/specs/APPLE/rgb_422.txt) extension enables texturing from some common video formats. (iOS 4.0)
    setCapability("GL::GL_APPLE_rgb_422", CheckForExtension(@"GL_APPLE_rgb_422"));
    
    // The GL_APPLE_sync extension provides fine-grain synchronization to your app. It allows you to choose a subset of submitted OpenGL ES commands and block until those commands complete. (iOS 6.0)
    setCapability("GL::GL_APPLE_sync", CheckForExtension(@"GL_APPLE_sync"));
    
    setCapability("GL::GL_APPLE_texture_format_BGRA8888", CheckForExtension(@"GL_APPLE_texture_format_BGRA8888"));
    
    // The APPLE_texture_max_level and EXT_shader_texture_lod extensions provide more control over texture sampling. (iOS 4.0)
    setCapability("GL::GL_APPLE_texture_max_level", CheckForExtension(@"GL_APPLE_texture_max_level"));
    setCapability("GL::suppBlendMinMax", CheckForExtension(@"GL_EXT_blend_minmax"));
    
    // The EXT_color_buffer_half_float extension allows 16-bit floating point formats to be specified for a frame buffer's color renderbuffer. (iOS 5.0)
    setCapability("GL::GL_EXT_color_buffer_half_float", CheckForExtension(@"GL_EXT_color_buffer_half_float"));
    // The EXT_debug_label and EXT_debug_marker extensions allow you to annotate your OpenGL ES drawing code with information specific to your app. The OpenGL ES Performance Detective, the OpenGL ES Debugger, and the OpenGL ES Analyzer tools provided by Xcode all take advantage of these annotations. (iOS 5.0)
    setCapability("GL::GL_EXT_debug_label", CheckForExtension(@"GL_EXT_debug_label"));
    setCapability("GL::GL_EXT_debug_marker", CheckForExtension(@"GL_EXT_debug_marker"));
    
    // The EXT_framebuffer_discard extension can be used to improve the performance of applications that use depth buffers or multisample framebuffers. (iOS 4.0)
    setCapability("GL::GL_EXT_discard_framebuffer", CheckForExtension(@"GL_EXT_discard_framebuffer"));

    // The GL_EXT_map_buffer_range extension improves performance when you only need to modify a subset of a buffer object. (iOS 6.0)
    setCapability("GL::GL_EXT_map_buffer_range", CheckForExtension(@"GL_EXT_map_buffer_range"));
    
    // The EXT_occlusion_query_boolean extension allows your app to determine whether any pixels would be drawn by a primitive or by a group of primitives. (iOS 5.0)
    setCapability("GL::GL_EXT_occlusion_query_boolean", CheckForExtension(@"GL_EXT_occlusion_query_boolean"));
    setCapability("GL::GL_EXT_read_format_bgra", CheckForExtension(@"GL_EXT_read_format_bgra"));
    
    // The EXT_separate_shader_objects extension allows your app to specify separate vertex and fragment shader programs. (iOS 5.0)
    setCapability("GL::GL_EXT_separate_shader_objects", CheckForExtension(@"GL_EXT_separate_shader_objects"));
    
    // The GL_EXT_shader_framebuffer_fetch extension is only available to OpenGL ES 2.0 applications and provides access to the framebuffer data as an input to your fragment shader. (iOS 6.0)
    setCapability("GL::GL_EXT_shader_framebuffer_fetch", CheckForExtension(@"GL_EXT_shader_framebuffer_fetch"));
    
    // The APPLE_texture_max_level and EXT_shader_texture_lod extensions provide more control over texture sampling. (iOS 4.0)
    setCapability("GL::GL_EXT_shader_texture_lod", CheckForExtension(@"GL_EXT_shader_texture_lod"));
    
    // The EXT_shadow_samplers extension provides support for shadow maps. (iOS 5.0)
    setCapability("GL::GL_EXT_shadow_samplers", CheckForExtension(@"GL_EXT_shadow_samplers"));
    
    setCapability("GL::suppAnisotropic", CheckForExtension(@"GL_EXT_texture_filter_anisotropic"));
    
    // The EXT_texture_rg extension adds one-component and two-component texture formats suitable for use in programmable shaders. (iOS 5.0)
    setCapability("GL::GL_EXT_texture_rg", CheckForExtension(@"GL_EXT_texture_rg"));
    
    // The GL_EXT_texture_storage extension allows your app to specify the entire structure of a texture in a single call, allowing your textures to be optimized further by OpenGL ES. (iOS 6.0)
    setCapability("GL::GL_EXT_texture_storage", CheckForExtension(@"GL_EXT_texture_storage"));
    
    setCapability("GL::GL_IMG_read_format", CheckForExtension(@"GL_IMG_read_format"));
    setCapability("GL::GL_IMG_texture_compression_pvrtc", CheckForExtension(@"GL_IMG_texture_compression_pvrtc"));
    
    // The OES_depth_texture extension enables rendering real-time shadows using shadow maps. (iOS 4.0)
    setCapability("GL::GL_OES_depth_texture", CheckForExtension(@"GL_OES_depth_texture"));

    setCapability("GL::GL_OES_depth24", CheckForExtension(@"GL_OES_depth24"));
    setCapability("GL::GL_OES_element_index_uint", CheckForExtension(@"GL_OES_element_index_uint"));
    setCapability("GL::GL_OES_fbo_render_mipmap", CheckForExtension(@"GL_OES_fbo_render_mipmap"));
    setCapability("GL::GL_OES_mapbuffer", CheckForExtension(@"GL_OES_mapbuffer"));
    setCapability("GL::GL_OES_packed_depth_stencil", CheckForExtension(@"GL_OES_packed_depth_stencil"));
    setCapability("GL::GL_OES_rgb8_rgba8", CheckForExtension(@"GL_OES_rgb8_rgba8"));
    setCapability("GL::GL_OES_standard_derivatives", CheckForExtension(@"GL_OES_standard_derivatives"));
    
    // The OES_texture_float (http://www.khronos.org/registry/gles/extensions/OES/OES_texture_float.txt) and OES_texture_half_float (http://www.khronos.org/registry/gles/extensions/OES/OES_texture_float.txt) extensions adds texture formats with floating point components to enable High Dynamic Range rendering. (iOS 4.0)
    setCapability("GL::suppFloatTexture", CheckForExtension(@"GL_OES_texture_float"));
    setCapability("GL::GL_OES_texture_half_float", CheckForExtension(@"GL_OES_texture_half_float"));
    
    // The OES_vertex_array_object (http://www.khronos.org/registry/gles/extensions/OES/OES_vertex_array_object.txt) API allows caching of vertex array state, to decrease driver overhead. (iOS 4.0)   
    setCapability("GL::GL_OES_vertex_array_object", CheckForExtension(@"GL_OES_vertex_array_object"));
}

