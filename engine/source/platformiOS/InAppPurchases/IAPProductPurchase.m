//
//  IAPProductPurchase.m
//  iTorque2D
//
//  Created by Paul on 1/11/13.
//
//

#import "IAPProductPurchase.h"

static NSString *const kProductIdentifierKey = @"ProductIdentifier";
static NSString *const kConsumableKey = @"Consumable";
static NSString *const kTimesPurchasedKey = @"TimesPurchased";
static NSString *const kLibraryRelativePathKey = @"LibraryRelativePath";
static NSString *const kContentVersionKey = @"ContentVersion";

@implementation IAPProductPurchase

- (id)initWithProductIdentifier:(NSString *)productIdentifier
                     consumable:(BOOL)consumable
                 timesPurchased:(int)timesPurchased
            libraryRelativePath:(NSString *)libraryRelativePath
                 contentVersion:(NSString *)contentVersion {
    if ((self = [super init])) {
        self.productIdentifier = productIdentifier;
        self.consumable = consumable;
        self.timesPurchased = timesPurchased;
        self.libraryRelativePath = libraryRelativePath;
        self.contentVersion = contentVersion;
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    NSString * productIdentifier = [aDecoder decodeObjectForKey:kProductIdentifierKey];
    BOOL consumable = [aDecoder decodeBoolForKey:kConsumableKey];
    int timesPurchased = [aDecoder decodeIntForKey:kTimesPurchasedKey];
    NSString * libraryRelativePath = [aDecoder decodeObjectForKey:kLibraryRelativePathKey];
    NSString * contentVersion = [aDecoder decodeObjectForKey:kContentVersionKey];
    return [self initWithProductIdentifier:productIdentifier
                                consumable:consumable
                            timesPurchased:timesPurchased
                       libraryRelativePath:libraryRelativePath
                            contentVersion:contentVersion];
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:self.productIdentifier
                   forKey:kProductIdentifierKey];
    [aCoder encodeBool:self.consumable
                forKey:kConsumableKey];
    [aCoder encodeInt:self.timesPurchased
                forKey:kTimesPurchasedKey];
    [aCoder encodeObject:self.libraryRelativePath
                  forKey:kLibraryRelativePathKey];
    [aCoder encodeObject:self.contentVersion
                  forKey:kContentVersionKey];
}

@end
