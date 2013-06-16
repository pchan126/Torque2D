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
//these can be set as low as 1 1, but we picked 320
//because the iPhone can init to 320x480 or 480x320
#define MIN_RESOLUTION_X			320
#define MIN_RESOLUTION_Y			320//for 320 x 480 or 480 x 320
#define MIN_RESOLUTION_BIT_DEPTH	16
#define MIN_RESOLUTION_XY_STRING	"320 320"
//%PUAP%-->

#define GLCOREARB_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#define WGL_WGLEXT_PROTOTYPES

#include <glew.h>
//#include "./glcorearb.h"
//#include "./glext.h"
//
//#pragma comment(lib,"opengl32.lib")

//extern PFNGLCREATEPROGRAMPROC glCreateProgram;
//extern PFNGLCREATEPROGRAMPROC glDeleteProgram;
//extern PFNGLCREATEPROGRAMPROC glUseProgram;
//extern PFNGLCREATEPROGRAMPROC glAttachShader;
//extern PFNGLCREATEPROGRAMPROC glDetachShader;
//extern PFNGLCREATEPROGRAMPROC glLinkProgram;
//extern PFNGLCREATEPROGRAMPROC glGetProgramiv;
//extern PFNGLCREATEPROGRAMPROC glGetShaderInfoLog;
//extern PFNGLCREATEPROGRAMPROC glGetUniformLocation;
//extern PFNGLCREATEPROGRAMPROC glUniform1i;
//extern PFNGLCREATEPROGRAMPROC glUniform1iv;
//extern PFNGLCREATEPROGRAMPROC glUniform2iv;
//extern PFNGLCREATEPROGRAMPROC glUniform3iv;
//extern PFNGLCREATEPROGRAMPROC glUniform4iv;
//extern PFNGLCREATEPROGRAMPROC glUniform1f;
//extern PFNGLCREATEPROGRAMPROC glUniform1fv;
//extern PFNGLCREATEPROGRAMPROC glUniform2fv;
//extern PFNGLCREATEPROGRAMPROC glUniform3fv;
//extern PFNGLCREATEPROGRAMPROC glUniform4fv;
//extern PFNGLCREATEPROGRAMPROC glUniformMatrix4fv;
//extern PFNGLCREATEPROGRAMPROC glGetAttribLocation;
//extern PFNGLCREATEPROGRAMPROC glVertexAttrib1f;
//extern PFNGLCREATEPROGRAMPROC glVertexAttrib1fv;
//extern PFNGLCREATEPROGRAMPROC glVertexAttrib2fv;
//extern PFNGLCREATEPROGRAMPROC glVertexAttrib3fv;
//extern PFNGLCREATEPROGRAMPROC glVertexAttrib4fv;
//extern PFNGLCREATEPROGRAMPROC glEnableVertexAttribArray;
//extern PFNGLCREATEPROGRAMPROC glBindAttribLocation;
//
//// Shader
//extern PFNGLCREATEPROGRAMPROC glCreateShader;
//extern PFNGLCREATEPROGRAMPROC glDeleteShader;
//extern PFNGLCREATEPROGRAMPROC glShaderSource;
//extern PFNGLCREATEPROGRAMPROC glCompileShader;
//extern PFNGLCREATEPROGRAMPROC glGetShaderiv;
//
//// VBO
//extern PFNGLCREATEPROGRAMPROC glGenBuffers;
//extern PFNGLCREATEPROGRAMPROC glBindBuffer;
//extern PFNGLCREATEPROGRAMPROC glBufferData;
//extern PFNGLCREATEPROGRAMPROC glVertexAttribPointer;

typedef enum {
	GLKVertexAttribPosition,
	GLKVertexAttribNormal,
	GLKVertexAttribColor,
	GLKVertexAttribTexCoord0,
	GLKVertexAttribTexCoord1,
} GLKVertexAttrib;


#endif // _PLATFORMGL_H_
