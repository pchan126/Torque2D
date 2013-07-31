//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/graphics/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/GFXOpenGLES20iOSWindowTarget.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureObject.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSUtils.h"

#import "platformiOS/windowManager/iOSWindow.h"
#include "platform/platformGL.h"
#import <GLKit/GLKit.h>

GFXOpenGLES20iOSWindowTarget::GFXOpenGLES20iOSWindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
   win->appEvent.notify(this, &GFXOpenGLES20iOSWindowTarget::_onAppSignal);
   mWindow = win;
    resetMode();
}

void GFXOpenGLES20iOSWindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

const Point2I GFXOpenGLES20iOSWindowTarget::getSize()
{
   iOSWindow *win = (iOSWindow*)mWindow;
   GLKView *view = (GLKView*)win->view;
   return Point2I(view.drawableWidth, view.drawableHeight);
}

bool GFXOpenGLES20iOSWindowTarget::present()
{
    // iOS GLKView class automatically presents renderbuffer
    return true;
}

void GFXOpenGLES20iOSWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
//
//   // TODO: Investigate this further.
//   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
//   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
//   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
//   // masking the issue for the most common case.
   static_cast<GFXOpenGLES20iOSDevice*>(mDevice)->mVolatileVBs.clear();
}
