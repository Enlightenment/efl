#import "ecore_cocoa_app.h"

static Eina_Bool
_ecore_cocoa_run_loop_cb(void *data EINA_UNUSED)
{
   @autoreleasepool {
        @try {
             NSEvent *e;
             do {
                  e = [NSApp nextEventMatchingMask:NSAnyEventMask
                                         untilDate:[NSApp eventExpirationDate]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
                  if (e != nil) {
                       //NSLog(@"Catching event %@", e);

                       [NSApp sendEvent:e];

                       /* Update (en/disable) the services menu's items */
                       NSEventType type = [e type];
                       if (type != NSPeriodic && type != NSMouseMoved) {
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
   [_mainMenu update];
   // FIXME Will not compile with GNUStep (member is named "_main_menu")
}

- (id)init
{
   self = [super init];
   if (self == nil) {
        // XXX Critical error. Abort right now! Log?
        return nil;
   }
   NSApp = self; // NSApp is used EVERYWHERE! Set it right now!
   return NSApp;
}

- (NSDate *)eventExpirationDate
{
   return _expiration;
}

- (void)run
{
   [self finishLaunching];

   _running = 1;
   _expiration = [NSDate distantPast];

   _poller = ecore_poller_add(ECORE_POLLER_CORE,
                              ecore_poller_poll_interval_get(ECORE_POLLER_CORE),
                              _ecore_cocoa_run_loop_cb, NULL);
   if (_poller == NULL) {
        // XXX ERROR
   }
}


- (void)sendEvent:(NSEvent *)anEvent
{
   Eina_Bool to_super;

   /* Some events shall be handled by Ecore (like single non-command keys).
    * If we dispatch all events right to NSApplication, it will complain
    * with NSBeep() when an event is not authorized */
   to_super = ecore_cocoa_feed_events(anEvent);
   if (to_super)
     [super sendEvent:anEvent];
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

- (id)init
{
   self = [super init];
   return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
   // XXX This should be alterable (by Elm_Window policy)
   return YES;
}

@end

