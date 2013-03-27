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

#import "platformOSX/platformOSX.h"
#import "platformOSX/osxTorqueView.h"
#import "game/gameInterface.h"
#import "gui/guiCanvas.h"
#import "platformOSX/graphics/gfxOpenGLDevice.h"
//#include "platform/platformVideo.h"

#pragma mark ---- OSXTorqueView Implementation ----

@interface OSXTorqueView (PrivateMethods)
- (void)windowFinishedLiveResize:(NSNotification *)notification;
- (void)getModifierKey:(U32&)modifiers event:(NSEvent *)event;
- (void)processMouseButton:(NSEvent *)event button:(KeyCodes)button action:(U8)action;
- (void)processKeyEvent:(NSEvent *)event make:(BOOL)make;
@end

@implementation OSXTorqueView

@synthesize contextInitialized = _contextInitialized;

//- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
//{
//	// There is no autorelease pool when this method is called
//	// because it will be called from a background thread
//	// It's important to create one or you will leak objects
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	
//	[self drawView];
//	
//	[pool release];
//	return kCVReturnSuccess;
//}
//
//// This is the renderer output callback function
//static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
//{
//    CVReturn result = [(OSXTorqueView*)displayLinkContext getFrameForTime:outputTime];
//    return result;
//}
//
//// Get the video settings from the prefs.
//static void osxGetInitialResolution(U32 &width, U32 &height, U32 &bpp, bool &fullScreen)
//{
//    const char* resString;
//    char *tempBuf;
//    
//    osxPlatState * platState = [osxPlatState sharedPlatState];
//    
//    // cache the desktop size of the selected screen in platState
//    Video::getDesktopResolution();
//    
//    // load pref variables, properly choose windowed / fullscreen
//    fullScreen = Con::getBoolVariable("$pref::Video::fullScreen");
//    
//    if (fullScreen)
//        resString = Con::getVariable("$pref::Video::resolution");
//    else
//        resString = Con::getVariable("$pref::Video::windowedRes");
//    
//    // dStrtok is destructive, work on a copy...
//    tempBuf = new char[dStrlen(resString) + 1];
//    dStrcpy(tempBuf, resString);
//    
//    // set window size
//    //DAW: Added min size checks for windowSize
//    width = (U32)dAtoi(dStrtok(tempBuf, " x\0"));
//    
//    if (width <= 0)
//        width = [platState windowWidth];
//    
//    height = (U32)dAtoi(dStrtok( NULL, " x\0"));
//    
//    if (height <= 0)
//        height = [platState windowHeight];
//    
//    // bit depth
//    if (fullScreen)
//    {
//        dAtoi(dStrtok(NULL, "\0"));
//        
//        if ( bpp <= 0 )
//            bpp = 16;
//    }
//    else
//        bpp = platState.desktopBitsPixel == 24 ? 32 : platState.desktopBitsPixel;
//    
//    delete [] tempBuf;
//}
//
//
- (void) awakeFromNib
{
    osxPlatState * platState = [osxPlatState sharedPlatState];
    [platState setTorqueView:self];
////    NSOpenGLPixelFormatAttribute attrs[] =
////	{
////		NSOpenGLPFADoubleBuffer,
////		NSOpenGLPFADepthSize, 24,
////		NSOpenGLPFAOpenGLProfile,
////		NSOpenGLProfileVersion3_2Core,
////		0
////	};
////	
////	NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
////	
////	if (!pf)
////	{
////		NSLog(@"No OpenGL pixel format");
////	}
////    
////    NSOpenGLContext* context = [[[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil] autorelease];
////    
////    [self setPixelFormat:pf];
////    
////    [self setOpenGLContext:context];
}
//
//- (void) prepareOpenGL
//{
////	[super prepareOpenGL];
////	
////	// Make all the OpenGL calls to setup rendering
////	//  and build the necessary rendering objects
////	[self initGL];
////	
////	// Create a display link capable of being used with all active displays
////	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
////	
////	// Set the renderer output callback function
////	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
////	
////	// Set the display link for the current renderer
////	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
////	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
////	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
////	
////	// Activate the display link
////	CVDisplayLinkStart(displayLink);
//}
//
//- (void) initGL
//{
////	// Make this openGL context current to the thread
////	// (i.e. all openGL on this thread calls will go to this context)
////	[[self openGLContext] makeCurrentContext];
////	
////	// Synchronize buffer swaps with vertical refresh rate
////	GLint swapInt = 1;
////	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
////	
////    Con::init();
////    GFXOpenGLDevice* device = new GFXOpenGLDevice((void*)[self openGLContext]);
////    Video::installDevice(device);
////    device->init();
////
////    U32 width;
////    U32 height;
////    U32 bpp;
////    bool fullScreen;
////    
////    osxGetInitialResolution(width, height, bpp, fullScreen);
//////    Platform::initWindow(Point2I(MIN_RESOLUTION_X, MIN_RESOLUTION_Y), NULL);
////	// Init our renderer.  Use 0 for the defaultFBO which is appropriate for MacOS (but not iOS)
//////	m_renderer = [[OpenGLRenderer alloc] initWithDefaultFBO:0];
//}
//
//- (void) reshape
//{
//	[super reshape];
//	
//	// We draw on a secondary thread through the display link
//	// When resizing the view, -reshape is called automatically on the main thread
//	// Add a mutex around to avoid the threads accessing the context simultaneously when resizing
//	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
//	
//	NSRect rect = [self bounds];
//	
////	[m_renderer resizeWithWidth:rect.size.width AndHeight:rect.size.height];
//	
//	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
//}
//
//- (void) drawView
//{
//	[[self openGLContext] makeCurrentContext];
//    
//	// We draw on a secondary thread through the display link
//	// When resizing the view, -reshape is called automatically on the main thread
//	// Add a mutex around to avoid the threads accessing the context simultaneously	when resizing
//	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
//	
////	[m_renderer render];
//	
//	CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
//	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
//}

- (void) dealloc
{
	// Stop the display link BEFORE releasing anything in the view
    // otherwise the display link thread may call into the view and crash
    // when it encounters something that has been release
//	CVDisplayLinkStop(displayLink);
//    
//	CVDisplayLinkRelease(displayLink);
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    // Drop the tracking rectangle for mouse events
    if (_trackingArea != nil)
    {
        [self removeTrackingArea:_trackingArea];
        [_trackingArea release];
    }
	
    // Release the display link AFTER display link has been released
//	[m_renderer release];

    // Custom memory cleanup
    if (self.openGLContext != nil)
    {
        [self.openGLContext release];
        self.openGLContext = nil;
    }
	
	[super dealloc];
}

//-----------------------------------------------------------------------------
// Custom initialization method for OSXTorqueView
- (void)initialize
{
    if (self)
    {
        // Make absolutely sure self.openGLContext is nil
        self.openGLContext = nil;

        NSTrackingAreaOptions trackingOptions = NSTrackingCursorUpdate |
                NSTrackingMouseMoved |
                NSTrackingMouseEnteredAndExited |
                NSTrackingInVisibleRect |
                NSTrackingActiveInActiveApp;

        _trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:trackingOptions owner:self userInfo:nil];

        [self addTrackingArea:_trackingArea];

        inputManager = (osxInputManager *) Input::getManager();
    }
}

////-----------------------------------------------------------------------------
//// Default dealloc override
//- (void)dealloc
//{
//    // End notifications
//    [[NSNotificationCenter defaultCenter] removeObserver:self];
//
//    // Drop the tracking rectangle for mouse events
//    if (_trackingArea != nil)
//    {
//        [self removeTrackingArea:_trackingArea];
//        [_trackingArea release];
//    }
//
//    // Custom memory cleanup
//    if (self.openGLContext != nil)
//    {
//        [self.openGLContext release];
//        self.openGLContext = nil;
//    }
//
//    // "Parent" cleanup
//    [super dealloc];
//}

////-----------------------------------------------------------------------------
//// This view an always be a first responder
//- (BOOL)acceptsFirstResponder
//{
//    return YES;
//}
//
////-----------------------------------------------------------------------------
//// Called whent the parent finishes its live resizing
//- (void)windowFinishedLiveResize:(NSNotification *)notification
//{
//    NSSize size = [[self window] frame].size;
//
//    [[osxPlatState sharedPlatState] setWindowSize:(S32)size.width height:(S32)size.height];
//    
//    NSRect frame = NSMakeRect(0, 0, size.width, size.height);
//    
//    S32 barHeight = frame.size.height;
//    frame = [NSWindow frameRectForContentRect:frame styleMask:NSTitledWindowMask];
//    barHeight -= frame.size.height;
//    
//    NSRect viewFrame = NSMakeRect(0, barHeight, frame.size.width, frame.size.height);
//    
//    [self setFrame:viewFrame];
//    [self updateContext];
//}
//
//#pragma mark ---- OSXTorqueView OpenGL Handling ----
//
//-----------------------------------------------------------------------------
// Allocates a new NSOpenGLContext with the specified pixel format and makes
// it the current OpenGL context automatically
- (void)createContextWithPixelFormat:(NSOpenGLPixelFormat *)pixelFormat
{
    self.openGLContext = [[[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil] retain];

    AssertFatal(self.openGLContext, "We could not create a valid NSOpenGL rendering context.");

    [self.openGLContext setView:self];

    [self.openGLContext makeCurrentContext];

    _contextInitialized = YES;
}

//-----------------------------------------------------------------------------
// Clears the current context, releases control from this view, and deallocates
// the NSOpenGLContext
- (void)clearContext
{
    if (self.openGLContext != nil)
    {
        [NSOpenGLContext clearCurrentContext];
        [self.openGLContext clearDrawable];

        [self.openGLContext release];
        self.openGLContext = nil;

        _contextInitialized = NO;
    }
}

//-----------------------------------------------------------------------------
// Perform an update on the NSOpenGLContext, which will match the surface
// size to the view's frame
- (void)updateContext
{
    if (self.openGLContext != nil)
        [self.openGLContext update];
}

//-----------------------------------------------------------------------------
// Perform a swap buffer if the NSOpenGLContext is initialized
- (void)flushBuffer
{
    if (self.openGLContext != nil)
        [self.openGLContext flushBuffer];
}

//-----------------------------------------------------------------------------
- (void)setVerticalSync:(bool)sync
{
    if (self.openGLContext != nil)
    {
        GLint swapInterval = sync ? 1 : 0;
        [self.openGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    }
}



@end
