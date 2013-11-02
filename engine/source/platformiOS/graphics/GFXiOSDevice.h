//
//  GFXiOSDevice.h
//  Torque2D
//
//  Created by Paul Jan on 11/2/2013.
//

#ifndef Torque2D_GFXiOSDevice_h
#define Torque2D_GFXiOSDevice_h

#include "graphics/gfxDevice.h"

class GFXiOSDevice {
protected:
   EAGLContext* mContext;

public:
   GFXiOSDevice() { mContext = nullptr; };

   EAGLContext* getEAGLContext() const { return mContext; };
   // special immediate function for drawing CIImages
   virtual void drawImage( CIImage* image, CGRect inRect, CGRect fromRect) = 0;
   virtual void refreshCIContext(void) = 0;
};

static String _getRendererForDisplay(UIScreen* display)
{
   EAGLContext* ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
   if (ctx == nil)
      ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   
   // Save the current context, just in case
   EAGLContext* currCtx = [EAGLContext currentContext];
   [EAGLContext setCurrentContext: ctx];
   
   // get the renderer string
   String ret((const char*)glGetString(GL_RENDERER));
   
   // Restore our old context, release the context and pixel format.
   [EAGLContext setCurrentContext: currCtx];
   return ret;
}
#endif
