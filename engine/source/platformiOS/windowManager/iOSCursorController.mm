//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import "./iOSWindow.h"
#import "./iOSCursorController.h"

// <Mat> just some random number 50, we'll get a proper value later
#define IOS_DOUBLE_CLICK_TIME	( 50 * 60.0f * 1000.0f)


void iOSCursorController::setCursorPosition(S32 x, S32 y)
{
}

void iOSCursorController::getCursorPosition( Point2I &point )
{
}

void iOSCursorController::setCursorVisible(bool visible)
{
}

bool iOSCursorController::isCursorVisible()
{
   return false;
}


void iOSCursorController::setCursorShape(U32 cursorID)
{
}

void iOSCursorController::setCursorShape( const UTF8 *fileName, bool reload )
{
}

//------------------------------------------------------------------------------
U32 iOSCursorController::getDoubleClickTime()
{
    return IOS_DOUBLE_CLICK_TIME;
}

//------------------------------------------------------------------------------
S32 iOSCursorController::getDoubleClickWidth()
{
    // this is an arbitrary value.
    return 10;
}

//------------------------------------------------------------------------------
S32 iOSCursorController::getDoubleClickHeight()
{
    return getDoubleClickWidth();
}



