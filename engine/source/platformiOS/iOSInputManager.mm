//
// Created by Paul L Jan on 2013-07-08.
//


#include "iOSInputManager.h"

iOSInputManager::iOSInputManager() {

}

bool iOSInputManager::enable() {
    mEnabled = true;
    return mEnabled;
}

void iOSInputManager::disable() {
    mEnabled = false;
}

void iOSInputManager::process() {
    GuiJoystickCtrl * dptr;
    for ( iterator ptr = inputs.begin(); ptr != inputs.end(); ptr++ )
    {
        dptr = dynamic_cast<GuiJoystickCtrl *>( *ptr );
        if ( dptr )
            dptr->process();
    }
}

void iOSInputManager::init() {

}

void iOSInputManager::addInput(SimObject *obj) {
    inputs.addObject(obj);
}

void iOSInputManager::removeInput(SimObject *obj) {
    inputs.removeObject(obj);
}
