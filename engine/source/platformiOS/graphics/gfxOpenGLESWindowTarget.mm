//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/graphics/gfxOpenGLESDevice.h"
#include "platformiOS/graphics/gfxOpenGLESWindowTarget.h"
#include "platformiOS/graphics/gfxOpenGLESTextureObject.h"
#include "platformiOS/graphics/gfxOpenGLESUtils.h"

#import "platformiOS/windowManager/iOSWindow.h"
#include "platform/platformGL.h"

GFXOpenGLESWindowTarget::GFXOpenGLESWindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
   win->appEvent.notify(this, &GFXOpenGLESWindowTarget::_onAppSignal);
   mWindow = win;
   size = win->getBounds().extent;
}

void GFXOpenGLESWindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

bool GFXOpenGLESWindowTarget::present()
{
    GFX->updateStates();
    iOSWindow* win = dynamic_cast<iOSWindow*>(mWindow);
    [(GLKView *)[win->mGLKWindow view] display];

//    GFXOpenGLESDevice* device = (GFXOpenGLESDevice*)GFX;
//    [device->mContext presentRenderbuffer:GL_RENDERBUFFER];
    return true;
}

void GFXOpenGLESWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
//
//   // TODO: Investigate this further.
//   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
//   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
//   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
//   // masking the issue for the most common case.
   static_cast<GFXOpenGLESDevice*>(mDevice)->mVolatileVBs.clear();
}

void GFXOpenGLESWindowTarget::resolveTo(GFXTextureObject* obj)
{
}

void GFXOpenGLESWindowTarget::makeActive()
{
    iOSWindow* win = dynamic_cast<iOSWindow*>(mWindow);
    [(GLKView *)[win->mGLKWindow view] bindDrawable];
}

//void GFXOpenGLESWindowTarget::makeActive()
//{
//    // Get the shared iOS platform state
//    iOSPlatState * platState = [iOSPlatState sharedPlatState];
//    
//    [(GLKView *)[platState.viewController view] bindDrawable];
//}

