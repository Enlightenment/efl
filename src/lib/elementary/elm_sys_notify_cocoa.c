#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_sys_notify_cocoa.eo.h"
#include "elm_sys_notify_cocoa.eo.legacy.h"

#define MY_CLASS ELM_SYS_NOTIFY_COCOA_CLASS

#ifdef HAVE_ELEMENTARY_COCOA
EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_close(const Eo *obj EINA_UNUSED,
                                                     void *sd EINA_UNUSED,
                                                     unsigned int id EINA_UNUSED)
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
   Ecore_Cocoa_Notification *n;

   printf("----> %s()\n", __func__);
   n = ecore_cocoa_notification_get(replaces_id);
   printf("-> %p\n", n);
   ecore_cocoa_notification_send(n, summary, body, icon, cb, cb_data);
}
#else
EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_close(const Eo *obj EINA_UNUSED,
                                                     void *sd EINA_UNUSED,
                                                     unsigned int id EINA_UNUSED)
{
}
EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_send(const Eo *obj EINA_UNUSED,
                                                    void *sd EINA_UNUSED,
                                                    unsigned int replaces_id EINA_UNUSED,
                                                    const char *icon EINA_UNUSED,
                                                    const char *summary EINA_UNUSED,
                                                    const char *body EINA_UNUSED,
                                                    Elm_Sys_Notify_Urgency urgency EINA_UNUSED,
                                                    int timeout EINA_UNUSED,
                                                    Elm_Sys_Notify_Send_Cb cb EINA_UNUSED,
                                                    const void *cb_data EINA_UNUSED)
{
}
#endif

EOLIAN static void
_elm_sys_notify_cocoa_elm_sys_notify_interface_simple_send(const Eo   *obj,
                                                           void       *sd,
                                                           const char *icon,
                                                           const char *summary,
                                                           const char *body)
{
   printf("----> %s()\n", __func__);
   _elm_sys_notify_cocoa_elm_sys_notify_interface_send(obj, sd,
                                                       0, icon, summary, body,
                                                       ELM_SYS_NOTIFY_URGENCY_NORMAL,
                                                       -1, NULL, NULL);
}

#if 0
EOLIAN static Eina_Bool
_elm_sys_notify_cocoa_elm_sys_notify_interface_available_get(const Eo   *obj,
                                                             void *sd)
{
#ifdef HAVE_ELEMENTARY_COCOA
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}
#endif

#include "elm_sys_notify_cocoa.eo.c"
