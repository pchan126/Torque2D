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

#ifndef _PLATFORMGL_H_
#define _PLATFORMGL_H_

//put this here so the GUI can get to it
//<--%PUAP% -Mat add #defines for min window/resolution size
#define MIN_RESOLUTION_X			320
#define MIN_RESOLUTION_Y			320//for 230 x 480 or 480 x 320
#define MIN_RESOLUTION_BIT_DEPTH	32
#define MIN_RESOLUTION_XY_STRING	"320 320"
//%PUAP%-->

/// We use the standard Apple OpenGL framework headers.
//#define GLFW_INCLUDE_GL3 1
//#include <GL/glfw.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>


typedef enum {
    GLKVertexAttribPosition,
    GLKVertexAttribNormal,
    GLKVertexAttribColor,
    GLKVertexAttribTexCoord0,
    GLKVertexAttribTexCoord1,
} GLKVertexAttrib;


#endif // _PLATFORMGL_H_
