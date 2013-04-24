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
#include "platformiOS/graphics/gfxOpenGLESShader.h"

#include "memory/frameAllocator.h"
#include "io/fileStream.h"
#include "platform/platformString.h"
#include "math/mPoint.h"
#include "graphics/gfxStructs.h"
#include "console/console.h"
#include "./gfxOpenGLESEnumTranslate.h"



class GFXOpenGLESShaderConstHandle : public GFXShaderConstHandle
{
   friend class GFXOpenGLESShader;

public:  
   
   GFXOpenGLESShaderConstHandle( GFXOpenGLESShader *shader );
   GFXOpenGLESShaderConstHandle( GFXOpenGLESShader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum );
   virtual ~GFXOpenGLESShaderConstHandle();
   
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
   GFXOpenGLESShader* mShader;
   GLuint mLocation;
   U32 mOffset;
   U32 mSize;  
   S32 mSamplerNum; 
};

GFXOpenGLESShaderConstHandle::GFXOpenGLESShaderConstHandle( GFXOpenGLESShader *shader )
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

GFXOpenGLESShaderConstHandle::GFXOpenGLESShaderConstHandle( GFXOpenGLESShader *shader, const GFXShaderConstDesc &desc, GLuint loc, S32 samplerNum ) 
 : mShader(shader)
{
   reinit(desc, loc, samplerNum);
}

void GFXOpenGLESShaderConstHandle::reinit( const GFXShaderConstDesc& desc, GLuint loc, S32 samplerNum )
{
   mDesc = desc;
   mLocation = loc;
   mSamplerNum = samplerNum;
   mOffset = 0;
   
   U32 elemSize = shaderConstTypeSize(mDesc.constType);
   AssertFatal(elemSize, "GFXOpenGLESShaderConst::GFXOpenGLESShaderConst - elemSize is 0");
   mSize = mDesc.arraySize * elemSize;
   mValid = true;
}


U32 GFXOpenGLESShaderConstHandle::getSize() const
{
   return mSize;
}

GFXOpenGLESShaderConstHandle::~GFXOpenGLESShaderConstHandle()
{
}

GFXOpenGLESShaderConstBuffer::GFXOpenGLESShaderConstBuffer(GFXOpenGLESShader* shader, U32 bufSize, U8* existingConstants):
                            GFXOpenGLShaderConstBuffer(shader, bufSize, existingConstants)
{
}

GFXOpenGLESShaderConstBuffer::~GFXOpenGLESShaderConstBuffer()
{
}


bool GFXOpenGLESShader::_init()
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
      glGetProgramInfoLog( mProgram, logLength, NULL, log );

      if ( linkStatus == GL_FALSE )
      {
         if ( smLogErrors )
         {
            Con::errorf( "GFXOpenGLESShader::init - Error linking shader!" );
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
   Vector<GFXShaderConstBuffer*>::iterator biter = mActiveBuffers.begin();
   for ( ; biter != mActiveBuffers.end(); biter++ )   
      ((GFXOpenGLESShaderConstBuffer*)(*biter))->onShaderReload( this );
   
   return true;
}

bool GFXOpenGLESShader::_loadShaderFromStream(  GLuint shader,
                                            const StringTableEntry path,
                                            FileStream* s,
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
    U32 shaderLen = s->getStreamSize();
    char* buffer = (char*)dMalloc(shaderLen + 1);
    s->read(shaderLen, buffer);
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

    glShaderSource(shader, buffers.size(), (const GLchar**)const_cast<const char**>(buffers.address()), NULL);

    // Cleanup the shader source buffer.
    for ( U32 i=0; i < buffers.size(); i++ )
        dFree( buffers[i] );

    glCompileShader(shader);

    return true;
}

bool GFXOpenGLESShader::initShader( const StringTableEntry file,
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
    FileStream stream;
    if ( !stream.open( file, FileStream::Read ) )
    {
        AssertISV(false, avar("GFXOpenGLShader::initShader - failed to open shader '%s'.", file));
        
        if ( smLogErrors )
            Con::errorf( "GFXOpenGLShader::initShader - Failed to open shader file '%s'.",
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
                Con::errorf( "GFXOpenGLShader::initShader - Error compiling shader!" );
                Con::errorf( "Program %s: %s", file, log );
            }
        }
        else if ( smLogWarnings )
            Con::warnf( "Program %s: %s", file, log );
    }
    
    return compileStatus != GL_FALSE;
}