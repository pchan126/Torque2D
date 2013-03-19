//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESShader_H_
#define _GFXOpenGLESShader_H_

#include "sim/refBase.h"
#include "graphics/gfxShader.h"
#include "collection/hashTable.h"

class GFXOpenGLESShaderConstHandle;
class FileStream;
class GFXOpenGLESShaderConstBuffer;

class GFXOpenGLESShader : public GFXShader
{
   typedef HashMap<String, GFXOpenGLESShaderConstHandle*> HandleMap;
public:
   GFXOpenGLESShader();
   virtual ~GFXOpenGLESShader();
   
   /// @name GFXShader interface
   /// @{
   virtual GFXShaderConstHandle* getShaderConstHandle(const String& name);

   /// Returns our list of shader constants, the material can get this and just set the constants it knows about
   virtual const Vector<GFXShaderConstDesc>& getShaderConstDesc() const;

   /// Returns the alignment value for constType
   virtual U32 getAlignmentValue(const GFXShaderConstType constType) const; 

   virtual GFXShaderConstBufferRef allocConstBuffer();

   /// @}
   
   /// @name GFXResource interface
   /// @{
   virtual void zombify();
   virtual void resurrect() { reload(); }
   virtual const String describeSelf() const;
   /// @}      

   /// Activates this shader in the GL context.
   void useProgram();
   
protected:

   friend class GFXOpenGLESShaderConstBuffer;
   friend class GFXOpenGLESShaderConstHandle;
   
   virtual bool _init();   

    bool initShader(  const StringTableEntry file,
                    bool isVertex,
                    const Vector<GFXShaderMacro> &macros );

   void clearShaders();
   void initConstantDescs();
   void initHandles();
   void setConstantsFromBuffer(GFXOpenGLESShaderConstBuffer* buffer);
   
    static char* _handleIncludes( const StringTableEntry path, FileStream *s );
    
    static bool _loadShaderFromStream(  GLuint shader,
                                      const StringTableEntry path,
                                      FileStream* s,
                                      const Vector<GFXShaderMacro>& macros );

   /// @name Internal GL handles
   /// @{
   GLuint mVertexShader;
   GLuint mPixelShader;
   GLuint mProgram;
   /// @}
    
   Vector<GFXShaderConstDesc> mConstants;
   U32 mConstBufferSize;
   U8* mConstBuffer;
   HandleMap mHandles;
   Vector<GFXOpenGLESShaderConstHandle*> mValidHandles;
};

class GFXOpenGLESShaderConstBuffer : public GFXShaderConstBuffer
{
public:
   GFXOpenGLESShaderConstBuffer(GFXOpenGLESShader* shader, U32 bufSize, U8* existingConstants);
   ~GFXOpenGLESShaderConstBuffer();
   
   /// Called by GFXOpenGLESDevice to activate this buffer.
   void activate();

   /// Called when the shader this buffer references is reloaded.
   void onShaderReload( GFXOpenGLESShader *shader );

   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }
   virtual void set(GFXShaderConstHandle* handle, const F32 fv);
   virtual void set(GFXShaderConstHandle* handle, const Point2F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4F& fv);
   virtual void set(GFXShaderConstHandle* handle, const PlaneF& fv);
   virtual void set(GFXShaderConstHandle* handle, const S32 f);
   virtual void set(GFXShaderConstHandle* handle, const Point2I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4I& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv);   
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv);
//   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType = GFXSCT_Float4x4);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType = GFXSCT_Float4x4);   

   // GFXResource
   virtual const String describeSelf() const;
   virtual void zombify() {}
   virtual void resurrect() {}

private:

   friend class GFXOpenGLESShader;
   U8* mBuffer;
   WeakRefPtr<GFXOpenGLESShader> mShader;
   
   template<typename ConstType>
   void internalSet(GFXShaderConstHandle* handle, const ConstType& param);
   
//   template<typename ConstType>
//   void internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv);
};

#endif // _GFXOpenGLESShader_H_