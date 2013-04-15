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

#include "./gfxOpenGLStateBlock.h"
#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLUtils.h"
#include "./gfxOpenGLTextureObject.h"
#include "platform/platformGL.h"


GFXOpenGLStateBlock::GFXOpenGLStateBlock(const GFXStateBlockDesc& desc) :
   mDesc(desc),
   mCachedHashValue(desc.getHashValue()),
   m_ModelMatrix(true),
   m_ViewMatrix(true),
   m_ProjectionMatrix(true),
   m_MVMatrix(true),
   m_MVPMatrix(true)
{
}

GFXOpenGLStateBlock::~GFXOpenGLStateBlock()
{
}

/// Returns the hash value of the desc that created this block
U32 GFXOpenGLStateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXOpenGLStateBlock::getDesc() const
{
   return mDesc;   
}

void GFXOpenGLStateBlock::setModel( const MatrixF& value)
{
    m_ModelMatrix = value;
    m_MVMatrix = m_ModelMatrix*m_ViewMatrix;
    m_MVPMatrix = m_MVMatrix*m_ProjectionMatrix;
};

void GFXOpenGLStateBlock::setView( const MatrixF& value)
{
    m_ViewMatrix = value;
    m_MVMatrix = m_ModelMatrix*m_ViewMatrix;
    m_MVPMatrix = m_MVMatrix*m_ProjectionMatrix;
};

void GFXOpenGLStateBlock::setProjection( const MatrixF& value)
{
    m_ProjectionMatrix = value;
    m_MVPMatrix = m_MVMatrix*m_ProjectionMatrix;
};



