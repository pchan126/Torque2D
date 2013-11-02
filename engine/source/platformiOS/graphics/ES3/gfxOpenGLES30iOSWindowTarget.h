//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30iOSWindowTarget_H_
#define _GFXOpenGLES30iOSWindowTarget_H_

#include "graphics/gfxTarget.h"
#include "windowManager/platformWindow.h"

@class EAGLContext;

class GFXOpenGLES30iOSWindowTarget : public GFXWindowTarget
{
public:

   GFXOpenGLES30iOSWindowTarget(PlatformWindow *win, GFXDevice *d);
   const Point2I getSize();

   virtual GFXFormat getFormat();

   void makeActive();
   virtual bool present();
   virtual void resetMode();
   virtual void zombify() { }
   virtual void resurrect() { }
   
   virtual void resolveTo(GFXTextureObject* obj) {};
   
   void _onAppSignal(WindowId wnd, S32 event);
   
private:
    typedef GFXDevice Parent;
    
    friend class GFXOpenGLES30iOSTextureObject;
//    friend class GFXOpenGLESCubemap;
    friend class GFXOpenGLES30iOSWindowTarget;
    friend class GFXOpenGLES30iOSVertexBuffer;
    friend class GFXOpenGLES30iOSDevice;

   PlatformWindow *mWindow;
   friend class GFXOpenGLES30iOSDevice;

    GFXDevice* mDevice;
   EAGLContext* mContext;
   void* mFullscreenContext;
   void _teardownCurrentMode() {};
   void _setupNewMode() {};
};

#endif
