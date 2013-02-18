//
//  T2DIAPHelper.m
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//

//#import "HMContentController.h"
#import "JSNotifier.h"
#import <StoreKit/StoreKit.h>

#import "IAPProduct.h"
#import "T2DIAPHelper.h"
#include "platformiOS/T2DAppDelegate.h"
#include "moduleManager.h"


@implementation T2DIAPHelper


+ (T2DIAPHelper *)sharedInstance {
    static dispatch_once_t once;
    static T2DIAPHelper * sharedInstance;
    dispatch_once(&once, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

- (void)notifyStatusForProduct:(IAPProduct *)product string:(NSString *)string {
    NSString *message = [ NSString stringWithFormat:@"%@: %@", product.skProduct.localizedTitle, string];
    JSNotifier *notify = [[JSNotifier alloc] initWithTitle:message];
    [notify showFor:2.0];
}

- (void)provideContentWithString:(NSString *)string {
    ModuleDatabase.scanModules([string UTF8String]);
}


@end
