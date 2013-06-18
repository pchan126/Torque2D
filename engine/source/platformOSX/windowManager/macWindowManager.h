//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _MACWINDOWMANAGER_H_
#define _MACWINDOWMANAGER_H_

#import "windowManager/platformWindowMgr.h"
#import "platform/platformGL.h"
#import "collection/vector.h"

class MacWindow;

class MacWindowManager : public PlatformWindowManager
{
private:
   typedef VectorPtr<MacWindow*> WindowList;
   WindowList mWindowList;
   U32 mFadeToken;
   Delegate<bool(void)> mNotifyShutdownDelegate;
   
public:
   MacWindowManager();
   ~MacWindowManager();

   virtual void setParentWindow(void* newParent) {
   }
   
   /// Get the parent window
   virtual void* getParentWindow()
   {
      return NULL;
   }
   
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode);
   
   /// @name Window Lookup
   /// @{
   
   /// Get the number of Window's in this system
   virtual S32 getWindowCount();
   
   /// Populate a list with references to all the windows created from this manager.
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows);
   
   /// Get a window from a device ID.
   ///
   /// @return The window associated with the specified ID, or NULL if no
   ///         match was found.
   virtual PlatformWindow *getWindowById(WindowId id);
   MacWindow      *getWindowByGLFW(GLFWwindow* window);

   virtual PlatformWindow *getFirstWindow();
   virtual PlatformWindow* getFocusedWindow();
      
   /// @}
   /// @name Command Line Usage
   /// @{
   
   /// Process command line arguments sent to this window manager 
   /// to manipulate it's windows.  
   virtual void _processCmdLineArgs(const S32 argc, const char **argv);
   
   /// @}
   
   static MacWindowManager* get() { return (MacWindowManager*)PlatformWindowManager::get(); }
   void _addWindow(MacWindow* window);
   void _removeWindow(MacWindow* window);
   
   void _onAppSignal(WindowId wnd, S32 event);
   
   bool onShutdown();
   bool canWindowGainFocus(MacWindow* window);
   
   virtual PlatformWindow* assignCanvas(GFXDevice* device, const GFXVideoMode &mode ,GuiCanvas* canvas);
   
private:
   bool mIsShuttingDown;
};

#endif
