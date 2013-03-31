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
#import "./osxTorqueView.h"
#import "game/gameInterface.h"
#import "gui/guiCanvas.h"
#import "platformOSX/graphics/gfxOpenGLDevice.h"
#import "./macWindow.h"
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
//@synthesize <#property#>

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

//-----------------------------------------------------------------------------
// Default dealloc override
- (void)dealloc
{
    // End notifications
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    // Drop the tracking rectangle for mouse events
    if (_trackingArea != nil)
    {
        [self removeTrackingArea:_trackingArea];
        [_trackingArea release];
    }
    
    // Custom memory cleanup
    if (self.openGLContext != nil)
    {
        [self.openGLContext release];
        self.openGLContext = nil;
    }
    
    // "Parent" cleanup
    [super dealloc];
}

//-----------------------------------------------------------------------------
// This view an always be a first responder
- (BOOL)acceptsFirstResponder
{
    return YES;
}

//-----------------------------------------------------------------------------
// Called whent the parent finishes its live resizing
- (void)windowFinishedLiveResize:(NSNotification *)notification
{
    NSSize size = [[self window] frame].size;
    
    mTorqueWindow->setSize(Point2I((S32)size.width, (S32)size.height));
//    [mTorqueWindow setWindowSize:(S32)size.width height:(S32)size.height];
    
    NSRect frame = NSMakeRect(0, 0, size.width, size.height);
    
    S32 barHeight = frame.size.height;
    frame = [NSWindow frameRectForContentRect:frame styleMask:NSTitledWindowMask];
    barHeight -= frame.size.height;
    
    NSRect viewFrame = NSMakeRect(0, barHeight, frame.size.width, frame.size.height);
    
    [self setFrame:viewFrame];
    [self updateContext];
}

////#pragma mark ---- OSXTorqueView OpenGL Handling ----
////
////-----------------------------------------------------------------------------
//// Allocates a new NSOpenGLContext with the specified pixel format and makes
//// it the current OpenGL context automatically
//- (void)createContextWithPixelFormat:(NSOpenGLPixelFormat *)pixelFormat
//{
//    self.openGLContext = [[[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil] retain];
//
//    AssertFatal(self.openGLContext, "We could not create a valid NSOpenGL rendering context.");
//
//    [self.openGLContext setView:self];
//
//    [self.openGLContext makeCurrentContext];
//
//    _contextInitialized = YES;
//}

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

- (void)setTorqueWindow:(MacWindow*)theWindow
{
    mTorqueWindow = theWindow;
    mLastMods = 0;
}

- (MacWindow*)torqueWindow
{
    return mTorqueWindow;
}

- (void) reshape
{
	[super reshape];
//	
//	// We draw on a secondary thread through the display link
//	// When resizing the view, -reshape is called automatically on the main thread
//	// Add a mutex around to avoid the threads accessing the context simultaneously when resizing
//	CGLLockContext([[self openGLContext] CGLContextObj]);
//	
//	NSRect rect = [self bounds];
//	
//	[m_renderer resizeWithWidth:rect.size.width AndHeight:rect.size.height];
//	
//	CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

//#pragma mark -
//#pragma mark Window Delegate
//- (BOOL)windowShouldClose:(NSWindow *)sender
//{
//    // We close the window ourselves
//    mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), WindowDestroy);
//    return NO;
//}
//
//- (void)windowWillClose:(NSNotification *) notification
//{
//    mTorqueWindow->_disassociateCocoaWindow();
//}
//
//- (void)windowDidBecomeKey:(NSNotification *)notification
//{
//    // when our window is the key window, we become the app delegate.
//    PlatformWindow* focusWindow = WindowManager->getFocusedWindow();
//    if(focusWindow && focusWindow != mTorqueWindow)
//        focusWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseFocus);
//    [NSApp setDelegate:self];
//    [self signalGainFocus];
//}
//
//- (void)windowDidResignKey:(NSNotification*)notification
//{
//    mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseScreen);
//    mTorqueWindow->_associateMouse();
//    mTorqueWindow->setCursorVisible(true);
//    [NSApp setDelegate:nil];
//}
//
//- (void)windowDidChangeScreen:(NSNotification*)notification
//{
//    NSWindow* wnd = [notification object];
//    // TODO: Add a category to NSScreen to deal with this
//    //   CGDirectDisplayID disp = (CGDirectDisplayID)[[[[wnd screen] deviceDescription] valueForKey:@"NSScreenNumber"] unsignedIntValue];
//    CGDirectDisplayID display = CGMainDisplayID();
//    mTorqueWindow->setDisplay(display);
//}
//
//- (void)windowDidResize:(NSNotification*)notification
//{
//    Point2I clientExtent = mTorqueWindow->getClientExtent();
//    mTorqueWindow->resizeEvent.trigger(mTorqueWindow->getWindowId(), clientExtent.x, clientExtent.y);
//}
//
//#pragma mark -
//#pragma mark responder status
//- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)becomeFirstResponder  { return YES; }
- (BOOL)resignFirstResponder  { return YES; }

// Basic implementation because NSResponder's default implementation can cause infinite loops when our keyDown: method calls interpretKeyEvents:
- (void)doCommandBySelector:(SEL)aSelector
{
    if([self respondsToSelector:aSelector])
        [self performSelector:aSelector withObject:nil];
}

@end
