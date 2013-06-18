//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GLFWCursorController_H_
#define _GLFWCursorController_H_

#include "windowManager/platformCursorController.h"

class GLFWCursorController : public PlatformCursorController
{
public:
   GLFWCursorController(GLFWWindow* owner)
      : PlatformCursorController( ( PlatformWindow* ) owner )
   {
      pushCursor(PlatformCursorController::curArrow);
   }
   
   virtual void setCursorPosition(S32 x, S32 y);
   virtual void getCursorPosition(Point2I &point);
   virtual void setCursorVisible(bool visible) {};
   virtual bool isCursorVisible() { return false; };
   
   virtual void setCursorShape(U32 cursorID) {};
   virtual void setCursorShape( const UTF8 *fileName, bool reload ) {};
   
   virtual U32 getDoubleClickTime();
   virtual S32 getDoubleClickWidth();
   virtual S32 getDoubleClickHeight();
};

#endif
