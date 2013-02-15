//
//  IAPHelper.h
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//

#import <Foundation/Foundation.h>
#import "IAPProductInfo.h"

@class IAPProduct;

typedef void (^RequestProductsCompletionHandler)
(BOOL success, NSArray * products);

@interface IAPHelper: NSObject

@property (nonatomic, strong) NSMutableDictionary * products;

- (void) requestProductsWithCompletionHandler:(RequestProductsCompletionHandler)completionHandler;
- (void) buyProduct:(IAPProduct * )product;
- (void)restoreCompletedTransactions;
- (void)pauseDownloads:(NSArray *)downloads;
- (void)resumeDownloads:(NSArray *)downloads;
- (void)cancelDownloads:(NSArray *)downloads;

@end
