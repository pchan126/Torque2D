//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20Shader_H_
#define _GFXOpenGLES20Shader_H_

#include "graphics/OpenGL/gfxOpenGLShader.h"

class GFXOpenGLES20ShaderConstHandle;
class FileStream;
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
//   virtual void set(GFXShaderConstHandle* handle, const F32 fv);
//   virtual void set(GFXShaderConstHandle* handle, const Point2F& fv);
//   virtual void set(GFXShaderConstHandle* handle, const Point3F& fv);
//   virtual void set(GFXShaderConstHandle* handle, const Point4F& fv);
//   virtual void set(GFXShaderConstHandle* handle, const PlaneF& fv);
//   virtual void set(GFXShaderConstHandle* handle, const S32 f);
//   virtual void set(GFXShaderConstHandle* handle, const Point2I& fv);
//   virtual void set(GFXShaderConstHandle* handle, const Point3I& fv);
//   virtual void set(GFXShaderConstHandle* handle, const Point4I& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv);   
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType = GFXSCT_Float4x4);
//   virtual void set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType = GFXSCT_Float4x4);   

   // GFXResource
   virtual const String describeSelf() const;
   virtual void zombify() {}
   virtual void resurrect() {}

private:

   friend class GFXOpenGLES20Shader;
   WeakRefPtr<GFXOpenGLES20Shader> mShader;
    
   
//   template<typename ConstType>
//   void internalSet(GFXShaderConstHandle* handle, const ConstType& param);
   
//   template<typename ConstType>
//   void internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv);
};

#endif // _GFXOpenGLESShader_H_