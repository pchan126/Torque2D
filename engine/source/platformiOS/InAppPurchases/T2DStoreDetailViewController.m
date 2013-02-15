//
//  T2DStoreDetailViewController.m
//  iTorque2D
//
//  Created by Paul on 1/10/13.
//
//

#import "T2DStoreDetailViewController.h"
#import "IAPProduct.h"
#import <StoreKit/StoreKit.h>
#import "T2DIAPHelper.h"
#import "IAPProductPurchase.h"
#import "IAPProductInfo.h"
#import "PrettyBytes.h"

@interface T2DStoreDetailViewController ()
@property (weak, nonatomic) IBOutlet UILabel *titleLabel;
@property (weak, nonatomic) IBOutlet UILabel *priceLabel;
@property (weak, nonatomic) IBOutlet UILabel *versionLabel;
@property (weak, nonatomic) IBOutlet UITextView *descriptionTextView;
@property (weak, nonatomic) IBOutlet UILabel *statusLabel;
@property (weak, nonatomic) IBOutlet UIProgressView *progressView;
@property (weak, nonatomic) IBOutlet UIButton *pauseButton;
@property (weak, nonatomic) IBOutlet UIButton *resumeButton;
@property (weak, nonatomic) IBOutlet UIButton *cancelButton;
@end

@implementation T2DStoreDetailViewController {
    NSNumberFormatter * _priceFormatter;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
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

    _priceFormatter = [[NSNumberFormatter alloc] init];
    [_priceFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [_priceFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)refresh {
    self.title = _product.skProduct.localizedTitle;
    self.titleLabel.text = _product.skProduct.localizedTitle;
    self.descriptionTextView.text =
    _product.skProduct.localizedDescription;
    [_priceFormatter setLocale:_product.skProduct.priceLocale];
    self.priceLabel.text = [_priceFormatter
                            stringFromNumber:_product.skProduct.price];
    if (_product.skProduct.downloadable) {
        int numBytes = [_product.skProduct.downloadContentLengths[0]
                        integerValue];
        NSString * numBytesString = prettyBytes(numBytes);
        self.versionLabel.text = [NSString
                                  stringWithFormat:@"Version %@ (%@)",
                                  _product.skProduct.downloadContentVersion,
                                  numBytesString];
    } else {
        self.versionLabel.text = @"Version 1.0";
    }
    
    
    if (_product.allowedToPurchase) {
        self.navigationItem.rightBarButtonItem =
        [[UIBarButtonItem alloc] initWithTitle:@"Buy"
                                         style:UIBarButtonItemStyleBordered target:self
                                        action:@selector(buyTapped:)];
        self.navigationItem.rightBarButtonItem.enabled = YES;
    } else {
        self.navigationItem.rightBarButtonItem = nil;
    }
    
    self.pauseButton.hidden = YES;
    self.resumeButton.hidden = YES;
    self.cancelButton.hidden = YES;
    
    if (self.product.purchaseInProgress) {
        self.statusLabel.hidden = NO;
        self.progressView.hidden = NO;
        if (self.product.skDownload) {
            self.pauseButton.hidden = NO;
            self.resumeButton.hidden = NO;
            self.cancelButton.hidden = NO;
            switch (self.product.skDownload.downloadState) {
                case SKDownloadStateActive: {
                    if (self.product.skDownload.timeRemaining >= 0) {
                        self.statusLabel.text = [NSString
                                                 stringWithFormat:@"Active (%0.2fs remaining)...",
                                                 self.product.skDownload.timeRemaining];
                    } else {
                        self.statusLabel.text = @"Active...";
                    }
                }
                    break;
                case SKDownloadStateWaiting: {
                    if (self.product.skDownload.timeRemaining >= 0) {
                        self.statusLabel.text = [NSString
                                                 stringWithFormat:@"Waiting (%0.2fs remaining)...",
                                                 self.product.skDownload.timeRemaining];
                    } else {
                        self.statusLabel.text = @"Waiting...";
                    }
                }
                    break;
                case SKDownloadStateFinished:
                    self.statusLabel.text = @"Download Finished.";
                    break;
                case SKDownloadStateFailed:
                    self.statusLabel.text = @"Download failed.";
                    break;
                case SKDownloadStatePaused:
                    self.statusLabel.text = @"Paused.";
                    break;
                case SKDownloadStateCancelled:
                    self.statusLabel.text = @"Cancelled";
                    break;
                default:
                    self.statusLabel.text = @"Unexpected state!";
                    break;
            }
            self.progressView.progress = self.product.progress;
        } else {
            self.statusLabel.text = @"Installing...";
            self.progressView.progress = self.product.progress;
        }
    } else if (!self.product.purchase.consumable &&
               self.product.purchase) {
        self.statusLabel.hidden = NO;
        self.progressView.hidden = YES;
        if (self.product.skProduct.downloadContentVersion &&
            ![self.product.skProduct.downloadContentVersion
              isEqualToString:self.product.purchase.contentVersion]) {
                self.statusLabel.text = @"Update Available, Please Restore";
            } else {
                self.statusLabel.text = @"Installed";
            }
    } else if (self.product.info.consumable) {
        self.statusLabel.hidden = NO;
        self.progressView.hidden = YES;
        int newValue = [[NSUserDefaults standardUserDefaults]
                        integerForKey:self.product.info.consumableIdentifier];
        self.statusLabel.text = [NSString stringWithFormat:@"Current value: %d", newValue];
    } else {
        self.statusLabel.hidden = YES;
        self.progressView.hidden = YES;
    }
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    self.statusLabel.hidden = YES;
    [self refresh];
    [self.product addObserver:self
                   forKeyPath:@"purchaseInProgress" options:0 context:nil];
    [self.product addObserver:self forKeyPath:@"purchase"
                      options:0 context:nil];
    [self.product addObserver:self forKeyPath:@"progress"
                      options:0 context:nil];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [self.product removeObserver:self
                      forKeyPath:@"purchaseInProgress" context:nil];
    [self.product removeObserver:self forKeyPath:@"purchase"
                         context:nil];
    [self.product removeObserver:self forKeyPath:@"progress"
                         context:nil];
}


- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object change:(NSDictionary *)change
                       context:(void *)context {
    [self refresh];
}

#pragma mark - Callbacks

- (void)buyTapped:(id)sender {
    NSLog(@"Buy tapped!");
    [[T2DIAPHelper sharedInstance] buyProduct:self.product];
}

- (IBAction)pauseTapped:(id)sender {
    [[T2DIAPHelper sharedInstance] pauseDownloads:@[self.product.skDownload]];
}

- (IBAction)resumeTapped:(id)sender {
    [[T2DIAPHelper sharedInstance] resumeDownloads:@[self.product.skDownload]];
}

- (IBAction)cancelTapped:(id)sender {
    [[T2DIAPHelper sharedInstance] cancelDownloads:@[self.product.skDownload]];
}

- (void) downloadFinished {
    
}

- (void) downloadReceivedData {
    
}

- (void) dataDownloadFailed: (NSString *) reason {
    
}


@end
