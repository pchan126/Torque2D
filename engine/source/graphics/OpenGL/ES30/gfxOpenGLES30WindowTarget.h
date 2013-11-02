//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30WindowTarget_H_
#define _GFXOpenGLES30WindowTarget_H_

#include "graphics/gfxTarget.h"
#include "windowManager/platformWindow.h"

class GFXOpenGLES30WindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGLES30WindowTarget(PlatformWindow *win, GFXDevice *d);
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
    
    friend class GFXOpenGLES30TextureObject;
    friend class GFXOpenGLES30Cubemap;
    friend class GFXOpenGLES30WindowTarget;
    friend class GFXOpenGLES30VertexBuffer;
    friend class GFXOpenGLES30Device;

   PlatformWindow *mWindow;
   friend class GFXOpenGLES30Device;

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