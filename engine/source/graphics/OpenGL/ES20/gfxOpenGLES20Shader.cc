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

#include <fstream>
#include "platform/platform.h"
#include "gfxOpenGLES20Shader.h"

#include "memory/frameAllocator.h"
#include "platform/platformString.h"
#include "math/mPoint.h"
#include "graphics/gfxStructs.h"
#include "console/console.h"
#include "gfxOpenGLES20EnumTranslate.h"
#include "StreamFn.h"


class GFXOpenGLES20ShaderConstHandle : public GFXShaderConstHandle
{
   friend class GFXOpenGLES20Shader;

public:  
   
   GFXOpenGLES20ShaderConstHandle( GFXOpenGLES20Shader *shader );
   GFXOpenGLES20ShaderConstHandle( GFXOpenGLES20Shader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum );
   virtual ~GFXOpenGLES20ShaderConstHandle();
   
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
   GFXOpenGLES20Shader* mShader;
   GLuint mLocation;
   U32 mOffset;
   U32 mSize;  
   S32 mSamplerNum; 
};

GFXOpenGLES20ShaderConstHandle::GFXOpenGLES20ShaderConstHandle( GFXOpenGLES20Shader *shader )
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

GFXOpenGLES20ShaderConstHandle::GFXOpenGLES20ShaderConstHandle( GFXOpenGLES20Shader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum ) 
 : mShader(shader)
{
   reinit(desc, loc, samplerNum);
}

void GFXOpenGLES20ShaderConstHandle::reinit( const GFXShaderConstDesc& desc, GLuint loc, S32 samplerNum )
{
   mDesc = desc;
   mLocation = loc;
   mSamplerNum = samplerNum;
   mOffset = 0;
   
   U32 elemSize = shaderConstTypeSize(mDesc.constType);
   AssertFatal(elemSize, "GFXOpenGLES20ShaderConst::GFXOpenGLES20ShaderConst - elemSize is 0");
   mSize = mDesc.arraySize * elemSize;
   mValid = true;
}


U32 GFXOpenGLES20ShaderConstHandle::getSize() const
{
   return mSize;
}

GFXOpenGLES20ShaderConstHandle::~GFXOpenGLES20ShaderConstHandle()
{
}

GFXOpenGLES20ShaderConstBuffer::GFXOpenGLES20ShaderConstBuffer(GFXOpenGLES20Shader* shader, U32 bufSize, U8* existingConstants):
                            GFXOpenGLShaderConstBuffer(shader, bufSize, existingConstants)
{
}

GFXOpenGLES20ShaderConstBuffer::~GFXOpenGLES20ShaderConstBuffer()
{
}


bool GFXOpenGLES20Shader::_init()
{
   // Don't initialize empty shaders.
   if ( mVertexFile == StringTable->EmptyString || mPixelFile == StringTable->EmptyString )
      return false;

   clearShaders();

   mAttributes.clear();
   mProgram = glCreateProgram();
   
   // Set the macros and add the global ones.
   Vector<GFXShaderMacro> macros;
//   macros.merge( mMacros );
//   macros.merge( smGlobalMacros );
//
//   // Add the shader version to the macros.
//   const U32 mjVer = (U32)mFloor( mPixVersion );
//   const U32 mnVer = (U32)( ( mPixVersion - F32( mjVer ) ) * 10.01f );
//   macros.increment();
//   macros.last().name = "TORQUE_SM";
//   macros.last().value = String::ToString( mjVer * 10 + mnVer );

   // Default to true so we're "successful" if a vertex/pixel shader wasn't specified.
   bool compiledVertexShader = true;
   bool compiledPixelShader = true;
   
   // Compile the vertex and pixel shaders if specified.
   if(mVertexFile != StringTable->EmptyString )
      compiledVertexShader = initShader(mVertexFile, true, macros);
   if(mPixelFile != StringTable->EmptyString )
      compiledPixelShader = initShader(mPixelFile, false, macros);
      
   // If either shader was present and failed to compile, bail.
   if(!compiledVertexShader || !compiledPixelShader)
      return false;

    for (int i = 0; i < mAttributes.size(); i++)
        glBindAttribLocation(mProgram, mAttributes[i], GFXGLShaderAttributes[mAttributes[i]]);

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
      glGetProgramInfoLog( mProgram, logLength, nullptr, log );

      if ( linkStatus == GL_FALSE )
      {
         if ( smLogErrors )
         {
            Con::errorf( "GFXOpenGLES20Shader::init - Error linking shader!" );
            Con::errorf( "Program %s / %s: %s", 
                mVertexFile, mPixelFile, log);
         }
      }
      else if ( smLogWarnings )
      {
         Con::warnf( "Program %s / %s: %s", 
             mVertexFile, mPixelFile, log);
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
   for ( auto biter:mActiveBuffers )
      ((GFXOpenGLES20ShaderConstBuffer*)(biter))->onShaderReload( this );
   
   return true;
}

bool GFXOpenGLES20Shader::_loadShaderFromStream(  GLuint shader,
                                            const StringTableEntry path,
                                            std::iostream &s,
                                            const Vector<GFXShaderMacro>& macros )
{
    Vector<char*> buffers;
    Vector<U32> lengths;
    
//    // Now add all the macros.
//    for( U32 i = 0; i < macros.size(); i++ )
//    {
//        String define = String::ToString( "#define %s %s\n", macros[i].name, macros[i].value );
//        buffers.push_back( dStrdup( define.c_str() ) );
//        lengths.push_back( define.length() );
//    }
    
    // Now finally add the shader source.
    U32 shaderLen = (U32)StreamFn::getStreamSize(s);
    char* buffer = (char*)dMalloc(shaderLen + 1);
    s.read(buffer, shaderLen);
    buffer[shaderLen] = 0;
    
    if ( !buffer )
        return false;
    
    for ( int i = 0; i < ATTRIB_TEXCOORD7+1; i++)
    {
        if (strstr(buffer, GFXGLShaderAttributes[i]))
            mAttributes.push_back(i);
    }

    buffers.push_back(buffer);
    lengths.push_back(shaderLen);

    glShaderSource(shader, (GLsizei)buffers.size(), (const GLchar**)const_cast<const char**>(buffers.address()), nullptr);

    // Cleanup the shader source buffer.
    for ( U32 i=0; i < buffers.size(); i++ )
        dFree( buffers[i] );

    glCompileShader(shader);

    return true;
}

bool GFXOpenGLES20Shader::initShader( const StringTableEntry file,
                                 bool isVertex,
                                 const Vector<GFXShaderMacro> &macros )
{
    GLuint activeShader = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    if(isVertex)
        mVertexShader = activeShader;
    else
        mPixelShader = activeShader;
    glAttachShader(mProgram, activeShader);
    
    //    char szFullPathBuffer[1024];
    //    Con::printf("%s", szFullPathBuffer);
    //    Con::expandScriptFilename( szFullPathBuffer, sizeof(szFullPathBuffer), file );
    
    // Ok it's not in the shader gen manager, so ask Torque for it
    std::fstream stream(file, std::fstream::in);
    if ( !stream )
    {
        AssertISV(false, avar("GFXOpenGLShader::initShader - failed to open shader '%s'.", file));
        
        if ( smLogErrors )
            Con::errorf( "GFXOpenGLShader::initShader - Failed to open shader file '%s'.",
                        file );
        
        return false;
    }
    
    if ( !_loadShaderFromStream( activeShader, file, stream, macros ) )
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
        glGetShaderInfoLog(activeShader, logLength, nullptr, log);
        
        // Always print errors
        glGetShaderiv( activeShader, GL_COMPILE_STATUS, &compileStatus );
        
        if ( compileStatus == GL_FALSE )
        {
            if ( smLogErrors )
            {
                Con::errorf( "GFXOpenGLShader::initShader - Error compiling shader!" );
                Con::errorf( "Program %s: %s", file, log );
            }
        }
        else if ( smLogWarnings )
            Con::warnf( "Program %s: %s", file, log );
    }
    
    return compileStatus != GL_FALSE;
}
