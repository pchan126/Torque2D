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
#import "platform/platform.h"
#import "platform/platformInput.h"
#import "platformOSX/osxInputManager.h"
#import "platform/event.h"
#include "platformOSX/windowManager/GLFWWindowManager.h"
#include "platformOSX/windowManager/GLFWWindow.h"

#pragma mark ---- Static Variables ----
//InputManager *Input::smManager = 0;
bool Input::smActive;
U8             Input::smModifierKeys;
InputEvent     Input::smInputEvent;

#pragma mark ---- Input Namespace Functions ----

//--------------------------------------------------------------------------
void Input::init()
{
    Con::printSeparator();
    Con::printf("Input initialization:");

    // Cleanup if initialized previously
    destroy();

    smActive = false;

    Input::enable();
}

//--------------------------------------------------------------------------
void Input::destroy()
{
    smActive = false;
}

//------------------------------------------------------------------------------
// Disable all input for the application
bool Input::enable()
{
    bool enabledValue = false;

    // Check for any restrictions or errors

//    // If the manager exists and is not enabled, dos o
//    if (smManager && !smManager->isEnabled())
//        enabledValue = smManager->enable();

//    // Also enable mouse and keyboard automatically
//    enableMouse();
//    enableKeyboard();

    return enabledValue;
}

//------------------------------------------------------------------------------
// Disable all input in the application
void Input::disable()
{
//    if (smManager && smManager->isEnabled())
//        return smManager->disable();

//    disableMouse();
//    disableKeyboard();
}

//------------------------------------------------------------------------------
// Not used on OS X
void Input::activate()
{
}

//------------------------------------------------------------------------------
// Not used on OS X
void Input::deactivate()
{
}

//------------------------------------------------------------------------------
// Not used on OS X
void Input::reactivate()
{
}

//------------------------------------------------------------------------------
// Print the entire input state to the console
// Not yet implemented or unnecessary. Will resolve in the next platform update
void Input::echoInputState()
{
}

//------------------------------------------------------------------------------
// Accesses the global input manager to see if it is enabled
bool Input::isEnabled()
{
//    if (smManager)
//        return smManager->isEnabled();

    return false;
}

////------------------------------------------------------------------------------
//// Accesses the global input manager to see if its mouse is enabled
//bool Input::isMouseEnabled()
//{
//    osxInputManager* inputManager = (osxInputManager*)getManager();
//
//    if (inputManager)
//        return inputManager->isMouseEnabled();
//
//    return false;
//}
//
////------------------------------------------------------------------------------
//// Accesses the global input manager to see if its keyboard is enabled
//bool Input::isKeyboardEnabled()
//{
//    osxInputManager* inputManager = (osxInputManager*)getManager();
//
//    if (inputManager)
//        return inputManager->isKeyboardEnabled();
//
//    return false;
//}

//------------------------------------------------------------------------------
// Does nothing on OS X.
bool Input::isActive()
{
    // Input is always "active" in an OS X application
    // We manually enable or disable it, but the OS
    // always assumes it is running
    return true;
}

////------------------------------------------------------------------------------
//// Access the global osxInputManager and enables its mouse
//void Input::enableMouse()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//        return inputManager->enableMouse();
//}
//
////------------------------------------------------------------------------------
//// Access the global osxInputManager and disables its mouse
//void Input::disableMouse()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//        return inputManager->disableMouse();
//}
//
////------------------------------------------------------------------------------
//// Access the global osxInputManager and enables its keyboard
//void Input::enableKeyboard()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//        inputManager->enableKeyboard();
//}
//
////------------------------------------------------------------------------------
//// Access the global osxInputManager and enables its keyboard
//void Input::disableKeyboard()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//        return inputManager->disableKeyboard();
//}
//
////------------------------------------------------------------------------------
//// This is essentially the same as enableKeyboard. The redunancy is due to
//// keep parallel functionality with Windows, which has a different usage
//// for activateKeyboard due to DirectInput
//bool Input::activateKeyboard()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//    {
//        inputManager->enableKeyboard();
//        return true;
//    }
//
//    return false;
//}
//
////------------------------------------------------------------------------------
//// This is essentially the same as disableKeyboard. The redunancy is due to
//// keep parallel functionality with Windows, which has a different usage
//// for deactivateKeyboard due to DirectInput
//void Input::deactivateKeyboard()
//{
//    osxInputManager *inputManager = (osxInputManager *) getManager();
//
//    if (inputManager)
//        inputManager->disableKeyboard();
//}

//------------------------------------------------------------------------------
// Enable joystick input
bool Input::enableJoystick()
{
    return false;
}

//------------------------------------------------------------------------------
// Disable joystick input
void Input::disableJoystick()
{
}

//------------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
U16 Input::getKeyCode(U16 asciiCode)
{
    return 0;
}

//------------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
U16 Input::getAscii(U16 keyCode, KEY_STATE keyState)
{
    return 0;
}

#pragma mark ---- Platform Namespace Functions ----

//------------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
const char *Platform::getClipboard()
{
   GLFWWindow* glfwWindow = dynamic_cast<GLFWWindow*>(WindowManager->getFirstWindow());
   if (glfwWindow != NULL)
      return glfwWindow->getClipboardString();
   else
      return NULL;
}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
bool Platform::setClipboard(const char *text)
{
   GLFWWindow* glfwWindow = dynamic_cast<GLFWWindow*>(WindowManager->getFirstWindow());
   if (glfwWindow != NULL)
   {
      glfwWindow->setClipboardString(text);
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::enableKeyboardTranslation(void)
{

}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::disableKeyboardTranslation(void)
{

}

//--------------------------------------------------------------------------
//#pragma message("input remap table might need tweaking - rumors of ibooks having diff virt keycodes, might need intermediate remap...")
static U8 VcodeRemap[256] =
{
        KEY_A,                     // 0x00
        KEY_S,                     // 0x01
        KEY_D,                     // 0x02
        KEY_F,                     // 0x03
        KEY_H,                     // 0x04
        KEY_G,                     // 0x05
        KEY_Z,                     // 0x06
        KEY_X,                     // 0x07
        KEY_C,                     // 0x08
        KEY_V,                     // 0x09
        KEY_Y,                     // 0x0A       // this is questionable - not normal Y code
        KEY_B,                     // 0x0B
        KEY_Q,                     // 0x0C
        KEY_W,                     // 0x0D
        KEY_E,                     // 0x0E
        KEY_R,                     // 0x0F
        KEY_Y,                     // 0x10
        KEY_T,                     // 0x11
        KEY_1,                     // 0x12
        KEY_2,                     // 0x13
        KEY_3,                     // 0x14
        KEY_4,                     // 0x15
        KEY_6,                     // 0x16
        KEY_5,                     // 0x17
        KEY_EQUALS,                // 0x18
        KEY_9,                     // 0x19
        KEY_7,                     // 0x1A
        KEY_MINUS,                 // 0x1B
        KEY_8,                     // 0x1C
        KEY_0,                     // 0x1D
        KEY_RBRACKET,              // 0x1E
        KEY_O,                     // 0x1F
        KEY_U,                     // 0x20
        KEY_LBRACKET,              // 0x21
        KEY_I,                     // 0x22
        KEY_P,                     // 0x23
        KEY_RETURN,                // 0x24
        KEY_L,                     // 0x25
        KEY_J,                     // 0x26
        KEY_APOSTROPHE,            // 0x27
        KEY_K,                     // 0x28
        KEY_SEMICOLON,             // 0x29
        KEY_BACKSLASH,             // 0x2A
        KEY_COMMA,                 // 0x2B
        KEY_SLASH,                 // 0x2C
        KEY_N,                     // 0x2D
        KEY_M,                     // 0x2E
        KEY_PERIOD,                // 0x2F
        KEY_TAB,                   // 0x30
        KEY_SPACE,                 // 0x31
        KEY_TILDE,                 // 0x32
        KEY_BACKSPACE,             // 0x33
        0,                         // 0x34 //?
        KEY_ESCAPE,                // 0x35
        0,                         // 0x36 //?
        KEY_ALT,                   // 0x37 // best mapping for mac Cmd key
        KEY_LSHIFT,                // 0x38
        KEY_CAPSLOCK,              // 0x39
        KEY_MAC_OPT,               // 0x3A // direct map mac Option key -- better than KEY_WIN_WINDOWS
        KEY_CONTROL,               // 0x3B
        KEY_RSHIFT,                // 0x3C
        0,                         // 0x3D
        0,                         // 0x3E
        0,                         // 0x3F
        0,                         // 0x40
        KEY_DECIMAL,               // 0x41
        0,                         // 0x42
        KEY_MULTIPLY,              // 0x43
        0,                         // 0x44
        KEY_ADD,                   // 0x45
        KEY_SUBTRACT,              // 0x46 // secondary code?
        KEY_NUMLOCK,               // 0x47 // also known as Clear on mac...
        KEY_SEPARATOR,             // 0x48 // secondary code? for KPEqual
        0,                         // 0x49
        0,                         // 0x4A
        KEY_DIVIDE,                // 0x4B
        KEY_NUMPADENTER,           // 0x4C
        KEY_DIVIDE,                // 0x4D // secondary code?
        KEY_SUBTRACT,              // 0x4E
        0,                         // 0x4F
        0,                         // 0x50
        KEY_SEPARATOR,             // 0x51 // WHAT IS SEP?  This is KPEqual on mac.
        KEY_NUMPAD0,               // 0x52
        KEY_NUMPAD1,               // 0x53
        KEY_NUMPAD2,               // 0x54
        KEY_NUMPAD3,               // 0x55
        KEY_NUMPAD4,               // 0x56
        KEY_NUMPAD5,               // 0x57
        KEY_NUMPAD6,               // 0x58
        KEY_NUMPAD7,               // 0x59
        0,                         // 0x5A
        KEY_NUMPAD8,               // 0x5B
        KEY_NUMPAD9,               // 0x5C
        0,                         // 0x5D
        0,                         // 0x5E
        0,                         // 0x5F
        KEY_F5,                    // 0x60
        KEY_F6,                    // 0x61
        KEY_F7,                    // 0x62
        KEY_F3,                    // 0x63
        KEY_F8,                    // 0x64
        KEY_F9,                    // 0x65
        0,                         // 0x66
        KEY_F11,                   // 0x67
        0,                         // 0x68
        KEY_PRINT,                 // 0x69
        0,                         // 0x6A
        KEY_SCROLLLOCK,            // 0x6B
        0,                         // 0x6C
        KEY_F10,                   // 0x6D
        0,                         // 0x6E
        KEY_F12,                   // 0x6F
        0,                         // 0x70
        KEY_PAUSE,                 // 0x71
        KEY_INSERT,                // 0x72 // also known as mac Help
        KEY_HOME,                  // 0x73
        KEY_PAGE_UP,               // 0x74
        KEY_DELETE,                // 0x75 // FwdDel
        KEY_F4,                    // 0x76
        KEY_END,                   // 0x77
        KEY_F2,                    // 0x78
        KEY_PAGE_DOWN,             // 0x79
        KEY_F1,                    // 0x7A
        KEY_LEFT,                  // 0x7B
        KEY_RIGHT,                 // 0x7C
        KEY_DOWN,                  // 0x7D
        KEY_UP,                    // 0x7E
        0,                         // 0x7F
        0,                         // 0x80
        0,                         // 0x81
        0,                         // 0x82
        0,                         // 0x83
        0,                         // 0x84
        0,                         // 0x85
        0,                         // 0x86
        0,                         // 0x87
        0,                         // 0x88
        0,                         // 0x89
        0,                         // 0x8A
        0,                         // 0x8B
        0,                         // 0x8C
        0,                         // 0x8D
        0,                         // 0x8E
        0,                         // 0x8F

        0,                         // 0x90
        0,                         // 0x91
        0,                         // 0x92
        0,                         // 0x93
        0,                         // 0x94
        0,                         // 0x95
        0,                         // 0x96
        0,                         // 0x97
        0,                         // 0x98
        0,                         // 0x99
        0,                         // 0x9A
        0,                         // 0x9B
        0,                         // 0x9C
        0,                         // 0x9D
        0,                         // 0x9E
        0,                         // 0x9F

        0,                         // 0xA0
        0,                         // 0xA1
        0,                         // 0xA2
        0,                         // 0xA3
        0,                         // 0xA4
        0,                         // 0xA5
        0,                         // 0xA6
        0,                         // 0xA7
        0,                         // 0xA8
        0,                         // 0xA9
        0,                         // 0xAA
        0,                         // 0xAB
        0,                         // 0xAC
        0,                         // 0xAD
        0,                         // 0xAE
        0,                         // 0xAF
        0,                         // 0xB0
        0,                         // 0xB1
        0,                         // 0xB2
        0,                         // 0xB3
        0,                         // 0xB4
        0,                         // 0xB5
        0,                         // 0xB6
        0,                         // 0xB7
        0,                         // 0xB8
        0,                         // 0xB9
        0,                         // 0xBA
        0,                         // 0xBB
        0,                         // 0xBC
        0,                         // 0xBD
        0,                         // 0xBE
        0,                         // 0xBF
        0,                         // 0xC0
        0,                         // 0xC1
        0,                         // 0xC2
        0,                         // 0xC3
        0,                         // 0xC4
        0,                         // 0xC5
        0,                         // 0xC6
        0,                         // 0xC7
        0,                         // 0xC8
        0,                         // 0xC9
        0,                         // 0xCA
        0,                         // 0xCB
        0,                         // 0xCC
        0,                         // 0xCD
        0,                         // 0xCE
        0,                         // 0xCF
        0,                         // 0xD0
        0,                         // 0xD1
        0,                         // 0xD2
        0,                         // 0xD3
        0,                         // 0xD4
        0,                         // 0xD5
        0,                         // 0xD6
        0,                         // 0xD7
        0,                         // 0xD8
        0,                         // 0xD9
        0,                         // 0xDA
        0,                         // 0xDB
        0,                         // 0xDC
        0,                         // 0xDD
        0,                         // 0xDE
        0,                         // 0xDF
        0,                         // 0xE0
        0,                         // 0xE1
        0,                         // 0xE2
        0,                         // 0xE3
        0,                         // 0xE4

        0,                         // 0xE5

        0,                         // 0xE6
        0,                         // 0xE7
        0,                         // 0xE8
        0,                         // 0xE9
        0,                         // 0xEA
        0,                         // 0xEB
        0,                         // 0xEC
        0,                         // 0xED
        0,                         // 0xEE
        0,                         // 0xEF

        0,                         // 0xF0
        0,                         // 0xF1
        0,                         // 0xF2
        0,                         // 0xF3
        0,                         // 0xF4
        0,                         // 0xF5

        0,                         // 0xF6
        0,                         // 0xF7
        0,                         // 0xF8
        0,                         // 0xF9
        0,                         // 0xFA
        0,                         // 0xFB
        0,                         // 0xFC
        0,                         // 0xFD
        0,                         // 0xFE
        0                          // 0xFF
};


U8 TranslateOSKeyCode(U8 vcode)
{
    return VcodeRemap[vcode];
}

//--------------------------------------------------------------------------
//#pragma message("input remap table might need tweaking - rumors of ibooks having diff virt keycodes, might need intermediate remap...")
static U8 GLFWcodeRemap[512] =
{
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0,
   KEY_SPACE,                 // 32
   0, 0, 0, 0, 0, 0,
   KEY_APOSTROPHE,            // 39
   0, 0, 0, 0,
   KEY_COMMA,                 // 44
   KEY_MINUS,                 // 45
   KEY_PERIOD,                // 46
   KEY_SLASH,                 // 47
   KEY_0,                     // 48
   KEY_1,                     // 49
   KEY_2,                     // 50
   KEY_3,                     // 51
   KEY_4,                     // 52
   KEY_5,                     // 53
   KEY_6,                     // 54
   KEY_7,                     // 55
   KEY_8,                     // 56
   KEY_9,                     // 57
   KEY_SEMICOLON,             // 58
   0,
   KEY_EQUALS,                // 61
   0, 0, 0,
   KEY_A,                     // 65
   KEY_B,                     // 66
   KEY_C,                     // 67
   KEY_D,                     // 68
   KEY_E,                     // 69
   KEY_F,                     // 70
   KEY_G,                     // 71
   KEY_H,                     // 72
   KEY_I,                     // 73
   KEY_J,                     // 74
   KEY_K,                     // 75
   KEY_L,                     // 76
   KEY_M,                     // 77
   KEY_N,                     // 78
   KEY_O,                     // 79
   KEY_P,                     // 80
   KEY_Q,                     // 81
   KEY_R,                     // 82
   KEY_S,                     // 83
   KEY_T,                     // 84
   KEY_U,                     // 85
   KEY_V,                     // 86
   KEY_W,                     // 87
   KEY_X,                     // 88
   KEY_Y,                     // 89
   KEY_Z,                     // 90
   KEY_LBRACKET,              // 91
   KEY_BACKSLASH,             // 92
   KEY_RBRACKET,              // 93
   0, 0,
   KEY_TILDE,                 // 96
   0, 0, 0, 0, 0, 0, 0,       // 103
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 203
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 253
   0, 0,
   KEY_ESCAPE,                // 256
   KEY_RETURN,                // 257
   KEY_TAB,                   // 258
   KEY_BACKSPACE,             // 259
   KEY_INSERT,                // 260 // also known as mac Help
   KEY_DELETE,                // 261 // FwdDel
   KEY_RIGHT,                 // 262
   KEY_LEFT,                  // 263
   KEY_DOWN,                  // 264
   KEY_UP,                    // 265
   KEY_PAGE_UP,               // 266
   KEY_PAGE_DOWN,             // 267
   KEY_HOME,                  // 268
   KEY_END,                   // 269
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 279
   KEY_CAPSLOCK,              // 280
   KEY_SCROLLLOCK,            // 281
   KEY_NUMLOCK,               // 282
   KEY_PRINT,                 // 283
   KEY_PAUSE,                 // 284
   0, 0, 0, 0, 0,             // 289
   KEY_F1,                    // 290
   KEY_F2,                    // 291
   KEY_F3,                    // 292
   KEY_F4,                    // 293
   KEY_F5,                    // 294
   KEY_F6,                    // 295
   KEY_F7,                    // 296
   KEY_F8,                    // 297
   KEY_F9,                    // 298
   KEY_F11,                   // 299
   KEY_F10,                   // 300
   KEY_F12,                   // 301
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 311
   0, 0, 0, 0, 0, 0, 0, 0,       // 319

   KEY_NUMPAD0,               // 320
   KEY_NUMPAD1,               // 321
   KEY_NUMPAD2,               // 322
   KEY_NUMPAD3,               // 323
   KEY_NUMPAD4,               // 324
   KEY_NUMPAD5,               // 325
   KEY_NUMPAD6,               // 326
   KEY_NUMPAD7,               // 327
   KEY_NUMPAD8,               // 328
   KEY_NUMPAD9,               // 329
   KEY_DECIMAL,               // 330
   KEY_DIVIDE,                // 331
   KEY_MULTIPLY,              // 332
   KEY_SUBTRACT,              // 333
   KEY_ADD,                   // 334
   KEY_NUMPADENTER,           // 335
   KEY_SEPARATOR,             // 336
   0, 0, 0,
   KEY_LSHIFT,                // 340
   KEY_LCONTROL,              // 341
   KEY_LALT,                  // 342
   0,                         // 343
   KEY_RSHIFT,                // 344
   KEY_RCONTROL,              // 345
   KEY_RALT,                  // 346
   0,                         //
   0,                         //
};

U8 TranslateGLFWKeyCode(S32 vcode)
{
   return GLFWcodeRemap[vcode];
}


