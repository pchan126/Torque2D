//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _TORQUE_GLFWWindow_H_
#define _TORQUE_GLFWWindow_H_

#include "windowManager/platformWindow.h"
#include "./GLFWWindowManager.h"
#include "./GLFWCursorController.h"

#include "graphics/gfxTarget.h"
#include "graphics/gfxStructs.h"

class GLFWwindow;

class GLFWWindow : public PlatformWindow
{
public:
   virtual ~GLFWWindow();

   virtual GFXDevice *getGFXDevice() { return mDevice; }
   virtual GFXWindowTargetRef getGFXTarget() { return mTarget; }
   virtual void setVideoMode(const GFXVideoMode &mode);
   virtual const GFXVideoMode &getVideoMode() { return mCurrentMode; }
   
   virtual WindowId getWindowId() { return mWindowId; }
   
   virtual bool clearFullscreen() 
   { 
      // TODO: properly drop out of full screen
      return true;
   }
   virtual bool isFullscreen() { return mFullscreen; }

   virtual PlatformWindow * getNextWindow() const;
   
   virtual void setMouseLocked( bool enable ) 
   { 
      mShouldMouseLock = enable; 
      if(isFocused()) 
         _doMouseLockNow(); 
   }
   virtual bool isMouseLocked() const { return mMouseLocked; }
   virtual bool shouldLockMouse() const { return mShouldMouseLock; }

   virtual bool setSize(const Point2I &newSize);

   virtual void setClientExtent( const Point2I newExtent );
   virtual const Point2I getClientExtent();
   
   virtual void setBounds( const RectI &newBounds );
   virtual const RectI getBounds() const;

   virtual void setPosition( const Point2I newPosition );
   virtual const Point2I getPosition();
   
   virtual void centerWindow();
   
   virtual Point2I clientToScreen( const Point2I& pos );
   virtual Point2I screenToClient( const Point2I& pos );

   virtual bool setCaption(const char *windowText);
   virtual const char *getCaption() { return mTitle; }

   virtual bool setType( S32 windowType ) { return true; }

   virtual void minimize();
   virtual void maximize();
   virtual void restore();
   virtual bool isMinimized();
   virtual bool isMaximized();
   virtual void show();
   virtual void close();
   virtual void hide();
   virtual bool isOpen();
   virtual bool isVisible();

   virtual bool isFocused();
   virtual void setFocus();
   virtual void clearFocus();
   
   virtual void* getPlatformDrawable() const;
   
   void swapBuffers();
   
   // TODO: These should be private, but GGMacView (an Obj-C class) needs access to these and we can't friend Obj-C classes
   bool _skipNextMouseEvent() { return mSkipMouseEvents != 0; }
   void _skipAnotherMouseEvent() { mSkipMouseEvents++; }
   void _skippedMouseEvent() { mSkipMouseEvents--; }
   
   /// Does the work of actually locking or unlocking the mouse, based on the
   /// value of shouldLockMouse().
   ///
   /// Disassociates the cursor movement from the mouse input and hides the mouse
   /// when locking. Re-associates cursor movement with mouse input and shows the
   /// mouse when unlocking.
   /// 
   /// Returns true if we locked or unlocked the mouse. Returns false if the mouse
   /// was already in the correct state.
   void _doMouseLockNow();
   
   // Helper methods for doMouseLockNow
   void _associateMouse();
   void _dissociateMouse();
   void _centerMouse();

   void makeContextCurrent();
   
#if defined(GLFW_EXPOSE_NATIVE_NSGL)
   NSOpenGLContext* getContext() { return glfwGetNSGLContext(window); };
#endif

#if defined(GLFW_EXPOSE_NATIVE_WGL)
   HGLRC getContext()  { return glfwGetWGLContext(window); };
#endif
   
   void getCursorPosition( Point2I &point );
   void setCursorPosition( const Point2D point );
   
   const char* getClipboardString() { return glfwGetClipboardString(window); };
   void setClipboardString(const char *string) { glfwSetClipboardString(window, string); };

protected:
   virtual void _setFullscreen(bool fullScreen);
   
private:
   friend class GLFWWindowManager;
   friend class GLFWCursorController;
   
   GLFWWindow(U32 windowId, const char* windowText, Point2I clientExtent);
   
   void setWindowId(U32 newid) { mWindowId = newid;}
   void signalGainFocus();

   static GLFWWindow* sInstance;
   GLFWwindow* window;

   GFXDevice *mDevice;
   GFXWindowTargetRef mTarget;
   GFXVideoMode mCurrentMode;
   
   GLFWWindow *mNextWindow;

   bool mMouseLocked;
   bool mShouldMouseLock;
      
   const char* mTitle;
   bool mMouseCaptured;
   
   GLFWWindowManager* mOwningWindowManager;
   U32 mSkipMouseEvents;
   
   bool mFullscreen;
   bool mShouldFullscreen;
   
   void _onAppEvent(WindowId,S32);
   
   static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void mousebutton_callback(GLFWwindow* window, int button, int action, int mods);
   static void mousemove_callback(GLFWwindow* window, double xpos, double ypos);
   static void window_close_callback(GLFWwindow* window);
   static void window_focus_callback(GLFWwindow* window, int focus);
   static void window_iconify_callback(GLFWwindow* window, int iconified);
   static void window_scroll_callback(GLFWwindow* window, double xoffset, double yoffset );
   static void window_resize_callback(GLFWwindow* window, int width, int height);
   static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

public:
    ButtonEvent       mouseButtonEvent;
    MouseEvent        mouseEvent;
    MouseWheelEvent   mouseWheelEvent;
    ResizeEvent       framebufferResizeEvent;
};

#endif
