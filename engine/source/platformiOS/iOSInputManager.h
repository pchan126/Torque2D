//
// Created by Paul L Jan on 2013-07-08.
//



#ifndef __iOSInputManager_H_
#define __iOSInputManager_H_


#include "platform/platformInput.h"
#include "guiJoystickCtrl.h"

class iOSInputManager: public InputManager
{
private:
    typedef SimGroup Parent;
    SimSet inputs;

public:
    iOSInputManager();

    static void init();

    bool enable();
    void disable();
    void process();

    void addInput( SimObject* obj);
    void removeInput( SimObject* obj);

};


#endif //__iOSInputManager_H_
