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

#include "platform/platform.h"
#include "platform/platformGL.h"
#include "./gfxOpenGLShader.h"

#include "memory/frameAllocator.h"
#include "io/fileStream.h"
#include "platform/platformString.h"
#include "math/mPoint.h"
#include "graphics/gfxStructs.h"
#include "console/console.h"
#include "./gfxOpenGLEnumTranslate.h"


class GFXOpenGLShaderConstHandle : public GFXShaderConstHandle
{
   friend class GFXOpenGLShader;

public:  
   
   GFXOpenGLShaderConstHandle( GFXOpenGLShader *shader );
   GFXOpenGLShaderConstHandle( GFXOpenGLShader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum );
   virtual ~GFXOpenGLShaderConstHandle();
   
   void reinit( const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum );

   const String& getName() const { return mDesc.name; }
   GFXShaderConstType getType() const { return mDesc.constType; }
   U32 getArraySize() const { return mDesc.arraySize; }

   U32 getSize() const;
   void setValid( bool valid ) { mValid = valid; }   
   /// @warning This will always return the value assigned when the shader was
   /// initialized.  If the value is later changed this method won't reflect that.
   S32 getSamplerRegister() const { return mSamplerNum; }

   GFXShaderConstDesc mDesc;
   GFXOpenGLShader* mShader;
   GLuint mLocation;
   U32 mOffset;
   U32 mSize;  
   S32 mSamplerNum; 
};

GFXOpenGLShaderConstHandle::GFXOpenGLShaderConstHandle( GFXOpenGLShader *shader )
 : mShader( shader ), mSamplerNum(-1)
{
   mValid = false;
}

static U32 shaderConstTypeSize(GFXShaderConstType type)
{
   switch(type) 
   {
   case GFXSCT_Float:
   case GFXSCT_Int:
   case GFXSCT_Sampler:
   case GFXSCT_SamplerCube:
      return 4;
   case GFXSCT_Float2:
   case GFXSCT_Int2:
      return 8;
   case GFXSCT_Float3:
   case GFXSCT_Int3:
      return 12;
   case GFXSCT_Float4:
   case GFXSCT_Int4:
      return 16;
   case GFXSCT_Float2x2:
      return 16;
   case GFXSCT_Float3x3:
      return 36;
   case GFXSCT_Float4x4:
      return 64;
   default:
      AssertFatal(false,"shaderConstTypeSize - Unrecognized constant type");
      return 0;
   }
}

GFXOpenGLShaderConstHandle::GFXOpenGLShaderConstHandle( GFXOpenGLShader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum ) 
 : mShader(shader)
{
   reinit(desc, loc, samplerNum);
}

void GFXOpenGLShaderConstHandle::reinit( const GFXShaderConstDesc& desc, GLuint loc, S32 samplerNum )
{
   mDesc = desc;
   mLocation = loc;
   mSamplerNum = samplerNum;
   mOffset = 0;
   
   U32 elemSize = shaderConstTypeSize(mDesc.constType);
   AssertFatal(elemSize, "GFXOpenGLShaderConst::GFXOpenGLShaderConst - elemSize is 0");
   mSize = mDesc.arraySize * elemSize;
   mValid = true;
}


U32 GFXOpenGLShaderConstHandle::getSize() const
{
   return mSize;
}

GFXOpenGLShaderConstHandle::~GFXOpenGLShaderConstHandle()
{
}

GFXOpenGLShaderConstBuffer::GFXOpenGLShaderConstBuffer(GFXOpenGLShader* shader, U32 bufSize, U8* existingConstants)
{
   mShader = shader;
   mBuffer = new U8[bufSize];
   mWasLost = true;

   // Copy the existing constant buffer to preserve sampler numbers
   /// @warning This preserves a lot more than sampler numbers, obviously. If there
   /// is any code that assumes a new constant buffer will have everything set to
   /// 0, it will break.
   dMemcpy(mBuffer, existingConstants, bufSize);
}

GFXOpenGLShaderConstBuffer::~GFXOpenGLShaderConstBuffer()
{
   delete[] mBuffer;

   if ( mShader )
      mShader->_unlinkBuffer( this );
}

template<typename ConstType>
void GFXOpenGLShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const ConstType& param)
{
   AssertFatal(handle, "GFXOpenGLShaderConstBuffer::internalSet - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGLShaderConstBuffer::internalSet - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXOpenGLShaderConstHandle*>(handle), "GFXOpenGLShaderConstBuffer::set - Incorrect const buffer type");

   GFXOpenGLShaderConstHandle* _glHandle = static_cast<GFXOpenGLShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGLShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
   dMemcpy(mBuffer + _glHandle->mOffset, &param, sizeof(ConstType));
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const F32 fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const PlaneF& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const S32 fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2I& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3I& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4I& fv)
{
   internalSet(handle, fv);
}

//template<typename ConstType>
//void GFXOpenGLShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv)
//{
//   AssertFatal(handle, "GFXOpenGLShaderConstBuffer::internalSet - Handle is NULL!" );
//   AssertFatal(handle->isValid(), "GFXOpenGLShaderConstBuffer::internalSet - Handle is not valid!" );
//   AssertFatal(dynamic_cast<GFXOpenGLShaderConstHandle*>(handle), "GFXOpenGLShaderConstBuffer::set - Incorrect const buffer type");
//
//   GFXOpenGLShaderConstHandle* _glHandle = static_cast<GFXOpenGLShaderConstHandle*>(handle);
//   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGLShaderConstBuffer::set - Should only set handles which are owned by our shader");
//   const U8* fvBuffer = static_cast<const U8*>(fv.getBuffer());
//   for(U32 i = 0; i < fv.size(); ++i)
//   {
//      dMemcpy(mBuffer + _glHandle->mOffset + i * sizeof(ConstType), fvBuffer, sizeof(ConstType));
//      fvBuffer += fv.getElementSize();
//   }
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv)   
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv)
//{
//   internalSet(handle, fv);
//}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType)
{
   AssertFatal(handle, "GFXOpenGLShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGLShaderConstBuffer::set - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXOpenGLShaderConstHandle*>(handle), "GFXOpenGLShaderConstBuffer::set - Incorrect const buffer type");

   GFXOpenGLShaderConstHandle* _glHandle = static_cast<GFXOpenGLShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGLShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
   switch(matType)
   {
   case GFXSCT_Float2x2:
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[0] = mat[0];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[1] = mat[1];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[2] = mat[4];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[3] = mat[5];
      break;
   case GFXSCT_Float3x3:
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[0] = mat[0];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[1] = mat[1];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[2] = mat[2];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[3] = mat[4];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[4] = mat[5];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[5] = mat[6];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[6] = mat[8];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[7] = mat[9];
      reinterpret_cast<F32*>(mBuffer + _glHandle->mOffset)[8] = mat[10];
      break;
   case GFXSCT_Float4x4:
      dMemcpy(mBuffer + _glHandle->mOffset, (const F32*)mat, sizeof(MatrixF));
      break;
   default:
      AssertFatal(false, "GFXOpenGLShaderConstBuffer::set - Invalid matrix type");
      break;
   }
}

void GFXOpenGLShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType)
{
   AssertFatal(handle, "GFXOpenGLShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGLShaderConstBuffer::set - Handle is not valid!" );

   GFXOpenGLShaderConstHandle* _glHandle = static_cast<GFXOpenGLShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGLShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
   switch (matrixType) {
      case GFXSCT_Float4x4:
         dMemcpy(mBuffer + _glHandle->mOffset, (F32*)mat, _glHandle->getSize());
         break;
      default:
         AssertFatal(false, "GFXOpenGLShaderConstBuffer::set - setting array of non 4x4 matrices!");
         break;
   }
}

void GFXOpenGLShaderConstBuffer::activate()
{
   mShader->setConstantsFromBuffer(this);
   mWasLost = false;
}

const String GFXOpenGLShaderConstBuffer::describeSelf() const
{
   return String();
}

void GFXOpenGLShaderConstBuffer::onShaderReload( GFXOpenGLShader *shader )
{
   AssertFatal( shader == mShader, "GFXOpenGLShaderConstBuffer::onShaderReload, mismatched shaders!" );

   delete[] mBuffer;
   mBuffer = new U8[mShader->mConstBufferSize];
   dMemset(mBuffer, 0, mShader->mConstBufferSize);
   mWasLost = true;
}

GFXOpenGLShader::GFXOpenGLShader() :
   mVertexShader(0),
   mPixelShader(0),
   mProgram(0),
   mConstBufferSize(0),
   mConstBuffer(NULL)
{
    mVertexFile = StringTable->EmptyString;
    mPixelFile = StringTable->EmptyString;
}

GFXOpenGLShader::~GFXOpenGLShader()
{
   clearShaders();
   for(HandleMap::iterator i = mHandles.begin(); i != mHandles.end(); i++)
      delete i->value;
   
   delete[] mConstBuffer;
}

void GFXOpenGLShader::clearShaders()
{
   glDeleteProgram(mProgram);
   glDeleteShader(mVertexShader);
   glDeleteShader(mPixelShader);
   
   mProgram = 0;
   mVertexShader = 0;
   mPixelShader = 0;
}


void GFXOpenGLShader::initConstantDescs()
{
   mConstants.clear();
   GLint numUniforms;
   glGetProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
   GLint maxNameLength;
   glGetProgramiv(mProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
   FrameTemp<GLchar> uniformName(maxNameLength);
   
   for(U32 i = 0; i < numUniforms; i++)
   {
      GLint size;
      GLenum type;
      glGetActiveUniform(mProgram, i, maxNameLength, NULL, &size, &type, uniformName);
      GFXShaderConstDesc desc;
      
      desc.name = String((char*)uniformName);
       Con::printf("%s", (char*)uniformName);
      
      // Remove array brackets from the name
      desc.name = desc.name.substr(0, desc.name.find('['));
      
      // Insert $ to match D3D behavior of having a $ prepended to parameters to main.
      desc.name.insert(0, '$');
      desc.arraySize = size;
      
      switch(type)
      {
         case GL_FLOAT:
            desc.constType = GFXSCT_Float;
            break;
         case GL_FLOAT_VEC2:
            desc.constType = GFXSCT_Float2;
            break;
         case GL_FLOAT_VEC3:
            desc.constType = GFXSCT_Float3;
            break;
         case GL_FLOAT_VEC4:
            desc.constType = GFXSCT_Float4;
            break;
         case GL_INT:
            desc.constType = GFXSCT_Int;
            break;
         case GL_INT_VEC2:
            desc.constType = GFXSCT_Int2;
            break;
         case GL_INT_VEC3:
            desc.constType = GFXSCT_Int3;
            break;
         case GL_INT_VEC4:
            desc.constType = GFXSCT_Int4;
            break;
         case GL_FLOAT_MAT2:
            desc.constType = GFXSCT_Float2x2;
            break;
         case GL_FLOAT_MAT3:
            desc.constType = GFXSCT_Float3x3;
            break;
         case GL_FLOAT_MAT4:
            desc.constType = GFXSCT_Float4x4;
            break;
         case GL_SAMPLER_2D:
            desc.constType = GFXSCT_Sampler;
            break;
         case GL_SAMPLER_CUBE:
            desc.constType = GFXSCT_SamplerCube;
            break;
         default:
            AssertFatal(false, "GFXOpenGLShader::initConstantDescs - unrecognized uniform type");
            // If we don't recognize the constant don't add its description.
            continue;
      }
      
      mConstants.push_back(desc);
   }
}

void GFXOpenGLShader::initHandles()
{      
   // Mark all existing handles as invalid.
   // Those that are found when parsing the descriptions will then be marked valid again.
   for ( HandleMap::iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter )      
      (iter->value)->setValid( false );  
   mValidHandles.clear();

   // Loop through all ConstantDescriptions, 
   // if they aren't in the HandleMap add them, if they are reinitialize them.
   S32 assignedSamplerNum = 0;
   for ( U32 i = 0; i < mConstants.size(); i++ )
   {
      GFXShaderConstDesc &desc = mConstants[i];            

      // Index element 1 of the name to skip the '$' we inserted earier.
       
      GLint loc = glGetUniformLocation(mProgram, &desc.name.c_str()[1]);
      
       
      HandleMap::iterator handle = mHandles.find(desc.name);
      S32 sampler = (desc.constType == GFXSCT_Sampler || desc.constType == GFXSCT_SamplerCube) ?
         assignedSamplerNum++ : -1;
      if ( handle != mHandles.end() )
      {
         handle->value->reinit( desc, loc, sampler );         
      }
      else 
      {
          Con::printf("initHandles - %i %s", loc, &desc.name.c_str()[1]);
         mHandles[desc.name] = new GFXOpenGLShaderConstHandle( this, desc, loc, sampler );
      }
   }

   // Loop through handles once more to set their offset and calculate our
   // constBuffer size.

   if ( mConstBuffer )
      delete[] mConstBuffer;
   mConstBufferSize = 0;

   for ( HandleMap::iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter )
   {
      GFXOpenGLShaderConstHandle* handle = iter->value;
      if ( handle->isValid() )
      {
      	mValidHandles.push_back(handle);
         handle->mOffset = mConstBufferSize;
         mConstBufferSize += handle->getSize();
      }
   }
   
   mConstBuffer = new U8[mConstBufferSize];
   dMemset(mConstBuffer, 0, mConstBufferSize);
   
   // Set our program so uniforms are assigned properly.
   glUseProgram(mProgram);
   // Iterate through uniforms to set sampler numbers.
   for (HandleMap::iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter)
   {
      GFXOpenGLShaderConstHandle* handle = iter->value;
      if(handle->isValid() && (handle->getType() == GFXSCT_Sampler || handle->getType() == GFXSCT_SamplerCube))
      {
         // Set sampler number on our program.
         glUniform1i(handle->mLocation, handle->mSamplerNum);
         // Set sampler in constant buffer so it does not get unset later.
         dMemcpy(mConstBuffer + handle->mOffset, &handle->mLocation, handle->getSize());
      }
   }
   glUseProgram(0);
}

GFXShaderConstHandle* GFXOpenGLShader::getShaderConstHandle(const String& name)
{
   HandleMap::iterator i = mHandles.find(name);
   if(i != mHandles.end())
      return i->value;
   else
   {
      GFXOpenGLShaderConstHandle* handle = new GFXOpenGLShaderConstHandle( this );
      mHandles[ name ] = handle;
      
      return handle;
   }
}

void GFXOpenGLShader::setConstantsFromBuffer(GFXOpenGLShaderConstBuffer* buffer)
{
   for(Vector<GFXOpenGLShaderConstHandle*>::iterator i = mValidHandles.begin(); i != mValidHandles.end(); ++i)
   {
      GFXOpenGLShaderConstHandle* handle = *i;
      AssertFatal(handle, "GFXOpenGLShader::setConstantsFromBuffer - Null handle");
      if (handle != NULL)
      {
         // Don't set if the value has not be changed.
         if(dMemcmp(mConstBuffer + handle->mOffset, buffer->mBuffer + handle->mOffset, handle->getSize()) == 0)
            continue;
          
         // Copy new value into our const buffer and set in GL.
         dMemcpy(mConstBuffer + handle->mOffset, buffer->mBuffer + handle->mOffset, handle->getSize());
         switch(handle->mDesc.constType)
         {
            case GFXSCT_Float:
               glUniform1fv(handle->mLocation, handle->mDesc.arraySize, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float2:
               glUniform2fv(handle->mLocation, handle->mDesc.arraySize, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float3:
               glUniform3fv(handle->mLocation, handle->mDesc.arraySize, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float4:
               glUniform4fv(handle->mLocation, handle->mDesc.arraySize, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Int:
            case GFXSCT_Sampler:
            case GFXSCT_SamplerCube:
               glUniform1iv(handle->mLocation, handle->mDesc.arraySize, (GLint*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Int2:
               glUniform2iv(handle->mLocation, handle->mDesc.arraySize, (GLint*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Int3:
               glUniform3iv(handle->mLocation, handle->mDesc.arraySize, (GLint*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Int4:
               glUniform4iv(handle->mLocation, handle->mDesc.arraySize, (GLint*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float2x2:
               glUniformMatrix2fv(handle->mLocation, handle->mDesc.arraySize, GL_FALSE, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float3x3:
               glUniformMatrix3fv(handle->mLocation, handle->mDesc.arraySize, GL_FALSE, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
            case GFXSCT_Float4x4:
               glUniformMatrix4fv(handle->mLocation, handle->mDesc.arraySize, GL_FALSE, (GLfloat*)(mConstBuffer + handle->mOffset));
               break;
         }
      }
   }
}

GFXShaderConstBufferRef GFXOpenGLShader::allocConstBuffer()
{
   GFXOpenGLShaderConstBuffer* buffer = new GFXOpenGLShaderConstBuffer(this, mConstBufferSize, mConstBuffer);
   buffer->registerResourceWithDevice(getOwningDevice());
   mActiveBuffers.push_back( buffer );
   return buffer;
}

void GFXOpenGLShader::useProgram()
{
   glUseProgram(mProgram);
}

void GFXOpenGLShader::zombify()
{
   clearShaders();
   dMemset(mConstBuffer, 0, mConstBufferSize);
}


/// Returns our list of shader constants, the material can get this and just set the constants it knows about
const Vector<GFXShaderConstDesc>& GFXOpenGLShader::getShaderConstDesc() const
{
   return mConstants;
}

/// Returns the alignment value for constType
U32 GFXOpenGLShader::getAlignmentValue(const GFXShaderConstType constType) const
{
   // Alignment is the same thing as size for us.
   return shaderConstTypeSize(constType);
}

const String GFXOpenGLShader::describeSelf() const
{
   String ret;
   ret = String::ToString("   Program: %i", mProgram);
   ret += String::ToString("   Vertex Path: %s", mVertexFile);
   ret += String::ToString("   Pixel Path: %s", mPixelFile);
   
   return ret;
}
