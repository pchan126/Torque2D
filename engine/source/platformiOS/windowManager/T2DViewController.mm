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

#import "./T2DViewController.h"
#import "platformiOS/platformGL.h"
#import "platformiOS/graphics/gfxOpenGLESDevice.h"
#import "game/gameInterface.h"
#import "platformiOS/windowManager/iOSWindow.h"

#define USE_DEPTH_BUFFER 0

extern bool retinaEnabled;

extern void ConvertToRetina (CGPoint *p);

extern bool _iOSTorqueFatalError;
//extern int _iOSRunTorqueMain( id appID,  UIView *Window, T2DViewController *Controller );

//-Mat we should update the accelereometer once per frame
U32  AccelerometerUpdateMS;
extern void _iOSGameInnerLoop();

@implementation T2DViewController

@synthesize context = _context;
@synthesize connectionData = _connectionData;
@synthesize connection = _connection;
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        mOrientationPortraitUpsideDownSupported = YES;
        mOrientationPortraitSupported = YES;
        mOrientationLandscapeRightSupported = YES;
        mOrientationLandscapeLeftSupported = YES;
    }
    return self;
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle


-(void)swapBuffers {
	if( isLayedOut == true ) {
		[self.context presentRenderbuffer:GL_RENDERBUFFER];
	}
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    
//    GFXOpenGLESDevice* device = dynamic_cast<GFXOpenGLESDevice*>(GFX);
//    EAGLContext* mainContext = device->getEAGLContext();
    
//    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:[mainContext sharegroup]];

    iOSPlatState *platState = [iOSPlatState sharedPlatState];
    platState.viewController = self;
    
    T2DView *view = (T2DView *) self.view;
//    view.context = self.context;
    
	if( AccelerometerUpdateMS <= 0 ) {
        //Luma:	This variable needs to be store MS value, not Seconds value
        AccelerometerUpdateMS = 33; // 33 ms
	}
	
	//Luma: Do division by 1000.0f here to get the seconds value that the UIAccelerometer needs
	//[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(AccelerometerUpdateMS / 1000.0f)];//this value is in seconds
	//[[UIAccelerometer sharedAccelerometer] setDelegate:self];
    	
//	[self createFramebuffer];
    view.isLayedOut = true;
    
    //by default, we are in portrait(upright) mode
	view.currentAngle = (M_PI / 2.0);
    view.delegate = self;
    
    platState.multipleTouchesEnabled = true;
    [self.view setMultipleTouchEnabled:YES];
    
    retinaEnabled = false;
    
    if([[UIScreen mainScreen] respondsToSelector:@selector(scale)] && [[UIScreen mainScreen] scale] == 2)
        retinaEnabled = true;
    
    UIApplication * application = [UIApplication sharedApplication];
   	
   _iOSTorqueFatalError = false;
}


- (void)dealloc
{
   [[NSNotificationCenter defaultCenter] removeObserver:self
                                                   name:UIScreenDidConnectNotification
                                                 object:nil];
   
	[[NSNotificationCenter defaultCenter] removeObserver:self
                                                   name:UIScreenDidDisconnectNotification
                                                 object:nil];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    self.context = nil;
}

- (void)update
{
//    EAGLContext *ctx1 = self.context;
//    GLKView *view = (T2DView *) self.view;
//    EAGLContext *ctx2 = [view context];
    
    if(Game->isRunning())
    {
        Game->mainLoop();
    }
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    
    switch (interfaceOrientation)
    {
        case UIDeviceOrientationPortraitUpsideDown:
            return mOrientationPortraitUpsideDownSupported;
            break;
            
        case UIDeviceOrientationPortrait:
            return mOrientationPortraitSupported;
            break;
            
        case UIDeviceOrientationLandscapeRight:
            return mOrientationLandscapeRightSupported;
            break;
            
        case UIDeviceOrientationLandscapeLeft:
            return mOrientationLandscapeLeftSupported;
            break;
            
        default:
            break;
    }
    
    return NO;
}


- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation
{
    return UIInterfaceOrientationLandscapeRight;
}

void supportLandscape( bool enable)
{
    iOSPlatState *platState = [iOSPlatState sharedPlatState];
    platState.viewController->mOrientationLandscapeLeftSupported = enable;
    platState.viewController->mOrientationLandscapeRightSupported = enable;
}

ConsoleFunction(supportLandscape, void, 2, 2, "supportLandscape( bool ) "
                "enable or disable Landscape")
{
    bool enable = true;
    if( argc > 1 )
        enable = dAtob( argv[1] );
    
    supportLandscape(enable);
}

void supportPortrait( bool enable )
{
    iOSPlatState *platState = [iOSPlatState sharedPlatState];
    platState.viewController->mOrientationPortraitSupported = enable;
    platState.viewController->mOrientationPortraitUpsideDownSupported = enable;
}

ConsoleFunction(supportPortrait, void, 2, 2, "supportPortrait( bool ) "
                "enable or disable portrait")
{
    bool enable = true;
    if( argc > 1 )
        enable = dAtob( argv[1] );
    
    supportPortrait(enable);
}


- (void)viewDidLayoutSubviews
{
   
//    [self destroyFramebuffer];
//	[self createFramebuffer];
}

@end
