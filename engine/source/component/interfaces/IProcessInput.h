//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _I_PROCESSINPUT_H_
#define _I_PROCESSINPUT_H_

#include "platform/event.h"


// CodeReview : Input can come from a number of places to end up in 
// torque, but in the torque world we don't want to expose this 
// information to the user.  This interface bridges the gap between 
// other input devices working details and input as torque understands it.
// Thoughts?  [7/6/2007 justind]



class IProcessInput
{
public:
    /// @name Input Processing
    /// @{
    
    /// Processes an input event
    /// @see InputEvent
    /// @param   event   Input event to process
    virtual bool processInputEvent(const InputEventInfo &event) = 0;
    
    /// Processes a mouse movement event
    /// @see MouseMoveEvent
    /// @param   event   Mouse move event to process
    virtual void processMouseMoveEvent(const MouseMoveEventInfo &event) = 0;
    
    virtual void processScreenTouchEvent(const ScreenTouchEventInfo &event) = 0;
    
    /// @}
};





#endif