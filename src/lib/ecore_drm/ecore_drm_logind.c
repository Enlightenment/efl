#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/major.h>
#include "ecore_drm_private.h"
#include "ecore_drm_logind.h"

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static Ecore_Event_Handler *active_hdl;
static char *sid;

static Eina_Bool
_ecore_drm_logind_cb_activate(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Event_Activate *e;
   Ecore_Drm_Device *dev;
   Ecore_Drm_Output *output;
   Eina_List *l;

   if ((!event) || (!data)) return ECORE_CALLBACK_RENEW;

   e = event;
   dev = data;

   if (e->active)
     {
        /* set output mode */
        EINA_LIST_FOREACH(dev->outputs, l, output)
           ecore_drm_output_enable(output);
     }
   else
     {
        Ecore_Drm_Sprite *sprite;

        /* disable hardware cursor */
        EINA_LIST_FOREACH(dev->outputs, l, output)
          ecore_drm_output_cursor_size_set(output, 0, 0, 0);

        /* disable sprites */
        EINA_LIST_FOREACH(dev->sprites, l, sprite)
          ecore_drm_sprites_fb_set(sprite, 0, 0);
     }

   return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool
_ecore_drm_logind_connect(Ecore_Drm_Device *dev)
{
#ifdef HAVE_SYSTEMD
   /* get sd-login properties we need */
   if (sd_pid_get_session(getpid(), &sid) < 0) return EINA_FALSE;
#endif

   /* try to init dbus */
   if (!_ecore_drm_dbus_init(sid))
     {
        free(sid);
        return EINA_FALSE;
     }

   active_hdl =
      ecore_event_handler_add(ECORE_DRM_EVENT_ACTIVATE,
                              _ecore_drm_logind_cb_activate, dev);

   return EINA_TRUE;
}

void
_ecore_drm_logind_disconnect(Ecore_Drm_Device *dev EINA_UNUSED)
{
   _ecore_drm_dbus_shutdown();

   if (active_hdl)
     {
        ecore_event_handler_del(active_hdl);
        active_hdl = NULL;
     }
}

Eina_Bool
_ecore_drm_logind_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data)
{
   struct stat st;

   if (stat(device, &st) < 0) return EINA_FALSE;
   if (!S_ISCHR(st.st_mode)) return EINA_FALSE;

   if (_ecore_drm_dbus_device_take(major(st.st_rdev), minor(st.st_rdev), 
                                   callback, data) < 0)
     return EINA_FALSE;

   return EINA_TRUE;
}

int
_ecore_drm_logind_device_open_no_pending(const char *device)
{
   struct stat st;

   if (stat(device, &st) < 0) return EINA_FALSE;
   if (!S_ISCHR(st.st_mode)) return EINA_FALSE;

   return _ecore_drm_dbus_device_take_no_pending(major(st.st_rdev), minor(st.st_rdev), NULL, -1);
}

void
_ecore_drm_logind_device_close(const char *device)
{
   struct stat st;

   if (stat(device, &st) < 0) return;
   if (!S_ISCHR(st.st_mode)) return;

   _ecore_drm_dbus_device_release(major(st.st_rdev), minor(st.st_rdev));
}
