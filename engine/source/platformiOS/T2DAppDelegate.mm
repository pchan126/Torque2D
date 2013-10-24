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

#import "platformiOS/T2DAppDelegate.h"

#import "platformiOS/platformiOS.h"
#include "platform/platformInput.h"
#include "platformiOS/iOSUtil.h"
#include "console/console.h"
#include "game/gameInterface.h"
#include "graphics/gfxInit.h"
#include "./graphics/ES20/GFXOpenGLES20iOSDevice.h"
#include "./windowManager/iOSWindow.h"

// Store current orientation for easy access
extern void _iOSGameChangeOrientation(S32 newOrientation);
extern Vector<StringTableEntry> gFilterIndexNames;

UIDeviceOrientation currentOrientation;

bool _iOSTorqueFatalError = false;

@implementation T2DAppDelegate

@synthesize window = _window;
@synthesize mainController = _mainController;
@synthesize extController = _extController;
@synthesize lastUpdate;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	// Register for screen connect and disconnect notifications.
	[[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(screenDidChange:)
                                                name:UIScreenDidConnectNotification
                                              object:nil];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(screenDidChange:)
                                                name:UIScreenDidDisconnectNotification
                                              object:nil];
    
//	NSArray *categoryNames = @[ kCICategoryBuiltIn ];
//    NSArray* filterNames = [CIFilter filterNamesInCategories:categoryNames];
   
//    for (NSString *temp in filterNames)
//    {
//        gFilterIndexNames.push_back(StringTable->insert([temp UTF8String]));
//    }
   
    BOOL initialResult = [[iOSPlatState sharedPlatState] initializeTorque2D];

//    for (int i = 0; i < gFilterIndexNames.size(); i++)
//    {
//        Con::printf("Filter: %s", gFilterIndexNames[i]);
//    }

    if (!initialResult)
   {
		Game->mainShutdown();
		exit(0);
   }
    return TRUE;
}

- (void)applicationDidReceiveMemoryWarning
{
    if ( Con::isFunction("oniOSMemoryWarning") )
        Con::executef( 1, "oniOSMemoryWarning" );
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    if ( Con::isFunction("oniOSResignActive") )
        Con::executef( 1, "oniOSResignActive" );
    //
    //    appIsRunning = false;
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if(!_iOSTorqueFatalError)
    {
        if ( Con::isFunction("oniOSBecomeActive") )
            Con::executef( 1, "oniOSBecomeActive" );
    }
}

- (void)applicationWillTerminate:(UIApplication *)application
{  
    if ( Con::isFunction("oniOSWillTerminate") )
        Con::executef( 1, "oniOSWillTerminate" );

	Con::executef( 1, "onExit" );

	Game->mainShutdown();
    
	[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
}

- (BOOL) canBecomeFirstResponder {return YES;}


- (void)screenDidChange:(NSNotification *)notification
{
   iOSWindowManager *winManager = dynamic_cast<iOSWindowManager*>(WindowManager);
    
    if (!winManager)
    {
        Con::warnf("missing iOSWindowManager!");
        return;
    }
    
   
   if ( Con::isFunction("oniOSScreenDidChange") )
      Con::executef( 1, "oniOSScreenDidChange" );

    GFXInit::enumerateAdapters();

    winManager->updateWindows();
}


#pragma mark - GLKView and GLKViewController delegate methods

- (void)glkViewControllerUpdate:(GLKViewController *)controller
{
    lastUpdate = [controller timeSinceLastUpdate];
    if(Game->isRunning())
    {
        Game->mainLoop();
    }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    Vector<PlatformWindow*> windows;
    WindowManager->getWindows(windows);
    for (PlatformWindow* itr: windows)
    {
        iOSWindow* temp = dynamic_cast<iOSWindow*>(itr);
        if (temp->view == view)
        {
            temp->displayEvent.trigger(temp->getWindowId());
            return;
        }
    }
    NSLog(@"%@", @"ack");
}


@end
