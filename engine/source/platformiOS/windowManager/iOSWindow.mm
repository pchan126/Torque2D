//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------


#import "./iOSWindow.h"

#import "console/console.h"
#import "platformiOS/platformiOS.h"
#import "./T2DViewController.h"
#import "platformiOS/graphics/GFXOpenGLESDevice.h"
#import "platformiOS/T2DAppDelegate.h"

iOSWindow* iOSWindow::sInstance = NULL;

iOSWindow::iOSWindow(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   
   mGLKWindow      = NULL;
   mCursorController = new iOSCursorController( this );
   mOwningWindowManager = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   mDefaultDisplayMode = NULL;
   
   mSkipMouseEvents = 0;
    
    mDisplay = [UIScreen mainScreen];
    mMainDisplayBounds = mDisplayBounds = [mDisplay bounds];
    
   mWindowId = windowId;
    _initCocoaWindow(windowText, clientExtent);
   
   appEvent.notify(this, &iOSWindow::_onAppEvent);
   
   sInstance = this;
}

iOSWindow::~iOSWindow()
{
   if(mFullscreen)
      _setFullscreen(false);

   appEvent.remove(this, &iOSWindow::_onAppEvent);
   
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   sInstance = NULL;
}


void iOSWindow::_initCocoaWindow(const char* windowText, Point2I clientExtent)
{
    iOSPlatState *platState = [iOSPlatState sharedPlatState];
    GFXOpenGLESDevice *device = dynamic_cast<GFXOpenGLESDevice*>(GFX);
    EAGLContext *ctx = device->getEAGLContext();

    UIStoryboard *storybord;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
    {
        storybord = [UIStoryboard storyboardWithName:@"iPadStoryboard" bundle:nil];
    }
    else
    {
         storybord = [UIStoryboard storyboardWithName:@"iPhoneStoryboard" bundle:nil];
    }

    T2DViewController *vc =(T2DViewController*)[storybord instantiateInitialViewController];
    platState.viewController = vc;
    [platState.viewController setContext:ctx];
    platState.viewController.context = ctx;
	[EAGLContext setCurrentContext:ctx];
    T2DView *view = (T2DView*)platState.viewController.view;
    view.context = ctx;
    view.enableSetNeedsDisplay = YES;
    vc.paused = NO;

    mGLKWindow = platState.viewController;
    T2DAppDelegate *app = [[UIApplication sharedApplication] delegate];
    app.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    app.window.rootViewController = mGLKWindow;
    [app.window makeKeyAndVisible];
    
    mGLKWindow.preferredFramesPerSecond = 60;
    mGLKWindow.paused = NO;
}

//void iOSWindow::_disassociateCocoaWindow()
//{
//   if( !mGLKWindow )
//      return;
//      
//   [mGLKWindow setContentView:nil];
//   [mGLKWindow setDelegate:nil];   
//
//   mGLKWindow = NULL;
//}

void iOSWindow::setVideoMode(const GFXVideoMode &mode)
{
   mCurrentMode = mode;
   setSize(mCurrentMode.resolution);
   
   if(mTarget.isValid())
      mTarget->resetMode();

   _setFullscreen(mCurrentMode.fullScreen);
}

void iOSWindow::_onAppEvent(WindowId, S32 evt)
{
   if(evt == LoseFocus && isFullscreen())
   {
      mShouldFullscreen = true;
      GFXVideoMode mode = mCurrentMode;
      mode.fullScreen = false;
      setVideoMode(mode);
   }
   
   if(evt == GainFocus && !isFullscreen() && mShouldFullscreen)
   {
      mShouldFullscreen = false;
      GFXVideoMode mode = mCurrentMode;
      mode.fullScreen = true;
      setVideoMode(mode);
   }
}

void iOSWindow::_setFullscreen(bool fullScreen)
{
   //iOS screen is always fullscreen
}

void* iOSWindow::getPlatformDrawable() const
{
   return NULL;
//   return [mGLKWindow contentView];
}

void iOSWindow::show()
{
//   [mGLKWindow makeKeyAndOrderFront:nil];
//   [mGLKWindow makeFirstResponder:[mGLKWindow contentView]];
   appEvent.trigger(getWindowId(), WindowShown);
   appEvent.trigger(getWindowId(), GainFocus);
}

void iOSWindow::close()
{
//   [mGLKWindow close];
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   delete this;
}

void iOSWindow::hide()
{
//   [mGLKWindow orderOut:nil];
   appEvent.trigger(getWindowId(), WindowHidden);
}

//void iOSWindow::setDisplay(CGDirectDisplayID display)
//{
//   mDisplay = display;
//   mDisplayBounds = CGDisplayBounds(mDisplay);
//}

PlatformWindow* iOSWindow::getNextWindow() const
{
   return mNextWindow;
}

bool iOSWindow::setSize(const Point2I &newSize)
{
//   NSSize newExtent = {static_cast<CGFloat>(newSize.x), static_cast<CGFloat>(newSize.y)};
//   [mGLKWindow setContentSize:newExtent];
//   [mGLKWindow center];
   return true;
}

void iOSWindow::setClientExtent( const Point2I newExtent )
{
//   if(!mFullscreen)
//   {
//      // Set the Client Area Extent (Resolution) of this window
//      NSSize newSize = {static_cast<CGFloat>(newExtent.x), static_cast<CGFloat>(newExtent.y)};
//      [mGLKWindow setContentSize:newSize];
//   }
//   else
//   {
//      // In fullscreen we have to resize the monitor (it'll be good to change it back too...)
//      if(!mDefaultDisplayMode)
//         mDefaultDisplayMode = (NSDictionary*)CGDisplayCurrentMode(mDisplay);
//      
//      NSDictionary* newMode = (NSDictionary*)CGDisplayBestModeForParameters(mDisplay, 32, newExtent.x, newExtent.y, NULL);
//      Con::printf("Switching to new display mode... width: %i height: %i bpp: %i", 
//                  [[newMode valueForKey:@"Width"] intValue], [[newMode valueForKey:@"Height"] intValue], [[newMode valueForKey:@"BitsPerPixel"] intValue]); 
//      CGDisplaySwitchToMode(mDisplay, (CFDictionaryRef)newMode);
//      mDisplayBounds = CGDisplayBounds(mDisplay);
//      if(mDisplay == kCGDirectMainDisplay)
//         mMainDisplayBounds = mDisplayBounds;
//   }
}

const Point2I iOSWindow::getClientExtent()
{
//   if(!mFullscreen)
//   {
//      // Get the Client Area Extent (Resolution) of this window
//      NSSize size = [[mGLKWindow contentView] frame].size;
//      return Point2I(size.width, size.height);
//   }
//   else
//   {
//      return Point2I(mDisplayBounds.size.width, mDisplayBounds.size.height);
//   }
}

void iOSWindow::setBounds( const RectI &newBounds )
{
//   CGRect newFrame = NSMakeRect(newBounds.point.x, newBounds.point.y, newBounds.extent.x, newBounds.extent.y);
//   [mGLKWindow setFrame:newFrame display:YES];
}

const RectI iOSWindow::getBounds() const
{
//   if(!mFullscreen)
//   {
//      // Get the position and size (fullscreen windows are always at (0,0)).
//      CGRect frame = [mGLKWindow frame];
//      return RectI(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
//   }
//   else
   {
      return RectI(0, 0, mDisplayBounds.size.width, mDisplayBounds.size.height);
   }
}

void iOSWindow::setPosition( const Point2I newPosition )
{
//   UIScreen *screen = [mGLKWindow screen];
//   CGRect screenFrame = [screen frame];
//
//   NSPoint pos = {static_cast<CGFloat>(newPosition.x), newPosition.y + screenFrame.size.height};
//   [mGLKWindow setFrameTopLeftPoint: pos];
}

const Point2I iOSWindow::getPosition()
{
   return Point2I(0, 0);
//   UIScreen *screen = [mGLKWindow screen];
//   CGRect screenFrame = [screen frame];
//   CGRect frame = [mGLKWindow frame];
//
//   return Point2I(frame.origin.x, screenFrame.size.height - (frame.origin.y + frame.size.height));
}

void iOSWindow::centerWindow()
{
//   [mGLKWindow center];
}

Point2I iOSWindow::clientToScreen( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mGLKWindow convertBaseToScreen: p ];
//   return Point2I( p.x, p.y );
}

Point2I iOSWindow::screenToClient( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mGLKWindow convertScreenToBase: p ];
//   return Point2I( p.x, p.y );
}

bool iOSWindow::isFocused()
{
   return true;
//   return [mGLKWindow isKeyWindow];
}

bool iOSWindow::isOpen()
{
   // Maybe check if _window != NULL ?
   return true;
}

bool iOSWindow::isVisible()
{
   return true;
}
   
void iOSWindow::setFocus()
{
//   [mGLKWindow makeKeyAndOrderFront:nil];
}

void iOSWindow::signalGainFocus()
{
   if(isFocused())
      [[mGLKWindow delegate] performSelector:@selector(signalGainFocus)];
}

void iOSWindow::minimize()
{
   return;
//   if(!isVisible())
//      return;
//      
//   [mGLKWindow miniaturize:nil];
//   appEvent.trigger(getWindowId(), WindowHidden);

//    //-----------------------------------------------------------------------------
//    // Simulates the user clicking the minimize button by momentarily highlighting
//    // the button, then minimizing the window.
//    void Platform::minimizeWindow()
//    {
//        NSApplication *application = [NSApplication sharedApplication];
//        NSWindow *keyWindow = [application keyWindow];
//        [keyWindow miniaturize:keyWindow];
//    }

}

void iOSWindow::maximize()
{
   return;
//   if(!isVisible())
//      return;
   
   // GFX2_RENDER_MERGE 
   //[mGLKWindow miniaturize:nil];
   //appEvent.trigger(getWindowId(), WindowHidden);
}

void iOSWindow::restore()
{
   return;
//   if(!isMinimized())
//      return;
//   
//   [mGLKWindow deminiaturize:nil];
//   appEvent.trigger(getWindowId(), WindowShown);
}

bool iOSWindow::isMinimized()
{
   return false;
//   return [mGLKWindow isMiniaturized] == YES;
}

bool iOSWindow::isMaximized()
{
   return false;
}

void iOSWindow::clearFocus()
{
   // Clear the focus state for this Window.  
   // If the Window does not have focus, nothing happens.
   // If the Window has focus, it relinquishes it's focus to the Operating System
   
   // TODO: find out if we can do anything correct here. We are instructed *not* to call [NSWindow resignKeyWindow], and we don't necessarily have another window to assign as key.
}

bool iOSWindow::setCaption(const char* windowText)
{
   mTitle = windowText;
   [mGLKWindow setTitle:@(mTitle)];
   return true;
}

void iOSWindow::_doMouseLockNow()
{
//   if(!isVisible())
//      return;
//      
//   if(mShouldMouseLock == mMouseLocked && mMouseLocked != isCursorVisible())
//      return;
//   
////   if(mShouldMouseLock)
////      _dissociateMouse();
////   else
////      _associateMouse();
//   
//   // hide the cursor if we're locking, show it if we're unlocking
//   setCursorVisible(!shouldLockMouse());
//
//   mMouseLocked = mShouldMouseLock;

   return;
}

//void iOSWindow::_associateMouse()
//{
//   CGAssociateMouseAndMouseCursorPosition(true);
//}
//
//void iOSWindow::_dissociateMouse()
//{
//   _centerMouse();
//   CGAssociateMouseAndMouseCursorPosition(false);
//}
//
//void iOSWindow::_centerMouse()
//{
//   CGRect frame = [mGLKWindow frame];
//   
//   // Deal with the y flip (really fun when more than one monitor is involved)
//   F32 offsetY = mMainDisplayBounds.size.height - mDisplayBounds.size.height;
//   frame.origin.y = (mDisplayBounds.size.height + offsetY) - (S32)frame.origin.y - (S32)frame.size.height;
//   mCursorController->setCursorPosition(frame.origin.x + frame.size.width / 2, frame.origin.y + frame.size.height / 2);
//}
