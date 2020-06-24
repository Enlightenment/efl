/* Portions of this code have been derived from Weston
 *
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2010-2011 Benjamin Franzke
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2010 Red Hat <mjg@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ecore_drm_private.h"
#include <ctype.h>

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static Ecore_Event_Handler *active_hdlr;

#ifdef HAVE_SYSTEMD
static Eina_Module *_libsystemd = NULL;
static Eina_Bool _libsystemd_broken = EINA_FALSE;

static int (*_ecore_sd_session_get_vt) (const char *session, unsigned *vtnr) = NULL;
static int (*_ecore_sd_pid_get_session) (pid_t pid, char **session) = NULL;
static int (*_ecore_sd_session_get_seat) (const char *session, char **seat) = NULL;

void
_ecore_drm_sd_init(void)
{
   if (_libsystemd_broken) return;
   _libsystemd = eina_module_new("libsystemd.so.0");
   if (_libsystemd)
     {
        if (!eina_module_load(_libsystemd))
          {
             eina_module_free(_libsystemd);
             _libsystemd = NULL;
          }
     }
   if (!_libsystemd)
     {
        _libsystemd_broken = EINA_TRUE;
        return;
     }
   _ecore_sd_session_get_vt =
     eina_module_symbol_get(_libsystemd, "sd_session_get_vt");
   _ecore_sd_pid_get_session =
     eina_module_symbol_get(_libsystemd, "sd_pid_get_session");
   _ecore_sd_session_get_seat =
     eina_module_symbol_get(_libsystemd, "sd_session_get_seat");
   if ((!_ecore_sd_session_get_vt) ||
       (!_ecore_sd_pid_get_session) ||
       (!_ecore_sd_session_get_seat))
     {
        _ecore_sd_session_get_vt = NULL;
        _ecore_sd_pid_get_session = NULL;
        _ecore_sd_session_get_seat = NULL;
        eina_module_free(_libsystemd);
        _libsystemd = NULL;
        _libsystemd_broken = EINA_TRUE;
     }
}

static inline Eina_Bool
_ecore_drm_logind_vt_get(Ecore_Drm_Device *dev)
{
   int ret;

   _ecore_drm_sd_init();
   if (!_ecore_sd_session_get_vt)
     {
        ERR("Could not get systemd tty");
        return EINA_FALSE;
     }
   ret = _ecore_sd_session_get_vt(dev->session, &dev->vt);
   if (ret < 0)
     {
        ERR("Could not get systemd tty");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
#endif

static Eina_Bool
_ecore_drm_logind_vt_setup(Ecore_Drm_Device *dev)
{
   char buff[64];

   snprintf(buff, sizeof(buff), "/dev/tty%d", dev->vt);
   buff[sizeof(buff) - 1] = 0;

   if (!ecore_drm_tty_open(dev, buff))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_drm_logind_cb_vt_signal(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Event_Signal_User *ev;
   siginfo_t sig;

   ev = event;
   sig = ev->data;

   if (sig.si_code != SI_KERNEL) return ECORE_CALLBACK_RENEW;
   if (!(dev = data)) return ECORE_CALLBACK_RENEW;

   switch (ev->number)
     {
      case 1:
        _ecore_drm_event_activate_send(EINA_FALSE);
        ioctl(dev->tty.fd, VT_RELDISP, 1);
        break;
      case 2:
        ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ);
        _ecore_drm_event_activate_send(EINA_TRUE);
        break;
      default:
        break;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_drm_logind_cb_activate(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Event_Activate *ev;
   Ecore_Drm_Device *dev;
   Ecore_Drm_Output *output;
   Ecore_Drm_Input *input;
   Eina_List *l;

   if ((!event) || (!data)) return ECORE_CALLBACK_RENEW;

   ev = event;
   dev = data;

   dev->active = ev->active;

   if (ev->active)
     {
        /* set output mode */
        EINA_LIST_FOREACH(dev->outputs, l, output)
          _ecore_drm_output_render_enable(output);

        /* enable inputs */
        EINA_LIST_FOREACH(dev->inputs, l, input)
          ecore_drm_inputs_enable(input);
     }
   else
     {
        Ecore_Drm_Sprite *sprite;

        /* disable inputs */
        EINA_LIST_FOREACH(dev->inputs, l, input)
          ecore_drm_inputs_disable(input);

        /* disable hardware cursor */
        EINA_LIST_FOREACH(dev->outputs, l, output)
          _ecore_drm_output_render_disable(output);

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
   char *seat = NULL;

   _ecore_drm_sd_init();
   if ((!_ecore_sd_pid_get_session) ||
       (!_ecore_sd_session_get_seat))
     {
        ERR("Could not get systemd session");
        return EINA_FALSE;
     }
   /* get session id */
   if (_ecore_sd_pid_get_session(getpid(), &dev->session) < 0)
     {
        ERR("Could not get systemd session");
        return EINA_FALSE;
     }
   if (_ecore_sd_session_get_seat(dev->session, &seat) < 0)
     {
        ERR("Could not get systemd seat");
        return EINA_FALSE;
     }
   else if (strcmp(dev->seat, seat))
     {
        ERR("Session seat '%s' differs from device seat '%s'", seat, dev->seat);
        free(seat);
        return EINA_FALSE;
     }
   free(seat);
   if (!_ecore_drm_logind_vt_get(dev)) return EINA_FALSE;
#endif

   if (!_ecore_drm_dbus_init(dev)) return EINA_FALSE;

   /* take control of session */
   if (!_ecore_drm_dbus_session_take())
     {
        ERR("Could not take control of session");
        goto take_err;
     }

   /* setup vt */
   if (!_ecore_drm_logind_vt_setup(dev))
     {
        ERR("Could not setup vt '%d'", dev->vt);
        goto vt_err;
     }

   /* setup handler for vt signals */
   if (!dev->tty.event_hdlr)
     {
        dev->tty.event_hdlr =
          ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                  _ecore_drm_logind_cb_vt_signal, dev);
     }

   if (!active_hdlr)
     {
        active_hdlr =
          ecore_event_handler_add(ECORE_DRM_EVENT_ACTIVATE,
                                  _ecore_drm_logind_cb_activate, dev);
     }

   return EINA_TRUE;

vt_err:
   _ecore_drm_dbus_session_release();
take_err:
   _ecore_drm_dbus_shutdown();
   return EINA_FALSE;
}

void
_ecore_drm_logind_disconnect(Ecore_Drm_Device *dev)
{
   if (active_hdlr) ecore_event_handler_del(active_hdlr);
   active_hdlr = NULL;

   ecore_drm_tty_close(dev);
   _ecore_drm_dbus_session_release();
   _ecore_drm_dbus_shutdown();
}

void
_ecore_drm_logind_restore(Ecore_Drm_Device *dev)
{
   _ecore_drm_tty_restore(dev);
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

   if (stat(device, &st) < 0) return -1;
   if (!S_ISCHR(st.st_mode)) return -1;

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
