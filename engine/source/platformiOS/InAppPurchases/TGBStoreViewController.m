//
//  TGBStoreViewController.m
//  iTorque2D
//
//  Created by Paul on 1/9/13.
//
//

#import "TGBStoreViewController.h"
#import "T2DStoreListViewCell.h"
#import "T2DIAPHelper.h"
#import "IAPProduct.h"
#import <StoreKit/StoreKit.h>
#import <UIKit/UIKit.h>
#import "T2DStoreDetailViewController.h"
#import "IAPProductPurchase.h"
#import "UIImageView+AFNetworking.h"

@interface TGBStoreViewController ()

@end

@implementation TGBStoreViewController {
    NSArray * _products;
    NSNumberFormatter * _priceFormatter;
}

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    
    _priceFormatter = [[NSNumberFormatter alloc] init];
    [_priceFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [_priceFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    
    self.refreshControl = [[UIRefreshControl alloc] init];
    [self.refreshControl addTarget:self action:@selector(reload) forControlEvents:UIControlEventValueChanged];
    [self reload];
    [self.refreshControl beginRefreshing];
}


- (void) reload {
    _products = nil;
    [self.tableView reloadData];
    [[T2DIAPHelper sharedInstance] requestProductsWithCompletionHandler:^(BOOL success, NSArray *products)
    {
        if (success) {
            _products = products;
            [self.tableView reloadData];
        }
        [self.refreshControl endRefreshing];
    }];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return _products.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    T2DStoreListViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    IAPProduct * product = _products[indexPath.row];
    
    cell.titleLabel.text = product.skProduct.localizedTitle;
    cell.descriptionLabel.text = product.skProduct.localizedDescription;
    [_priceFormatter setLocale:product.skProduct.priceLocale];
    if (!product.purchase.consumable && product.purchase){
        cell.priceLabel.text = @"Installed";
    } else {
        cell.priceLabel.text = [_priceFormatter stringFromNumber:product.skProduct.price];
    }
    NSLog(@"set icon: %@", product.info.icon);
    [cell.iconImageView setImageWithURL:[NSURL URLWithString:product.info.icon] placeholderImage:[UIImage imageNamed:@"icon_placeholder.png"]];
    
    
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

#pragma mark - UIAlertViewDelegate

- (void)alertView:(UIAlertView *)alertView
didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if (buttonIndex == alertView.firstOtherButtonIndex) {
        [[T2DIAPHelper sharedInstance]
         restoreCompletedTransactions];
    }
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.row < _products.count) {
        [self performSegueWithIdentifier:@"PushDetail"
                                  sender:indexPath];
    }
}


#pragma mark - Callbacks

- (IBAction)closeTapped:(id)sender {
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"PushDetail"]) {
        T2DStoreDetailViewController * detailViewController = (T2DStoreDetailViewController *)segue.destinationViewController;
        NSIndexPath * indexPath = (NSIndexPath *)sender;
        IAPProduct *product = _products[indexPath.row];
        detailViewController.product = product;
    }
}

- (void) restoreTapped:(id)sender {
}

@end
