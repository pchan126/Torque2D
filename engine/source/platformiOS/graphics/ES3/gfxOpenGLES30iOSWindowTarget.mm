//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGLES30iOSDevice.h"
#include "./GFXOpenGLES30iOSWindowTarget.h"
#include "./gfxOpenGLES30iOSTextureObject.h"
#include "./gfxOpenGLES30iOSUtils.h"

#import "platformiOS/windowManager/iOSWindow.h"
#include "platform/platformGL.h"
#import <GLKit/GLKit.h>

GFXOpenGLES30iOSWindowTarget::GFXOpenGLES30iOSWindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(nullptr), mFullscreenContext(nullptr)
{
   win->appEvent.notify(this, &GFXOpenGLES30iOSWindowTarget::_onAppSignal);
   mWindow = win;
    resetMode();
}

void GFXOpenGLES30iOSWindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

const Point2I GFXOpenGLES30iOSWindowTarget::getSize()
{
   return mWindow->getBounds().extent;
}

bool GFXOpenGLES30iOSWindowTarget::present()
{
    // iOS GLKView class automatically presents renderbuffer
    return true;
}

void GFXOpenGLES30iOSWindowTarget::makeActive()
{
   iOSWindow *win = (iOSWindow*)mWindow;
   GLKView *view = win->view;
   [view bindDrawable];
}


void GFXOpenGLES30iOSWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
//
//   // TODO: Investigate this further.
//   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
//   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
//   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
//   // masking the issue for the most common case.
   static_cast<GFXOpenGLES30iOSDevice*>(mDevice)->mVolatileVBs.clear();
}

GFXFormat GFXOpenGLES30iOSWindowTarget::getFormat() {
    iOSWindow *win = (iOSWindow*)mWindow;
    GLKView *view = win->view;

    if (view.drawableColorFormat == GLKViewDrawableColorFormatRGB565)
        return GFXFormatR5G6B5;

    return GFXFormatR8G8B8A8;
}
