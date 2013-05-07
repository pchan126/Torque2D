//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import "./iOSWindowManager.h"
#import "./iOSWindow.h"
#import "console/console.h"
#import "graphics/gfxDevice.h"
#import "game/version.h"
#import <UIKit/UIKit.h>
#import "platformiOS/T2DAppDelegate.h"

PlatformWindowManager* CreatePlatformWindowManager()
{
   return new iOSWindowManager();
}

static inline RectI convertCGRectToRectI(CGRect r)
{
   return RectI(r.origin.x, r.origin.y, r.size.width, r.size.height);
}

iOSWindowManager::iOSWindowManager() : mNotifyShutdownDelegate(this, &iOSWindowManager::onShutdown),
                                        mIsShuttingDown(false)
{
   extWindow = nil;
   extScreen = nil;
   extViewController = nil;

    mWindowList.clear();
    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    appDelegate.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    viewController = [[GLKViewController alloc] initWithNibName:nil bundle:nil];
    viewController.delegate = appDelegate;
    viewController.preferredFramesPerSecond = 60;
    viewController.paused = NO;
    appDelegate.mainController = viewController;

    appDelegate.window.rootViewController = viewController;
    appDelegate.window.backgroundColor = [UIColor blackColor];
}

iOSWindowManager::~iOSWindowManager()
{  
   for(U32 i = 0; i < mWindowList.size(); i++)
      delete mWindowList[i];
   mWindowList.clear();
    extWindow = nil;
    extScreen = nil;
}

RectI iOSWindowManager::getPrimaryDesktopArea()
{
   // Get the area of the main desktop that isn't taken by the dock or menu bar.
   return convertCGRectToRectI([[UIScreen mainScreen] bounds]);
}

void iOSWindowManager::getMonitorRegions(Vector<RectI> &regions)
{
   // Populate a vector with all monitors and their extents in window space.
   NSArray *screenList = [UIScreen screens];
   for(U32 i = 0; i < [screenList count]; i++)
   {
      CGRect screenBounds = [screenList[i] frame];
      regions.push_back(convertCGRectToRectI(screenBounds));
   }
}

S32 iOSWindowManager::getDesktopBitDepth()
{
   // get the current desktop bit depth
   // TODO: return -1 if an error occurred
//   return NSBitsPerPixelFromDepth([[UIScreen mainScreen] depth]);
    return 32;
}

Point2I iOSWindowManager::getDesktopResolution()
{
   // get the current desktop width/height
   // TODO: return Point2I(-1,-1) if an error occurred
   CGRect desktopBounds = [[UIScreen mainScreen] bounds];
   return Point2I((U32)desktopBounds.size.width, (U32)desktopBounds.size.height);
}

S32 iOSWindowManager::getWindowCount()
{
   // Get the number of PlatformWindow's in this manager
   return mWindowList.size();
}

void iOSWindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
   windows.merge(mWindowList);
}

PlatformWindow * iOSWindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return mWindowList[0];
      
   return NULL;
}


PlatformWindow* iOSWindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return mWindowList[i];
   }

   return NULL;
}

PlatformWindow* iOSWindowManager::getWindowById(WindowId zid)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      PlatformWindow* w = mWindowList[i];
      if( w->getWindowId() == zid)
         return w;
   }
   return NULL;
}

void iOSWindowManager::lowerCurtain()
{
}

void iOSWindowManager::raiseCurtain()
{
}


void iOSWindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

void iOSWindowManager::updateWindows()
{
    NSArray	*screens = [UIScreen screens];
    NSUInteger screenCount = [screens count];

    if (screenCount > 1)
    {
        extScreen = screens[1]; //index 0 is your iPhone/iPad
        NSArray	*availableModes = [extScreen availableModes];

      // Select the highest resolution in this sample
      NSInteger selectedRow = [availableModes count] - 1;
      extScreen.currentMode = [availableModes objectAtIndex:selectedRow];

      // Set a proper overscanCompensation mode
      extScreen.overscanCompensation = UIScreenOverscanCompensationInsetApplicationFrame;

      if (extWindow == nil) {
         extWindow = [[UIWindow alloc] initWithFrame:[extScreen bounds]];
      }
        extWindow.screen = extScreen;
       
      T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
                
        UIScreen *phoneScreen = screens[0];
        GLKViewController *newController = [[GLKViewController alloc] initWithNibName:nil bundle:nil];
        newController.preferredFramesPerSecond = 30;
        appDelegate.window.rootViewController = newController;

        extWindow.rootViewController = appDelegate.mainController;
        [extWindow makeKeyAndVisible];
    }
    else
    {
        
    }
}


PlatformWindow *iOSWindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   iOSWindow* window = new iOSWindow(getNextId(), getVersionString(), mode.resolution);
   _addWindow(window);
   
   // Set the video mode on the window
   window->setVideoMode(mode);

   // Make sure our window is shown and drawn to.
   window->show();

   // Bind the window to the specified device.
   if(device)
   {
      window->mDevice = device;
      window->mTarget = device->allocWindowTarget(window);
      AssertISV(window->mTarget, 
         "iOSWindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("iOSWindowManager::createWindow - created a window with no device!");
   }

   return window;
}


void iOSWindowManager::_addWindow(iOSWindow* window)
{
#ifdef TORQUE_DEBUG
   // Make sure we aren't adding the window twice
   for(U32 i = 0; i < mWindowList.size(); i++)
      AssertFatal(window != mWindowList[i], "iOSWindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.last();
   else
      window->mNextWindow = NULL;

   viewController.view = window->view;

    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    
   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &iOSWindowManager::_onAppSignal);
    
   if (mWindowList.size() > 0)
       [appDelegate.window makeKeyAndVisible];
}

void iOSWindowManager::_removeWindow(iOSWindow* window)
{
   for(WindowList::iterator i = mWindowList.begin(); i != mWindowList.end(); i++)
   {
      if(*i == window)
      {
         mWindowList.erase(i);
         return;
      }
   }
   AssertFatal(false, avar("iOSWindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void iOSWindowManager::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
   
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      if(mWindowList[i]->getWindowId() == wnd)
         continue;
      
      mWindowList[i]->signalGainFocus();
   }
}

bool iOSWindowManager::onShutdown()
{
   mIsShuttingDown = true;
   return true;
}

bool iOSWindowManager::canWindowGainFocus(iOSWindow* window)
{
   return !mIsShuttingDown;
}
