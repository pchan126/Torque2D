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

// Mich Note: Do not try to spend too much time analyzing this class.
// It is planned to refactor the iOS platform layer and replace this
// with a platform state similar to what OS X uses.
@interface iOSPlatState : NSObject
{
    // Number of arguments passed into this application
    U32 _argc;

    // Arguments passed into this application
    const char** _argv;

    // Location of the folder containing the main.cs
    NSString* _mainCSDirectory;
    
    // Threaded alert object
    void* _alertSemaphore;

    // Random generator
    RandomLCG*        _platformRandom;

    // Reports the quit state for the applications
    BOOL _quit;
};

@property RandomLCG* platformRandom;
@property U32 argc;
@property const char** argv;
@property U32 currentSimTime;
@property (nonatomic,strong) NSString* mainCSDirectory;
@property BOOL quit;

/// Global singleton that encapsulates a lot of mac platform state & globals.
+ (id)sharedPlatState;

- (BOOL)initializeTorque2D;

@end
