//
//  IAPProduct.h
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//

#import <Foundation/Foundation.h>

@class SKProduct;
@class SKDownload;
@class IAPProductInfo;
@class IAPProductPurchase;

@interface IAPProduct : NSObject

- (id)initWithProductIdentifier:(NSString *)productIdentifier;
- (BOOL) allowedToPurchase;

@property (nonatomic, assign) BOOL availableForPurchase;
@property (nonatomic, assign) BOOL purchaseInProgress;
@property (nonatomic, strong) IAPProductPurchase * purchase;
@property (nonatomic, strong) NSString * productIdentifier;
@property (nonatomic, strong) SKProduct * skProduct;
@property (nonatomic, strong) IAPProductInfo * info;
@property (nonatomic, strong) SKDownload * skDownload;
@property (nonatomic, assign) float progress;
@end
