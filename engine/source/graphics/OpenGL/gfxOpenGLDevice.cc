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
#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"


GFXOpenGLDevice::GFXOpenGLDevice( U32 adapterIndex ) :
            currentCullMode(GFXCullNone)
{
    
}


void GFXOpenGLDevice::setCullMode(GFXCullMode mode)
{
    if (mode == currentCullMode)
        return;
    
    // Culling
    if (mode == GFXCullNone)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GFXGLCullMode[mode]);
    }
}

// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
GLsizei GFXOpenGLDevice::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
    switch (primType)
    {
        case GFXPointList :
            return primitiveCount;
            break;
        case GFXLineList :
            return primitiveCount * 2;
            break;
        case GFXLineStrip :
            return primitiveCount + 1;
            break;
        case GFXTriangleList :
            return primitiveCount * 3;
            break;
        case GFXTriangleStrip :
            return 2 + primitiveCount;
            break;
        case GFXTriangleFan :
            return 2 + primitiveCount;
            break;
        default:
            AssertFatal(false, "GFXOpenGLDevice::primCountToIndexCount - unrecognized prim type");
            break;
    }
    
    return 0;
}

