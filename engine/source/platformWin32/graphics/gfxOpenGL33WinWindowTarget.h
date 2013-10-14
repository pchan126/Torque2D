//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGL33WinWindowTarget_H_
#define _GFXOpenGL33WinWindowTarget_H_

#include "graphics/gfxTarget.h"
#include "platformWin32/windowManager/GLFWWindow.h"
#include <memory>

class GFXOpenGL33WinWindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGL33WinWindowTarget(PlatformWindow *win, GFXDevice *d);
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

	HGLRC getContext();
   
private:
    typedef GFXWindowTarget Parent;
    
    friend class GFXOpenGL33WinTextureObject;
//    friend class GFXOpenGLCubemap;
    friend class GFXOpenGL33WinWindowTarget;
    friend class GFXOpenGLVertexBuffer;
    friend class GFXOpenGL33WinDevice;

   Point2I size;
   GFXDevice* mDevice;
   GLFWWindow* mWindow;

   void _teardownCurrentMode();
   void _setupNewMode();
};

#endif