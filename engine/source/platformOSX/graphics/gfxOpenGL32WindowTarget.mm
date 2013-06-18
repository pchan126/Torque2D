//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGL32Device.h"
#include "./gfxOpenGL32WindowTarget.h"
#include "./gfxOpenGL32TextureObject.h"
#include "./gfxOpenGL32Utils.h"

#include "platform/platformGL.h"


GFXOpenGL32WindowTarget::GFXOpenGL32WindowTarget(PlatformWindow *window, GFXDevice *d)
      : GFXWindowTarget(window), mDevice(d)
{
    mWindow = dynamic_cast<MacWindow*>(window);
    window->appEvent.notify(this, &GFXOpenGL32WindowTarget::_onAppSignal);
    size = window->getBounds().extent;
}

void GFXOpenGL32WindowTarget::resetMode()
{

}

void GFXOpenGL32WindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
    if(event != WindowHidden)
        return;
    
    // TODO: Investigate this further.
    // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
    // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
    // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply
    // masking the issue for the most common case.
    static_cast<GFXOpenGL32Device*>(mDevice)->mVolatileVBs.clear();
}

bool GFXOpenGL32WindowTarget::present()
{
    GFX->updateStates();
    mWindow->swapBuffers();
    return true;
}


void GFXOpenGL32WindowTarget::makeActive()
{
   mWindow->makeContextCurrent();
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

NSOpenGLContext* GFXOpenGL32WindowTarget::getContext()
{
   return mWindow->getContext();
}

void GFXOpenGL32WindowTarget::_teardownCurrentMode()
{
    GFX->setActiveRenderTarget(this);
    static_cast<GFXOpenGL32Device*>(mDevice)->zombify();
}


void GFXOpenGL32WindowTarget::_setupNewMode()
{

}


