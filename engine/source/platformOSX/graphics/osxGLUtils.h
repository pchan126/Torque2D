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

#ifndef _MACGLUTILS_H_
#define _MACGLUTILS_H_

static Vector<NSOpenGLPixelFormatAttribute> _beginPixelFormatAttributesForDisplay(CGDirectDisplayID display)
{
   Vector<NSOpenGLPixelFormatAttribute> attributes;
   attributes.reserve(16); // Most attribute lists won't exceed this
   
    attributes.push_back(NSOpenGLPFAScreenMask);
   attributes.push_back((NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(display));
   attributes.push_back(NSOpenGLPFANoRecovery);
   attributes.push_back(NSOpenGLPFADoubleBuffer);
   attributes.push_back(NSOpenGLPFAAccelerated);
   attributes.push_back(NSOpenGLPFAAuxBuffers);
   attributes.push_back((NSOpenGLPixelFormatAttribute)1);
   return attributes;
}

static void _addColorAlphaDepthStencilAttributes(Vector<NSOpenGLPixelFormatAttribute>& attributes, U32 color, U32 alpha, U32 depth, U32 stencil)
{
   attributes.push_back(NSOpenGLPFAColorSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)color);
   attributes.push_back(NSOpenGLPFAAlphaSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)alpha);
   attributes.push_back(NSOpenGLPFADepthSize);   attributes.push_back((NSOpenGLPixelFormatAttribute)depth);
   attributes.push_back(NSOpenGLPFAStencilSize); attributes.push_back((NSOpenGLPixelFormatAttribute)stencil);
}

static void _addFullscreenAttributes(Vector<NSOpenGLPixelFormatAttribute>& attributes)
{
   attributes.push_back(NSOpenGLPFAFullScreen);
}

static void _endAttributeList(Vector<NSOpenGLPixelFormatAttribute>& attributes)
{
   attributes.push_back((NSOpenGLPixelFormatAttribute)0);
}

static Vector<NSOpenGLPixelFormatAttribute> _createStandardPixelFormatAttributesForDisplay(CGDirectDisplayID display)
{
   Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(display);
   _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
   _endAttributeList(attributes);
   
   return attributes;
}

/// returns an opengl pixel format suitable for creating shared opengl contexts.
static NSOpenGLPixelFormat* _createStandardPixelFormat()
{
    NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion3_2Core,
		0
	};
 
	NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
	
	if (!pf)
	{
		NSLog(@"No OpenGL pixel format");
	}

   return pf;
}

static void _createInitialContextAndFormat(NSOpenGLContext* &ctx, NSOpenGLPixelFormat* &fmt)
{
    AssertFatal(!fmt && !ctx, "_createInitialContextAndFormat - Already created initial context and format");
    
    fmt = _createStandardPixelFormat();
    AssertFatal(fmt, "_createInitialContextAndFormat - Unable to create an OpenGL pixel format");
    
    ctx = [[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)fmt shareContext: nil];
    AssertFatal(ctx, "_createInitialContextAndFormat - Unable to create an OpenGL context");
}

static NSOpenGLContext* _createContextForWindow()
{
    osxPlatState * platState = [osxPlatState sharedPlatState];

    NSOpenGLView* view = static_cast< NSOpenGLView* >([platState torqueView]);
    NSOpenGLContext* ctx = NULL;
    static NSOpenGLPixelFormat* pixelFormat = NULL;
    static NSOpenGLContext* context = NULL;
    
    if ([view isKindOfClass:[NSOpenGLView class]])
    {
        if( context == NULL || pixelFormat == NULL)
        {
            // Create the initial opengl context that the device and the first window will hold.
            _createInitialContextAndFormat(context, pixelFormat);
            ctx = (NSOpenGLContext*)context;
        }
        else
        {
            // Create a context which shares its resources with the device's initial context
            ctx = [[NSOpenGLContext alloc] initWithFormat: (NSOpenGLPixelFormat*)pixelFormat shareContext: (NSOpenGLContext*)context];
            AssertFatal(ctx, "Unable to create a shared OpenGL context");
        }
        
        [view setPixelFormat: (NSOpenGLPixelFormat*)pixelFormat];
        [view setOpenGLContext: ctx];
        
        return ctx;
    }
    else
    {
        AssertFatal([view isKindOfClass:[NSOpenGLView class]], avar("_createContextForWindow - Supplied a %s instead of a NSOpenGLView", [[view className] UTF8String]));
        return ctx;
    }
}


#endif