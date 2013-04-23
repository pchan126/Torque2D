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

#ifndef TORQUE_GFX_GL32_GFXGLUTILS_H_
#define TORQUE_GFX_GL32_GFXGLUTILS_H_

#include "./gfxOpenGL32Device.h"
//#include "core/util/preprocessorHelpers.h"
/// @defgroup preprocess_helpers Preprocessor Helpers
/// These are some handy preprocessor macros to simplify certain tasks, like
/// preprocessor concatenation that works properly with __LINE__.

#define _TORQUE_CONCAT(x, y) x ## y

/// @ingroup preprocess_helpers
/// This command concatenates two tokens in a way that will work with things such
/// as __LINE__.
/// @hideinitializer
#define TORQUE_CONCAT(x, y) _TORQUE_CONCAT(x, y)


#include "./gfxOpenGL32EnumTranslate.h"

static inline GLenum minificationFilter(U32 minFilter, U32 mipFilter, U32 mipLevels)
{
   if(mipLevels == 1)
      return GFXGLTextureFilter[minFilter];

   // the compiler should interpret this as array lookups
   switch( minFilter ) 
   {
      case GFXTextureFilterLinear:
         switch( mipFilter ) 
         {
         case GFXTextureFilterLinear:
            return GL_LINEAR_MIPMAP_LINEAR;
         case GFXTextureFilterPoint:
            return GL_LINEAR_MIPMAP_NEAREST;
         default: 
            return GL_LINEAR;
         }
      default:
         switch( mipFilter ) {
      case GFXTextureFilterLinear:
         return GL_NEAREST_MIPMAP_LINEAR;
      case GFXTextureFilterPoint:
         return GL_NEAREST_MIPMAP_NEAREST;
      default:
         return GL_NEAREST;
         }
   }
}

/// Simple class which preserves a given GL integer.
/// This class determines the integer to preserve on construction and restores 
/// it on destruction.
class GFXOpenGLPreserveInteger
{
public:
   typedef void(*BindFn)(GLenum, GLuint);

   /// Preserve the integer.
   /// @param binding The binding which should be set on destruction.
   /// @param getBinding The parameter to be passed to glGetIntegerv to determine
   /// the integer to be preserved.
   /// @param binder The gl function to call to restore the integer.
   GFXOpenGLPreserveInteger(GLenum binding, GLint getBinding, BindFn binder) :
      mBinding(binding), mPreserved(0), mBinder(binder)
   {
      AssertFatal(mBinder, "GFXOpenGLPreserveInteger - Need a valid binder function");
      glGetIntegerv(getBinding, &mPreserved);
   }
   
   /// Restores the integer.
   ~GFXOpenGLPreserveInteger()
   {
      mBinder(mBinding, mPreserved);
   }

private:
   GLenum mBinding;
   GLint mPreserved;
   BindFn mBinder;
};

/// Helper macro to preserve the current VBO binding.
#define PRESERVE_VERTEX_BUFFER() \
GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING, glBindBuffer)

/// Helper macro to preserve the current element array binding.
#define PRESERVE_INDEX_BUFFER() \
GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING, glBindBuffer)

/// Helper macro to preserve the current 2D texture binding.
#define PRESERVE_2D_TEXTURE() \
GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D, glBindTexture)

// No 3D texture in OpenGL 2.0
///// Helper macro to preserve the current 3D texture binding.
//#define PRESERVE_3D_TEXTURE() \
//GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D, glBindTexture)

#define PRESERVE_FRAMEBUFFER() \
GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_READ_FRAMEBUFFER, GL_READ_FRAMEBUFFER_BINDING, glBindFramebuffer);\
GFXOpenGLPreserveInteger TORQUE_CONCAT(preserve2_, __LINE__) (GL_DRAW_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER_BINDING, glBindFramebuffer)

#endif
