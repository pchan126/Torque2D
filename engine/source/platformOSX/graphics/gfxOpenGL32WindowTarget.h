//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGL32WindowTarget_H_
#define _GFXOpenGL32WindowTarget_H_

#include "graphics/gfxTarget.h"
#include "platformOSX/windowManager/macWindow.h"
#include "memory/autoPtr.h"

@class NSOpenGLContext;

class GFXOpenGL32WindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGL32WindowTarget(PlatformWindow *win, GFXDevice *d);
   const Point2I getSize() 
   {
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
   
   void _onAppSignal(WindowId wnd, S32 event);
   
   NSOpenGLContext* getContext();
   
private:
    typedef GFXWindowTarget Parent;
    
    friend class GFXOpenGL32TextureObject;
//    friend class GFXOpenGLCubemap;
    friend class GFXOpenGL32WindowTarget;
    friend class GFXOpenGLVertexBuffer;
    friend class GFXOpenGL32Device;

   Point2I size;
   GFXDevice* mDevice;
   MacWindow* mWindow;
   void _teardownCurrentMode();
   void _setupNewMode();
};

#endif