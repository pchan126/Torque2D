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
#import "platformiOS/graphics/GFXiOSDevice.h"
#import "platformiOS/T2DViewController.h"
#include "gfxInit.h"

std::unique_ptr<PlatformWindowManager> CreatePlatformWindowManager()
{
   return std::unique_ptr<PlatformWindowManager>(new iOSWindowManager());
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
   extController = nil;

    mWindowList.clear();
    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    appDelegate.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    viewController = [[T2DViewController alloc] initWithNibName:nil bundle:nil];
    viewController.delegate = appDelegate;
    viewController.preferredFramesPerSecond = 30;
    viewController.paused = NO;
    appDelegate.mainController = viewController;

    appDelegate.window.rootViewController = viewController;
    appDelegate.window.backgroundColor = [UIColor blackColor];
   
   updateWindows();
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
   return (S32)mWindowList.size();
}

void iOSWindowManager::getWindows(Vector<PlatformWindow*> &windows)
{
    for (iOSWindow* item: mWindowList)
    {
        PlatformWindow* temp = dynamic_cast<PlatformWindow*>(item);
        if (temp != nullptr)
            windows.push_back(temp);
    }
}

PlatformWindow * iOSWindowManager::getFirstWindow()
{
   if ( !mWindowList.empty() )
      return mWindowList[0];
      
   return nullptr;
}


PlatformWindow* iOSWindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return mWindowList[i];
   }

   return nullptr;
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
   return nullptr;
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
//    NSUInteger screenCount = [screens count];

    // Create a device.
    GFXAdapter *a = GFXInit::getBestAdapterChoice();

    GFXDevice *newDevice = GFX;
    if(newDevice == nullptr)
        newDevice = GFXInit::createDevice(a);

    GFXiOSDevice *device = dynamic_cast<GFXiOSDevice*>(GFX);
    EAGLContext *context = device->getEAGLContext();

    if ([UIScreen screens].count > 1)
    {
        [[UIApplication sharedApplication] setStatusBarHidden:NO];

      extScreen = screens[1]; //index 0 is your iPhone/iPad
      extScreen.currentMode = [extScreen preferredMode];

      CGRect rect = (CGRect){.size = [extScreen preferredMode].size};

//      // Set a proper overscanCompensation mode
//      extScreen.overscanCompensation = UIScreenOverscanCompensationInsetApplicationFrame;

      if (extWindow == nil) {
         extWindow = [[UIWindow alloc] initWithFrame:[extScreen bounds]];
      }
      extWindow.screen = extScreen;
      [extWindow makeKeyAndVisible];
      extWindow.frame = rect;

       T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
       GLKView* baseView = [[GLKView alloc] initWithFrame:rect];
        [extWindow addSubview:baseView];
        [baseView setContext:context];
       baseView.delegate = appDelegate;
       baseView.drawableDepthFormat = GLKViewDrawableDepthFormat16;

       extController = [[T2DViewController alloc] initWithNibName:nil bundle:nil];
       extController.delegate = appDelegate;
       extController.preferredFramesPerSecond = 30;
       extController.paused = NO;
       appDelegate.extController = extController;

       // Do script callback.
        Con::executef( 1, "onCreateIOSExternalWindow" );

        GLKView* view = mWindowList[0]->view;
        [view.window makeKeyAndVisible];
    }
    else
    {
        extWindow = nil;
        extController = nil;
//       if (getWindowCount() < 1)
//          createWindow(GFX, const GFXVideoMode &mode)
    }
}


PlatformWindow* iOSWindowManager::assignCanvas(GFXDevice* device, const GFXVideoMode &mode, GuiCanvas* canvas)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      PlatformWindow* w = mWindowList[i];
      if (w->mBoundCanvas == nullptr)
      {
         w->setVideoMode(mode);
         w->bindCanvas(canvas);
         return w;
      }
   }
   PlatformWindow* w = createWindow(device, mode);
   w->bindCanvas(canvas);
   return w;
}



PlatformWindow *iOSWindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   if (mWindowList.size() >= [[UIScreen screens]count])    // only one window per available screen
       return nullptr;
    
   iOSWindow* window = new iOSWindow((U32)mWindowList.size());
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
   if ( !mWindowList.empty() )
      window->mNextWindow = mWindowList.back();
   else
      window->mNextWindow = nullptr;

   viewController.view = window->view;

    T2DAppDelegate *appDelegate = (T2DAppDelegate*)[[UIApplication sharedApplication] delegate];
    
   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &iOSWindowManager::_onAppSignal);
    
   if ( !mWindowList.empty() )
       [appDelegate.window makeKeyAndVisible];
}

void iOSWindowManager::_removeWindow(iOSWindow* window)
{
   for(auto i : mWindowList)
   {
      if(i == window)
      {
         mWindowList.remove(i);
         return;
      }
   }
   AssertFatal(false, avar("iOSWindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void iOSWindowManager::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
   
   for(iOSWindow* i:mWindowList)
   {
      if(i->getWindowId() == wnd)
         continue;
      
      i->signalGainFocus();
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
