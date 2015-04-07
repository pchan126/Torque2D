//
//  T2DViewController.m
//  Torque2D
//
//  Created by Paul L Jan on 2013-06-25.
//

#import "T2DViewController.h"
#include "platformiOS/graphics/gfxiOSDevice.h"

@interface T2DViewController ()

@end

@implementation T2DViewController

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration {
   
   GFXiOSDevice* device = dynamic_cast<GFXiOSDevice*>(GFX);

   if (device)
      device->refreshCIContext();
   }

- (void) applicationDidBecomeActive {
   GFXiOSDevice* device = dynamic_cast<GFXiOSDevice*>(GFX);
   
   if (device)
      device->refreshCIContext();
   }

- (BOOL)prefersStatusBarHidden {
   return YES;
}


@end
