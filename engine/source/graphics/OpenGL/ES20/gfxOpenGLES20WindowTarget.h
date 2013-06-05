//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20WindowTarget_H_
#define _GFXOpenGLES20WindowTarget_H_

#include "graphics/gfxTarget.h"
#include "windowManager/platformWindow.h"

class GFXOpenGLES20WindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGLES20WindowTarget(PlatformWindow *win, GFXDevice *d);
   const Point2I getSize();

   virtual GFXFormat getFormat()
   {
      // TODO: Fix me!
      return GFXFormatR8G8B8A8;
   }
   void makeActive();
   virtual bool present();
   virtual void resetMode();
   virtual void zombify() { }
   virtual void resurrect() { }
   
   virtual void resolveTo(GFXTextureObject* obj);
   
   void _onAppSignal(WindowId wnd, S32 event);
   
private:
    typedef GFXDevice Parent;
    
    friend class GFXOpenGLES20TextureObject;
    friend class GFXOpenGLES20Cubemap;
    friend class GFXOpenGLES20WindowTarget;
    friend class GFXOpenGLES20VertexBuffer;
    friend class GFXOpenGLES20Device;

   PlatformWindow *mWindow;
   friend class GFXOpenGLES20Device;

    GFXDevice* mDevice;
   void* mFullscreenContext;
   void _teardownCurrentMode();
   void _setupNewMode();

    // The pixel dimensions of the CAEAGLLayer.
    GLint framebufferWidth;
    GLint framebufferHeight;

    // The OpenGL ES names for the framebuffer and renderbuffers used to render
    // to this view.
    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
};

#endif