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
// IN THE SOFTWARE./Users/pauljan1/Torque2D-main/engine/source/platformOSX/graphics/gfxOpenGL32CardProfiler.h
//-----------------------------------------------------------------------------

#include "./gfxOpenGL33WinCardProfiler.h"
#include "./GFXOpenGL33WinDevice.h"
#include "./gfxOpenGL33WinEnumTranslate.h"
#include "platform/platformGL.h"
#include "./osxGLUtils.h"

void GFXOpenGL33WinCardProfiler::init()
{
   Parent::init();
}

void GFXOpenGL33WinCardProfiler::setupCardCapabilities()
{
    GLint maxTexSize;
    GLint numCompressedTexFormats;
    GLint temp;
   
   Parent::setupCardCapabilities();

   glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp);
   setCapability("maxVertexAttributes", temp);
   
   glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &temp);
   setCapability("maxUniformVertexVectors", temp);
   
   glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &temp);
   setCapability("maxUniformFragmentVectors", temp);
   
   glGetIntegerv(GL_MAX_VARYING_FLOATS, &temp);
   setCapability("maxVaryingVectors", temp);
   
   glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
   setCapability("maxVertTextureImageUnits", temp);

   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &temp);
    setCapability("maxFragTextureImageUnits", temp);
}

bool GFXOpenGL33WinCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   return glfwExtensionSupported(query.c_str());
}

bool GFXOpenGL33WinCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
	// We assume if the format is valid that we can use it for any purpose.
   // This may not be the case, but we have no way to check short of in depth 
   // testing of every format for every purpose.  And by testing, I mean sitting
   // down and doing it by hand, because there is no OpenGL API to check these
   // things.
   return GFXGLTextureInternalFormat[fmt] != GL_ZERO;
}
