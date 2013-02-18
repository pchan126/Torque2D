#include "platformiOS.h"
#include "console/console.h"

extern iOSPlatState platState;

void openIAPWindow()
{
    T2DViewController *controller = platState.viewController;
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"iPadIAPStore" bundle:nil];
    if (storyboard)
    {
        UIViewController *vc = [storyboard instantiateViewControllerWithIdentifier:@"LoginViewController"];
        if (vc)
        {
            [vc setModalPresentationStyle:UIModalPresentationFullScreen];
            controller.navController = [[UINavigationController alloc] initWithRootViewController:vc];
            UIBarButtonItem *rightButton = [[UIBarButtonItem alloc] initWithTitle:@"Close" style:UIBarButtonItemStylePlain target:vc action:@selector(closeTapped:)];
            vc.navigationItem.rightBarButtonItem = rightButton;
            [controller presentViewController:controller.navController animated:YES completion:NULL];
        }
    }
};


ConsoleFunction(openIAPWindow, void, 1, 1, "openIAPWindow( ) "
                "open the IAP storefront")
{
    openIAPWindow();
}

