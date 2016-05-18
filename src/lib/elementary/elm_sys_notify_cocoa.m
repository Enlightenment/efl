#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_sys_notify_cocoa.eo.h"
#include "elm_sys_notify_cocoa.eo.legacy.h"

#define MY_CLASS ELM_SYS_NOTIFY_COCOA_CLASS

EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_close(const Eo *obj,
                                                     void *sd,
                                                     unsigned int id)
{

}

EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_send(const Eo *obj EINA_UNUSED,
                                                    void *sd EINA_UNUSED,
                                                    unsigned int replaces_id EINA_UNUSED,
                                                    const char *icon EINA_UNUSED,
                                                    const char *summary,
                                                    const char *body,
                                                    Elm_Sys_Notify_Urgency urgency EINA_UNUSED,
                                                    int timeout EINA_UNUSED,
                                                    Elm_Sys_Notify_Send_Cb cb EINA_UNUSED,
                                                    const void *cb_data EINA_UNUSED)
{
   NSUserNotification *const n = [[NSUserNotification alloc] init];
   n.title = [NSString stringWithUTF8String: summary];
   n.informativeText = [NSString stringWithUTF8String: body];

   [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: n];
}

EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_simple_send(const Eo   *obj,
                                                           void       *sd,
                                                           const char *icon,
                                                           const char *summary,
                                                           const char *body)
{
   _elm_sys_notify_cocoa_elm_sys_notify_interface_send(obj, sd,
                                                       0, icon, summary, body,
                                                       ELM_SYS_NOTIFY_URGENCY_NORMAL,
                                                       -1, NULL, NULL);
}
