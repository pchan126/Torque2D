//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformWin32/platformWin32.h"
#include "./win32WindowMgr.h"
#include "./win32Window.h"
#include "graphics/gfxDevice.h"
#include "./winDispatch.h"
#include "delegates/process.h"
#include "string/unicode.h"
#include "game/version.h"

PlatformWindowManager* CreatePlatformWindowManager()
{
   return new Win32WindowManager();
}


Win32WindowManager::Win32WindowManager() : mNotifyShutdownDelegate(this, &Win32WindowManager::onShutdown), mIsShuttingDown(false)
{
   mWindowList.clear();
//   Process::notifyShutdown(mNotifyShutdownDelegate);
}

Win32WindowManager::~Win32WindowManager()
{  
   for(U32 i = 0; i < mWindowList.size(); i++)
      delete mWindowList[i];
   mWindowList.clear();
}

RectI Win32WindowManager::getPrimaryDesktopArea()
{
   // Get the area of the main desktop that isn't taken by the dock or menu bar.
//   return convertCGRectToRectI([[NSScreen mainScreen] visibleFrame]);
    return RectI( 0, 0, 1024, 768 );
}

void Win32WindowManager::getMonitorRegions(Vector<RectI> &regions)
{
	return;
}

S32 Win32WindowManager::getDesktopBitDepth()
{
   // get the current desktop bit depth
   // TODO: return -1 if an error occurred
	return -1;
}

Point2I Win32WindowManager::getDesktopResolution()
{
   // get the current desktop width/height
   // TODO: return Point2I(-1,-1) if an error occurred
	return Point2I(0,0);
}

S32 Win32WindowManager::getWindowCount()
{
   // Get the number of PlatformWindow's in this manager
   return mWindowList.size();
}

void Win32WindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
   windows.merge(mWindowList);
}

PlatformWindow * Win32WindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return (PlatformWindow*)mWindowList[0];
      
   return NULL;
}


PlatformWindow* Win32WindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return (PlatformWindow*)mWindowList[i];
   }

   return NULL;
}

PlatformWindow* Win32WindowManager::getWindowById(WindowId zid)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      PlatformWindow* w = (PlatformWindow*)mWindowList[i];
      if( w->getWindowId() == zid)
         return w;
   }
   return NULL;
}

Win32Window* Win32WindowManager::getWindowByGLFW(GLFWwindow* window)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      Win32Window* w = dynamic_cast<Win32Window*>(mWindowList[i]);
      if( w->window == window)
         return w;
   }
   return NULL;
}


void Win32WindowManager::lowerCurtain()
{
   //// fade all displays.
   //CGError err;
   //err = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval, &mFadeToken);
   //AssertWarn(!err, "Win32WindowManager::lowerCurtain() could not get a token");
   //if(err) return;
   //
   //err = CGDisplayFade(mFadeToken, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, true);
   //AssertWarn(!err, "Win32WindowManager::lowerCurtain() failed the fade");
   //if(err) return;
   
   // we do not release the token, because that will un-fade the screen!
   // the token will last for 15 sec, and then the screen will un-fade regardless.
   //CGReleaseDisplayFadeReservation(mFadeToken);
}

void Win32WindowManager::raiseCurtain()
{
   // release the fade on all displays
   //CGError err;
   //err = CGDisplayFade(mFadeToken, 0.3, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, false);
   //AssertWarn(!err, "Win32WindowManager::raiseCurtain() failed the fade");
   //
   //err = CGReleaseDisplayFadeReservation(mFadeToken);
   //AssertWarn(!err, "Win32WindowManager::raiseCurtain() failed releasing the token");
}


void Win32WindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

PlatformWindow* Win32WindowManager::assignCanvas(GFXDevice* device, const GFXVideoMode &mode, GuiCanvas* canvas)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      PlatformWindow* w = mWindowList[i];
      if (w->mBoundCanvas == NULL)
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


PlatformWindow *Win32WindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   Win32Window* window = new Win32Window(getNextId(), getVersionString(), mode.resolution);
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
         "Win32WindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("Win32WindowManager::createWindow - created a window with no device!");
   }

   return window;
}

void Win32WindowManager::_addWindow(Win32Window* window)
{
#ifdef TORQUE_DEBUG
   // Make sure we aren't adding the window twice
   for(U32 i = 0; i < mWindowList.size(); i++)
      AssertFatal(window != mWindowList[i], "Win32WindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.last();
   else
      window->mNextWindow = NULL;

   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &Win32WindowManager::_onAppSignal);
}

void Win32WindowManager::_removeWindow(Win32Window* window)
{
   for(WindowList::iterator i = mWindowList.begin(); i != mWindowList.end(); i++)
   {
      if(*i == window)
      {
         mWindowList.erase(i);
         return;
      }
   }
    
    if (mWindowList.size() == 0)
    {
		Platform::shutdown();
    }
    
        
   AssertFatal(false, avar("Win32WindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void Win32WindowManager::_onAppSignal(WindowId wnd, S32 event)
{
   switch (event) {
      case WindowHidden:
         for(U32 i = 0; i < mWindowList.size(); i++)
         {
            if(mWindowList[i]->getWindowId() == wnd)
               continue;
            
            mWindowList[i]->signalGainFocus();
         }
         break;
         
      default:
         break;
   }
}

bool Win32WindowManager::onShutdown()
{
   mIsShuttingDown = true;
   return true;
}

bool Win32WindowManager::canWindowGainFocus(Win32Window* window)
{
   return !mIsShuttingDown;
}
