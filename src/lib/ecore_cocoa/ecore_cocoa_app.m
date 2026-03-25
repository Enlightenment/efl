#import "ecore_cocoa_app.h"
#import "ecore_cocoa_window.h"
#include "ecore_cocoa_private.h"

static Eina_Bool
_ecore_cocoa_run_loop_cb(void *data EINA_UNUSED)
{
   @autoreleasepool {
        @try {
             NSEvent *e;
             do {
                  e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                         untilDate:[NSApp eventExpirationDate]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
                  if (e != nil) {
                       [NSApp sendEvent:e];

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
             [NSApp reportException:except];
        }
        [NSApp updateWindows];
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
   NSApp = self;

   /* Set the process to be a foreground process,
    * without that it prevents the window to become the key window and
    * receive all mouse mouve events.
    * Activation is deferred to applicationDidFinishLaunching so the app
    * is properly registered with the window server first. */
   [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

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
   /*
    * Use the GLFW-style bootstrap: call [super run] which performs all
    * internal NSApplication setup (beyond what finishLaunching alone does),
    * then return control via [NSApp stop:nil] in applicationDidFinishLaunching.
    * This ensures the app is fully registered with the window server.
    */
   [super run];

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

- (void)setReopenCb:(Ecore_Cocoa_Reopen_Cb)cb
{
   _reopen_cb = cb;
}

- (Ecore_Cocoa_Reopen_Cb)reopenCb
{
   return _reopen_cb;
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

- (void)applicationDidFinishLaunching:(NSNotification *) EINA_UNUSED notification
{
   /* Now that the app has finished launching and is registered with the
    * window server, activate it so it can receive focus and display windows. */
   [NSApp activateIgnoringOtherApps:YES];

   /* Break out of [super run] so ecore can own the main loop.
    * Post an empty event to ensure the stop is processed immediately. */
   [NSApp stop:nil];
   NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                       location:NSMakePoint(0, 0)
                                  modifierFlags:0
                                      timestamp:0
                                   windowNumber:0
                                        context:nil
                                        subtype:0
                                          data1:0
                                          data2:0];
   [NSApp postEvent:event atStart:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *) EINA_UNUSED sender
{
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

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
   Ecore_Cocoa_Reopen_Cb cb = [(Ecore_Cocoa_Application *)sender reopenCb];
   if (cb)
     {
        Eina_Bool handled = cb(flag ? EINA_TRUE : EINA_FALSE);
        return handled ? YES : NO;
     }
   return NO;
}

@end
