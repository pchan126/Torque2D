//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _iOSCursorController_H_
#define _iOSCursorController_H_

#include "windowManager/platformCursorController.h"

class iOSCursorController : public PlatformCursorController
{
public:
   iOSCursorController(iOSWindow* owner)
      : PlatformCursorController( ( PlatformWindow* ) owner )
   {
      pushCursor(PlatformCursorController::curArrow);
   }
   
   virtual void setCursorPosition(S32 x, S32 y);
   virtual void getCursorPosition(Point2I &point);
   virtual void setCursorVisible(bool visible);
   virtual bool isCursorVisible();
   
   virtual void setCursorShape(U32 cursorID);
   virtual void setCursorShape( const UTF8 *fileName, bool reload );
   
   virtual U32 getDoubleClickTime();
   virtual S32 getDoubleClickWidth();
   virtual S32 getDoubleClickHeight();
};

#endif
