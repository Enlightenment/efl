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

static Eina_Bool logind = EINA_FALSE;

static Eina_Bool
_ecore_drm_launcher_cb_vt_switch(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Event_Key *ev;
   int keycode;
   int vt;

   dev = data;
   ev = event;
   keycode = ev->keycode - 8;

   if ((ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       (ev->modifiers & ECORE_EVENT_MODIFIER_ALT) &&
       (keycode >= KEY_F1) && (keycode <= KEY_F8))
     {
        vt = (keycode - KEY_F1 + 1);

        if (!_ecore_drm_tty_switch(dev, vt))
          ERR("Failed to activate vt");
     }

   return ECORE_CALLBACK_PASS_ON;
}

int
_ecore_drm_launcher_device_flags_set(int fd, int flags)
{
   int fl;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0) return -1;

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   if (fcntl(fd, F_SETFL, fl) < 0)
     return -1;

   fl = fcntl(fd, F_GETFD);
   if (fl < 0) return -1;

   if (!(flags & O_CLOEXEC))
     fl &= ~FD_CLOEXEC;

   if (fcntl(fd, F_SETFD, fl) < 0)
     return -1;

   return fd;
}

EAPI Eina_Bool
ecore_drm_launcher_connect(Ecore_Drm_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);

   /* try to connect to logind */
   if (!(logind = _ecore_drm_logind_connect(dev)))
     {
        DBG("Launcher: Logind not supported");
        if (geteuid() == 0)
          {
             DBG("Launcher: Trying to continue with root privileges");
             if (!ecore_drm_tty_open(dev, NULL))
               {
                  ERR("Launcher: Could not setup tty");
                  return EINA_FALSE;
               }
          }
        else
          {
             ERR("Launcher: Root privileges needed");
             return EINA_FALSE;
          }
     }

   dev->tty.switch_hdlr =
     ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                             _ecore_drm_launcher_cb_vt_switch, dev);

   return EINA_TRUE;
}

EAPI void
ecore_drm_launcher_disconnect(Ecore_Drm_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN(dev);

   if (dev->tty.switch_hdlr) ecore_event_handler_del(dev->tty.switch_hdlr);
   dev->tty.switch_hdlr = NULL;

   if (!logind)
     {
        if (!ecore_drm_tty_close(dev))
          ERR("Launcher: Could not close tty");
     }
   else
     {
        _ecore_drm_logind_disconnect(dev);
     }
}

Eina_Bool
_ecore_drm_launcher_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data, int flags)
{
   int fd = -1;
   struct stat s;

   if (logind)
     {
        if (!_ecore_drm_logind_device_open(device, callback, data))
          return EINA_FALSE;
     }
   else
     {
        fd = open(device, flags | O_CLOEXEC);
        if (fd < 0) return EINA_FALSE;
        if (fstat(fd, &s) == -1)
          {
             close(fd);
             fd = -1;
             return EINA_FALSE;
          }

        callback(data, fd, EINA_FALSE);
     }

   return EINA_TRUE;
}

int
_ecore_drm_launcher_device_open_no_pending(const char *device, int flags)
{
   int fd = -1;
   struct stat s;

   if (logind)
     {
        fd = _ecore_drm_logind_device_open_no_pending(device);
        if (fd < 0) return -1;
        if (_ecore_drm_launcher_device_flags_set(fd, flags | O_CLOEXEC) < 0)
          {
             close(fd);
             _ecore_drm_logind_device_close(device);
             return -1;
          }
     }
   else
     {
        fd = open(device, flags | O_CLOEXEC);
        if (fd < 0) return fd;
        if (fstat(fd, &s) == -1)
          {
             close(fd);
             return -1;
          }
     }

   return fd;
}

void
_ecore_drm_launcher_device_close(const char *device, int fd)
{
   if ((logind) && (device)) _ecore_drm_logind_device_close(device);

   if (fd < 0) return;
   close(fd);
}
