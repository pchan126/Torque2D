//
//  iOSEvents.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-04-30.
//

#ifndef Torque2D_iOSWindowEvents_h
#define Torque2D_iOSWindowEvents_h

#include "delegates/delegateSignal.h"

/// void event(WindowId,U32 modifier,S32 x,S32 y, U32 action, U32 touchid)
typedef Signal<void(WindowId,U32,S32,S32,U32,U32)> TapEvent;

/// void event(WindowId,U32 modifier,F32 translation x,F32 translation y, F32 velocity x, F32 velocity y, U32 action)
typedef Signal<void(WindowId,U32,F32,F32,F32,F32,U32)> PanEvent;

/// void event(WindowId,U32 modifier,U32 direction)
typedef Signal<void(WindowId,U32,U32)> SwipeEvent;

/// void event(WindowId,U32 modifier,S32 x, S32 y, F32 rotation (radians), F32 velocity(radians))
typedef Signal<void(WindowId,U32,S32, S32, F32,F32)> RotationEvent;

/// void event(WindowId,U32 modifier,S32 x,S32 y, U32 action, U32 touchid)
typedef Signal<void(WindowId,U32,S32,S32,U32,U32)> LongPressEvent;

/// void event(WindowId,U32 modifier,F32 scale, F32 velocity, U32 action)
typedef Signal<void(WindowId,U32, F32,F32,U32)> PinchEvent;

/// void event(WindowId,U32 modifier,S32 x,S32 y, U32 touchID, U8 action, U32 numTouches)
typedef Signal<void(WindowId,U32,S32,S32,U32,U32,U32)> TouchEvent;
#endif

