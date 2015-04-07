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

#import "platformiOS/platformiOS.h"
#include "game/gameInterface.h"
#include "platform/platformInput.h"

#pragma mark ---- IOSPlatState Implementation ----

@interface iOSPlatState (PrivateMethods)
@end

@implementation iOSPlatState

@synthesize argc = _argc;
@synthesize argv = _argv;
@synthesize platformRandom = _platformRandom;
@synthesize currentSimTime = _currentSimTime;
@synthesize mainCSDirectory = _mainCSDirectory;
@synthesize quit = _quit;


static iOSPlatState * tempSharedPlatState = nil;

bool getStatusBarHidden();
bool setStatusBarHidden(bool);
void setStatusBarType(S32);

//Hidden by Default. 1 Black Opaque, 2 Black Translucent
S32 gStatusBarType = 0;
bool gStatusBarHidden = true;

//-----------------------------------------------------------------------------

- (instancetype)init
{
    self = [super init];
    
    if (self)
    {
        // Default window behaviors
        _quit = false;
        
        // Default system variables
        _currentSimTime = 0;
        _argc = 0;
        _alertSemaphore = Semaphore::createSemaphore(0);
        _platformRandom = new RandomLCG();
    }
    
    return self;
}

//-----------------------------------------------------------------------------

- (void)dealloc
{
    if (_platformRandom)
        delete _platformRandom;
}


#pragma mark ---- Singleton Functions ----

//-----------------------------------------------------------------------------

+ (iOSPlatState*)sharedPlatState
{
    @synchronized(self)
    {
        if (tempSharedPlatState == nil)
            tempSharedPlatState = (iOSPlatState *) [[super allocWithZone:NULL] init];
    }
    
    return tempSharedPlatState;
}

//-----------------------------------------------------------------------------

+ (id)allocWithZone:(NSZone *)zone
{
    return [self sharedPlatState];
}

//-----------------------------------------------------------------------------

- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

//-----------------------------------------------------------------------------

#pragma mark ---- Torque 2D Functions ----

//-----------------------------------------------------------------------------
// Initializes the Game and any additional inits you need to perform
- (BOOL) initializeTorque2D
{
    return Game->mainInitialize(_argc, _argv);
}


@end

#pragma mark ---- Platform Namespace Functions ----

#pragma mark ---- Init funcs  ----
//------------------------------------------------------------------------------
void Platform::init()
{
   Con::setVariable("$platform", "iOS");
   
   if ([[UIScreen mainScreen] scale] == 2)
      Con::setIntVariable("$pref::iOS::RetinaEnabled", 2);
   else
      Con::setIntVariable("$pref::iOS::RetinaEnabled", 1);
   
   // Set the platform variable for the scripts
   if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
   {
      Con::setIntVariable("$pref::iOS::DeviceType", 1);
   }
   else
   {
      F32 screenHeight = [[UIScreen mainScreen] bounds].size.height;
      bool iPhone5 = (fabs((double)screenHeight - (double)568 ) < DBL_EPSILON);
      if (iPhone5)
      {
         Con::setIntVariable("$pref::iOS::DeviceType", 2);
         Con::setIntVariable("$pref::iOS::RetinaEnabled", 1);
      }
      else
      {
         Con::setIntVariable("$pref::iOS::DeviceType", 0);
      }
   }
   
   //    iOSConsole::create();
   Input::init();
   //    Video::init();
   Con::printf("");
}


//-----------------------------------------------------------------------------
// processing
void Platform::process()
{
}

//------------------------------------------------------------------------------
void Platform::shutdown()
{
   //    setMouseLock(false);
   //    Video::destroy();
   Input::destroy();
   //    iOSConsole::destroy();
   
}

//-----------------------------------------------------------------------------
// Completely closes and restarts the simulation
void Platform::restartInstance()
{
    // Not allowed in iOS
}

//-----------------------------------------------------------------------------
// Starts the quit process for the main Game instance
void Platform::postQuitMessage(const U32 in_quitVal)
{
//    Event quitEvent;
//    quitEvent.type = QuitEventType;
//    
//    Game->postEvent(quitEvent);
}

//-----------------------------------------------------------------------------
// Kicks off the termination process for the NSApp
void Platform::forceShutdown(S32 returnValue)
{
    // Note: AppCode states terminate is deprecated, which is not true
//    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

//-----------------------------------------------------------------------------
void Platform::debugBreak()
{
    raise(SIGTRAP);
}

//-----------------------------------------------------------------------------

void Platform::outputDebugString(const char *string)
{
    fprintf(stderr, string, NULL );
    fprintf(stderr, "\n" );
    fflush(stderr);
}

bool Platform::openWebBrowser(const char *webAddress)
{
   NSString *string = [[NSString alloc] initWithUTF8String:webAddress];
   NSURL *url = [[NSURL alloc] initWithString:string];
   bool ret = [[UIApplication sharedApplication] openURL:url];
   
   return ret;// this bails on the application, switching to Safari
}

bool isStatusBarHidden()
{
   // Get the shared iOS platform state
   return [UIApplication sharedApplication].statusBarHidden;
}

bool setStatusBarHidden(bool hidden)
{
   if (hidden)
   {
       [UIApplication sharedApplication].statusBarHidden = YES;
      gStatusBarHidden = true;
      
      return true;
   }
   else
   {
       [UIApplication sharedApplication].statusBarHidden = NO;
      gStatusBarHidden = false;
      
      return false;
   }
}

void setStatusBarType(S32 type)
{
   switch (type)
   {
      case 0: //Hidden
         setStatusBarHidden(true);
         break;
      case 1: //Black Opaque
         [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
         setStatusBarHidden(false);
         break;
      case 2: //Black Transparent
          [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
         setStatusBarHidden(false);
         break;
      default:
          [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleDefault;
   }
   
   gStatusBarType = type;
}



ConsoleFunction(getStatusBarHidden, bool, 1, 1, " Checks whether the status bar is hidden\n"
                "@return Returns true if hidden and false if not"){
   return isStatusBarHidden();
}

ConsoleFunction(setStatusBarHidden, bool, 2, 2, " Hides/unhides the iOS status bar \n"
                "@return true == status bar is hidden, false == status bar is visible"){
   return setStatusBarHidden(dAtob(argv[1]));
}

ConsoleFunction(setStatusBarType, void, 2, 2, " Set the status bar type. 0 hidden, 1 Black Opaque, 2 Black Translucent \n"){
   return setStatusBarType(dAtoi(argv[1]));
}

