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
#include "./gfxOpenGL32Shader.h"

#include "memory/frameAllocator.h"
#include "io/fileStream.h"
#include "platform/platformString.h"
#include "math/mPoint.h"
#include "graphics/gfxStructs.h"
#include "console/console.h"
#include "./gfxOpenGL32EnumTranslate.h"

//#if (defined TORQUE_OS_IPHONE || defined TORQUE_OS_MAC)
//#import <GLKit/GLKit.h>
//#endif

class GFXOpenGL32ShaderConstHandle : public GFXShaderConstHandle
{
   friend class GFXOpenGL32Shader;

public:  
   
   GFXOpenGL32ShaderConstHandle( GFXOpenGL32Shader *shader );
   GFXOpenGL32ShaderConstHandle( GFXOpenGL32Shader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum );
   virtual ~GFXOpenGL32ShaderConstHandle();
   
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
   GFXOpenGL32Shader* mShader;
   GLuint mLocation;
   U32 mOffset;
   U32 mSize;  
   S32 mSamplerNum; 
};

GFXOpenGL32ShaderConstHandle::GFXOpenGL32ShaderConstHandle( GFXOpenGL32Shader *shader )
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

GFXOpenGL32ShaderConstHandle::GFXOpenGL32ShaderConstHandle( GFXOpenGL32Shader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum ) 
 : mShader(shader)
{
   reinit(desc, loc, samplerNum);
}

void GFXOpenGL32ShaderConstHandle::reinit( const GFXShaderConstDesc& desc, GLuint loc, S32 samplerNum )
{
   mDesc = desc;
   mLocation = loc;
   mSamplerNum = samplerNum;
   mOffset = 0;
   
   U32 elemSize = shaderConstTypeSize(mDesc.constType);
   AssertFatal(elemSize, "GFXOpenGL32ShaderConst::GFXOpenGL32ShaderConst - elemSize is 0");
   mSize = mDesc.arraySize * elemSize;
   mValid = true;
}


U32 GFXOpenGL32ShaderConstHandle::getSize() const
{
   return mSize;
}

GFXOpenGL32ShaderConstHandle::~GFXOpenGL32ShaderConstHandle()
{
}

GFXOpenGL32ShaderConstBuffer::GFXOpenGL32ShaderConstBuffer(GFXOpenGL32Shader* shader, U32 bufSize, U8* existingConstants)
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

GFXOpenGL32ShaderConstBuffer::~GFXOpenGL32ShaderConstBuffer()
{
   delete[] mBuffer;

   if ( mShader )
      mShader->_unlinkBuffer( this );
}

template<typename ConstType>
void GFXOpenGL32ShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const ConstType& param)
{
   AssertFatal(handle, "GFXOpenGL32ShaderConstBuffer::internalSet - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGL32ShaderConstBuffer::internalSet - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXOpenGL32ShaderConstHandle*>(handle), "GFXOpenGL32ShaderConstBuffer::set - Incorrect const buffer type");

   GFXOpenGL32ShaderConstHandle* _glHandle = static_cast<GFXOpenGL32ShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGL32ShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
   dMemcpy(mBuffer + _glHandle->mOffset, &param, sizeof(ConstType));
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const F32 fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4F& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const PlaneF& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const S32 fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2I& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3I& fv)
{
   internalSet(handle, fv);
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4I& fv)
{
   internalSet(handle, fv);
}

//template<typename ConstType>
//void GFXOpenGL32ShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv)
//{
//   AssertFatal(handle, "GFXOpenGL32ShaderConstBuffer::internalSet - Handle is NULL!" );
//   AssertFatal(handle->isValid(), "GFXOpenGL32ShaderConstBuffer::internalSet - Handle is not valid!" );
//   AssertFatal(dynamic_cast<GFXOpenGL32ShaderConstHandle*>(handle), "GFXOpenGL32ShaderConstBuffer::set - Incorrect const buffer type");
//
//   GFXOpenGL32ShaderConstHandle* _glHandle = static_cast<GFXOpenGL32ShaderConstHandle*>(handle);
//   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGL32ShaderConstBuffer::set - Should only set handles which are owned by our shader");
//   const U8* fvBuffer = static_cast<const U8*>(fv.getBuffer());
//   for(U32 i = 0; i < fv.size(); ++i)
//   {
//      dMemcpy(mBuffer + _glHandle->mOffset + i * sizeof(ConstType), fvBuffer, sizeof(ConstType));
//      fvBuffer += fv.getElementSize();
//   }
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv)   
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv)
//{
//   internalSet(handle, fv);
//}
//
//void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv)
//{
//   internalSet(handle, fv);
//}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType)
{
   AssertFatal(handle, "GFXOpenGL32ShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGL32ShaderConstBuffer::set - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXOpenGL32ShaderConstHandle*>(handle), "GFXOpenGL32ShaderConstBuffer::set - Incorrect const buffer type");

   GFXOpenGL32ShaderConstHandle* _glHandle = static_cast<GFXOpenGL32ShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGL32ShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
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
      AssertFatal(false, "GFXOpenGL32ShaderConstBuffer::set - Invalid matrix type");
      break;
   }
}

void GFXOpenGL32ShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType)
{
   AssertFatal(handle, "GFXOpenGL32ShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXOpenGL32ShaderConstBuffer::set - Handle is not valid!" );

   GFXOpenGL32ShaderConstHandle* _glHandle = static_cast<GFXOpenGL32ShaderConstHandle*>(handle);
   AssertFatal(mShader == _glHandle->mShader, "GFXOpenGL32ShaderConstBuffer::set - Should only set handles which are owned by our shader");
   
   switch (matrixType) {
      case GFXSCT_Float4x4:
         dMemcpy(mBuffer + _glHandle->mOffset, (F32*)mat, _glHandle->getSize());
         break;
      default:
         AssertFatal(false, "GFXOpenGL32ShaderConstBuffer::set - setting array of non 4x4 matrices!");
         break;
   }
}

void GFXOpenGL32ShaderConstBuffer::activate()
{
   mShader->setConstantsFromBuffer(this);
   mWasLost = false;
}

const String GFXOpenGL32ShaderConstBuffer::describeSelf() const
{
   return String();
}

void GFXOpenGL32ShaderConstBuffer::onShaderReload( GFXOpenGL32Shader *shader )
{
   AssertFatal( shader == mShader, "GFXOpenGL32ShaderConstBuffer::onShaderReload, mismatched shaders!" );

   delete[] mBuffer;
   mBuffer = new U8[mShader->mConstBufferSize];
   dMemset(mBuffer, 0, mShader->mConstBufferSize);
   mWasLost = true;
}

GFXOpenGL32Shader::GFXOpenGL32Shader() :
   mVertexShader(0),
   mPixelShader(0),
   mProgram(0),
   mConstBufferSize(0),
   mConstBuffer(NULL)
{
    mVertexFile = StringTable->EmptyString;
    mPixelFile = StringTable->EmptyString;
}

GFXOpenGL32Shader::~GFXOpenGL32Shader()
{
   clearShaders();
   for(HandleMap::iterator i = mHandles.begin(); i != mHandles.end(); i++)
      delete i->value;
   
   delete[] mConstBuffer;
}

void GFXOpenGL32Shader::clearShaders()
{
   glDeleteProgram(mProgram);
   glDeleteShader(mVertexShader);
   glDeleteShader(mPixelShader);
   
   mProgram = 0;
   mVertexShader = 0;
   mPixelShader = 0;
}

bool GFXOpenGL32Shader::_init()
{
   // Don't initialize empty shaders.
   if ( mVertexFile == StringTable->EmptyString || mPixelFile == StringTable->EmptyString )
      return false;

   clearShaders();

   mProgram = glCreateProgram();
   
   // Set the macros and add the global ones.
   Vector<GFXShaderMacro> macros;
   macros.merge( mMacros );
   macros.merge( smGlobalMacros );

   // Add the shader version to the macros.
   const U32 mjVer = (U32)mFloor( mPixVersion );
   const U32 mnVer = (U32)( ( mPixVersion - F32( mjVer ) ) * 10.01f );
   macros.increment();
   macros.last().name = "TORQUE_SM";
   macros.last().value = String::ToString( mjVer * 10 + mnVer );

   // Default to true so we're "successful" if a vertex/pixel shader wasn't specified.
   bool compiledVertexShader = true;
   bool compiledPixelShader = true;
   
   // Compile the vertex and pixel shaders if specified.
   compiledVertexShader = initShader(mVertexFile, true, macros);
   compiledPixelShader = initShader(mPixelFile, false, macros);
      
   // If either shader was present and failed to compile, bail.
   if(!compiledVertexShader || !compiledPixelShader)
      return false;

    glBindAttribLocation(mProgram, GLKVertexAttribPosition, "Position");
    glBindAttribLocation(mProgram, GLKVertexAttribColor, "SourceColor");
    glBindAttribLocation(mProgram, GLKVertexAttribNormal, "Normal");
    glBindAttribLocation(mProgram, GLKVertexAttribTexCoord0, "inTexCoord");
    glBindAttribLocation(mProgram, GLKVertexAttribTexCoord1, "inTexCoord2");
    // Link it!
   glLinkProgram( mProgram );

   GLint linkStatus;
   glGetProgramiv( mProgram, GL_LINK_STATUS, &linkStatus );
   
   // Dump the info log to the console
   U32 logLength = 0;
   glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, (GLint*)&logLength);
   if ( logLength )
   {
      FrameAllocatorMarker fam;
      char* log = (char*)fam.alloc( logLength );
      glGetProgramInfoLog( mProgram, logLength, NULL, log );

      if ( linkStatus == GL_FALSE )
      {
         if ( smLogErrors )
         {
            Con::errorf( "GFXOpenGL32Shader::init - Error linking shader!" );
            Con::errorf( "Program %s: %s",
                mVertexFile, log);
         }
      }
      else if ( smLogWarnings )
      {
         Con::warnf( "Program %s: %s",
             mVertexFile, log);
      }
   }

   // If we failed to link, bail.
   if ( linkStatus == GL_FALSE )
      return false;

   initConstantDescs();   
   initHandles();
   
   // Notify Buffers we might have changed in size. 
   // If this was our first init then we won't have any activeBuffers 
   // to worry about unnecessarily calling.
   Vector<GFXShaderConstBuffer*>::iterator biter = mActiveBuffers.begin();
   for ( ; biter != mActiveBuffers.end(); biter++ )   
      ((GFXOpenGL32ShaderConstBuffer*)(*biter))->onShaderReload( this );
   
   return true;
}

void GFXOpenGL32Shader::initConstantDescs()
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
            AssertFatal(false, "GFXOpenGL32Shader::initConstantDescs - unrecognized uniform type");
            // If we don't recognize the constant don't add its description.
            continue;
      }
      
      mConstants.push_back(desc);
   }
}

void GFXOpenGL32Shader::initHandles()
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
         mHandles[desc.name] = new GFXOpenGL32ShaderConstHandle( this, desc, loc, sampler );
      }
   }

   // Loop through handles once more to set their offset and calculate our
   // constBuffer size.

   if ( mConstBuffer )
      delete[] mConstBuffer;
   mConstBufferSize = 0;

   for ( HandleMap::iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter )
   {
      GFXOpenGL32ShaderConstHandle* handle = iter->value;
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
      GFXOpenGL32ShaderConstHandle* handle = iter->value;
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

GFXShaderConstHandle* GFXOpenGL32Shader::getShaderConstHandle(const String& name)
{
   HandleMap::iterator i = mHandles.find(name);
   if(i != mHandles.end())
      return i->value;
   else
   {
      GFXOpenGL32ShaderConstHandle* handle = new GFXOpenGL32ShaderConstHandle( this );
      mHandles[ name ] = handle;
      
      return handle;
   }
}

void GFXOpenGL32Shader::setConstantsFromBuffer(GFXOpenGL32ShaderConstBuffer* buffer)
{
   for(Vector<GFXOpenGL32ShaderConstHandle*>::iterator i = mValidHandles.begin(); i != mValidHandles.end(); ++i)
   {
      GFXOpenGL32ShaderConstHandle* handle = *i;
      AssertFatal(handle, "GFXOpenGL32Shader::setConstantsFromBuffer - Null handle");
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

GFXShaderConstBufferRef GFXOpenGL32Shader::allocConstBuffer()
{
   GFXOpenGL32ShaderConstBuffer* buffer = new GFXOpenGL32ShaderConstBuffer(this, mConstBufferSize, mConstBuffer);
   buffer->registerResourceWithDevice(getOwningDevice());
   mActiveBuffers.push_back( buffer );
   return buffer;
}

void GFXOpenGL32Shader::useProgram()
{
   glUseProgram(mProgram);
}

void GFXOpenGL32Shader::zombify()
{
   clearShaders();
   dMemset(mConstBuffer, 0, mConstBufferSize);
}

char* GFXOpenGL32Shader::_handleIncludes( const StringTableEntry path, FileStream *s )
{
   // TODO:  The #line pragma on GLSL takes something called a
   // "source-string-number" which it then never explains.
   //
   // Until i resolve this mystery i disabled this.
   //
   //String linePragma = String::ToString( "#line 1 \r\n");
   //U32 linePragmaLen = linePragma.length();

   U32 shaderLen = s->getStreamSize();
   char* buffer = (char*)dMalloc(shaderLen + 1);
   //dStrncpy( buffer, linePragma.c_str(), linePragmaLen );
   s->read(shaderLen, buffer);
   buffer[shaderLen] = 0;
   
//   char* p = dStrstr(buffer, (const char*)"#include");
//   while(p)
//   {
//      char* q = p;
//      p += 8;
//      if(dIsspace(*p))
//      {
//         U32 n = 0;
//         while(dIsspace(*p)) ++p;
//         AssertFatal(*p == '"', "Bad #include directive");
//         ++p;
//         static char includeFile[256];
//         while(*p != '"')
//         {
//            AssertFatal(*p != 0, "Bad #include directive");
//            includeFile[n++] = *p++;
//            AssertFatal(n < sizeof(includeFile), "#include directive too long");
//         }
//         ++p;
//         includeFile[n] = 0;
//
//         // First try it as a local file.
//         String includePath = String::Join(path.getPath(), '/', includeFile);
//         includePath = String::CompressPath(includePath);
//         
//         FileStream includeStream;
//
//         if ( !includeStream.open( includePath.getFullFileName(), FileStream::Read ) )
//         {
//            // Try again assuming the path is absolute 
//            // and/or relative.
//            includePath = String( includeFile );
//            includePath = String::CompressPath(includePath);
//            if ( !includeStream.open( includePath.getFullFileName(), FileStream::Read ) )
//            {
//               AssertISV(false, avar("failed to open include '%s'.", includePath.getFullPath().c_str()));
//
//               if ( smLogErrors )
//                  Con::errorf( "GFXOpenGL32Shader::_handleIncludes - Failed to open include '%s'.", 
//                     includePath.getFullPath().c_str() );
//
//               // Fail... don't return the buffer.
//               dFree(buffer);
//               return NULL;
//            }
//         }
//
//         char* includedText = _handleIncludes(includePath, &includeStream);
//         
//         // If a sub-include fails... cleanup and return.
//         if ( !includedText )
//         {
//            dFree(buffer);
//            return NULL;
//         }
//         
//         // TODO: Disabled till this is fixed correctly.
//         //
//         // Count the number of lines in the file 
//         // before the include.
//         /*
//         U32 includeLine = 0;
//         {
//            char* nl = dStrstr( buffer, "\n" );
//            while ( nl )
//            {
//               includeLine++;
//               nl = dStrstr( nl, "\n" );
//               if(nl) ++nl;
//            }
//         }
//         */
//
//         String manip(buffer);
//         manip.erase(q-buffer, p-q);
//         String sItx(includedText);
//
//         // TODO: Disabled till this is fixed correctly.
//         //
//         // Add a new line pragma to restore the proper
//         // file and line number after the include.
//         //sItx += String::ToString( "\r\n#line %d \r\n", includeLine );
//         
//         dFree(includedText);
//         manip.insert(q-buffer, sItx);
//         char* manipBuf = dStrdup(manip.c_str());
//         p = manipBuf + (p - buffer);
//         dFree(buffer);
//         buffer = manipBuf;
//      }
//      p = dStrstr(p, (const char*)"#include");
//   }
   
   return buffer;
}


bool GFXOpenGL32Shader::_loadShaderFromStream(  GLuint shader,
                                  const StringTableEntry path,
                                  FileStream* s,
                                  const Vector<GFXShaderMacro>& macros )
{
   Vector<char*> buffers;
   Vector<U32> lengths;
   
   // The GLSL version declaration must go first!
   const char *versionDecl = "#version 150\r\n\r\n";
   buffers.push_back( dStrdup( versionDecl ) );
   lengths.push_back( dStrlen( versionDecl ) );

   // Now add all the macros.
   for( U32 i = 0; i < macros.size(); i++ )
   {
      String define = String::ToString( "#define %s %s\n", macros[i].name.c_str(), macros[i].value.c_str() );
      buffers.push_back( dStrdup( define.c_str() ) );
      lengths.push_back( define.length() );
   }
   
   // Now finally add the shader source.
   U32 shaderLen = s->getStreamSize();
   char *buffer = _handleIncludes(path, s);
   if ( !buffer )
      return false;
   
   buffers.push_back(buffer);
   lengths.push_back(shaderLen);
   
   glShaderSource(shader, buffers.size(), (const GLchar**)const_cast<const char**>(buffers.address()), NULL);

   // Cleanup the shader source buffer.
   for ( U32 i=0; i < buffers.size(); i++ )
      dFree( buffers[i] );

   glCompileShader(shader);

   return true;
}

bool GFXOpenGL32Shader::initShader( const StringTableEntry file,
                              bool isVertex, 
                              const Vector<GFXShaderMacro> &macros )
{
   GLuint activeShader = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
   if(isVertex)
      mVertexShader = activeShader;
   else
      mPixelShader = activeShader;
   glAttachShader(mProgram, activeShader);
   
   // Ok it's not in the shader gen manager, so ask Torque for it
   FileStream stream;
   if ( !stream.open( file, FileStream::Read ) )
   {
      AssertISV(false, avar("GFXOpenGL32Shader::initShader - failed to open shader '%s'.", file));

      if ( smLogErrors )
         Con::errorf( "GFXOpenGL32Shader::initShader - Failed to open shader file '%s'.", 
            file );

      return false;
   }
   
   if ( !_loadShaderFromStream( activeShader, file, &stream, macros ) )
      return false;
   
   GLint compile;
   glGetShaderiv(activeShader, GL_COMPILE_STATUS, &compile);

   // Dump the info log to the console
   U32 logLength = 0;
   glGetShaderiv(activeShader, GL_INFO_LOG_LENGTH, (GLint*)&logLength);
   
   GLint compileStatus = GL_TRUE;
   if ( logLength )
   {
      FrameAllocatorMarker fam;
      char* log = (char*)fam.alloc(logLength);
      glGetShaderInfoLog(activeShader, logLength, NULL, log);

      // Always print errors
      glGetShaderiv( activeShader, GL_COMPILE_STATUS, &compileStatus );

      if ( compileStatus == GL_FALSE )
      {
         if ( smLogErrors )
         {
            Con::errorf( "GFXOpenGL32Shader::initShader - Error compiling shader!" );
            Con::errorf( "Program %s: %s", file, log );
         }
      }
      else if ( smLogWarnings )
         Con::warnf( "Program %s: %s", file, log );
   }

   return compileStatus != GL_FALSE;
}

/// Returns our list of shader constants, the material can get this and just set the constants it knows about
const Vector<GFXShaderConstDesc>& GFXOpenGL32Shader::getShaderConstDesc() const
{
   return mConstants;
}

/// Returns the alignment value for constType
U32 GFXOpenGL32Shader::getAlignmentValue(const GFXShaderConstType constType) const
{
   // Alignment is the same thing as size for us.
   return shaderConstTypeSize(constType);
}

const String GFXOpenGL32Shader::describeSelf() const
{
   String ret;
   ret = String::ToString("   Program: %i", mProgram);
   ret += String::ToString("   Vertex Path: %s", mVertexFile);
   ret += String::ToString("   Pixel Path: %s", mPixelFile);
   
   return ret;
}
