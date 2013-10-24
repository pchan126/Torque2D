//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------


#import "./iOSWindow.h"

#import "console/console.h"
#import "platformiOS/platformiOS.h"
#import "platformiOS/graphics/ES20/GFXOpenGLES20iOSDevice.h"
#import "platformiOS/T2DAppDelegate.h"
#import "platformiOS/windowManager/iOSGestureRecognizer.h"
#import "platformiOS/windowManager/iOSWindowInputGenerator.h"

iOSWindow* iOSWindow::sInstance = NULL;

iOSWindow::iOSWindow(U32 windowId)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   
   // This controller maps window input (Mouse/Keyboard) to a generic input consumer
   mWindowInputGenerator = new iOSWindowInputGenerator( this );

   mCursorController = new iOSCursorController( this );
   mOwningWindowManager = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   mDefaultDisplayMode = NULL;
   
   mSkipMouseEvents = 0;
    
    mDisplay = [UIScreen screens][windowId];
    mDisplayScale = [mDisplay scale];
    
    mMainDisplayBounds = mDisplayBounds = [mDisplay bounds];
    
   mWindowId = windowId;

    GFXOpenGLES20iOSDevice *device = dynamic_cast<GFXOpenGLES20iOSDevice*>(GFX);
    EAGLContext *context = device->getEAGLContext();
    
    S32 tempType = Con::getIntVariable("$pref::iOS::StatusBarType");
    if (tempType == 0)
        [[UIApplication sharedApplication] setStatusBarHidden:YES];
    
    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    
    view = [[GLKView alloc] initWithFrame:[mDisplay bounds] context:context];
    view.delegate = appDelegate;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    
   gestureRecognizer = [[iOSGestureRecognizer alloc]initWithT2DWindow:this];
   
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


void iOSWindow::_disassociateCocoaWindow()
{
    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];

    view = nil;
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
    CGSize size = [view bounds].size;
    return Point2I(size.width*mDisplayScale,size.height*mDisplayScale);

//    return mSize;
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
    return pos;
}

Point2I iOSWindow::screenToClient( const Point2I& pos )
{
   return pos;
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
}

bool iOSWindow::setCaption(const char* windowText)
{
   return true;
}

void iOSWindow::_doMouseLockNow()
{
   return;
}

void iOSWindow::addGestureRecognizer( UIGestureRecognizer* gestureRecognizer)
{
    [view addGestureRecognizer:gestureRecognizer];
    return;
}

void iOSWindow::removeGestureRecognizer( UIGestureRecognizer* gestureRecognizer)
{
    [view removeGestureRecognizer:gestureRecognizer];
    return;
}

