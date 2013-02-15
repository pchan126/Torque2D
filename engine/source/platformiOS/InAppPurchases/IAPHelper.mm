//
//  IAPHelper.m
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//
#import "IAPProduct.h"
#import "IAPHelper.h"
#import "IAPProductPurchase.h"
#import <StoreKit/StoreKit.h>
#import "AFNetworking.h"
#import "AFHTTPClient.h"
#import "AFHTTPRequestOperation.h"
#import "NSData+Base64.h"
#include "console/console.h"

static NSString *const IAPServerBaseURL = @"http://xxxxxxx.xxxxxxxxx.xxx/xxxxxxxx/";
static NSString *const IAPServerProductsURL = @"productInfos.plist";
static NSString *const IAPServerVerifyURL = @"validate.php";
static NSString *const IAPHelperPurchasesPlist = @"purchases.plist";

@interface IAPHelper () <SKProductsRequestDelegate, SKPaymentTransactionObserver>
@end

@implementation IAPHelper {
    SKProductsRequest * _productsRequest;
    RequestProductsCompletionHandler _completionHandler;
    BOOL _productsLoaded;
}

- (id)init {
    if ((self = [super init])) {
        _products = [NSMutableDictionary dictionary];
        [self loadPurchases];
        [self loadProductsWithCompletionHandler:^(BOOL success, NSError *error) {
        }];
    }
    return self;
}

- (IAPProduct*) addProductForProductIdentifier: (NSString *)productIdentifier {
    IAPProduct * product = _products[productIdentifier];
    if (product == nil) {
        product = [[IAPProduct alloc] initWithProductIdentifier:productIdentifier];
        _products[productIdentifier] = product;
    }
    return product;
}


- (void)addInfo:(IAPProductInfo *)info forProductIdentifier:(NSString *)productIdentifier {
    IAPProduct * product = [self addProductForProductIdentifier:productIdentifier];
    product.info = info;
}


- (void) buyProduct:(IAPProduct *)product {
    NSAssert(product.allowedToPurchase, @"This product isn't allowed to be purchased!");
    
    NSLog(@"Buying %@...", product.productIdentifier);
    
    product.purchaseInProgress = YES;
    SKPayment * payment = [SKPayment paymentWithProduct:product.skProduct];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

- (void) requestProductsWithCompletionHandler:(RequestProductsCompletionHandler)completionHandler
{
    _completionHandler = [completionHandler copy];
    
    [self loadProductsWithCompletionHandler:^(BOOL success, NSError *error) {
        NSMutableSet * productIdentifiers = [NSMutableSet setWithCapacity:_products.count];
        for (IAPProduct * product in _products.allValues) {
            if (product.info) {
                product.availableForPurchase = NO;
                [productIdentifiers addObject:product.productIdentifier];
            }
        }
        
        _productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
        _productsRequest.delegate = self;
        [_productsRequest start];
    }];
}

#pragma mark - SKProductsRequestDelegate

- (void) productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    NSLog(@"Loaded list of products...");
    _productsRequest = nil;
    
    NSArray * skProducts = response.products;
    for(SKProduct * skProduct in skProducts) {
        IAPProduct * product =
        _products[skProduct.productIdentifier];
        product.skProduct = skProduct;
        product.availableForPurchase = YES;
    }
    
    for (NSString * invalidProductIdentifier in
         response.invalidProductIdentifiers)
    {
        IAPProduct * product = _products[invalidProductIdentifier];
        product.availableForPurchase = NO;
        
        NSLog(@"invalid product identifier, removing: %@", invalidProductIdentifier);
    }
    
    NSMutableArray * availableProducts = [NSMutableArray array];
    for (IAPProduct * product in _products.allValues)
    {
        if (product.availableForPurchase) {
            [availableProducts addObject:product];
        }
    }
    
    _completionHandler(YES, availableProducts);
    _completionHandler = nil;
}

-(void)request:(SKRequest *)request didFailWithError:(NSError *)error {
    NSLog(@"Failed to load list of products.");
    _productsRequest = nil;
    
    _completionHandler(FALSE, nil);
    _completionHandler = nil;
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    for (SKPaymentTransaction * transaction in transactions) {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                [self restoreTransaction:transaction];
                break;
        }
    }
}

- (void)completeTransaction:(SKPaymentTransaction *)transaction {
    NSLog(@"completeTransaction...");
    [self validateReceiptForTransaction:transaction];
}

- (void)restoreTransaction:(SKPaymentTransaction *)transaction {
    NSLog(@"restoreTransaction...");
    [self validateReceiptForTransaction:transaction];
}

- (void)failedTransaction:(SKPaymentTransaction *)transaction {
    NSLog(@"failedTransaction...");
    if (transaction.error.code != SKErrorPaymentCancelled)
    {
        NSLog(@"Transaction error: %@",
              transaction.error.localizedDescription);
    }
    
    IAPProduct * product =
    _products[transaction.payment.productIdentifier];
    [self notifyStatusForProductIdentifier:
     transaction.payment.productIdentifier
                                    string:@"Purchase failed."];
    product.purchaseInProgress = NO;
    [[SKPaymentQueue defaultQueue]
     finishTransaction: transaction];
}

- (void)notifyStatusForProductIdentifier:(NSString *)productIdentifier string:(NSString *)string {
    IAPProduct *product = _products[productIdentifier];
    [self notifyStatusForProduct:product string:string];
}

- (void)notifyStatusForProduct:(IAPProduct *)product string:(NSString *)string {
    
}

- (void)provideContentForTransaction:
(SKPaymentTransaction *)transaction
                   productIdentifier:(NSString *)productIdentifier {
    
    IAPProduct * product = _products[productIdentifier];
    
    if (transaction.downloads) {
        
        product.skDownload = transaction.downloads[0];
        if (transaction.downloads.count > 1) {
            NSLog(@"Unexpected number of downloads!");
        }
        [[SKPaymentQueue defaultQueue]
         startDownloads:transaction.downloads];
        
    } else {
        
        // Put the code from before here
        IAPProduct * product = _products[productIdentifier];
        
        if (product.info.consumable) {
            [self
             purchaseConsumable:product.info.consumableIdentifier
             forProductIdentifier:productIdentifier
             amount:product.info.consumableAmount];
        } else {
            NSURL * bundleURL = [[NSURL alloc] initWithString:(product.info.bundleDir)];
            [self purchaseNonconsumableAtURL:bundleURL
                        forProductIdentifier:productIdentifier];
        }
        
        [self notifyStatusForProductIdentifier:productIdentifier
                                        string:@"Purchase complete!"];
        
        product.purchaseInProgress = NO;
        [[SKPaymentQueue defaultQueue] finishTransaction:
         transaction];
        
    }
    
}


- (void)purchaseConsumable:(NSString *)consumableIdentifier
      forProductIdentifier:(NSString *)productIdentifier
                    amount:(int)consumableAmount
{
    int previousAmount = [[NSUserDefaults standardUserDefaults] integerForKey:consumableIdentifier];
    int newAmount = previousAmount + consumableAmount;
    [[NSUserDefaults standardUserDefaults] setInteger:newAmount forKey:consumableIdentifier];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    // Let the scripts know.
    char* buffer = Con::getReturnBuffer(8);
    dItoa(newAmount, buffer );
    Con::executef(3, "updateIAPConsumeable", [consumableIdentifier UTF8String], buffer);
    
    IAPProductPurchase * previousPurchase = [self purchaseForProductIdentifier:productIdentifier];
    if (previousPurchase) {
        previousPurchase.timesPurchased++;
    } else {
        IAPProductPurchase * purchase = [[IAPProductPurchase alloc]
                                         initWithProductIdentifier:productIdentifier
                                         consumable:YES
                                         timesPurchased:1
                                         libraryRelativePath:@""
                                         contentVersion:@""];
        [self addPurchase:purchase forProductIdentifier:productIdentifier];
    }
    [self savePurchases];
}


- (void)provideContentWithString:(NSString *)string {
    
}


- (void)purchaseNonconsumableAtURL:(NSURL *)nonLocalURL
              forProductIdentifier:(NSString *)productIdentifier {
    
    NSError * error = nil;
    BOOL success = FALSE;
    BOOL exists = FALSE;
    BOOL isDirectory = FALSE;
    
    // 1
    NSString * libraryRelativePath = nonLocalURL.lastPathComponent;
    NSString * localPath = [[self libraryPath]
                            stringByAppendingPathComponent:libraryRelativePath];
    NSURL * localURL = [NSURL fileURLWithPath:localPath
                                  isDirectory:NO];
    exists = [[NSFileManager defaultManager]
              fileExistsAtPath:localPath isDirectory:&isDirectory];
    
    // 2
    if (exists) {
        BOOL success = [[NSFileManager defaultManager]
                        removeItemAtURL:localURL error:&error];
        if (!success) {
            NSLog(@"Couldn't delete directory at %@: %@",
                  localURL, error.localizedDescription);
        }
    }
    
    // 3
    NSLog(@"Copying directory from %@ to %@", nonLocalURL,
          localURL);

    success = [[NSFileManager defaultManager]
               copyItemAtURL:nonLocalURL toURL:localURL error:&error];
    if (!success) {
        NSLog(@"Failed to copy directory: %@",
              error.localizedDescription);
        [self notifyStatusForProductIdentifier:productIdentifier
                                        string:@"Copying failed."];
        return;
    }
    
    // 1
    NSString * contentVersion = @"";
    NSURL * contentInfoURL = [localURL
                              URLByAppendingPathComponent:@"ContentInfo.plist"];
    exists = [[NSFileManager defaultManager]
              fileExistsAtPath:contentInfoURL.path
              isDirectory:&isDirectory];
    if (exists) {
        // 2
        NSDictionary * contentInfo = [NSDictionary
                                      dictionaryWithContentsOfURL:contentInfoURL];
        contentVersion = contentInfo[@"ContentVersion"];
        NSString * contentsPath = [libraryRelativePath
                                   stringByAppendingPathComponent:@"Contents"];
        // 3
        NSString * fullContentsPath = [[self libraryPath]
                                       stringByAppendingPathComponent:contentsPath];
        if ([[NSFileManager defaultManager]
             fileExistsAtPath:fullContentsPath]) {
            libraryRelativePath = contentsPath;
            localPath = [[self libraryPath]
                         stringByAppendingPathComponent:libraryRelativePath];
            localURL = [NSURL fileURLWithPath:localPath
                                  isDirectory:YES];
        }
    }
    
    // 4
    [self provideContentWithString:localPath];
    
    // 5
    IAPProductPurchase * previousPurchase = [self purchaseForProductIdentifier:productIdentifier];
    if (previousPurchase) {
        previousPurchase.timesPurchased++;
        
        // 6
        NSString * oldPath = [[self libraryPath]
                              stringByAppendingPathComponent:
                              previousPurchase.libraryRelativePath];
        success = [[NSFileManager defaultManager]
                   removeItemAtPath:oldPath error:&error];
        if (!success) {
            NSLog(@"Could not remove old purchase at %@",
                  oldPath);
        } else {
            NSLog(@"Removed old purchase at %@", oldPath);
        }
        
        // 7
        previousPurchase.libraryRelativePath = libraryRelativePath;
        previousPurchase.contentVersion = contentVersion;
    } else {
        IAPProductPurchase * purchase = [[IAPProductPurchase alloc]  initWithProductIdentifier:productIdentifier
                                                                     consumable:NO
                                                                     timesPurchased:1
                                                                     libraryRelativePath:libraryRelativePath
                                                                     contentVersion:contentVersion];
        [self addPurchase:purchase forProductIdentifier:productIdentifier];
    }
    
    [self notifyStatusForProductIdentifier:productIdentifier
                                    string:@"Purchase complete!"];
    
    // 8
    [self savePurchases];
    
}

- (void)restoreCompletedTransactions {
    [[SKPaymentQueue defaultQueue]
     restoreCompletedTransactions];
}

- (NSString *)libraryPath {
    NSArray * libraryPaths =
    NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    return libraryPaths[0];
}

- (NSString *)purchasesPath {
    return [[self libraryPath]
            stringByAppendingPathComponent:IAPHelperPurchasesPlist];
}

- (void)addPurchase:(IAPProductPurchase*)purchase
forProductIdentifier:(NSString*)productIdentifier {
    IAPProduct * product = [self addProductForProductIdentifier:productIdentifier];
    product.purchase = purchase;
}

- (IAPProductPurchase *)purchaseForProductIdentifier:
(NSString*)productIdentifier {
    IAPProduct* product = _products[productIdentifier];
    if (!product) return nil;
    
    return product.purchase;
}

- (void)loadPurchases {
    NSString * purchasesPath = [self purchasesPath];
    NSArray * purchasesArray = [NSKeyedUnarchiver unarchiveObjectWithFile:purchasesPath];
    for (IAPProductPurchase * purchase in purchasesArray) {
        if (purchase.libraryRelativePath) {
            NSString * localPath = [[self libraryPath]
                                    stringByAppendingPathComponent:purchase.libraryRelativePath];
            [self provideContentWithString:localPath];
        }
        [self addPurchase:purchase forProductIdentifier:purchase.productIdentifier];
        NSLog(@"Loaded purchase for %@ (%@)",
              purchase.productIdentifier, purchase.contentVersion);
    }
}


- (void)savePurchases {
    NSString * purchasesPath = [self purchasesPath];
    NSMutableArray *purchasesArray = [NSMutableArray array];
    for (IAPProduct * product in _products.allValues) {
        if (product.purchase) {
            [purchasesArray addObject:product.purchase];
        }
    }
    
    BOOL success = [NSKeyedArchiver
                    archiveRootObject:purchasesArray toFile:purchasesPath];
    if (!success)
        NSLog(@"Failed to save purchases to %@", purchasesPath);
}

- (void)loadProductsWithCompletionHandler:(void (^)( BOOL success, NSError * error))completionHandler {
    
    for (IAPProduct * product in _products.allValues) {
        product.info = nil;
        product.availableForPurchase = NO;
    }
    
    NSURL * baseUrl = [NSURL URLWithString:IAPServerBaseURL];
    AFHTTPClient * httpClient = [[AFHTTPClient alloc] initWithBaseURL:baseUrl];
    NSURL * url = [NSURL URLWithString:IAPServerProductsURL relativeToURL:baseUrl];
    
    
    NSMutableURLRequest * request = [NSURLRequest requestWithURL:url
                                            cachePolicy:NSURLRequestReloadIgnoringCacheData
                                                 timeoutInterval:60];
    
    AFHTTPRequestOperation *operation = [httpClient HTTPRequestOperationWithRequest:request success:^(AFHTTPRequestOperation *operation, id responseObject) {
        
        NSData * productInfosData = [operation responseData];
        NSError * error = nil;
        NSArray * productInfosArray = [NSPropertyListSerialization propertyListWithData:productInfosData
                                                                                options:NSPropertyListImmutable
                                                                                 format:NULL
                                                                                  error:&error];
        if (productInfosArray == nil) {
            completionHandler(FALSE, error);
        } else {
            for (NSDictionary * productInfoDict in productInfosArray) {
                IAPProductInfo *info = [[IAPProductInfo alloc]
                                      initFromDict:productInfoDict];
                [self addInfo:info forProductIdentifier:info.productIdentifier];
            };
            
            if (!_productsLoaded) {
                _productsLoaded = YES;
                [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
            }
            
            completionHandler(TRUE, nil);
        }
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        completionHandler(FALSE, error);
    }];
    
    [httpClient enqueueHTTPRequestOperation:operation];
    
}

- (void)validateReceiptForTransaction:(SKPaymentTransaction *)transaction {
    
    IAPProduct * product = _products[transaction.payment.productIdentifier];
    
    NSString * receiptString = [transaction.transactionReceipt base64EncodedString];
    
    NSURL * url = [NSURL URLWithString:IAPServerBaseURL];
    AFHTTPClient * httpClient = [[AFHTTPClient alloc] initWithBaseURL:url];
    NSDictionary * params = @{ @"receipt" : receiptString, @"sandbox" : @YES };
    [httpClient postPath:IAPServerVerifyURL parameters:params success:^(AFHTTPRequestOperation *operation, id responseObject) {
        
        NSError * jsonError;
        NSDictionary * json = [NSJSONSerialization JSONObjectWithData:operation.responseData options:kNilOptions error:&jsonError];
        if (!json) {
            NSString * responseString = operation.responseString;
            NSLog(@"Failer parsing resonse: %@. Server response: %@", jsonError, responseString);
            [self notifyStatusForProductIdentifier:transaction.payment.productIdentifier string:@"Validation failed."];
            product.purchaseInProgress = NO;
            [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
        } else {
            
            int status = [json[@"status"] integerValue];
            NSString * error = json[@"error"];
            if (status != 0) {
                NSLog(@"Failure verifying receipt: %@", error);
                [self notifyStatusForProductIdentifier:transaction.payment.productIdentifier string:@"Validation failed."];
                product.purchaseInProgress = NO;
                [[SKPaymentQueue defaultQueue]
                 finishTransaction:transaction];
            }
            
            else {
                NSLog(@"Successfully verified receipt!");
                [self provideContentForTransaction:transaction productIdentifier:transaction.payment.productIdentifier];
            }
        }
        
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        NSLog(@"Failure connecting to server: %@", error);
        [self notifyStatusForProductIdentifier:transaction.payment.productIdentifier string:@"Validation failed."];
        product.purchaseInProgress = NO;
        [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
    }];
}



- (void)paymentQueue:(SKPaymentQueue *)queue
    updatedDownloads:(NSArray *)downloads {
    
    // 1
    SKDownload * download = downloads[0];
    SKPaymentTransaction * transaction = download.transaction;
    SKPayment * payment = transaction.payment;
    NSString * productIdentifier = payment.productIdentifier;
    IAPProduct * product = _products[productIdentifier];
    
    // 2
    product.progress = download.progress;
    
    // 3
    NSLog(@"Download state: %d", download.downloadState);
    if (download.downloadState == SKDownloadStateFinished) {
        
        // 4
        [self purchaseNonconsumableAtURL:download.contentURL
                    forProductIdentifier:productIdentifier];
        product.purchaseInProgress = NO;
        [[SKPaymentQueue defaultQueue] finishTransaction:
         transaction];
        
    } else if (download.downloadState ==
               SKDownloadStateFailed) {
        
        // 5
        NSLog(@"Download failed.");
        [self notifyStatusForProductIdentifier:productIdentifier
                                        string:@"Download failed."];
        product.purchaseInProgress = NO;
        [[SKPaymentQueue defaultQueue] finishTransaction:
         transaction];
        
    } else if (download.downloadState ==
               SKDownloadStateCancelled) {
        
        // 6
        NSLog(@"Download cancelled.");
        [self notifyStatusForProductIdentifier:productIdentifier
                                        string:@"Download cancelled."];
        product.purchaseInProgress = NO;
        [[SKPaymentQueue defaultQueue] finishTransaction:
         transaction];
        
    } else {
        // 7
        NSLog(@"Download for %@: %0.2f complete",
              productIdentifier, product.progress);
    }
}

- (void)pauseDownloads:(NSArray *)downloads {
    [[SKPaymentQueue defaultQueue] pauseDownloads:downloads];
}

- (void)resumeDownloads:(NSArray *)downloads {
    [[SKPaymentQueue defaultQueue] resumeDownloads:downloads];
}

- (void)cancelDownloads:(NSArray *)downloads {
    [[SKPaymentQueue defaultQueue] cancelDownloads:downloads];
}

@end
