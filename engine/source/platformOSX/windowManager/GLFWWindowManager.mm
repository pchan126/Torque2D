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

void GLFWWindowManager::getWindows(Vector<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
    for (GLFWWindow* item: mWindowList)
    {
        PlatformWindow* temp = dynamic_cast<PlatformWindow*>(item);
        if (temp != nullptr)
            windows.push_back(temp);
    }
}

PlatformWindow * GLFWWindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return mWindowList[0];
      
   return nullptr;
}


PlatformWindow* GLFWWindowManager::getFocusedWindow()
{
   for (PlatformWindow* window:mWindowList)
   {
      if( window->isFocused() )
         return window;
   }

   return nullptr;
}

PlatformWindow* GLFWWindowManager::getWindowById(WindowId zid)
{
   // Find the window by its arbitrary WindowId.
   for(PlatformWindow* w:mWindowList)
   {
      if( w->getWindowId() == zid)
         return w;
   }
   return nullptr;
}

GLFWWindow* GLFWWindowManager::getWindowByGLFW(GLFWwindow* window)
{
   // Find the window by its arbirary WindowId.
   for(PlatformWindow* pw:mWindowList)
   {
      GLFWWindow* w = dynamic_cast<GLFWWindow*>(pw);
      if( w->window == window)
         return w;
   }
   return nullptr;
}


void GLFWWindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

PlatformWindow* GLFWWindowManager::assignCanvas(GFXDevice* device, const GFXVideoMode &mode, GuiCanvas* canvas)
{
   // Find the window by its arbitrary WindowId.
   for(PlatformWindow* w:mWindowList)
   {
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
   for(PlatformWindow* w:mWindowList)
      AssertFatal(window != w, "GLFWWindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.back();
   else
      window->mNextWindow = nullptr;

   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &GLFWWindowManager::_onAppSignal);
}

void GLFWWindowManager::_removeWindow(GLFWWindow* window)
{
   mWindowList.remove(window);

    if (mWindowList.empty())
       Process::shutdown();
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
