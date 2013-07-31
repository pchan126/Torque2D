//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSShader_H_
#define _GFXOpenGLES20iOSShader_H_

#include "graphics/OpenGL/ES20/gfxOpenGLES20Shader.h"

class GFXOpenGLES20iOSShaderConstHandle;
class FileStream;
class GFXOpenGLES20iOSShaderConstBuffer;

class GFXOpenGLES20iOSShader : public GFXOpenGLES20Shader
{
   typedef HashMap<String, GFXOpenGLES20iOSShaderConstHandle*> HandleMap;
public:
   GFXOpenGLES20iOSShader() {};
    virtual ~GFXOpenGLES20iOSShader() {};
   
protected:

   friend class GFXOpenGLES20iOSShaderConstBuffer;
   friend class GFXOpenGLES20iOSShaderConstHandle;
   
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

class GFXOpenGLES20iOSShaderConstBuffer : public GFXOpenGLShaderConstBuffer
{
public:
   GFXOpenGLES20iOSShaderConstBuffer(GFXOpenGLES20iOSShader* shader, U32 bufSize, U8* existingConstants);
   ~GFXOpenGLES20iOSShaderConstBuffer();
   
   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }

   // GFXResource
//   virtual const String describeSelf() const;

private:

   friend class GFXOpenGLES20iOSShader;
   WeakRefPtr<GFXOpenGLES20iOSShader> mShader;
};

#endif // _GFXOpenGLES20iOSShader_H_
