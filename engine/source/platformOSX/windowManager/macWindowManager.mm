//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "./macWindowManager.h"
#import "./macWindow.h"
#import "delegates/process.h"
#import "console/console.h"
#import "graphics/gfxDevice.h"
#import "game/version.h"
#import "platformOSX/platformOSX.h"

PlatformWindowManager* CreatePlatformWindowManager()
{
   return new MacWindowManager();
}

MacWindowManager::MacWindowManager() : mNotifyShutdownDelegate(this, &MacWindowManager::onShutdown), mIsShuttingDown(false)
{
   mWindowList.clear();
   Process::notifyShutdown(mNotifyShutdownDelegate);
}

MacWindowManager::~MacWindowManager()
{  
   for(U32 i = 0; i < mWindowList.size(); i++)
      delete mWindowList[i];
   mWindowList.clear();
}

S32 MacWindowManager::getWindowCount()
{
   // Get the number of PlatformWindow's in this manager
   return mWindowList.size();
}

void MacWindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
   windows.merge(mWindowList);
}

PlatformWindow * MacWindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return mWindowList[0];
      
   return NULL;
}


PlatformWindow* MacWindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return mWindowList[i];
   }

   return NULL;
}

PlatformWindow* MacWindowManager::getWindowById(WindowId zid)
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

MacWindow* MacWindowManager::getWindowByGLFW(GLFWwindow* window)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      MacWindow* w = dynamic_cast<MacWindow*>(mWindowList[i]);
      if( w->window == window)
         return w;
   }
   return NULL;
}


void MacWindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

PlatformWindow* MacWindowManager::assignCanvas(GFXDevice* device, const GFXVideoMode &mode, GuiCanvas* canvas)
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


PlatformWindow *MacWindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   MacWindow* window = new MacWindow(getNextId(), getVersionString(), mode.resolution);
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
         "MacWindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("MacWindowManager::createWindow - created a window with no device!");
   }

   return window;
}

void MacWindowManager::_addWindow(MacWindow* window)
{
#ifdef TORQUE_DEBUG
   // Make sure we aren't adding the window twice
   for(U32 i = 0; i < mWindowList.size(); i++)
      AssertFatal(window != mWindowList[i], "MacWindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.last();
   else
      window->mNextWindow = NULL;

   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &MacWindowManager::_onAppSignal);
}

void MacWindowManager::_removeWindow(MacWindow* window)
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
        osxPlatState * platState = [osxPlatState sharedPlatState];
        [platState shutDownTorque2D];
    }
    
        
   AssertFatal(false, avar("MacWindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void MacWindowManager::_onAppSignal(WindowId wnd, S32 event)
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

bool MacWindowManager::onShutdown()
{
   mIsShuttingDown = true;
   return true;
}

bool MacWindowManager::canWindowGainFocus(MacWindow* window)
{
   return !mIsShuttingDown;
}
