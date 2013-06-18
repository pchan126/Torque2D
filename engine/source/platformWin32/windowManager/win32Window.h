//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef  _WINDOWMANAGER_WIN32_WIN32WINDOW_
#define  _WINDOWMANAGER_WIN32_WIN32WINDOW_

#include "windowManager/platformWindow.h"
#include "windowManager/platformWindowMgr.h"
#include "platform/platformGL.h"

#include "graphics/gfxTarget.h"
#include "graphics/gfxStructs.h"

class GLFWwindow;

/// Implementation of a window on Win32.
class Win32Window : public PlatformWindow
{
public:
   virtual ~Win32Window();

   virtual GFXDevice *getGFXDevice() { return mDevice; }
   virtual GFXWindowTarget *getGFXTarget() { return mTarget; }
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
   HGLRC getContext();

protected:
   virtual void _setFullscreen(bool fullScreen);
   
private:
   friend class Win32WindowManager;
   friend class MacCursorController;
   
   Win32Window(U32 windowId, const char* windowText, Point2I clientExtent);
   
   void setWindowId(U32 newid) { mWindowId = newid;}
   void signalGainFocus();

   GLFWwindow* window;

   GFXDevice *mDevice;
   GFXWindowTargetRef mTarget;
   GFXVideoMode mCurrentMode;
   
   Win32Window *mNextWindow;

   bool mMouseLocked;
   bool mShouldMouseLock;
      
   const char* mTitle;
   bool mMouseCaptured;
   
   Win32WindowManager* mOwningWindowManager;
   U32 mSkipMouseEvents;
   
   bool mFullscreen;
   bool mShouldFullscreen;
   
   void _onAppEvent(WindowId,S32);
   
   static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void mousebutton_callback(GLFWwindow* window, int button, int action, int mods);
   static void mousemove_callback(GLFWwindow* window, double xpos, double ypos);
   static void window_close_callback(GLFWwindow* window);

public:
    ButtonEvent       mouseButtonEvent;
    MouseEvent        mouseEvent;
};

#endif
