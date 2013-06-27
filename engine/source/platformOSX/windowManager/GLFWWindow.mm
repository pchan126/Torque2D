//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GLFWWindow.h"
#include "./GLFWWindowInputGenerator.h"

GLFWWindow* GLFWWindow::sInstance = NULL;

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

GLFWWindow::GLFWWindow(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   mBoundCanvas = NULL;
   mWindowInputGenerator = new GLFWWindowInputGenerator( this);
   mCursorController = new GLFWCursorController( this );
   mOwningWindowManager = NULL;
   mTarget = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   
   mSkipMouseEvents = 0;
   
   mWindowId = windowId;
   
   GLFWwindow* sWindow = NULL;
   GLFWWindow* shareWindow = dynamic_cast<GLFWWindow*>(WindowManager->getFirstWindow());

   if (shareWindow != NULL)
      sWindow = shareWindow->window;
   
   window = glfwCreateWindow(640, 480, "Torque", NULL, sWindow);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   glfwSetKeyCallback(window, &GLFWWindow::key_callback);
   glfwSetMouseButtonCallback(window, &GLFWWindow::mousebutton_callback);
   glfwSetCursorPosCallback(window, &GLFWWindow::mousemove_callback);
   glfwSetWindowCloseCallback(window, &GLFWWindow::window_close_callback);
   glfwSetWindowFocusCallback(window, &GLFWWindow::window_focus_callback);
   glfwSetWindowIconifyCallback(window, &GLFWWindow::window_iconify_callback);
   glfwSetScrollCallback(window, &GLFWWindow::window_scroll_callback);
   
   appEvent.notify(this, &GLFWWindow::_onAppEvent);
   
   sInstance = this;
}

GLFWWindow::~GLFWWindow()
{
   if(mFullscreen)
      _setFullscreen(false);

   appEvent.remove(this, &GLFWWindow::_onAppEvent);

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


void GLFWWindow::setVideoMode(const GFXVideoMode &mode)
{
   mCurrentMode = mode;
   setSize(mCurrentMode.resolution);
   
//   if(mTarget.isValid())
//      mTarget->resetMode();

   _setFullscreen(mCurrentMode.fullScreen);
}

void GLFWWindow::_onAppEvent(WindowId, S32 evt)
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

void GLFWWindow::_setFullscreen(bool fullScreen)
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

void* GLFWWindow::getPlatformDrawable() const
{
   return NULL;
}

void GLFWWindow::show()
{
   appEvent.trigger(getWindowId(), WindowShown);
   appEvent.trigger(getWindowId(), GainFocus);
}

void GLFWWindow::close()
{
   glfwDestroyWindow(window);
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   delete this;
}

void GLFWWindow::hide()
{
   glfwHideWindow(window);
   appEvent.trigger(getWindowId(), WindowHidden);
}

//void GLFWWindow::setDisplay(CGDirectDisplayID display)
//{
//   mDisplay = display;
//   mDisplayBounds = CGDisplayBounds(mDisplay);
//}

PlatformWindow* GLFWWindow::getNextWindow() const
{
   return mNextWindow;
}

bool GLFWWindow::setSize(const Point2I &newSize)
{
   glfwSetWindowSize(window, newSize.x, newSize.y);
   return true;
}

void GLFWWindow::setClientExtent( const Point2I newExtent )
{
   if(!mFullscreen)
   {
      setSize(newExtent);
   }
   else
   {
   }
}

const Point2I GLFWWindow::getClientExtent()
{
      // Get the Client Area Extent (Resolution) of this window
      Point2I ret;
      glfwGetWindowSize(window, &ret.x, &ret.y);
      return ret;
}

void GLFWWindow::setBounds( const RectI &newBounds )
{
   glfwSetWindowPos(window, newBounds.point.x, newBounds.point.y);
   glfwSetWindowSize(window, newBounds.extent.x, newBounds.extent.y);
//   NSRect newFrame = NSMakeRect(newBounds.point.x, newBounds.point.y, newBounds.extent.x, newBounds.extent.y);
//   [mCocoaWindow setFrame:newFrame display:YES];
}

const RectI GLFWWindow::getBounds() const
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

void GLFWWindow::setPosition( const Point2I newPosition )
{
   glfwSetWindowPos(window, newPosition.x, newPosition.y);
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//
//   NSPoint pos = {static_cast<CGFloat>(newPosition.x), newPosition.y + screenFrame.size.height};
//   [mCocoaWindow setFrameTopLeftPoint: pos];
}

const Point2I GLFWWindow::getPosition()
{
//   NSScreen *screen = [mCocoaWindow screen];
//   NSRect screenFrame = [screen frame];
//   NSRect frame = [mCocoaWindow frame];
   Point2I ret;
   glfwGetWindowPos(window, &ret.x, &ret.y);
   return ret;
   
//   return Point2I(frame.origin.x, screenFrame.size.height - (frame.origin.y + frame.size.height));
}

void GLFWWindow::centerWindow()
{
//   [mCocoaWindow center];
}

Point2I GLFWWindow::clientToScreen( const Point2I& pos )
{
    return pos;
}

Point2I GLFWWindow::screenToClient( const Point2I& pos )
{
    return pos;
}

bool GLFWWindow::isFocused()
{
//   return [mCocoaWindow isKeyWindow];
   return false;
}

bool GLFWWindow::isOpen()
{
   // Maybe check if _window != NULL ?
   return true;
}

bool GLFWWindow::isVisible()
{
//   return !isMinimized() && ([mCocoaWindow isVisible] == YES);
   return true;
}
   
void GLFWWindow::setFocus()
{
//   [mCocoaWindow makeKeyAndOrderFront:nil];
}

void GLFWWindow::signalGainFocus()
{
//   if(isFocused())
//      [[mCocoaWindow delegate] performSelector:@selector(signalGainFocus)];
}

void GLFWWindow::minimize()
{
   glfwIconifyWindow(window);
}

void GLFWWindow::maximize()
{
   if(!isVisible())
      return;
}

void GLFWWindow::restore()
{
   glfwRestoreWindow(window);
}

bool GLFWWindow::isMinimized()
{
    return false;
}

bool GLFWWindow::isMaximized()
{
   return false;
}

void GLFWWindow::clearFocus()
{
}

bool GLFWWindow::setCaption(const char* windowText)
{
   mTitle = windowText;
   glfwSetWindowTitle(window, windowText);
   return true;
}

void GLFWWindow::_doMouseLockNow()
{
   return;
}

void GLFWWindow::_associateMouse()
{
}

void GLFWWindow::_dissociateMouse()
{
}

void GLFWWindow::_centerMouse()
{
}

void GLFWWindow::getCursorPosition( Point2I &point )
{
   double xpos, ypos;
   glfwGetCursorPos(window, &xpos, &ypos);
   point.x = (U32)xpos;
   point.y = (U32)ypos;
}

void GLFWWindow::setCursorPosition( const Point2D point )
{
   glfwSetCursorPos(window, (double)point.x, (double)point.y);
}

void GLFWWindow::swapBuffers()
{
   glfwSwapBuffers(window);
}

void GLFWWindow::makeContextCurrent()
{
   glfwMakeContextCurrent(window);
}


void GLFWWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
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

void GLFWWindow::mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
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

void GLFWWindow::mousemove_callback(GLFWwindow* window, double xpos, double ypos)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->mouseEvent.trigger(torqueWindow->getWindowId(), 0, xpos, ypos, false);
}

void GLFWWindow::window_close_callback(GLFWwindow* window)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->appEvent.trigger(torqueWindow->getWindowId(), WindowDestroy);
   delete torqueWindow;
}

void GLFWWindow::window_focus_callback(GLFWwindow* window, int focused)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   if (focused == GL_TRUE)
      torqueWindow->appEvent.trigger(torqueWindow->getWindowId(), GainFocus);
   else if (focused == GL_FALSE)
      torqueWindow->appEvent.trigger(torqueWindow->getWindowId(), LoseFocus);
}

void GLFWWindow::window_iconify_callback(GLFWwindow* window, int iconified)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   if (iconified == GL_TRUE)
      torqueWindow->appEvent.trigger(torqueWindow->getWindowId(), WindowHidden);
   else if (iconified == GL_FALSE)
      torqueWindow->appEvent.trigger(torqueWindow->getWindowId(), WindowShown);
}

void GLFWWindow::window_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->mouseWheelEvent.trigger(torqueWindow->getWindowId(), 0, (S32)xoffset, (S32)yoffset);
}

void GLFWWindow::window_resize_callback(GLFWwindow* window, int width, int height)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->resizeEvent.trigger(torqueWindow->getWindowId(), width, height);
}

void GLFWWindow::framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
   GLFWWindow* torqueWindow = GLFWWindowManager::get()->getWindowByGLFW(window);
   torqueWindow->framebufferResizeEvent.trigger(torqueWindow->getWindowId(), width, height);
}
