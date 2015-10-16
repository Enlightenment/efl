#import <Cocoa/Cocoa.h>
#include "Ecore_Cocoa.h"
#include <Ecore.h>

#define ECORE_COCOA_MAINLOOP_PERIOD ( 0.0166)

@interface Ecore_Cocoa_Application : NSApplication
{
   Ecore_Timer  *_timer;
   NSDate       *_expiration;
}

- (NSDate *)eventExpirationDate;

+ (Ecore_Cocoa_Application *)sharedApplication;
- (void)run;
- (void)sendEvent:(NSEvent *)anEvent;
- (id)init;
- (void)internalUpdate;

- (void) pauseNSRunLoopMonitoring;
- (void) resumeNSRunLoopMonitoring;

@end


@interface Ecore_Cocoa_AppDelegate : NSObject <NSApplicationDelegate>

+ (Ecore_Cocoa_AppDelegate *)appDelegate;

@end

