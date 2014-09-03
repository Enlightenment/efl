#import <Cocoa/Cocoa.h>
#include "Ecore_Cocoa.h"
#include <Ecore.h>

@interface Ecore_Cocoa_Application : NSApplication
{
   Ecore_Poller *_poller;
   NSDate       *_expiration;
}

- (NSDate *)eventExpirationDate;

+ (Ecore_Cocoa_Application *)sharedApplication;
- (void)run;
- (void)sendEvent:(NSEvent *)anEvent;
- (id)init;
- (void)internalUpdate;

@end


@interface Ecore_Cocoa_AppDelegate : NSObject <NSApplicationDelegate>

+ (Ecore_Cocoa_AppDelegate *)appDelegate;
- (id)init;

@end

