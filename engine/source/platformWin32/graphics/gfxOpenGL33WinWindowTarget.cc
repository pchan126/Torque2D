//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./gfxOpenGL33WinDevice.h"
#include "./gfxOpenGL33WinWindowTarget.h"
#include "./gfxOpenGL33WinTextureObject.h"
#include "./gfxOpenGL33WinUtils.h"

#include "platform/platformGL.h"
#include "platformWin32/windowsManager/win32Window.h"


GFXOpenGL33WinWindowTarget::GFXOpenGL33WinWindowTarget(PlatformWindow *window, GFXDevice *d)
      : GFXWindowTarget(window), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
{
    window->appEvent.notify(this, &GFXOpenGL33WinWindowTarget::_onAppSignal);
    size = window->getBounds().extent;
}

void GFXOpenGL33WinWindowTarget::resetMode()
{
    if(mWindow->getVideoMode().fullScreen != mWindow->isFullscreen())
    {
        _teardownCurrentMode();
        _setupNewMode();
    }
}

void GFXOpenGL33WinWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
    if(event != WindowHidden)
        return;
    
    // TODO: Investigate this further.
    // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
    // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
    // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply
    // masking the issue for the most common case.
    static_cast<GFXOpenGL33WinDevice*>(mDevice)->mVolatileVBs.clear();
}

bool GFXOpenGL33WinWindowTarget::present()
{
	HWND hwnd = GETHWND(getWindow());
	SwapBuffers(GetDC(hwnd));
    return true;
}


void GFXOpenGL33WinWindowTarget::resolveTo(GFXTextureObject* obj)
{
    AssertFatal(dynamic_cast<GFXOpenGL33WinTextureObject*>(obj), "GFXGLTextureTarget::resolveTo - Incorrect type of texture, expected a GFXGLTextureObject");
    GFXOpenGL33WinTextureObject* glTexture = static_cast<GFXOpenGL33WinTextureObject*>(obj);
    
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

void GFXOpenGL33WinWindowTarget::makeActive()
{
    //[(NSOpenGLContext*)mContext makeCurrentContext];
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GFXOpenGL33WinWindowTarget::_teardownCurrentMode()
{
    GFX->setActiveRenderTarget(this);
    static_cast<GFXOpenGL33WinDevice*>(mDevice)->zombify();
    //if(mFullscreenContext)
    //{
    //    [NSOpenGLContext clearCurrentContext];
    //    [(NSOpenGLContext*)mFullscreenContext clearDrawable];
    //}
}


void GFXOpenGL33WinWindowTarget::_setupNewMode()
{
    //if(mWindow->getVideoMode().fullScreen && !mFullscreenContext)
    //{
    //    // We have to create a fullscreen context.
    //    Vector<NSOpenGLPixelFormatAttribute> attributes = _beginPixelFormatAttributesForDisplay(static_cast<MacWindow*>(mWindow)->getDisplay());
    //    _addColorAlphaDepthStencilAttributes(attributes, 24, 8, 24, 8);
    //    _addFullscreenAttributes(attributes);
    //    _endAttributeList(attributes);

    //    NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes.address()];
    //    mFullscreenContext = [[[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil] retain];
    //    [fmt release];
    //    [(NSOpenGLContext*)mFullscreenContext setFullScreen];
    //    [(NSOpenGLContext*)mFullscreenContext makeCurrentContext];
    //    // Restore resources in new context
    //    static_cast<GFXOpenGL33WinDevice*>(mDevice)->resurrect();
    //    GFX->updateStates(true);
    //}
    //else if(!mWindow->getVideoMode().fullScreen && mFullscreenContext)
    //{
    //    [(NSOpenGLContext*)mFullscreenContext release];
    //    mFullscreenContext = NULL;
    //    [(NSOpenGLContext*)mContext makeCurrentContext];
    //    GFX->clear(GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI(0, 0, 0), 1.0f, 0);
    //    static_cast<GFXOpenGL33WinDevice*>(mDevice)->resurrect();
    //    GFX->updateStates(true);
    //}
}


