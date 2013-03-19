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
#import "platformiOS/platformiOS.h"
#include "platform/platformVideo.h"
#include "graphics/gfxDevice.h"
#include "T2DAppDelegate.h"

GFXVideoMode Video::getDesktopResolution()
{
    UIApplication * application = [UIApplication sharedApplication];
    // Get the screen the application window resides in and is receiving input
    UIWindow* mainScreen = [application keyWindow];
    
    // Get the visible boundaries
    CGRect screenRect = [mainScreen frame];
    
    // Get the screen depth. You cannot access depth directly. It must be passed
    // into a function that will return the bpp
//    int bitDepth = NSBitsPerPixelFromDepth([mainScreen depth]);
    
    // Build the return resolution
    GFXVideoMode resolution;
    resolution.resolution.x = (U32)screenRect.size.width;
    resolution.resolution.y = (U32)screenRect.size.height;
//    resolution.bitDepth = (U32)bitDepth;
    
    iOSPlatState * platState = [iOSPlatState sharedPlatState];
    
    [platState setDesktopWidth:resolution.resolution.x];
    [platState setDesktopHeight:resolution.resolution.y];
    [platState setDesktopBitsPixel:resolution.bitDepth];
    
    // Return the new resolution
    return resolution;
}