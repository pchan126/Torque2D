//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------


#import "./iOSWindow.h"

#import "console/console.h"
#import "platformiOS/platformiOS.h"
#import "platformiOS/graphics/GFXOpenGLESDevice.h"
#import "platformiOS/T2DAppDelegate.h"

iOSWindow* iOSWindow::sInstance = NULL;

iOSWindow::iOSWindow(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   
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
    view = nil;
    viewController = nil;
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
    GFXOpenGLESDevice *device = dynamic_cast<GFXOpenGLESDevice*>(GFX);
    EAGLContext *context = device->getEAGLContext();

    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    appDelegate.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    view = [[GLKView alloc] initWithFrame:[[UIScreen mainScreen] bounds] context:context];
    view.delegate = appDelegate;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat16;

    viewController = [[GLKViewController alloc] initWithNibName:nil bundle:nil];
    viewController.view = view;
    viewController.delegate = appDelegate;
    viewController.preferredFramesPerSecond = 30;
    viewController.paused = NO;
    
    appDelegate.T2DWindow = this;
    appDelegate.window.rootViewController = viewController;
    appDelegate.window.backgroundColor = [UIColor whiteColor];
    [appDelegate.window makeKeyAndVisible];
}

void iOSWindow::_disassociateCocoaWindow()
{
    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];

    view = nil;
    viewController = nil;
    appDelegate.window = nil;
}

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
   Con::printf("iOSWindow setSize %i %i", newSize.x, newSize.y);
   mSize = newSize;
   mDisplayBounds.size = CGSizeMake(newSize.x, newSize.y);
   resizeEvent.trigger(getWindowId(), newSize.x, newSize.y);
   return true;
}

void iOSWindow::setClientExtent( const Point2I newExtent )
{
   setSize(newExtent);
}

const Point2I iOSWindow::getClientExtent()
{
   return mSize;
}

void iOSWindow::setBounds( const RectI &newBounds )
{

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
   // always fullscreen
   return;
}

const Point2I iOSWindow::getPosition()
{
   // always fullscreen
   return Point2I(0, 0);
}

void iOSWindow::centerWindow()
{
   // always fullscreen
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
//   if(isFocused())
//      [[mGLKWindow delegate] performSelector:@selector(signalGainFocus)];
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
}

void iOSWindow::restore()
{
   return;
}

bool iOSWindow::isMinimized()
{
   return false;
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
//   mTitle = windowText;
//   [mGLKWindow setTitle:@(mTitle)];
   return true;
}

void iOSWindow::_doMouseLockNow()
{
   return;
}

