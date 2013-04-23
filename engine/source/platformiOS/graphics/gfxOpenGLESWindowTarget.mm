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
   resetMode();
}

void GFXOpenGLESWindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

bool GFXOpenGLESWindowTarget::present()
{
    GFX->updateStates();

    GFXOpenGLESDevice* device = (GFXOpenGLESDevice*)GFX;
//    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
//    if (glIsRenderbuffer(colorRenderbuffer))
//        Con::printf("is render buffer");
//    GLint temp;
//    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &temp);
    [device->mContext presentRenderbuffer:GL_RENDERBUFFER];
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
//    GFXOpenGLESDevice* device = (GFXOpenGLESDevice*)GFX;
//    [EAGLContext setCurrentContext:device->getEAGLContext()];
//
//    if (glIsFramebuffer(defaultFramebuffer))
//        Con::printf("is frame buffer");
    
//    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
}

void GFXOpenGLESWindowTarget::_teardownCurrentMode()
{
    iOSWindow* win = dynamic_cast<iOSWindow*>(mWindow);
    GLKView* view = (GLKView *)[win->mGLKWindow view];

    if (view.context) {
        [EAGLContext setCurrentContext:view.context];
        
        if (defaultFramebuffer) {
            glDeleteFramebuffers(1, &defaultFramebuffer);
            defaultFramebuffer = 0;
        }
        
        if (colorRenderbuffer) {
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            colorRenderbuffer = 0;
        }
        
        if (depthRenderbuffer) {
            glDeleteRenderbuffers(1, &depthRenderbuffer);
            depthRenderbuffer = 0;
        }
    }
}


void GFXOpenGLESWindowTarget::_setupNewMode()
{
    iOSWindow* win = dynamic_cast<iOSWindow*>(mWindow);
    GLKView* view = (GLKView *)[win->mGLKWindow view];

    if (view.context && !defaultFramebuffer) {
        [EAGLContext setCurrentContext:view.context];
        
        // Create default framebuffer object
        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        // Create colour render buffer and allocate backing store
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        // Allocate the renderbuffer's storage (shared with the drawable object)
        CAEAGLLayer * drawLayer = (CAEAGLLayer *)view.layer;
        NSLog(@"width: %f height %f", drawLayer.bounds.size.width, drawLayer.bounds.size.height);
        [view.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:drawLayer];
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);
        
        // Create the depth render buffer and allocate storage
        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
        
        // Attach colour and depth render buffers to the frame buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        
        // Leave the colour render buffer bound so future rendering operations will act on it
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }
    }
}

