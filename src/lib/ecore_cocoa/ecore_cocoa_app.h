#import <Cocoa/Cocoa.h>
#include "Ecore_Cocoa.h"
#include <Ecore.h>

#define ECORE_COCOA_MAINLOOP_PERIOD ( 0.0166)

@interface Ecore_Cocoa_Application : NSApplication
{
   Ecore_Timer  *_timer;
   NSDate       *_expiration;
   Ecore_Cocoa_Terminate_Cb _terminate_cb;
   BOOL          _is_running;
}

- (NSDate *)eventExpirationDate;

+ (Ecore_Cocoa_Application *)sharedApplication;
- (void)run;
- (void)sendEvent:(NSEvent *)anEvent;
- (id)init;
- (void)internalUpdate;
- (void)setTerminateCb:(Ecore_Cocoa_Terminate_Cb)cb;
- (Ecore_Cocoa_Terminate_Cb)terminateCb;

- (void) pauseNSRunLoopMonitoring;
- (void) resumeNSRunLoopMonitoring;

@end


@interface Ecore_Cocoa_AppDelegate : NSObject <NSApplicationDelegate>

+ (Ecore_Cocoa_AppDelegate *)appDelegate;

@end

