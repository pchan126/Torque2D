//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSShader_H_
#define _GFXOpenGLES20iOSShader_H_

#include "graphics/OpenGL/gfxOpenGLShader.h"

class GFXOpenGLES20iOSShaderConstHandle;
class FileStream;
class GFXOpenGLES20iOSShaderConstBuffer;

class GFXOpenGLES20iOSShader : public GFXOpenGLShader
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

   friend class GFXOpenGLES20iOSShader;
   WeakRefPtr<GFXOpenGLES20iOSShader> mShader;
    
   
//   template<typename ConstType>
//   void internalSet(GFXShaderConstHandle* handle, const ConstType& param);
   
//   template<typename ConstType>
//   void internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv);
};

#endif // _GFXOpenGLES20iOSShader_H_