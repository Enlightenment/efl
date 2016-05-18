#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#import <Cocoa/Cocoa.h>

#include "Ecore_Cocoa.h"
#include "ecore_cocoa_window.h"
#include "ecore_cocoa_private.h"

@interface EcoreCocoaNotificationDelegate: NSObject <NSUserNotificationCenterDelegate>
@end

@implementation EcoreCocoaNotificationDelegate

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
        didDeliverNotification:(NSUserNotification *)notification
{
   NSLog(@"Hey, I just met you...\n");
}

@end

struct _Ecore_Cocoa_Notification
{
   NSUserNotification *ref;
   unsigned int uid;
};

EAPI void
ecore_cocoa_notification_send(Ecore_Cocoa_Notification *replaces EINA_UNUSED,
                              const char *name,
                              const char *description,
                              const char *icon EINA_UNUSED,
                              Ecore_Cocoa_Notify_Send_Cb cb,
                              const void *cb_data)
{
   NSUserNotification *const n = [[NSUserNotification alloc] init];
   n.title = [NSString stringWithUTF8String: name];
   n.informativeText = [NSString stringWithUTF8String: description];
   [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: n];
}

EAPI Ecore_Cocoa_Notification *
ecore_cocoa_notification_get(unsigned int uid)
{
   if (!uid) return NULL;
   CRI("NOT IMPLEMENTED!!!!\n");
   return NULL; /* TODO FIXME */
}

void
ecore_cocoa_notification_close(Ecore_Cocoa_Notification *n EINA_UNUSED)
{
   CRI("Not implemented");
}

Eina_Bool
_ecore_cocoa_notification_init(void)
{
  // NSUserNotificationCenter *def;
  // EcoreCocoaNotificationDelegate *delegate;

  // def = [NSUserNotificationCenter defaultUserNotificationCenter];
  // delegate = [[EcoreCocoaNotificationDelegate alloc] init];
  // if (EINA_UNLIKELY(delegate == nil))
  //   {
  //      CRI("Failed to create delegate of user notification center");
  //      return EINA_FALSE;
  //   }
  // def.delegate = delegate;

   return EINA_TRUE;
}

void
_ecore_cocoa_notification_shutdown(void)
{
  // NSUserNotificationCenter *def;
  // EcoreCocoaNotificationDelegate *delegate;

  // def = [NSUserNotificationCenter defaultUserNotificationCenter];
  // delegate = def.delegate;
  // [delegate release];
  // def.delegate = nil;
}
