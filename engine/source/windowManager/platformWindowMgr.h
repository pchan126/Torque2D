//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _PLATFORM_PLATFORMWINDOWMGR_H_
#define _PLATFORM_PLATFORMWINDOWMGR_H_

#include "math/mRect.h"
#include "delegates/delegateSignal.h"
#include "windowManager/platformWindow.h"


// Global macro
#define WindowManager PlatformWindowManager::get()

/// Abstract representation of a manager for native OS windows.
///
/// The PlatformWindowManager interface provides a variety of methods for querying 
/// the current desktop configuration, as well as allocating and retrieving
/// existing windows. It may also manage application-level event handling.
class PlatformWindowManager
{
   // Generator for window IDs.
   S32 mIdSource;
   
protected:
   /// Get the next available window Id
   inline S32 getNextId() { return mIdSource++; }
public:

   /// Get Global Singleton
   static PlatformWindowManager *get();
   
   PlatformWindowManager() : mIdSource(0) {};

   virtual ~PlatformWindowManager() 
   {
   }

   static void processCmdLineArgs(const S32 argc, const char **argv);

   /// Create a new window, appropriate for the specified device and mode.
   ///
   /// @return Pointer to the new window.
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode) = 0;

   /// Populate a list with references to all the windows created from this manager.
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows) = 0;

   /// Get the window that currently has the input focus or NULL.
   virtual PlatformWindow* getFocusedWindow() = 0;

   /// Get a window from a device ID.
   ///
   /// @return The window associated with the specified ID, or NULL if no
   ///         match was found.
   virtual PlatformWindow *getWindowById(WindowId id)=0;

   /// Get the first window in the window list
   ///
   /// @return The first window in the list, or NULL if no windows found
   virtual PlatformWindow *getFirstWindow()=0;

   virtual PlatformWindow* assignCanvas(GFXDevice* device, const GFXVideoMode &mode ,GuiCanvas* canvas)=0;

   /// Set the parent window
   ///
   /// This can be used to render in a child window.
   virtual void setParentWindow(void* newParent) = 0;

   /// Get the parent window
   virtual void* getParentWindow() = 0;

private:
   /// Process command line arguments from StandardMainLoop. This is done to
   /// allow web plugin functionality, where we are passed platform-specific
   /// information allowing us to set ourselves up in the web browser,
   /// to occur in a platform-neutral way.
   virtual void _processCmdLineArgs(const S32 argc, const char **argv)=0;
};

/// Global function to allocate a new platform window manager.
///
/// This returns an instance of the appropriate window manager for the current OS.
///
/// Depending on situation (for instance, if we are a web plugin) we may
/// need to get the window manager from somewhere else.
PlatformWindowManager *CreatePlatformWindowManager();

#endif