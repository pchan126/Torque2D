//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESWindowTarget_H_
#define _GFXOpenGLESWindowTarget_H_

#include "graphics/gfxTarget.h"
#include "windowManager/platformWindow.h"

@class EAGLContext;

class GFXOpenGLESWindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGLESWindowTarget(PlatformWindow *win, GFXDevice *d);
   const Point2I getSize() 
   {
       return size;
       return mWindow->getClientExtent();
   }
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
    
    friend class GFXOpenGLESTextureObject;
//    friend class GFXOpenGLESCubemap;
    friend class GFXOpenGLESWindowTarget;
    friend class GFXOpenGLESVertexBuffer;
    friend class GFXOpenGLESDevice;

   PlatformWindow *mWindow;
   friend class GFXOpenGLESDevice;
   Point2I size;
   GFXDevice* mDevice;
   EAGLContext* mContext;
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