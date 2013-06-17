//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "./macWindow.h"
#import "platform/platformGL.h"
#import "platformOSX/platformOSX.h"
#import "platformOSX/windowManager/osxWindowInputGenerator.h"
#import "console/console.h"
#import "AppDelegate.h"

MacWindow* MacWindow::sInstance = NULL;

extern InputModifiers convertModifierBits( const U32 in );

inline U32 GLFWModifiersToTorqueModifiers( int mods )
{
   U32 torqueMods = 0;
   
   if( mods & GLFW_MOD_SHIFT )
      torqueMods |= IM_LSHIFT;
   if( mods & GLFW_MOD_CONTROL )
      torqueMods |= IM_LCTRL;
   if( mods & GLFW_MOD_ALT )
      torqueMods |= IM_LALT;
   
   Input::setModifierKeys( convertModifierBits( torqueMods ) );
   
   return torqueMods;
}

MacWindow::MacWindow(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   
   mWindowInputGenerator = new osxWindowInputGenerator( this);
   mCursorController = new MacCursorController( this );
   mOwningWindowManager = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   mDefaultDisplayMode = NULL;
   
   mSkipMouseEvents = 0;
   
   mDisplay = kCGDirectMainDisplay;
   mMainDisplayBounds = mDisplayBounds = CGDisplayBounds(mDisplay);
   
   mWindowId = windowId;
   
   window = glfwCreateWindow(640, 480, "Torque", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   glfwSetKeyCallback(window, &MacWindow::key_callback);
   glfwSetMouseButtonCallback(window, &MacWindow::mousebutton_callback);
   appEvent.notify(this, &MacWindow::_onAppEvent);
   
   sInstance = this;
}

MacWindow::~MacWindow()
{
   if(mFullscreen)
      _setFullscreen(false);

   appEvent.remove(this, &MacWindow::_onAppEvent);

   //ensure our view isn't the delegate
   [NSApp setDelegate:nil];
   
   if( window )
   {
      glfwDestroyWindow (window);
   }
   
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   sInstance = NULL;
}


void MacWindow::setVideoMode(const GFXVideoMode &mode)
{
   mCurrentMode = mode;
   setSize(mCurrentMode.resolution);
   
//   if(mTarget.isValid())
//      mTarget->resetMode();

   _setFullscreen(mCurrentMode.fullScreen);
}

void MacWindow::_onAppEvent(WindowId, S32 evt)
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

void MacWindow::_setFullscreen(bool fullScreen)
{
   if(mFullscreen == fullScreen)
      return;
   
   mFullscreen = fullScreen;
   
   if(mFullscreen)
   {
   }
   else
   {
   }
}

void* MacWindow::getPlatformDrawable() const
{
   return NULL;
}

void MacWindow::show()
{
   appEvent.trigger(getWindowId(), WindowShown);
   appEvent.trigger(getWindowId(), GainFocus);
}

void MacWindow::close()
{
   glfwDestroyWindow(window);
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   delete this;
}

void MacWindow::hide()
{
   glfwHideWindow(window);
   appEvent.trigger(getWindowId(), WindowHidden);
}

//void MacWindow::setDisplay(CGDirectDisplayID display)
//{
//   mDisplay = display;
//   mDisplayBounds = CGDisplayBounds(mDisplay);
//}

PlatformWindow* MacWindow::getNextWindow() const
{
   return mNextWindow;
}

bool MacWindow::setSize(const Point2I &newSize)
{
   glfwSetWindowSize(window, newSize.x, newSize.y);
   return true;
}

void MacWindow::setClientExtent( const Point2I newExtent )
{
   if(!mFullscreen)
   {
      setSize(newExtent);
   }
   else
   {
   }
}

const Point2I MacWindow::getClientExtent()
{
      // Get the Client Area Extent (Resolution) of this window
      Point2I ret;
      glfwGetWindowSize(window, &ret.x, &ret.y);
      return ret;
}

void MacWindow::setBounds( const RectI &newBounds )
{
   glfwSetWindowPos(window, newBounds.point.x, newBounds.point.y);
   glfwSetWindowSize(window, newBounds.extent.x, newBounds.extent.y);
//   NSRect newFrame = NSMakeRect(newBounds.point.x, newBounds.point.y, newBounds.extent.x, newBounds.extent.y);
//   [mCocoaWindow setFrame:newFrame display:YES];
}

const RectI MacWindow::getBounds() const
{
//   if(!mFullscreen)
//   {
//      // Get the position and size (fullscreen windows are always at (0,0)).
//      NSRect frame = [mCocoaWindow frame];
//      return RectI(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
//   }
//   else
//   {
   RectI ret;
   glfwGetWindowPos(window, &ret.point.x, &ret.point.y);
   glfwGetWindowSize(window, &ret.extent.x, &ret.extent.y);
   return ret;
//      return RectI(0, 0, mDisplayBounds.size.width, mDisplayBounds.size.height);
//   }
}

void MacWindow::setPosition( const Point2I newPosition )
{
   glfwSetWindowPos(window, newPosition.x, newPosition.y);
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//
//   NSPoint pos = {static_cast<CGFloat>(newPosition.x), newPosition.y + screenFrame.size.height};
//   [mCocoaWindow setFrameTopLeftPoint: pos];
}

const Point2I MacWindow::getPosition()
{
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//   NSRect frame = [mCocoaWindow frame];
   Point2I ret;
   glfwGetWindowPos(window, &ret.x, &ret.y);
   return ret;
   
//   return Point2I(frame.origin.x, screenFrame.size.height - (frame.origin.y + frame.size.height));
}

void MacWindow::centerWindow()
{
//   [mCocoaWindow center];
}

Point2I MacWindow::clientToScreen( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mCocoaWindow convertBaseToScreen: p ];
//   return Point2I( p.x, p.y );
}

Point2I MacWindow::screenToClient( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mCocoaWindow convertScreenToBase: p ];
//   return Point2I( p.x, p.y );
}

bool MacWindow::isFocused()
{
//   return [mCocoaWindow isKeyWindow];
   return false;
}

bool MacWindow::isOpen()
{
   // Maybe check if _window != NULL ?
   return true;
}

bool MacWindow::isVisible()
{
//   return !isMinimized() && ([mCocoaWindow isVisible] == YES);
}
   
void MacWindow::setFocus()
{
//   [mCocoaWindow makeKeyAndOrderFront:nil];
}

void MacWindow::signalGainFocus()
{
//   if(isFocused())
//      [[mCocoaWindow delegate] performSelector:@selector(signalGainFocus)];
}

void MacWindow::minimize()
{
   glfwIconifyWindow(window);
}

void MacWindow::maximize()
{
   if(!isVisible())
      return;
}

void MacWindow::restore()
{
   glfwRestoreWindow(window);
}

bool MacWindow::isMinimized()
{
    return false;
}

bool MacWindow::isMaximized()
{
   return false;
}

void MacWindow::clearFocus()
{
}

bool MacWindow::setCaption(const char* windowText)
{
   mTitle = windowText;
   glfwSetWindowTitle(window, windowText);
//   [mCocoaWindow setTitle:[NSString stringWithUTF8String:mTitle]];
   return true;
}

void MacWindow::_doMouseLockNow()
{
//   if(!isVisible())
//      return;
//      
//   if(mShouldMouseLock == mMouseLocked && mMouseLocked != isCursorVisible())
//      return;
//   
//   if(mShouldMouseLock)
//      _dissociateMouse();
//   else
//      _associateMouse();
//   
//   // hide the cursor if we're locking, show it if we're unlocking
//   setCursorVisible(!shouldLockMouse());
//
//   mMouseLocked = mShouldMouseLock;

   return;
}

void MacWindow::_associateMouse()
{
//   CGAssociateMouseAndMouseCursorPosition(true);
}

void MacWindow::_dissociateMouse()
{
//   _centerMouse();
//   CGAssociateMouseAndMouseCursorPosition(false);
}

void MacWindow::_centerMouse()
{
//   NSRect frame = [mCocoaWindow frame];
//   
//   // Deal with the y flip (really fun when more than one monitor is involved)
//   F32 offsetY = mMainDisplayBounds.size.height - mDisplayBounds.size.height;
//   frame.origin.y = (mDisplayBounds.size.height + offsetY) - (S32)frame.origin.y - (S32)frame.size.height;
//   mCursorController->setCursorPosition(frame.origin.x + frame.size.width / 2, frame.origin.y + frame.size.height / 2);
}

void MacWindow::swapBuffers()
{
   glfwSwapBuffers(window);
}

void MacWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   MacWindow* torqueWindow = MacWindowManager::get()->getWindowByGLFW(window);
   U32 eventAction = SI_MAKE;
   switch (action) {
      case GLFW_PRESS:
         eventAction = SI_MAKE;
         break;
      case GLFW_RELEASE:
         eventAction = SI_BREAK;
         break;
      case GLFW_REPEAT:
         eventAction = SI_REPEAT;
         break;
         
      default:
         break;
   }
   
   U8 torqueKeyCode = TranslateGLFWKeyCode(key);
   U32 mLastMods = GLFWModifiersToTorqueModifiers( mods );
   torqueWindow->keyEvent.trigger(torqueWindow->getWindowId(), mLastMods, eventAction, torqueKeyCode);
}

void MacWindow::mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
   MacWindow* torqueWindow = MacWindowManager::get()->getWindowByGLFW(window);
   U32 eventAction = SI_MAKE;
   switch (action) {
      case GLFW_PRESS:
         eventAction = SI_MAKE;
         break;
      case GLFW_RELEASE:
         eventAction = SI_BREAK;
         break;
   }
   U16 buttonNumber = KEY_BUTTON0 + button;
   U32 mLastMods = GLFWModifiersToTorqueModifiers( mods );
   torqueWindow->buttonEvent.trigger(torqueWindow->getWindowId(), mLastMods, eventAction, buttonNumber);
}

void MacWindow::mousemove_callback(GLFWwindow* window, int xpos, int ypos)
{
   MacWindow* torqueWindow = MacWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->mouseEvent.trigger(torqueWindow->getWindowId(), 0, xpos, ypos, false);
}


