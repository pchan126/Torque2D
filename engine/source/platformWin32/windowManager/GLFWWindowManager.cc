//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GLFWWindowManager.h"
#include "./GLFWWindow.h"
#include "delegates/process.h"
#include "console/console.h"
#include "graphics/gfxDevice.h"
#include "game/version.h"

PlatformWindowManager* CreatePlatformWindowManager()
{
   return new GLFWWindowManager();
}

GLFWWindowManager::GLFWWindowManager() : mNotifyShutdownDelegate(this, &GLFWWindowManager::onShutdown), mIsShuttingDown(false)
{
   mWindowList.clear();
   Process::notifyShutdown(mNotifyShutdownDelegate);
}

GLFWWindowManager::~GLFWWindowManager()
{  
   for(U32 i = 0; i < mWindowList.size(); i++)
      delete mWindowList[i];
   mWindowList.clear();
}

S32 GLFWWindowManager::getWindowCount()
{
   // Get the number of PlatformWindow's in this manager
   return mWindowList.size();
}

void GLFWWindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
   windows.merge(mWindowList);
}

PlatformWindow * GLFWWindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return mWindowList[0];
      
   return NULL;
}


PlatformWindow* GLFWWindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return mWindowList[i];
   }

   return NULL;
}

PlatformWindow* GLFWWindowManager::getWindowById(WindowId zid)
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

GLFWWindow* GLFWWindowManager::getWindowByGLFW(GLFWwindow* window)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      GLFWWindow* w = dynamic_cast<GLFWWindow*>(mWindowList[i]);
      if( w->window == window)
         return w;
   }
   return NULL;
}


void GLFWWindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

PlatformWindow* GLFWWindowManager::assignCanvas(GFXDevice* device, const GFXVideoMode &mode, GuiCanvas* canvas)
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


PlatformWindow *GLFWWindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   GLFWWindow* window = new GLFWWindow(getNextId(), getVersionString(), mode.resolution);
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
         "GLFWWindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("GLFWWindowManager::createWindow - created a window with no device!");
   }

   return window;
}

void GLFWWindowManager::_addWindow(GLFWWindow* window)
{
#ifdef TORQUE_DEBUG
   // Make sure we aren't adding the window twice
   for(U32 i = 0; i < mWindowList.size(); i++)
      AssertFatal(window != mWindowList[i], "GLFWWindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.last();
   else
      window->mNextWindow = NULL;

   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &GLFWWindowManager::_onAppSignal);
}

void GLFWWindowManager::_removeWindow(GLFWWindow* window)
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
       Process::shutdown();
    }
    
        
   AssertFatal(false, avar("GLFWWindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void GLFWWindowManager::_onAppSignal(WindowId wnd, S32 event)
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

bool GLFWWindowManager::onShutdown()
{
   mIsShuttingDown = true;
   return true;
}

bool GLFWWindowManager::canWindowGainFocus(GLFWWindow* window)
{
   return !mIsShuttingDown;
}
