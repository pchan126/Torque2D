//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30Shader_H_
#define _GFXOpenGLES30Shader_H_

#include "graphics/OpenGL/gfxOpenGLShader.h"

class GFXOpenGLES30ShaderConstHandle;
class GFXOpenGLES30ShaderConstBuffer;

class GFXOpenGLES30Shader : public GFXOpenGLShader
{
   typedef HashMap<String, GFXOpenGLES30ShaderConstHandle*> HandleMap;
public:
   GFXOpenGLES30Shader() {};
    virtual ~GFXOpenGLES30Shader() {};
   
protected:

   friend class GFXOpenGLES30ShaderConstBuffer;
   friend class GFXOpenGLES30ShaderConstHandle;
   
    bool _loadShaderFromStream(GLuint shader,
            const StringTableEntry path,
            std::iostream &s,
            const Vector<GFXShaderMacro>& macros);

    virtual bool _init();

    bool initShader(  const StringTableEntry file,
                    bool isVertex,
                    const Vector<GFXShaderMacro> &macros ) ;

    Vector<U32> mAttributes;
};

class GFXOpenGLES30ShaderConstBuffer : public GFXOpenGLShaderConstBuffer
{
public:
   GFXOpenGLES30ShaderConstBuffer(GFXOpenGLES30Shader* shader, U32 bufSize, U8* existingConstants);
   ~GFXOpenGLES30ShaderConstBuffer();
   
   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }

private:

   friend class GFXOpenGLES30Shader;
   WeakRefPtr<GFXOpenGLES30Shader> mShader;
};

#endif // _GFXOpenGLESShader_H_
