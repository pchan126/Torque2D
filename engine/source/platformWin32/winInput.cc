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

#include "platformWin32/platformWin32.h"
#include "platform/platformInput.h"
#include "platformWin32/winDirectInput.h"
#include "platform/event.h"
#include "console/console.h"
#include "delegates/process.h"

//#include "platform/platformInput_ScriptBinding.h"

// Static class variables:
bool           Input::smActive;
//CursorManager* Input::smCursorManager = 0; //*** DAW: Added
U8             Input::smModifierKeys; //*** DAW: Added
bool           Input::smLastKeyboardActivated;
bool           Input::smLastMouseActivated;
bool           Input::smLastJoystickActivated;
InputEvent     Input::smInputEvent;

static void fillAsciiTable();

//------------------------------------------------------------------------------
//
// This function translates an internal key code to the associated
// DirectInput scan code
//
//------------------------------------------------------------------------------
U8 Key_to_DIK( U16 keyCode )
{
   switch ( keyCode )
   {
      case KEY_BACKSPACE:     return DIK_BACK;
      case KEY_TAB:           return DIK_TAB;
      case KEY_RETURN:        return DIK_RETURN;
      //KEY_CONTROL:
      //KEY_ALT:
      //KEY_SHIFT:
      case KEY_PAUSE:         return DIK_PAUSE;
      case KEY_CAPSLOCK:      return DIK_CAPSLOCK;
      case KEY_ESCAPE:        return DIK_ESCAPE;

      case KEY_SPACE:         return DIK_SPACE;
      case KEY_PAGE_DOWN:     return DIK_PGDN;
      case KEY_PAGE_UP:       return DIK_PGUP;
      case KEY_END:           return DIK_END;
      case KEY_HOME:          return DIK_HOME;
      case KEY_LEFT:          return DIK_LEFT;
      case KEY_UP:            return DIK_UP;
      case KEY_RIGHT:         return DIK_RIGHT;
      case KEY_DOWN:          return DIK_DOWN;
      case KEY_PRINT:         return DIK_SYSRQ;
      case KEY_INSERT:        return DIK_INSERT;
      case KEY_DELETE:        return DIK_DELETE;
      case KEY_HELP:          return 0;

      case KEY_0:             return DIK_0;
      case KEY_1:             return DIK_1;
      case KEY_2:             return DIK_2;
      case KEY_3:             return DIK_3;
      case KEY_4:             return DIK_4;
      case KEY_5:             return DIK_5;
      case KEY_6:             return DIK_6;
      case KEY_7:             return DIK_7;
      case KEY_8:             return DIK_8;
      case KEY_9:             return DIK_9;

      case KEY_A:             return DIK_A;
      case KEY_B:             return DIK_B;
      case KEY_C:             return DIK_C;
      case KEY_D:             return DIK_D;
      case KEY_E:             return DIK_E;
      case KEY_F:             return DIK_F;
      case KEY_G:             return DIK_G;
      case KEY_H:             return DIK_H;
      case KEY_I:             return DIK_I;
      case KEY_J:             return DIK_J;
      case KEY_K:             return DIK_K;
      case KEY_L:             return DIK_L;
      case KEY_M:             return DIK_M;
      case KEY_N:             return DIK_N;
      case KEY_O:             return DIK_O;
      case KEY_P:             return DIK_P;
      case KEY_Q:             return DIK_Q;
      case KEY_R:             return DIK_R;
      case KEY_S:             return DIK_S;
      case KEY_T:             return DIK_T;
      case KEY_U:             return DIK_U;
      case KEY_V:             return DIK_V;
      case KEY_W:             return DIK_W;
      case KEY_X:             return DIK_X;
      case KEY_Y:             return DIK_Y;
      case KEY_Z:             return DIK_Z;

      case KEY_TILDE:         return DIK_GRAVE;
      case KEY_MINUS:         return DIK_MINUS;
      case KEY_EQUALS:        return DIK_EQUALS;
      case KEY_LBRACKET:      return DIK_LBRACKET;
      case KEY_RBRACKET:      return DIK_RBRACKET;
      case KEY_BACKSLASH:     return DIK_BACKSLASH;
      case KEY_SEMICOLON:     return DIK_SEMICOLON;
      case KEY_APOSTROPHE:    return DIK_APOSTROPHE;
      case KEY_COMMA:         return DIK_COMMA;
      case KEY_PERIOD:        return DIK_PERIOD;
      case KEY_SLASH:         return DIK_SLASH;

      case KEY_NUMPAD0:       return DIK_NUMPAD0;
      case KEY_NUMPAD1:       return DIK_NUMPAD1;
      case KEY_NUMPAD2:       return DIK_NUMPAD2;
      case KEY_NUMPAD3:       return DIK_NUMPAD3;
      case KEY_NUMPAD4:       return DIK_NUMPAD4;
      case KEY_NUMPAD5:       return DIK_NUMPAD5;
      case KEY_NUMPAD6:       return DIK_NUMPAD6;
      case KEY_NUMPAD7:       return DIK_NUMPAD7;
      case KEY_NUMPAD8:       return DIK_NUMPAD8;
      case KEY_NUMPAD9:       return DIK_NUMPAD9;
      case KEY_MULTIPLY:      return DIK_MULTIPLY;
      case KEY_ADD:           return DIK_ADD;
      case KEY_SEPARATOR:     return DIK_NUMPADCOMMA;
      case KEY_SUBTRACT:      return DIK_SUBTRACT;
      case KEY_DECIMAL:       return DIK_DECIMAL;
      case KEY_DIVIDE:        return DIK_DIVIDE;
      case KEY_NUMPADENTER:   return DIK_NUMPADENTER;

      case KEY_F1:            return DIK_F1;
      case KEY_F2:            return DIK_F2;
      case KEY_F3:            return DIK_F3;
      case KEY_F4:            return DIK_F4;
      case KEY_F5:            return DIK_F5;
      case KEY_F6:            return DIK_F6;
      case KEY_F7:            return DIK_F7;
      case KEY_F8:            return DIK_F8;
      case KEY_F9:            return DIK_F9;
      case KEY_F10:           return DIK_F10;
      case KEY_F11:           return DIK_F11;
      case KEY_F12:           return DIK_F12;
      case KEY_F13:           return DIK_F13;
      case KEY_F14:           return DIK_F14;
      case KEY_F15:           return DIK_F15;
      case KEY_F16:
      case KEY_F17:
      case KEY_F18:
      case KEY_F19:
      case KEY_F20:
      case KEY_F21:
      case KEY_F22:
      case KEY_F23:
      case KEY_F24:           return 0;

      case KEY_NUMLOCK:       return DIK_NUMLOCK;
      case KEY_SCROLLLOCK:    return DIK_SCROLL;
      case KEY_LCONTROL:      return DIK_LCONTROL;
      case KEY_RCONTROL:      return DIK_RCONTROL;
      case KEY_LALT:          return DIK_LALT;
      case KEY_RALT:          return DIK_RALT;
      case KEY_LSHIFT:        return DIK_LSHIFT;
      case KEY_RSHIFT:        return DIK_RSHIFT;

      case KEY_WIN_LWINDOW:   return DIK_LWIN;
      case KEY_WIN_RWINDOW:   return DIK_RWIN;
      case KEY_WIN_APPS:      return DIK_APPS;
      case KEY_OEM_102:       return DIK_OEM_102;

   };

   return 0;
}


//------------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//------------------------------------------------------------------------------
struct AsciiData
{
   struct KeyData
   {
      U16   ascii;
      bool  isDeadChar;
   };

   KeyData upper;
   KeyData lower;
   KeyData goofy;
};


#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE
static AsciiData AsciiTable[NUM_KEYS];

//------------------------------------------------------------------------------
void Input::init()
{
   Con::printSeparator();
   Con::printf( "Input Initialization:" );

   destroy();

   smActive = false;
   smLastKeyboardActivated = true;
   smLastMouseActivated = true;
   smLastJoystickActivated = true;

   //OSVERSIONINFO OSVersionInfo;
   //dMemset( &OSVersionInfo, 0, sizeof( OSVERSIONINFO ) );
   //OSVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
   //if ( GetVersionEx( &OSVersionInfo ) )
   //{

   //   if ( !( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && OSVersionInfo.dwMajorVersion < 5 ) )
   //   {
   //      smManager = new DInputManager;
   //      if ( !smManager->enable() )
   //      {
   //         Con::printf( "   DirectInput not enabled." );
   //         delete smManager;
   //         smManager = NULL;
   //      }
   //      else
   //      {
   //         DInputManager::init();
   //         Con::printf( "   DirectInput enabled." );
   //      }
   //   }
   //   else
   //      Con::printf( "  WinNT detected -- DirectInput not enabled." );
   //}

   //// Startup the Cursor Manager
   //if(!smCursorManager)
   //{
   //   smCursorManager = new CursorManager();
   //   if(smCursorManager)
   //   {
   //      // Add the arrow cursor to the stack
   //      smCursorManager->pushCursor(CursorManager::curArrow);
   //   }
   //   else
   //   {
   //      Con::printf("   Cursor Manager not enabled.");
   //   }
   //}

   // Init the current modifier keys
   setModifierKeys(0);
   fillAsciiTable();
   Con::printf( "" );

   // Set ourselves to participate in per-frame processing.
   Process::notify(Input::process, PROCESS_INPUT_ORDER);

}

////------------------------------------------------------------------------------
//ConsoleFunction( isJoystickDetected, bool, 1, 1, "() Use the isJoystickDetected function to determine if one or more joysticks are connected to the system.\n"
//																"This doesn't tell us how many joysticks there are, just that there are joysticks. It is our job to find out how many and to attach them.\n"
//																"@return Returns true if one or more joysticks are attached and detected, false otherwise.\n"
//																"@sa disableJoystick, enableJoystick, getJoystickAxes")
//{
//   argc; argv;
//   return( DInputDevice::joystickDetected() );
//}
//
////------------------------------------------------------------------------------
//ConsoleFunction( getJoystickAxes, const char*, 2, 2, "( instance ) Use the getJoystickAxes function to get the current axes position (x and y ) of any intance of a joystick.\n"
//																"@param instance A non-negative number value selecting a specific joystick instance attached to this computer.\n"
//																"@return Returns a string containing the \"x y\" position of the joystick.\n"
//																"@sa disableJoystick, enableJoystick, isJoystickDetected")
//{
//   argc;
//   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
//   if ( mgr )
//      return( mgr->getJoystickAxesString( dAtoi( argv[1] ) ) );
//
//   return( "" );
//}

//------------------------------------------------------------------------------
static void fillAsciiTable()
{
   //HKL   layout = GetKeyboardLayout( 0 );
   U8    state[256];
   U16   ascii[2];
   U32   dikCode, vKeyCode, keyCode;
   S32   result;

   dMemset( &AsciiTable, 0, sizeof( AsciiTable ) );
   dMemset( &state, 0, sizeof( state ) );

   for ( keyCode = KEY_FIRST; keyCode < NUM_KEYS; keyCode++ )
   {
      ascii[0] = ascii[1] = 0;
      dikCode  = Key_to_DIK( keyCode );
      if ( dikCode )
      {
         //vKeyCode = MapVirtualKeyEx( dikCode, 1, layout );
         vKeyCode = MapVirtualKey( dikCode, 1 );

         // Lower case:
         ascii[0] = ascii[1] = 0;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );

         if ( result == 2 )
            AsciiTable[keyCode].lower.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].lower.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].lower.ascii = ascii[0];
            AsciiTable[keyCode].lower.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }

         // Upper case:
         ascii[0] = ascii[1] = 0;
         state[VK_SHIFT] = 0x80;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );
          
         if ( result == 2 )
            AsciiTable[keyCode].upper.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].upper.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].upper.ascii = ascii[0];
            AsciiTable[keyCode].upper.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }
         state[VK_SHIFT] = 0;

         // Foreign mod case:
         ascii[0] = ascii[1] = 0;
         state[VK_CONTROL] = 0x80;
         state[VK_MENU] = 0x80;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );
          
         if ( result == 2 )
            AsciiTable[keyCode].goofy.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].goofy.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].goofy.ascii = ascii[0];
            AsciiTable[keyCode].goofy.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }
         state[VK_CONTROL] = 0;
         state[VK_MENU] = 0;
      }
   }
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
   U16 keyCode = 0;
   U16 i;

   // This is done three times so the lowerkey will always
   // be found first. Some foreign keyboards have duplicate
   // chars on some keys.
   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].lower.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].upper.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].goofy.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   return( keyCode );
}

//------------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   if ( keyCode >= NUM_KEYS )
      return 0;

   switch ( keyState )
   {
   case STATE_LOWER:
      return AsciiTable[keyCode].lower.ascii;
   case STATE_UPPER:
      return AsciiTable[keyCode].upper.ascii;
   case STATE_GOOFY:
      return AsciiTable[keyCode].goofy.ascii;
   default:
      return(0);

   }
}

//------------------------------------------------------------------------------
void Input::destroy()
{
   //if ( smManager && smManager->isEnabled() )
   //{
   //   smManager->disable();
   //   delete smManager;
   //   smManager = NULL;
   //}
}

//------------------------------------------------------------------------------
bool Input::enable()
{
   //if ( smManager && !smManager->isEnabled() )
   //   //return( smManager->enable() );

   return( false );
}

//------------------------------------------------------------------------------
void Input::disable()
{
   //if ( smManager && smManager->isEnabled() )
   //   smManager->disable();
}

//------------------------------------------------------------------------------

void Input::activate()
{
#ifdef UNICODE
   //winState.imeHandle = ImmGetContext( getWin32WindowHandle() );
   //ImmReleaseContext( getWin32WindowHandle(), winState.imeHandle );
#endif

   if ( !Con::getBoolVariable( "$enableDirectInput" ) )
      return;

//   if ( smManager && smManager->isEnabled() && !smActive )
//   {
//      Con::printf( "Activating DirectInput..." );
//#ifdef LOG_INPUT
//      Input::log( "Activating DirectInput...\n" );
//#endif
//      smActive = true;
//      DInputManager* dInputManager = dynamic_cast<DInputManager*>( smManager );
//      if ( dInputManager )
//      {
//         if ( dInputManager->isJoystickEnabled() && smLastJoystickActivated )
//            dInputManager->activateJoystick();
//      }
//   }
}

//------------------------------------------------------------------------------
void Input::deactivate()
{
//   if ( smManager && smManager->isEnabled() && smActive )
//   {
//#ifdef LOG_INPUT
//      Input::log( "Deactivating DirectInput...\n" );
//#endif
//      DInputManager* dInputManager = dynamic_cast<DInputManager*>( smManager );
//
//      if ( dInputManager )
//      {
//         smLastJoystickActivated = dInputManager->isJoystickActive();
//         dInputManager->deactivateJoystick();
//      }
//
//      smActive = false;
//      Con::printf( "DirectInput deactivated." );
//   }
}


//------------------------------------------------------------------------------
void Input::reactivate()
{
   // This is soo hacky...
   SetForegroundWindow( winState.appWindow );
   PostMessage( winState.appWindow, WM_ACTIVATE, WA_ACTIVE, NULL );
}

//------------------------------------------------------------------------------
bool Input::isEnabled()
{
   //if ( smManager )
   //   return smManager->isEnabled();
   return false;
}

//------------------------------------------------------------------------------
bool Input::isActive()
{
   return smActive;
}

//------------------------------------------------------------------------------
void Input::process()
{
   //if ( smManager && smManager->isEnabled() && smActive )
   //   smManager->process();
}


//------------------------------------------------------------------------------
bool Input::enableJoystick()
{
	//return( DInputManager::enableJoystick() );
	return false;
}

//------------------------------------------------------------------------------
void Input::disableJoystick()
{
	//DInputManager::disableJoystick();
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static U8 VcodeRemap[256] =
{
   0,                   // 0x00
   0,                   // 0x01  VK_LBUTTON
   0,                   // 0x02  VK_RBUTTON
   0,                   // 0x03  VK_CANCEL
   0,                   // 0x04  VK_MBUTTON
   0,                   // 0x05
   0,                   // 0x06
   0,                   // 0x07
   KEY_BACKSPACE,       // 0x08  VK_BACK
   KEY_TAB,             // 0x09  VK_TAB
   0,                   // 0x0A
   0,                   // 0x0B
   0,                   // 0x0C  VK_CLEAR
   KEY_RETURN,          // 0x0D  VK_RETURN
   0,                   // 0x0E
   0,                   // 0x0F
   KEY_SHIFT,           // 0x10  VK_SHIFT
   KEY_CONTROL,         // 0x11  VK_CONTROL
   KEY_ALT,             // 0x12  VK_MENU
   KEY_PAUSE,           // 0x13  VK_PAUSE
   KEY_CAPSLOCK,        // 0x14  VK_CAPITAL
   0,                   // 0x15  VK_KANA, VK_HANGEUL, VK_HANGUL
   0,                   // 0x16
   0,                   // 0x17  VK_JUNJA
   0,                   // 0x18  VK_FINAL
   0,                   // 0x19  VK_HANJA, VK_KANJI
   0,                   // 0x1A
   KEY_ESCAPE,          // 0x1B  VK_ESCAPE

   0,                   // 0x1C  VK_CONVERT
   0,                   // 0x1D  VK_NONCONVERT
   0,                   // 0x1E  VK_ACCEPT
   0,                   // 0x1F  VK_MODECHANGE

   KEY_SPACE,           // 0x20  VK_SPACE
   KEY_PAGE_UP,         // 0x21  VK_PRIOR
   KEY_PAGE_DOWN,       // 0x22  VK_NEXT
   KEY_END,             // 0x23  VK_END
   KEY_HOME,            // 0x24  VK_HOME
   KEY_LEFT,            // 0x25  VK_LEFT
   KEY_UP,              // 0x26  VK_UP
   KEY_RIGHT,           // 0x27  VK_RIGHT
   KEY_DOWN,            // 0x28  VK_DOWN
   0,                   // 0x29  VK_SELECT
   KEY_PRINT,           // 0x2A  VK_PRINT
   0,                   // 0x2B  VK_EXECUTE
   0,                   // 0x2C  VK_SNAPSHOT
   KEY_INSERT,          // 0x2D  VK_INSERT
   KEY_DELETE,          // 0x2E  VK_DELETE
   KEY_HELP,            // 0x2F  VK_HELP

   KEY_0,               // 0x30  VK_0   VK_0 thru VK_9 are the same as ASCII '0' thru '9' (// 0x30 - // 0x39)
   KEY_1,               // 0x31  VK_1
   KEY_2,               // 0x32  VK_2
   KEY_3,               // 0x33  VK_3
   KEY_4,               // 0x34  VK_4
   KEY_5,               // 0x35  VK_5
   KEY_6,               // 0x36  VK_6
   KEY_7,               // 0x37  VK_7
   KEY_8,               // 0x38  VK_8
   KEY_9,               // 0x39  VK_9
   0,                   // 0x3A
   0,                   // 0x3B
   0,                   // 0x3C
   0,                   // 0x3D
   0,                   // 0x3E
   0,                   // 0x3F
   0,                   // 0x40

   KEY_A,               // 0x41  VK_A      VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (// 0x41 - // 0x5A)
   KEY_B,               // 0x42  VK_B
   KEY_C,               // 0x43  VK_C
   KEY_D,               // 0x44  VK_D
   KEY_E,               // 0x45  VK_E
   KEY_F,               // 0x46  VK_F
   KEY_G,               // 0x47  VK_G
   KEY_H,               // 0x48  VK_H
   KEY_I,               // 0x49  VK_I
   KEY_J,               // 0x4A  VK_J
   KEY_K,               // 0x4B  VK_K
   KEY_L,               // 0x4C  VK_L
   KEY_M,               // 0x4D  VK_M
   KEY_N,               // 0x4E  VK_N
   KEY_O,               // 0x4F  VK_O
   KEY_P,               // 0x50  VK_P
   KEY_Q,               // 0x51  VK_Q
   KEY_R,               // 0x52  VK_R
   KEY_S,               // 0x53  VK_S
   KEY_T,               // 0x54  VK_T
   KEY_U,               // 0x55  VK_U
   KEY_V,               // 0x56  VK_V
   KEY_W,               // 0x57  VK_W
   KEY_X,               // 0x58  VK_X
   KEY_Y,               // 0x59  VK_Y
   KEY_Z,               // 0x5A  VK_Z


   KEY_WIN_LWINDOW,     // 0x5B  VK_LWIN
   KEY_WIN_RWINDOW,     // 0x5C  VK_RWIN
   KEY_WIN_APPS,        // 0x5D  VK_APPS
   0,                   // 0x5E
   0,                   // 0x5F

   KEY_NUMPAD0,         // 0x60  VK_NUMPAD0
   KEY_NUMPAD1,         // 0x61  VK_NUMPAD1
   KEY_NUMPAD2,         // 0x62  VK_NUMPAD2
   KEY_NUMPAD3,         // 0x63  VK_NUMPAD3
   KEY_NUMPAD4,         // 0x64  VK_NUMPAD4
   KEY_NUMPAD5,         // 0x65  VK_NUMPAD5
   KEY_NUMPAD6,         // 0x66  VK_NUMPAD6
   KEY_NUMPAD7,         // 0x67  VK_NUMPAD7
   KEY_NUMPAD8,         // 0x68  VK_NUMPAD8
   KEY_NUMPAD9,         // 0x69  VK_NUMPAD9
   KEY_MULTIPLY,        // 0x6A  VK_MULTIPLY
   KEY_ADD,             // 0x6B  VK_ADD
   KEY_SEPARATOR,       // 0x6C  VK_SEPARATOR
   KEY_SUBTRACT,        // 0x6D  VK_SUBTRACT
   KEY_DECIMAL,         // 0x6E  VK_DECIMAL
   KEY_DIVIDE,          // 0x6F  VK_DIVIDE
   KEY_F1,              // 0x70  VK_F1
   KEY_F2,              // 0x71  VK_F2
   KEY_F3,              // 0x72  VK_F3
   KEY_F4,              // 0x73  VK_F4
   KEY_F5,              // 0x74  VK_F5
   KEY_F6,              // 0x75  VK_F6
   KEY_F7,              // 0x76  VK_F7
   KEY_F8,              // 0x77  VK_F8
   KEY_F9,              // 0x78  VK_F9
   KEY_F10,             // 0x79  VK_F10
   KEY_F11,             // 0x7A  VK_F11
   KEY_F12,             // 0x7B  VK_F12
   KEY_F13,             // 0x7C  VK_F13
   KEY_F14,             // 0x7D  VK_F14
   KEY_F15,             // 0x7E  VK_F15
   KEY_F16,             // 0x7F  VK_F16
   KEY_F17,             // 0x80  VK_F17
   KEY_F18,             // 0x81  VK_F18
   KEY_F19,             // 0x82  VK_F19
   KEY_F20,             // 0x83  VK_F20
   KEY_F21,             // 0x84  VK_F21
   KEY_F22,             // 0x85  VK_F22
   KEY_F23,             // 0x86  VK_F23
   KEY_F24,             // 0x87  VK_F24
   0,                   // 0x88
   0,                   // 0x89
   0,                   // 0x8A
   0,                   // 0x8B
   0,                   // 0x8C
   0,                   // 0x8D
   0,                   // 0x8E
   0,                   // 0x8F

   KEY_NUMLOCK,         // 0x90  VK_NUMLOCK
   KEY_SCROLLLOCK,      // 0x91  VK_OEM_SCROLL
   0,                   // 0x92
   0,                   // 0x93
   0,                   // 0x94
   0,                   // 0x95
   0,                   // 0x96
   0,                   // 0x97
   0,                   // 0x98
   0,                   // 0x99
   0,                   // 0x9A
   0,                   // 0x9B
   0,                   // 0x9C
   0,                   // 0x9D
   0,                   // 0x9E
   0,                   // 0x9F

   KEY_LSHIFT,          // 0xA0  VK_LSHIFT
   KEY_RSHIFT,          // 0xA1  VK_RSHIFT
   KEY_LCONTROL,        // 0xA2  VK_LCONTROL
   KEY_RCONTROL,        // 0xA3  VK_RCONTROL
   KEY_LALT,            // 0xA4  VK_LMENU
   KEY_RALT,            // 0xA5  VK_RMENU
   0,                   // 0xA6
   0,                   // 0xA7
   0,                   // 0xA8
   0,                   // 0xA9
   0,                   // 0xAA
   0,                   // 0xAB
   0,                   // 0xAC
   0,                   // 0xAD
   0,                   // 0xAE
   0,                   // 0xAF
   0,                   // 0xB0
   0,                   // 0xB1
   0,                   // 0xB2
   0,                   // 0xB3
   0,                   // 0xB4
   0,                   // 0xB5
   0,                   // 0xB6
   0,                   // 0xB7
   0,                   // 0xB8
   0,                   // 0xB9
   KEY_SEMICOLON,       // 0xBA  VK_OEM_1
   KEY_EQUALS,          // 0xBB  VK_OEM_PLUS
   KEY_COMMA,           // 0xBC  VK_OEM_COMMA
   KEY_MINUS,           // 0xBD  VK_OEM_MINUS
   KEY_PERIOD,          // 0xBE  VK_OEM_PERIOD
   KEY_SLASH,           // 0xBF  VK_OEM_2
   KEY_TILDE,           // 0xC0  VK_OEM_3
   0,                   // 0xC1
   0,                   // 0xC2
   0,                   // 0xC3
   0,                   // 0xC4
   0,                   // 0xC5
   0,                   // 0xC6
   0,                   // 0xC7
   0,                   // 0xC8
   0,                   // 0xC9
   0,                   // 0xCA
   0,                   // 0xCB
   0,                   // 0xCC
   0,                   // 0xCD
   0,                   // 0xCE
   0,                   // 0xCF
   0,                   // 0xD0
   0,                   // 0xD1
   0,                   // 0xD2
   0,                   // 0xD3
   0,                   // 0xD4
   0,                   // 0xD5
   0,                   // 0xD6
   0,                   // 0xD7
   0,                   // 0xD8
   0,                   // 0xD9
   0,                   // 0xDA
   KEY_LBRACKET,        // 0xDB  VK_OEM_4
   KEY_BACKSLASH,       // 0xDC  VK_OEM_5
   KEY_RBRACKET,        // 0xDD  VK_OEM_6
   KEY_APOSTROPHE,      // 0xDE  VK_OEM_7
   0,                   // 0xDF  VK_OEM_8
   0,                   // 0xE0
   0,                   // 0xE1  VK_OEM_AX  AX key on Japanese AX keyboard
   KEY_OEM_102,         // 0xE2  VK_OEM_102
   0,                   // 0xE3
   0,                   // 0xE4

   0,                   // 0xE5  VK_PROCESSKEY

   0,                   // 0xE6
   0,                   // 0xE7
   0,                   // 0xE8
   0,                   // 0xE9
   0,                   // 0xEA
   0,                   // 0xEB
   0,                   // 0xEC
   0,                   // 0xED
   0,                   // 0xEE
   0,                   // 0xEF

   0,                   // 0xF0
   0,                   // 0xF1
   0,                   // 0xF2
   0,                   // 0xF3
   0,                   // 0xF4
   0,                   // 0xF5

   0,                   // 0xF6  VK_ATTN
   0,                   // 0xF7  VK_CRSEL
   0,                   // 0xF8  VK_EXSEL
   0,                   // 0xF9  VK_EREOF
   0,                   // 0xFA  VK_PLAY
   0,                   // 0xFB  VK_ZOOM
   0,                   // 0xFC  VK_NONAME
   0,                   // 0xFD  VK_PA1
   0,                   // 0xFE  VK_OEM_CLEAR
   0                    // 0xFF
};


//------------------------------------------------------------------------------
//
// This function translates a virtual key code to our corresponding internal
// key code using the preceding table.
//
//------------------------------------------------------------------------------
U8 TranslateOSKeyCode(U8 vcode)
{
   return VcodeRemap[vcode];
}

U8 TranslateKeyCodeToOS(U8 keycode)
{
   for(S32 i = 0;i < sizeof(VcodeRemap) / sizeof(U8);++i)
   {
      if(VcodeRemap[i] == keycode)
         return i;
   }
   return 0;
}

//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
   HGLOBAL hGlobal;
   LPVOID  pGlobal;

   //make sure we can access the clipboard
   if (!IsClipboardFormatAvailable(CF_TEXT))
      return "";
   if (!OpenClipboard(NULL))
      return "";

   hGlobal = GetClipboardData(CF_TEXT);
   pGlobal = GlobalLock(hGlobal);
   S32 cbLength = strlen((char *)pGlobal);
   char  *returnBuf = Con::getReturnBuffer(cbLength + 1);
   strcpy(returnBuf, (char *)pGlobal);
   returnBuf[cbLength] = '\0';
   GlobalUnlock(hGlobal);
   CloseClipboard();

   //note - this function never returns NULL
   return returnBuf;
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
   if (!text)
      return false;

   //make sure we can access the clipboard
   if (!OpenClipboard(NULL))
      return false;

   S32 cbLength = strlen(text);

   HGLOBAL hGlobal;
   LPVOID  pGlobal;

   hGlobal = GlobalAlloc(GHND, cbLength + 1);
   pGlobal = GlobalLock (hGlobal);

   strcpy((char *)pGlobal, text);

   GlobalUnlock(hGlobal);

   EmptyClipboard();
   SetClipboardData(CF_TEXT, hGlobal);
   CloseClipboard();

   return true;
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