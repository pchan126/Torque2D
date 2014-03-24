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

#ifndef _GFXOpenGLStateBlock_H_
#define _GFXOpenGLStateBlock_H_

#include "graphics/gfxStateBlock.h"
#include "math/mMatrix.h"

class GFXOpenGLStateBlock : public GFXStateBlock
{   
public:
   // 
   // GFXOpenGLStateBlock interface
   //
   GFXOpenGLStateBlock(const GFXStateBlockDesc& desc);
   virtual ~GFXOpenGLStateBlock();

   // 
   // GFXStateBlock interface
   //

   /// Returns the hash value of the desc that created this block
   virtual size_t getHashValue() const;

   /// Returns a GFXStateBlockDesc that this block represents
   virtual const GFXStateBlockDesc& getDesc() const;   

   //
   // GFXResource
   //
   virtual void zombify() { }
   /// When called the resource should restore all device sensitive information destroyed by zombify()
   virtual void resurrect() { }

    // A Framework for GLSL Engine Uniforms (Game Engine Gems 2)
    const MatrixF& getModel() const { return m_ModelMatrix; };
    void setModel(const MatrixF& value);
    
    const MatrixF& getView() const { return m_ViewMatrix; };
    void setView(const MatrixF& value);
    
    const MatrixF& getProjection() const { return m_ProjectionMatrix; };
    void setProjection( const MatrixF& value);

    const MatrixF& getModelView() const { return m_MVMatrix; };
    const MatrixF& getModelViewProjection() const { return m_MVPMatrix; };

private:
   GFXStateBlockDesc mDesc;
   size_t mCachedHashValue;
    MatrixF m_ModelMatrix, m_ViewMatrix, m_ProjectionMatrix, m_MVMatrix, m_MVPMatrix;
    
};

//typedef StrongRefPtr<GFXOpenGLStateBlock> GFXOpenGLStateBlockRef;
typedef std::shared_ptr<GFXOpenGLStateBlock> GFXOpenGLStateBlockRef;

#endif