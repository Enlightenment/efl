#import "ecore_cocoa_app.h"
#import "ecore_cocoa_window.h"
#include "ecore_cocoa_private.h"

static Eina_Bool
_ecore_cocoa_run_loop_cb(void *data EINA_UNUSED)
{
        @try {
             NSEvent *e;
             do {
                  e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                         untilDate:[NSApp eventExpirationDate]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
                  if (e != nil) {
                       //NSLog(@"Catching event %@", e);

                       [NSApp sendEvent:e];

                       /* Update (en/disable) the services menu's items */
                       NSEventType type = [e type];
                       if ((type != NSEventTypePeriodic) &&
                           (type != NSEventTypeMouseMoved)) {
                            [NSApp internalUpdate];
                       }
                  }
             } while (e != nil);
        }
        @catch (NSException *except) {
             NSLog(@"EXCEPTION: %@: %@", [except name], [except reason]);
             /* Show the "fancy" annoying report panel */
             [NSApp reportException:except];
             // XXX Maybe use Eina_Log to report the error instead
        }

   return ECORE_CALLBACK_RENEW;
}

@implementation Ecore_Cocoa_Application

+ (Ecore_Cocoa_Application *)sharedApplication
{
   return (Ecore_Cocoa_Application *)[super sharedApplication];
}

- (void)internalUpdate
{
   [[self mainMenu] update];
}

- (id)init
{
   self = [super init];
   if (self == nil) {
      CRI("Failed to [super init]");
      return nil;
   }
   NSApp = self; // NSApp is used EVERYWHERE! Set it right now!

   /* Set the process to be a foreground process,
    * without that it prevents the window to become the key window and
    * receive all mouse mouve events. */
   [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
   [NSApp activateIgnoringOtherApps:YES];

   return NSApp;
}

- (NSDate *)eventExpirationDate
{
   return _expiration;
}

- (BOOL)isRunning
{
   return _is_running;
}

- (void)run
{
   [self finishLaunching];

   _is_running = YES;
   _expiration = [NSDate distantPast];

   _timer = ecore_timer_add(ECORE_COCOA_MAINLOOP_PERIOD,
                             _ecore_cocoa_run_loop_cb, NULL);
}


- (void)sendEvent:(NSEvent *)anEvent
{
   Eina_Bool to_super;

   /* Some events shall be handled by Ecore (like single non-command keys).
    * If we dispatch all events right to NSApplication, it will complain
    * with NSBeep() when an event is not authorized */
   to_super = _ecore_cocoa_feed_events(anEvent);
   if (to_super)
     [super sendEvent:anEvent];
}

- (void) pauseNSRunLoopMonitoring
{
   /*
    * After calling this method, we will run an iteration of
    * the main loop. We don't want this timer to be fired while
    * calling manually the ecore loop, because it will query the
    * NSRunLoop, which blocks during live resize.
    */
   ecore_timer_freeze(_timer);
}

- (void) resumeNSRunLoopMonitoring
{
   ecore_timer_thaw(_timer);
}

- (void)setTerminateCb:(Ecore_Cocoa_Terminate_Cb)cb
{
   _terminate_cb = cb;
}

- (Ecore_Cocoa_Terminate_Cb)terminateCb
{
   return _terminate_cb;
}

@end



static Ecore_Cocoa_AppDelegate *_appDelegate = nil;

@implementation Ecore_Cocoa_AppDelegate

+ (Ecore_Cocoa_AppDelegate *)appDelegate
{
   if (_appDelegate == nil) {
        _appDelegate = [[self alloc] init];
   }
   return _appDelegate;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *) EINA_UNUSED sender
{
   // XXX This should be alterable (by Elm_Window policy)
   return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
   NSApplicationTerminateReply status = NSTerminateNow;
   const Ecore_Cocoa_Terminate_Cb cb = [(Ecore_Cocoa_Application *)sender terminateCb];
   if (cb)
     {
         const Eina_Bool ret = cb(sender);
         if (!ret) status = NSTerminateCancel;
     }
   return status;
}

@end
