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

#import <UIKit/UIKit.h>

#import "platform/platform.h"
#import "math/mMath.h"
#import "platformiOS/iOSEvents.h"

// Mich Note: Do not try to spend too much time analyzing this class.
// It is planned to refactor the iOS platform layer and replace this
// with a platform state similar to what OS X uses.
@interface iOSPlatState : NSObject
{
    UIView*			  _window;

    // Process ID for this application instance
    UIApplication*    _application;

    EAGLContext*      _ctx;
    
    bool              ctxNeedsUpdate;

    U32               currentTime;
    bool				 fullscreen;

    // Version of operating system
    U32 osVersion;

    // Number of arguments passed into this application
    U32 _argc;

    // Arguments passed into this application
    const char** _argv;

    U32 _lastTimeTick;
    
    U32 _sleepTicks;
    
    // Location of the folder containing the main.cs
    NSString* _mainCSDirectory;
    
    // Threaded alert object
    void* _alertSemaphore;
    S32               alertHit;

    // Random generator
    RandomLCG*        _platformRandom;

    // Used to report is mouse is locked to the main window or not
    BOOL _mouseLocked;
    
    // Used to report if the window has been pushed to the background or not
    BOOL _backgrounded;
    
    // Use to report if the window has been minimized or not
    BOOL _minimized;

    U32               appReturn;

    NSTimeInterval	 timerInterval;
    UIApplication	*application;

    // Reports the quit state for the applications
    BOOL _quit;

    // Timer
    NSTimer* _iOSTimer;
};

@property (strong) EAGLContext* ctx;
@property (strong) UIView* window;
@property (strong) UIApplication* application;
@property void* alertSemaphore;
@property RandomLCG* platformRandom;
@property BOOL fullScreen;
@property U32 argc;
@property const char** argv;
@property U32 currentSimTime;
@property U32 lastTimeTick;
@property U32 sleepTicks;
@property (nonatomic,strong) NSString* mainCSDirectory;
@property (nonatomic,strong) NSString* windowTitle;
@property BOOL mouseLocked;
@property BOOL backgrounded;
@property BOOL minimized;
@property BOOL quit;

/// Global singleton that encapsulates a lot of mac platform state & globals.
+ (id)sharedPlatState;

- (BOOL)initializeTorque2D;
- (void)shutDownTorque2D;

- (void)updateWindowTitle:(const char*)title;

@end