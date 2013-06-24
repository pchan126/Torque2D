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
#include "./graphics/GFXOpenGLES20iOSDevice.h"
#include "./windowManager/iOSWindow.h"

// Store current orientation for easy access
extern void _iOSGameChangeOrientation(S32 newOrientation);
extern Vector<StringTableEntry> gFilterIndexNames;

UIDeviceOrientation currentOrientation;

bool _iOSTorqueFatalError = false;

@implementation T2DAppDelegate

@synthesize window = _window;
@synthesize mainController = _mainController;
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
    
    iOSPlatState * platState = [iOSPlatState sharedPlatState];
    
//	NSArray *categoryNames = @[ kCICategoryBuiltIn ];
//    NSArray* filterNames = [CIFilter filterNamesInCategories:categoryNames];
   
//    for (NSString *temp in filterNames)
//    {
//        gFilterIndexNames.push_back(StringTable->insert([temp UTF8String]));
//    }
   
    BOOL initialResult = [platState initializeTorque2D];

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
    
//	if (screenCount > 1)
//   {
//       winManager
      //      // 2. If an external screen is available, get the screen object and look at the values in its availableModes
      //      // property. This property contains the configurations supported by the screen.
      //
      //      // Select first external screen
//		self.extScreen = [screens objectAtIndex:1]; //index 0 is your iPhone/iPad
//		NSArray	*availableModes = [self.extScreen availableModes];
      //
      //      // 3. Select the UIScreenMode object corresponding to the desired resolution and assign it to the currentMode
      //      // property of the screen object.
      //
      //      // Select the highest resolution in this sample
      //      NSInteger selectedRow = [availableModes count] - 1;
      //      self.extScreen.currentMode = [availableModes objectAtIndex:selectedRow];
      //
      //      // Set a proper overscanCompensation mode
      //      self.extScreen.overscanCompensation = UIScreenOverscanCompensationInsetApplicationFrame;
      //
      //      if (self.extWindow == nil) {
      //         // 4. Create a new window object (UIWindow) to display your content.
      //         UIWindow *extWindow = [[UIWindow alloc] initWithFrame:[self.extScreen bounds]];
      //         self.extWindow = extWindow;
      //      }
      //
      //      // 5. Assign the screen object to the screen property of your new window.
      //      self.extWindow.screen = self.extScreen;
      //
      //      // 6. Configure the window (by adding views or setting up your OpenGL ES rendering context).
      //
      //      // Resize the GL view to fit the external screen
      //      self.glController.view.frame = self.extWindow.frame;
      //
      //      // Set the target screen to the external screen
      //      // This will let the GL view create a CADisplayLink that fires at the native fps of the target display.
      //      [(GLViewController *)self.glController setTargetScreen:self.extScreen];
      //
      //      // Configure user interface
      //      // In this sample, we use the same UI layout when an external display is connected or not.
      //      // In your real application, you probably want to provide distinct UI layouts for best user experience.
      //      [(GLViewController *)self.glController screenDidConnect:self.userInterfaceController];
      //
      //      // Add the GL view
      //      [self.extWindow addSubview:self.glController.view];
      //
      //      // 7. Show the window.
      //      [self.extWindow makeKeyAndVisible];
      //
      //      // On the iPhone/iPad screen
      //      // Remove the GL view (it is displayed on the external screen)
      //      for (UIView* v in [self.view subviews])
      //         [v removeFromSuperview];
      //
      //      // Display the fullscreen UI on the iPhone/iPad screen
      //      [self.view addSubview:self.userInterfaceController.view];
//	}
//	else //handles disconnection of the external display
//   {
      //      // Release external screen and window
      //		self.extScreen = nil;
      //		self.extWindow = nil;
      //
      //      // On the iPhone/iPad screen
      //      // Remove the fullscreen UI (a window version will be displayed atop the GL view)
      //      for (UIView* v in [self.view subviews])
      //         [v removeFromSuperview];
      //
      //      // Resize the GL view to fit the iPhone/iPad screen
      //      self.glController.view.frame = self.view.frame;
      //
      //      // Set the target screen to the main screen
      //      // This will let the GL view create a CADisplayLink that fires at the native fps of the target display.
      //      [(GLViewController *)self.glController setTargetScreen:[UIScreen mainScreen]];
      //
      //      // Configure user interface
      //      // In this sample, we use the same UI layout when an external display is connected or not.
      //      // In your real application, you probably want to provide distinct UI layouts for best user experience.
      //      [(GLViewController *)self.glController screenDidDisconnect:self.userInterfaceController];
      //      
      //      // Display the GL view on the iPhone/iPad screen
      //      [self.view addSubview:self.glController.view];
//	}
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
    VectorPtr<PlatformWindow*> windows;
    WindowManager->getWindows(windows);
    for (iOSWindowManager::WindowList::iterator itr = (iOSWindowManager::WindowList::iterator)windows.begin(); itr != (iOSWindowManager::WindowList::iterator)windows.end(); ++itr)
    {
        if ((*itr)->view == view)
        {
            (*itr)->displayEvent.trigger((*itr)->getWindowId());
            return;
        }
    }
    NSLog(@"%@", @"ack");
}


@end
