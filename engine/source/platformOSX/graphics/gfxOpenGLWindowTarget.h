//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLWindowTarget_H_
#define _GFXOpenGLWindowTarget_H_

#include "graphics/gfxTarget.h"

class GFXOpenGLWindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGLWindowTarget(void *win, GFXDevice *d);
   const Point2I getSize() 
   {
       return size;//
//       return mWindow->getClientExtent();
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
    
    friend class GFXOpenGLTextureObject;
//    friend class GFXOpenGLCubemap;
    friend class GFXOpenGLWindowTarget;
    friend class GFXOpenGLPrimitiveBuffer;
    friend class GFXOpenGLVertexBuffer;
    friend class GFXOpenGLDevice;

   void *mWindow;
   friend class GFXOpenGLDevice;
   Point2I size;
   GFXDevice* mDevice;
   void* mContext;
   void* mFullscreenContext;
   void _teardownCurrentMode();
   void _setupNewMode();
    
    GLuint mFramebuffer, viewRenderbuffer, depthRenderbuffer;

};

#endif