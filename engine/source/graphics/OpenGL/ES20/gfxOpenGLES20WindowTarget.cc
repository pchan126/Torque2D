//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GFXOpenGLES20Device.h"
#include "./GFXOpenGLES20WindowTarget.h"
#include "./GFXOpenGLES20TextureObject.h"
#include "./GFXOpenGLES20Utils.h"

#include "platform/platformGL.h"

GFXOpenGLES20WindowTarget::GFXOpenGLES20WindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
   win->appEvent.notify(this, &GFXOpenGLES20WindowTarget::_onAppSignal);
   mWindow = win;
    resetMode();
}

void GFXOpenGLES20WindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

//const Point2I GFXOpenGLES20WindowTarget::getSize()
//{
//   iOSWindow *win = (iOSWindow*)mWindow;
//   GLKView *view = (GLKView*)win->view;
//   return Point2I(view.drawableWidth, view.drawableHeight);
//}

bool GFXOpenGLES20WindowTarget::present()
{
    // iOS GLKView class automatically presents renderbuffer
    return true;
}

void GFXOpenGLES20WindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
//
//   // TODO: Investigate this further.
//   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
//   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
//   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
//   // masking the issue for the most common case.
   static_cast<GFXOpenGLES20Device*>(mDevice)->mVolatileVBs.clear();
}

void GFXOpenGLES20WindowTarget::resolveTo(GFXTextureObject* obj)
{
}

void GFXOpenGLES20WindowTarget::makeActive()
{
}

void GFXOpenGLES20WindowTarget::_teardownCurrentMode()
{

}


void GFXOpenGLES20WindowTarget::_setupNewMode()
{
}

