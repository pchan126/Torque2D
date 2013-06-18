//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GLFWWindowMANAGER_H_
#define _GLFWWindowMANAGER_H_

#import "windowManager/platformWindowMgr.h"
#import "platform/platformGL.h"
#import "collection/vector.h"

class GLFWWindow;

class GLFWWindowManager : public PlatformWindowManager
{
private:
   typedef VectorPtr<GLFWWindow*> WindowList;
   WindowList mWindowList;
   U32 mFadeToken;
   Delegate<bool(void)> mNotifyShutdownDelegate;
   
public:
   GLFWWindowManager();
   ~GLFWWindowManager();

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
   GLFWWindow      *getWindowByGLFW(GLFWwindow* window);

   virtual PlatformWindow *getFirstWindow();
   virtual PlatformWindow* getFocusedWindow();
      
   /// @}
   /// @name Command Line Usage
   /// @{
   
   /// Process command line arguments sent to this window manager 
   /// to manipulate it's windows.  
   virtual void _processCmdLineArgs(const S32 argc, const char **argv);
   
   /// @}
   
   static GLFWWindowManager* get() { return (GLFWWindowManager*)PlatformWindowManager::get(); }
   void _addWindow(GLFWWindow* window);
   void _removeWindow(GLFWWindow* window);
   
   void _onAppSignal(WindowId wnd, S32 event);
   
   bool onShutdown();
   bool canWindowGainFocus(GLFWWindow* window);
   
   virtual PlatformWindow* assignCanvas(GFXDevice* device, const GFXVideoMode &mode ,GuiCanvas* canvas);
   
private:
   bool mIsShuttingDown;
};

#endif
