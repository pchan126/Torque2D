//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20Shader_H_
#define _GFXOpenGLES20Shader_H_

#include "graphics/OpenGL/gfxOpenGLShader.h"

class GFXOpenGLES20ShaderConstHandle;
class GFXOpenGLES20ShaderConstBuffer;

class GFXOpenGLES20Shader : public GFXOpenGLShader
{
   typedef HashMap<String, GFXOpenGLES20ShaderConstHandle*> HandleMap;
public:
   GFXOpenGLES20Shader() {};
    virtual ~GFXOpenGLES20Shader() {};
   
protected:

   friend class GFXOpenGLES20ShaderConstBuffer;
   friend class GFXOpenGLES20ShaderConstHandle;
   
    bool _loadShaderFromStream(  GLuint shader,
                                      const StringTableEntry path,
                                      FileStream* s,
                                      const Vector<GFXShaderMacro>& macros );

    virtual bool _init();

    bool initShader(  const StringTableEntry file,
                    bool isVertex,
                    const Vector<GFXShaderMacro> &macros ) ;

    Vector<U32> mAttributes;
};

class GFXOpenGLES20ShaderConstBuffer : public GFXOpenGLShaderConstBuffer
{
public:
   GFXOpenGLES20ShaderConstBuffer(GFXOpenGLES20Shader* shader, U32 bufSize, U8* existingConstants);
   ~GFXOpenGLES20ShaderConstBuffer();
   
   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }

private:

   friend class GFXOpenGLES20Shader;
   WeakRefPtr<GFXOpenGLES20Shader> mShader;
};

#endif // _GFXOpenGLESShader_H_
