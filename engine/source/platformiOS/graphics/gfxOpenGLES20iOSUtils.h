//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef TORQUE_GFX_GLES20IOS_GFXGLUTILS_H_
#define TORQUE_GFX_GLES20IOS_GFXGLUTILS_H_

/// @defgroup preprocess_helpers Preprocessor Helpers
/// These are some handy preprocessor macros to simplify certain tasks, like
/// preprocessor concatenation that works properly with __LINE__.

#define _TORQUE_CONCAT(x, y) x ## y

/// @ingroup preprocess_helpers
/// This command concatenates two tokens in a way that will work with things such
/// as __LINE__.
/// @hideinitializer
#define TORQUE_CONCAT(x, y) _TORQUE_CONCAT(x, y)


#include "platformiOS/graphics/gfxOpenGLES20iOSEnumTranslate.h"
#import <OpenGLES/ES2/glext.h>

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
class GFXOpenGLESPreserveInteger
{
public:
   typedef void(*BindFn)(GLenum, GLuint);

   /// Preserve the integer.
   /// @param binding The binding which should be set on destruction.
   /// @param getBinding The parameter to be passed to glGetIntegerv to determine
   /// the integer to be preserved.
   /// @param binder The gl function to call to restore the integer.
   GFXOpenGLESPreserveInteger(GLenum binding, GLint getBinding, BindFn binder) :
      mBinding(binding), mPreserved(0), mBinder(binder)
   {
      AssertFatal(mBinder, "GFXOpenGLESPreserveInteger - Need a valid binder function");
      glGetIntegerv(getBinding, &mPreserved);
   }
   
   /// Restores the integer.
   ~GFXOpenGLESPreserveInteger()
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
GFXOpenGLESPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING, glBindBuffer)

/// Helper macro to preserve the current element array binding.
#define PRESERVE_INDEX_BUFFER() \
GFXOpenGLESPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING, glBindBuffer)

/// Helper macro to preserve the current 2D texture binding.
#define PRESERVE_2D_TEXTURE() \
GFXOpenGLESPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D, glBindTexture)

#endif
