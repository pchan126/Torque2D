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
//   mTarget = target;
//   PlatformWindow* window = mTarget->getWindow();
//   RectI bounds = window->getBounds();
//
//	glGenTextures(1, &colorTexture);
//	glBindTexture(GL_TEXTURE_2D, colorTexture);
//	
//	// Set up filter and wrap modes for this texture object
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
//				 bounds.extent.x, bounds.extent.y, 0,
//				 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//   
//    // Create colour render buffer and allocate backing store
//   glGenRenderbuffers(1, &depthRenderbuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, bounds.extent.x, bounds.extent.y);
//
//   glGenFramebuffers(1, &FBOname);
//   glBindFramebuffer(GL_RENDERBUFFER, depthRenderbuffer);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
//
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE)
//    {
//        switch (status) {
//            case GL_FRAMEBUFFER_UNDEFINED:
//                Con::printf("default framebuffer does not exist.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.");
//                break;
//            case GL_FRAMEBUFFER_UNSUPPORTED:
//                Con::printf("combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS.");
//                break;
//                
//            default:
//                break;
//        }
//    }
}

_GFXOpenGLWindowTargetFBOImpl::~_GFXOpenGLWindowTargetFBOImpl()
{
//    glDeleteTextures(1, &colorTexture);
//    glDeleteFramebuffers(1, &FBOname);
//    glDeleteRenderbuffers(1, &depthRenderbuffer);
}


void _GFXOpenGLWindowTargetFBOImpl::makeActive()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBOname);
//    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
//
//    Con::printf("_GFXOpenGLWindowTargetFBOImpl::makeActive glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer); ");
//
//    
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE)
//    {
//        switch (status) {
//            case GL_FRAMEBUFFER_UNDEFINED:
//                Con::printf("default framebuffer does not exist.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.");
//                break;
//            case GL_FRAMEBUFFER_UNSUPPORTED:
//                Con::printf("combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE.");
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
//                Con::printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS.");
//                break;
//                
//            default:
//                break;
//        }
//    }

}

void _GFXOpenGLWindowTargetFBOImpl::finish()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//    glBindRenderbuffer(GL_RENDERBUFFER, 0);
//    
////    _GFXGLTargetDesc* color0 = mTarget->getTargetDesc(GFXTextureTarget::Color0);
////    if(!color0 || !(color0->hasMips()))
////        return;
//    
//    // Generate mips if necessary
//    // Assumes a 2D texture.
//    glActiveTexture(GL_TEXTURE0);
////    //   PRESERVE_2D_TEXTURE();
////    glBindTexture(GL_TEXTURE_2D, color0->getHandle());
////    glGenerateMipmap(GL_TEXTURE_2D);
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
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glClearColor(1.0, 0.0, 0.0, 1.0);
//        MacWindow* window = (MacWindow*)mWindow;
//        OSXTorqueView* tView = window->_torqueView;
//        [tView flushBuffer];
//        NSOpenGLContext* ctx = (NSOpenGLContext*)mContext;
//        NSOpenGLContext* tempctx = [NSOpenGLContext currentContext];
//        [tempctx flushBuffer];
//        [tempctx view];
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
//    mFramebuffer = 0;
//	viewRenderbuffer = 0;
//    depthRenderbuffer = 0;
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


