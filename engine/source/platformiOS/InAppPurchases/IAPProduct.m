
//
//  IAPProduct.m
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//

#import "IAPProduct.h"
#import "IAPProductInfo.h"

@implementation IAPProduct

- (id)initWithProductIdentifier:(NSString *)productIdentifier {
    if ((self = [super init])) {
        self.availableForPurchase = NO;
        self.productIdentifier = productIdentifier;
        self.skProduct = nil;
    }
    return self;
}

- (BOOL) allowedToPurchase {
    if (!self.availableForPurchase) return NO;
    if (self.purchaseInProgress) return NO;
    if (!self.info) return NO;
    if (!self.info.consumable && self.purchase) return NO;
    return YES;
}

@end
