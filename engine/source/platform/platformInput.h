//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMINPUT_H_
#define _PLATFORMINPUT_H_

#ifndef _SIMBASE_H_
#include "sim/simBase.h"
#endif
//#include "input/actionMap.h"
#include "platform/event.h"

//------------------------------------------------------------------------------
U8 TranslateOSKeyCode( U8 vcode );
U8 TranslateKeyCodeToOS(U8 keycode);

//------------------------------------------------------------------------------

class InputDevice : public SimObject
{
public:
    struct ObjInfo
    {
        U16   mType;
        U16   mInst;
        S32   mMin, mMax;
    };

protected:
    char mName[30];

public:
    const char* getDeviceName();
    virtual bool process() = 0;
};


//------------------------------------------------------------------------------

inline const char* InputDevice::getDeviceName()
{
    return mName;
}


//------------------------------------------------------------------------------

class InputManager : public SimGroup
{
protected:
    bool  mEnabled;

public:
    bool  isEnabled();

    virtual bool enable() = 0;
    virtual void disable() = 0;

    virtual void process() = 0;
};


//------------------------------------------------------------------------------

inline bool InputManager::isEnabled()
{
    return mEnabled;
}

enum KEY_STATE
{
    STATE_LOWER,
    STATE_UPPER,
    STATE_GOOFY
};


//------------------------------------------------------------------------------

class Input
{
protected:
    static InputManager* smManager; ///< Input Manager Singleton
    static bool smActive; ///< Is Input Active
    static bool smLastKeyboardActivated;
    static U8 smModifierKeys; ///< Current Modifier Keys Pressed
    static bool smLastMouseActivated;
    static bool smLastJoystickActivated;

public:
    static void init();
    static void destroy();

    static bool enable();
    static void disable();

    static void activate();
    static void deactivate();
    static void reactivate();

 //   static void enableMouse();
 //   static void disableMouse();
 //   static void enableKeyboard();
 //   static void disableKeyboard();
	//static bool activateKeyboard();
	//static void deactivateKeyboard();
	static bool enableJoystick();
	static void disableJoystick();
	static void echoInputState();

    static U16  getAscii( U16 keyCode, KEY_STATE keyState );
    static U16  getKeyCode( U16 asciiCode );

    static bool isEnabled();
    static bool isActive();
    //static bool isMouseEnabled();
    //static bool isKeyboardEnabled();

    static void process();
    //static void setCursorPos(S32 x, S32 y);
    //static void setCursorState(bool on); ///< If True, turn on the platform's cursor

    //static void setCursorShape(U32 cursorID);

    //static void pushCursor(S32 cursorID); ///< Push a cursor shape using the Cursor Manager
    //static void popCursor(); ///< Pop the current cursor off of the Cursor Manager stack
    //static void refreshCursor(); ///< Refresh the current cursor's shape.

    //static U32 getDoubleClickTime();
    //static S32 getDoubleClickWidth();
    //static S32 getDoubleClickHeight();

    static InputManager* getManager(); ///< Return InputManager Singleton

    static U8 getModifierKeys() {return smModifierKeys;}
    static void setModifierKeys(U8 mod) {smModifierKeys = mod;}
    
    /// Global input routing JournaledSignal; post input events here for
    /// processing.
    static InputEvent smInputEvent;
};

#endif // _H_PLATFORMINPUT_
