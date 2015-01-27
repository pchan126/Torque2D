#import "./GFXiOSEnumTranslate.h"
#include "graphics/gfxEnums.h"
#import "./GFXiOSDevice.h"

GLenum GFXGLPrimType[GFXPT_COUNT];
GLenum GFXGLBlend[GFXBlend_COUNT];
GLenum GFXGLBlendOp[GFXBlendOp_COUNT];
GLenum GFXGLSamplerState[GFXSAMP_COUNT];
GLenum GFXGLTextureFilter[GFXTextureFilter_COUNT];
GLenum GFXGLTextureAddress[GFXAddress_COUNT];
GLenum GFXGLCmpFunc[GFXCmp_COUNT];
GLenum GFXGLStencilOp[GFXStencilOp_COUNT];
GLenum GFXGLTextureInternalFormat[GFXFormat_COUNT];
GLenum GFXGLTextureFormat[GFXFormat_COUNT];
GLenum GFXGLTextureType[GFXFormat_COUNT];
GLenum GFXGLBufferType[GFXBufferType_COUNT];
GLenum GFXGLCullMode[GFXCull_COUNT];
GLenum GFXGLFillMode[GFXFill_COUNT];
StringTableEntry GFXGLShaderAttributes[GFXAttribute_Count];

BOOL CheckForExtension(NSString *searchName)
{
   // For performance, the array can be created once and cached.
   static NSString *extensionsString = nil;
   if (extensionsString == nil)
      extensionsString = [[NSString alloc ]initWithUTF8String:(const char*)glGetString(GL_EXTENSIONS)];
   NSArray *extensionsNames = [extensionsString componentsSeparatedByString:@" "];
   return [extensionsNames containsObject: searchName];
}
