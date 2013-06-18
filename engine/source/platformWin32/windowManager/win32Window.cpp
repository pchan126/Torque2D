//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformWin32/platformWin32.h"
#include "platform/platformGL.h"
#include "platformWin32/windowManager/win32Window.h"
#include "platformWin32/windowManager/Win32WindowInputGenerator.h"
#include "platformWin32/windowManager/win32WindowMgr.h"
#include "platformWin32/windowManager/win32CursorController.h"
#include "console/console.h"

extern U32 convertModifierBits( const U32 in );

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

Win32Window::Win32Window(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   mBoundCanvas = NULL;
   mWindowInputGenerator = new Win32WindowInputGenerator( this);
   mCursorController = new Win32CursorController( this );
   mOwningWindowManager = NULL;
   mTarget = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   
   mSkipMouseEvents = 0;
      
   mWindowId = windowId;
   
   GLFWwindow* sWindow = NULL;
   Win32Window* shareWindow = dynamic_cast<Win32Window*>(WindowManager->getFirstWindow());

   if (shareWindow != NULL)
      sWindow = shareWindow->window;
   
   window = glfwCreateWindow(640, 480, "Torque", NULL, sWindow);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   glfwSetKeyCallback(window, &Win32Window::key_callback);
   glfwSetMouseButtonCallback(window, &Win32Window::mousebutton_callback);
   glfwSetCursorPosCallback(window, &Win32Window::mousemove_callback);
   glfwSetWindowCloseCallback(window, &Win32Window::window_close_callback);
   appEvent.notify(this, &Win32Window::_onAppEvent);
}

Win32Window::~Win32Window()
{
   if(mFullscreen)
      _setFullscreen(false);

   appEvent.remove(this, &Win32Window::_onAppEvent);

   if( window )
   {
      glfwDestroyWindow (window);
   }
   
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
}


void Win32Window::setVideoMode(const GFXVideoMode &mode)
{
   mCurrentMode = mode;
   setSize(mCurrentMode.resolution);
   
//   if(mTarget.isValid())
//      mTarget->resetMode();

   _setFullscreen(mCurrentMode.fullScreen);
}

void Win32Window::_onAppEvent(WindowId, S32 evt)
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

void Win32Window::_setFullscreen(bool fullScreen)
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

void* Win32Window::getPlatformDrawable() const
{
   return NULL;
}

void Win32Window::show()
{
   appEvent.trigger(getWindowId(), WindowShown);
   appEvent.trigger(getWindowId(), GainFocus);
}

void Win32Window::close()
{
   glfwDestroyWindow(window);
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   delete this;
}

void Win32Window::hide()
{
   glfwHideWindow(window);
   appEvent.trigger(getWindowId(), WindowHidden);
}

//void Win32Window::setDisplay(CGDirectDisplayID display)
//{
//   mDisplay = display;
//   mDisplayBounds = CGDisplayBounds(mDisplay);
//}

PlatformWindow* Win32Window::getNextWindow() const
{
   return mNextWindow;
}

bool Win32Window::setSize(const Point2I &newSize)
{
   glfwSetWindowSize(window, newSize.x, newSize.y);
   return true;
}

void Win32Window::setClientExtent( const Point2I newExtent )
{
   if(!mFullscreen)
   {
      setSize(newExtent);
   }
   else
   {
   }
}

const Point2I Win32Window::getClientExtent()
{
      // Get the Client Area Extent (Resolution) of this window
      Point2I ret;
      glfwGetWindowSize(window, &ret.x, &ret.y);
      return ret;
}

void Win32Window::setBounds( const RectI &newBounds )
{
   glfwSetWindowPos(window, newBounds.point.x, newBounds.point.y);
   glfwSetWindowSize(window, newBounds.extent.x, newBounds.extent.y);
//   NSRect newFrame = NSMakeRect(newBounds.point.x, newBounds.point.y, newBounds.extent.x, newBounds.extent.y);
//   [mCocoaWindow setFrame:newFrame display:YES];
}

const RectI Win32Window::getBounds() const
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

void Win32Window::setPosition( const Point2I newPosition )
{
   glfwSetWindowPos(window, newPosition.x, newPosition.y);
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//
//   NSPoint pos = {static_cast<CGFloat>(newPosition.x), newPosition.y + screenFrame.size.height};
//   [mCocoaWindow setFrameTopLeftPoint: pos];
}

const Point2I Win32Window::getPosition()
{
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//   NSRect frame = [mCocoaWindow frame];
   Point2I ret;
   glfwGetWindowPos(window, &ret.x, &ret.y);
   return ret;
   
//   return Point2I(frame.origin.x, screenFrame.size.height - (frame.origin.y + frame.size.height));
}

void Win32Window::centerWindow()
{
//   [mCocoaWindow center];
}

Point2I Win32Window::clientToScreen( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mCocoaWindow convertBaseToScreen: p ];
//   return Point2I( p.x, p.y );
	return Point2I( 0, 0);
}

Point2I Win32Window::screenToClient( const Point2I& pos )
{
//   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
//   
//   p = [ mCocoaWindow convertScreenToBase: p ];
//   return Point2I( p.x, p.y );
	return Point2I( 0, 0);
}

bool Win32Window::isFocused()
{
//   return [mCocoaWindow isKeyWindow];
   return false;
}

bool Win32Window::isOpen()
{
   // Maybe check if _window != NULL ?
   return true;
}

bool Win32Window::isVisible()
{
//   return !isMinimized() && ([mCocoaWindow isVisible] == YES);
   return true;
}
   
void Win32Window::setFocus()
{
//   [mCocoaWindow makeKeyAndOrderFront:nil];
}

void Win32Window::signalGainFocus()
{
//   if(isFocused())
//      [[mCocoaWindow delegate] performSelector:@selector(signalGainFocus)];
}

void Win32Window::minimize()
{
   glfwIconifyWindow(window);
}

void Win32Window::maximize()
{
   if(!isVisible())
      return;
}

void Win32Window::restore()
{
   glfwRestoreWindow(window);
}

bool Win32Window::isMinimized()
{
    return false;
}

bool Win32Window::isMaximized()
{
   return false;
}

void Win32Window::clearFocus()
{
}

bool Win32Window::setCaption(const char* windowText)
{
   mTitle = windowText;
   glfwSetWindowTitle(window, windowText);
//   [mCocoaWindow setTitle:[NSString stringWithUTF8String:mTitle]];
   return true;
}

void Win32Window::_doMouseLockNow()
{
   return;
}

void Win32Window::_associateMouse()
{
}

void Win32Window::_dissociateMouse()
{
}

void Win32Window::_centerMouse()
{
}

void Win32Window::swapBuffers()
{
   glfwSwapBuffers(window);
}

void Win32Window::makeContextCurrent()
{
   glfwMakeContextCurrent(window);
}

HGLRC Win32Window::getContext()
{
	return glfwGetWGLContext(window);
}


void Win32Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Win32Window* torqueWindow = Win32WindowManager::get()->getWindowByGLFW(window);
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

void Win32Window::mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
   Win32Window* torqueWindow = Win32WindowManager::get()->getWindowByGLFW(window);
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

void Win32Window::mousemove_callback(GLFWwindow* window, double xpos, double ypos)
{
   Win32Window* torqueWindow = Win32WindowManager::get()->getWindowByGLFW(window);
   torqueWindow->mouseEvent.trigger(torqueWindow->getWindowId(), 0, xpos, ypos, false);
}

void Win32Window::window_close_callback(GLFWwindow* window)
{
   Win32Window* torqueWindow = Win32WindowManager::get()->getWindowByGLFW(window);
//   torqueWindow->appEvent.trigger(<#unsigned int a#>, <#int b#>)
}
