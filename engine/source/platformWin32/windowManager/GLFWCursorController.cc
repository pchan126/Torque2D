//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GLFWWindow.h"
#include "./GLFWCursorController.h"

void GLFWCursorController::setCursorPosition(S32 x, S32 y)
{
   GLFWWindow* glfwWindow = static_cast<GLFWWindow*>(mOwner);
   glfwWindow->setCursorPosition(Point2D(x, y));
}

void GLFWCursorController::getCursorPosition( Point2I &point )
{
   GLFWWindow* glfwWindow = static_cast<GLFWWindow*>(mOwner);
   glfwWindow->getCursorPosition(point);
}

////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void GLFWCursorController::pushCursor(S32 cursorID)
//{
//}
//
////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void GLFWCursorController::popCursor()
//{
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void GLFWCursorController::refreshCursor()
//{
//}

//------------------------------------------------------------------------------
U32 GLFWCursorController::getDoubleClickTime()
{
   return GetDoubleClickTime();
}

//------------------------------------------------------------------------------
S32 GLFWCursorController::getDoubleClickWidth()
{
   return GetSystemMetrics(SM_CXDOUBLECLK);
}

//------------------------------------------------------------------------------
S32 GLFWCursorController::getDoubleClickHeight()
{
   return GetSystemMetrics(SM_CYDOUBLECLK);
}


