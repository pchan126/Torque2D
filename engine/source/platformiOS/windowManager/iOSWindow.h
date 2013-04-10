//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _TORQUE_iOSWindow_H_
#define _TORQUE_iOSWindow_H_

#import "windowManager/platformWindow.h"
#import "./iOSWindowManager.h"
#import "./iOSCursorController.h"
#import "./T2DViewController.h"

#import "graphics/gfxTarget.h"
#import "graphics/gfxStructs.h"

class iOSWindow : public PlatformWindow
{
public:
   virtual ~iOSWindow();

   virtual GFXDevice *getGFXDevice() { return mDevice; }
   virtual GFXWindowTarget *getGFXTarget() { return mTarget; }
   virtual void setVideoMode(const GFXVideoMode &mode);
   virtual const GFXVideoMode &getVideoMode() { return mCurrentMode; }
   
   virtual WindowId getWindowId() { return mWindowId; }
   
//   void setDisplay(CGDirectDisplayID display);
//   CGDirectDisplayID getDisplay() { return mDisplay; }
//   CGRect getMainDisplayBounds() { return mMainDisplayBounds; }
//   CGRect getDisplayBounds() { return mDisplayBounds; }

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
//   void _associateMouse();
//   void _dissociateMouse();
   void _centerMouse();

   // For GGMacView
   void _disassociateCocoaWindow();
   
protected:
   virtual void _setFullscreen(bool fullScreen);
   
private:
   friend class iOSWindowManager;
   friend class iOSCursorController;
   
   iOSWindow(U32 windowId, const char* windowText, Point2I clientExtent);
   
//   void _initCocoaWindow(const char* windowText, Point2I clientExtent);
   void setWindowId(U32 newid) { mWindowId = newid;}
   void signalGainFocus();

   static iOSWindow* sInstance;
   
   T2DViewController* mGLKWindow;
   GFXDevice *mDevice;
   GFXWindowTargetRef mTarget;
   GFXVideoMode mCurrentMode;
   
   iOSWindow *mNextWindow;

   bool mMouseLocked;
   bool mShouldMouseLock;
      
   const char* mTitle;
   bool mMouseCaptured;
   
   iOSWindowManager* mOwningWindowManager;
   U32 mSkipMouseEvents;
   
   bool mFullscreen;
   bool mShouldFullscreen;
   NSDictionary* mDefaultDisplayMode;
   
   void _onAppEvent(WindowId,S32);
   
   UIScreen* mDisplay;
   CGRect mDisplayBounds;
   CGRect mMainDisplayBounds;
};

#endif
