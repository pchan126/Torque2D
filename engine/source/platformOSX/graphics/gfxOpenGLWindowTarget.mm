//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLWindowTarget.h"
#include "./gfxOpenGLTextureObject.h"
#include "./gfxOpenGLUtils.h"

#include "platform/platformGL.h"
#import "platformOSX/windowManager/macWindow.h"
#import "osxGLUtils.h"


// Internal implementations
class _GFXOpenGLWindowTargetImpl
{
public:
    GFXOpenGLWindowTarget* mTarget;
    
    virtual ~_GFXOpenGLWindowTargetImpl() {}
    
    virtual void makeActive() = 0;
    virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXOpenGLWindowTargetFBOImpl : public _GFXOpenGLWindowTargetImpl
{
public:
    GLuint FBOname;
    GLuint colorTexture, depthRenderbuffer;
    
    _GFXOpenGLWindowTargetFBOImpl(GFXOpenGLWindowTarget* target);
    virtual ~_GFXOpenGLWindowTargetFBOImpl();
    
    virtual void makeActive();
    virtual void finish();
};


_GFXOpenGLWindowTargetFBOImpl::_GFXOpenGLWindowTargetFBOImpl(GFXOpenGLWindowTarget* target)
{
   // NSOpenGLView contains its own framebuffer which is active at 0
   FBOname = 0;
}

_GFXOpenGLWindowTargetFBOImpl::~_GFXOpenGLWindowTargetFBOImpl()
{
}


void _GFXOpenGLWindowTargetFBOImpl::makeActive()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBOname);
}

void _GFXOpenGLWindowTargetFBOImpl::finish()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



GFXOpenGLWindowTarget::GFXOpenGLWindowTarget(PlatformWindow *window, GFXDevice *d)
      : GFXWindowTarget(window), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
    window->appEvent.notify(this, &GFXOpenGLWindowTarget::_onAppSignal);

    _impl = new _GFXOpenGLWindowTargetFBOImpl(this);
    size = window->getBounds().extent;
}

void GFXOpenGLWindowTarget::resetMode()
{
    if(mWindow->getVideoMode().fullScreen != mWindow->isFullscreen())
    {
        _teardownCurrentMode();
        _setupNewMode();
    }
}

void GFXOpenGLWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
    if(event != WindowHidden)
        return;
    
    // TODO: Investigate this further.
    // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
    // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
    // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply
    // masking the issue for the most common case.
    static_cast<GFXOpenGLDevice*>(mDevice)->mVolatileVBs.clear();
}

bool GFXOpenGLWindowTarget::present()
{
    GFX->updateStates();
    if (mFullscreenContext)
    {
        [(NSOpenGLContext*)mFullscreenContext flushBuffer];
    }
    else
    {
       [mContext flushBuffer];
    }
    return true;
}


void GFXOpenGLWindowTarget::resolveTo(GFXTextureObject* obj)
{
    AssertFatal(dynamic_cast<GFXOpenGLTextureObject*>(obj), "GFXGLTextureTarget::resolveTo - Incorrect type of texture, expected a GFXGLTextureObject");
    GFXOpenGLTextureObject* glTexture = static_cast<GFXOpenGLTextureObject*>(obj);
    
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
    _impl->makeActive();
}


void GFXOpenGLWindowTarget::_teardownCurrentMode()
{
    GFX->setActiveRenderTarget(this);
    static_cast<GFXOpenGLDevice*>(mDevice)->zombify();
    if(mFullscreenContext)
    {
        [NSOpenGLContext clearCurrentContext];
        [(NSOpenGLContext*)mFullscreenContext clearDrawable];
    }
}


void GFXOpenGLWindowTarget::_setupNewMode()
{
    if(mWindow->getVideoMode().fullScreen && !mFullscreenContext)
    {
        // We have to create a fullscreen context.
        Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(static_cast<MacWindow*>(mWindow)->getDisplay());
        _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
        _addFullscreenAttributes(attributes);
        _endAttributeList(attributes);

        NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
        mFullscreenContext = [[[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil] retain];
        [fmt release];
        [(NSOpenGLContext*)mFullscreenContext setFullScreen];
        [(NSOpenGLContext*)mFullscreenContext makeCurrentContext];
        // Restore resources in new context
        static_cast<GFXOpenGLDevice*>(mDevice)->resurrect();
        GFX->updateStates(true);
    }
    else if(!mWindow->getVideoMode().fullScreen && mFullscreenContext)
    {
        [(NSOpenGLContext*)mFullscreenContext release];
        mFullscreenContext = NULL;
        [(NSOpenGLContext*)mContext makeCurrentContext];
        GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(0, 0, 0), 1.0f, 0);
        static_cast<GFXOpenGLDevice*>(mDevice)->resurrect();
        GFX->updateStates(true);
    }
}


