//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLWindowTarget.h"
#include "./gfxOpenGLTextureObject.h"
#include "./gfxOpenGLUtils.h"

#include "platform/platformGL.h"


GFXOpenGLWindowTarget::GFXOpenGLWindowTarget(void *window, GFXDevice *d)
      : GFXWindowTarget(window), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
   mWindow = window;
   OSXTorqueView *torqueView = (OSXTorqueView*)(window);
   mContext = (CGLContextObj)[[torqueView openGLContext] CGLContextObj];

    NSRect rect = [torqueView bounds];
    size = Point2I(rect.size.width, rect.size.height);
    mFramebuffer = 0;
    viewRenderbuffer = 0;
    depthRenderbuffer = 0;
//   GFXVideoMode mode = win->getVideoMode();
//   size = mode.resolution;
//
//    _teardownCurrentMode();
//    _setupNewMode();
////    platState.viewController.windowTarget = this;
//    win->appEvent.notify(this, &GFXOpenGLWindowTarget::_onAppSignal);
}

void GFXOpenGLWindowTarget::resetMode()
{
    _teardownCurrentMode();
    _setupNewMode();
}

void GFXOpenGLWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
//    if(event != WindowHidden)
//        return;
    
    // TODO: Investigate this further.
    // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
    // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
    // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply
    // masking the issue for the most common case.
//    static_cast<GFXOpenGLDevice*>(mDevice)->mVolatileVBs.clear();
}

bool GFXOpenGLWindowTarget::present()
{
    GFX->updateStates();
    if (mFullscreenContext)
        [(NSOpenGLContext*)mFullscreenContext flushBuffer];
    else
        [(NSOpenGLContext*)mContext flushBuffer];
    return true;
}

//void GFXOpenGLWindowTarget::_onAppSignal(WindowId wnd, S32 event)
//{
//   if(event != WindowHidden)
//      return;
//      
//   // TODO: Investigate this further.
//   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
//   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
//   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
//   // masking the issue for the most common case.
//   static_cast<GFXOpenGLDevice*>(mDevice)->mVolatileVBs.clear();
//}

void GFXOpenGLWindowTarget::resolveTo(GFXTextureObject* obj)
{
    AssertFatal(dynamic_cast<GFXOpenGLTextureObject*>(obj), "GFXGLTextureTarget::resolveTo - Incorrect type of texture, expected a GFXGLTextureObject");
    GFXOpenGLTextureObject* glTexture = static_cast<GFXOpenGLTextureObject*>(obj);
    
//    PRESERVE_FRAMEBUFFER();
    
    GLuint dest;
    
    glGenFramebuffers(1, &dest);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture->getHandle(), 0);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    glBlitFramebuffer(0, 0, getSize().x, getSize().y,
                         0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    glDeleteFramebuffers(1, &dest);
}

void GFXOpenGLWindowTarget::makeActive()
{
    GL_CHECK();

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer));
//	glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, viewRenderbuffer);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
//    if( !mFullscreenContext && mWindow->getVideoMode().fullScreen )
//    {
//        static_cast< GFXOpenGLDevice* >( mDevice )->zombify();
//        _setupNewMode();
//    }

//    if (mFullscreenContext)
//        [(NSOpenGLContext*)mFullscreenContext makeCurrentContext];
//    else
//        [(NSOpenGLContext*)mContext makeCurrentContext];
}


void GFXOpenGLWindowTarget::_teardownCurrentMode()
{
    mFramebuffer = 0;
	viewRenderbuffer = 0;
    depthRenderbuffer = 0;
}


void GFXOpenGLWindowTarget::_setupNewMode()
{
    mFramebuffer = 0;
	viewRenderbuffer = 0;
    depthRenderbuffer = 0;
//    if(mWindow->getVideoMode().fullScreen && !mFullscreenContext)
//    {
//        // We have to create a fullscreen context.
//        Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(static_cast<OSXWindow*>(mWindow)->getDisplay());
//        _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
//        _addFullscreenAttributes(attributes);
//        _endAttributeList(attributes);
//
//        NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
//        mFullscreenContext = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
//        [fmt release];
//        [(NSOpenGLContext*)mFullscreenContext setFullScreen];
//        [(NSOpenGLContext*)mFullscreenContext makeCurrentContext];
//        // Restore resources in new context
//        static_cast<GFXOpenGLDevice*>(mDevice)->resurrect();
//        GFX->updateStates(true);
//    }
//    else if(!mWindow->getVideoMode().fullScreen && mFullscreenContext)
//    {
//        [(NSOpenGLContext*)mFullscreenContext release];
//        mFullscreenContext = NULL;
//        [(NSOpenGLContext*)mContext makeCurrentContext];
//        GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(0, 0, 0), 1.0f, 0);
//        static_cast<GFXOpenGLDevice*>(mDevice)->resurrect();
//        GFX->updateStates(true);
//    }
}


