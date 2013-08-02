//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _iOSWindowManager_H_
#define _iOSWindowManager_H_

#import "windowManager/platformWindowMgr.h"
#import "collection/vector.h"
#import <GLKit/GLKit.h>
#import "platformiOS/T2DViewController.h"

class iOSWindow;

class iOSWindowManager : public PlatformWindowManager
{
public:
    typedef Vector<iOSWindow*> WindowList;

private:
   WindowList mWindowList;
   U32 mFadeToken;
   Delegate<bool(void)> mNotifyShutdownDelegate;
    UIWindow *extWindow;
    UIScreen *extScreen;
   
public:
   iOSWindowManager();
   ~iOSWindowManager();

   virtual void setParentWindow(void* newParent) {
   }
   
   /// Get the parent window
   virtual void* getParentWindow()
   {
      return NULL;
   }
   
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode);
   
   /// @name Desktop Queries
   /// @{
   
   /// Return the extents in window coordinates of the primary desktop
   /// area. On a single monitor system this is just the display extents.
   /// On a multi-monitor system this is the primary monitor (which Torque should
   /// launch on).
   virtual RectI getPrimaryDesktopArea();
   
   /// Populate a vector with all monitors and their extents in window space.
   virtual void getMonitorRegions(Vector<RectI> &regions);
   
   /// Retrieve the currently set desktop bit depth
   /// @return The current desktop bit depth, or -1 if an error occurred
   virtual S32 getDesktopBitDepth();
   
   /// Retrieve the currently set desktop resolution
   /// @return The current desktop bit depth, or Point2I(-1,-1) if an error occurred
   virtual Point2I getDesktopResolution();
   
   /// @}
   
   /// @name Window Lookup
   /// @{
    
    void updateWindows();
   
   /// Get the number of Window's in this system
   virtual S32 getWindowCount();
   
   /// Populate a list with references to all the windows created from this manager.
   virtual void getWindows(Vector<PlatformWindow*> &windows);
   
   /// Get a window from a device ID.
   ///
   /// @return The window associated with the specified ID, or NULL if no
   ///         match was found.
   virtual PlatformWindow *getWindowById(WindowId id);

   virtual PlatformWindow *getFirstWindow();
   virtual PlatformWindow* getFocusedWindow();
   
   /// During full-screen toggles we want to suppress ugly transition states,
   /// which we do (on Win32) by showing and hiding a full-monitor black window.
   ///
   /// This method cues the appearance of that window ("lowering the curtain").
   virtual void lowerCurtain();
   
   /// @see lowerCurtain
   ///
   /// This method removes the curtain window.
   virtual void raiseCurtain();
   
   /// @}
   /// @name Command Line Usage
   /// @{
   
   /// Process command line arguments sent to this window manager 
   /// to manipulate it's windows.  
   virtual void _processCmdLineArgs(const S32 argc, const char **argv);
   
   /// @}
   
   // static iOSWindowManager* get() { return (iOSWindowManager*)PlatformWindowManager::get(); }
   void _addWindow(iOSWindow* window);
   void _removeWindow(iOSWindow* window);
   
   void _onAppSignal(WindowId wnd, S32 event);
   
   bool onShutdown();
   bool canWindowGainFocus(iOSWindow* window);
   
   virtual PlatformWindow* assignCanvas(GFXDevice* device, const GFXVideoMode &mode ,GuiCanvas* canvas);

   T2DViewController *viewController;
   T2DViewController *extController;
private:
   bool mIsShuttingDown;
};

#endif
