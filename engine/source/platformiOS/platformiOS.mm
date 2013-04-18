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

#include "platformiOS/graphics/gfxOpenGLESDevice.h"

#pragma mark ---- IOSPlatState Implementation ----

@interface iOSPlatState (PrivateMethods)
- (void)mainTorqueLoop:(NSTimer *)obj;
@end

@implementation iOSPlatState

@synthesize ctx = _ctx;
@synthesize window = _window;
//@synthesize torqueView = _torqueView;
//@synthesize cgDisplay = _cgDisplay;
@synthesize application = _application;
@synthesize alertSemaphore = _alertSemaphore;
@synthesize fullScreen = _fullscreen;
@synthesize argc = _argc;
@synthesize argv = _argv;
@synthesize platformRandom = _platformRandom;
@synthesize desktopBitsPixel = _desktopBitsPixel;
@synthesize desktopWidth = _desktopWidth;
@synthesize desktopHeight = _desktopHeight;
@synthesize currentSimTime = _currentSimTime;
@synthesize lastTimeTick = _lastTimeTick;
@synthesize sleepTicks = _sleepTicks;
@synthesize mainCSDirectory = _mainCSDirectory;
@synthesize backgrounded = _backgrounded;
@synthesize minimized = _minimized;
@synthesize mouseLocked = _mouseLocked;
@synthesize windowTitle = _windowTitle;
@synthesize quit = _quit;


static iOSPlatState * tempSharedPlatState = nil;

//bool setScreenOrientation(bool, bool);
bool getStatusBarHidden();
bool setStatusBarHidden(bool);
void setStatusBarType(S32);

//Hidden by Default. 1 Black Opaque, 2 Black Translucent
S32 gStatusBarType = 0;
bool gStatusBarHidden = true;

//Landscape by default. 0 Landscape, 1 Portrait
S32 gScreenOrientation = 0;
bool gScreenUpsideDown = true;

//-----------------------------------------------------------------------------

- (id)init
{
    self = [super init];
    
    if (self)
    {
        // Default window behaviors
        _backgrounded = false;
        _minimized = false;
        _mouseLocked = true;
        _fullscreen = false;
        _quit = false;
        
        _application = [UIApplication sharedApplication];
        
        // Default window resolution
        _desktopBitsPixel = 32;
        _desktopWidth = 1024;
        _desktopHeight = 768;
        _windowSize.x = 1024;
        _windowSize.y = 768;
        
        _windowTitle = @"Torque 2D IOS";
        
        // Default window
        _window = nil;
        _ctx = nil;
        
        // Default system variables
        _currentSimTime = 0;
        _sleepTicks = 0;
        _lastTimeTick = 0;
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

-(void) updateWindowTitle:(const char*)title
{
    _windowTitle = [NSString stringWithFormat:@"%s", title];
}

//-----------------------------------------------------------------------------

- (void)setWindowSize:(int)width height:(int)height
{
//    // Store the width and height in the state
//    _windowSize.x = width;
//    _windowSize.y = height;
//    
//    // Get the window's current frame
//    CGRect frame = NSMakeRect([_window frame].origin.x, [_window frame].origin.y, width, height);
//    
//    // Get the starting position of the bar height
//    F32 barOffset = frame.size.height;
//    
//    // If we are not going to full screen mode, get a new frame offset that accounts
//    // for the title bar height
//    if (!_fullscreen)
//    {
//        frame = [NSWindow frameRectForContentRect:frame styleMask:NSTitledWindowMask];
//        
//        // Set the new window frame
//        [_window setFrame:frame display:YES];
//        
//        // Get the new position of the title bar
//        barOffset -= frame.size.height;
//    }
//    else
//    {
//        // Otherwise, just go straight full screen
//        [_window toggleFullScreen:self];
//    }
//    
//    // Update the frame of the torqueView to match the window
//    frame = NSMakeRect([_window frame].origin.x, [_window frame].origin.y, width, height);
//    CGRect viewFrame = NSMakeRect(0, 0, frame.size.width, frame.size.height);
//    
//    [_torqueView setFrame:viewFrame];
//    
//    [_torqueView updateContext];
//    
//    [_window makeKeyAndOrderFront:NSApp];
//    [_window makeFirstResponder:_torqueView];
}


#pragma mark ---- Singleton Functions ----

//-----------------------------------------------------------------------------

+ (id)sharedPlatState
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

//-----------------------------------------------------------------------------
// Runs the main Game instance and any other looping calls you need to perform
- (void) mainTorqueLoop:(NSTimer *)obj
{
//    if(Game->isRunning())
//    {
//        Game->mainLoop();
//    }
//    else
//    {
//		Game->mainShutdown();
//        
//		// Need to actually exit the application now
//		exit(0);
//    }
}

//-----------------------------------------------------------------------------
// Entry function for running Torque 2D
- (void) runTorque2D
{
    BOOL initialResult = [self initializeTorque2D];
    
    if (initialResult)
    {
        // Setting the interval to 1ms to handle Box2D and rendering        
        _iOSTimer = [NSTimer scheduledTimerWithTimeInterval:0.001 target:self selector:@selector(mainTorqueLoop:) userInfo:self repeats:YES];
    }
    else
    {
		Game->mainShutdown();
        
		// Need to actually exit the application now
		exit(0);
    }
}

//-----------------------------------------------------------------------------
// Shut down the main Game instancea nd perform any additional cleanup
- (void) shutDownTorque2D
{
    // Shutdown the game
    Game->mainShutdown();
    
    // Perform any platform cleanup
}

@end

#pragma mark ---- Platform Namespace Functions ----

#pragma mark ---- Init funcs  ----
//------------------------------------------------------------------------------
void Platform::init()
{
   Con::setVariable("$platform", "iOS");
   
   if ([[UIScreen mainScreen] scale] == 2)
      Con::setBoolVariable("$pref::iOS::RetinaEnabled", true);
   else
      Con::setBoolVariable("$pref::iOS::RetinaEnabled", false);
   
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
         Con::setBoolVariable("$pref::iOS::RetinaEnabled", false);
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
    Event quitEvent;
    quitEvent.type = QuitEventType;
    
    Game->postEvent(quitEvent);
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
   bool ret = [[[iOSPlatState sharedPlatState] application] openURL:url];
   
   return ret;// this bails on the application, switching to Safari
}

bool isStatusBarHidden()
{
   // Get the shared iOS platform state
   iOSPlatState * platState = [iOSPlatState sharedPlatState];
   if ([platState application].statusBarHidden == YES)
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool setStatusBarHidden(bool hidden)
{
   // Get the shared iOS platform state
   iOSPlatState * platState = [iOSPlatState sharedPlatState];
   
   if (hidden)
   {
      [platState application].statusBarHidden = YES;
      gStatusBarHidden = true;
      
      return true;
   }
   else
   {
      [platState application].statusBarHidden = NO;
      gStatusBarHidden = false;
      
      return false;
   }
}

void setStatusBarType(S32 type)
{
   // Get the shared iOS platform state
   iOSPlatState * platState = [iOSPlatState sharedPlatState];
   
   switch (type)
   {
      case 0: //Hidden
         setStatusBarHidden(true);
         break;
      case 1: //Black Opaque
         [platState application].statusBarStyle = UIStatusBarStyleBlackOpaque;
         setStatusBarHidden(false);
         break;
      case 2: //Black Transparent
         [platState application].statusBarStyle = UIStatusBarStyleBlackTranslucent;
         setStatusBarHidden(false);
         break;
      default:
         [platState application].statusBarStyle = UIStatusBarStyleDefault;
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

