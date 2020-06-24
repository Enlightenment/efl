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

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static int kbd_mode = 0;

static Eina_Bool
_ecore_drm_tty_cb_vt_signal(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Input *input;
   Eina_List *l;
   Ecore_Event_Signal_User *ev;
   siginfo_t sig;

   ev = event;
   sig = ev->data;
   if (sig.si_code != SI_KERNEL) return ECORE_CALLBACK_PASS_ON;
   if (!(dev = data)) return ECORE_CALLBACK_PASS_ON;

   DBG("TTY SWITCH SIGNAL");

   switch (ev->number)
     {
      case 1:
        EINA_LIST_FOREACH(dev->inputs, l, input)
          ecore_drm_inputs_disable(input);
        ecore_drm_device_master_drop(dev);
        ioctl(dev->tty.fd, VT_RELDISP, 1);
        break;
      case 2:
        ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ);
        ecore_drm_device_master_set(dev);
        EINA_LIST_FOREACH(dev->inputs, l, input)
          ecore_drm_inputs_enable(input);
        break;
      default:
        break;
     }

   return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool
_ecore_drm_tty_switch(Ecore_Drm_Device *dev, int activate_vt)
{
   return ioctl(dev->tty.fd, VT_ACTIVATE, activate_vt) >= 0;
}

static Eina_Bool
_ecore_drm_tty_setup(Ecore_Drm_Device *dev)
{
   struct stat st;
   int kmode;
   struct vt_mode vtmode = { 0, 0, SIGUSR1, SIGUSR2, 0 };

   if ((fstat(dev->tty.fd, &st) == -1) ||
       (major(st.st_rdev) != TTY_MAJOR) || (minor(st.st_rdev) <= 0) ||
       (minor(st.st_rdev) >= 64))
     {
        ERR("Failed to get stats for tty");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, KDGETMODE, &kmode))
     {
        ERR("Could not get tty mode: %m");
        return EINA_FALSE;
     }

   if (kmode != KD_TEXT)
     WRN("Virtual Terminal already in KD_GRAPHICS mode");

   /* if (ioctl(dev->tty.fd, VT_ACTIVATE, minor(st.st_rdev)) < 0) */
   /*   { */
   /*      ERR("Failed to activate vt: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   /* if (ioctl(dev->tty.fd, VT_WAITACTIVE, minor(st.st_rdev)) < 0) */
   /*   { */
   /*      ERR("Failed to wait active: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   if (ioctl(dev->tty.fd, KDGKBMODE, &kbd_mode))
     {
        WRN("Could not get current kbd mode: %m");
        dev->tty.kbd_mode = K_UNICODE;
     }
   else if (dev->tty.kbd_mode == K_OFF)
     dev->tty.kbd_mode = K_UNICODE;

   if (ioctl(dev->tty.fd, KDSKBMUTE, 1) &&
       ioctl(dev->tty.fd, KDSKBMODE, K_OFF))
     {
        ERR("Could not set K_OFF keyboard mode: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, KDSETMODE, KD_GRAPHICS))
     {
        ERR("Could not set graphics mode: %m");
        goto err_kmode;
     }

   vtmode.mode = VT_PROCESS;
   vtmode.waitv = 0;
   vtmode.relsig = SIGUSR1;
   vtmode.acqsig = SIGUSR2;

   if (ioctl(dev->tty.fd, VT_SETMODE, &vtmode) < 0)
     {
        ERR("Could not set Terminal Mode: %m");
        goto err_setmode;
     }

   return EINA_TRUE;
err_setmode:
   ioctl(dev->tty.fd, KDSETMODE, KD_TEXT);
err_kmode:
   ioctl(dev->tty.fd, KDSKBMUTE, 0);
   ioctl(dev->tty.fd, KDSKBMODE, dev->tty.kbd_mode);
   return EINA_FALSE;
}

/**
 * @defgroup Ecore_Drm_Tty_Group Tty manipulation functions
 *
 * Functions that deal with opening, closing, and otherwise using a tty
 */

EAPI Eina_Bool
ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name)
{
   char tty[32] = "<stdin>";

   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (!dev->drm.name), EINA_FALSE);

   /* assign default tty fd of -1 */
   dev->tty.fd = -1;

   if (!name)
     {
        char *env;

        if ((env = getenv("ECORE_DRM_TTY")))
          snprintf(tty, sizeof(tty), "%s", env);
        else
          {
             dev->tty.fd = dup(STDIN_FILENO);
             if (dev->tty.fd < 0)
               {
                  ERR("Could not dup stdin: %m");
                  return EINA_FALSE;
               }
          }
     }
   else
     snprintf(tty, sizeof(tty), "%s", name);

   if (dev->tty.fd < 0)
     {
        DBG("Trying to Open Tty: %s", tty);

        dev->tty.fd = open(tty, (O_RDWR | O_CLOEXEC | O_NONBLOCK));
        if (dev->tty.fd < 0)
          {
             DBG("Failed to Open Tty %s: %m", tty);
             return EINA_FALSE;
          }
     }

   /* DBG("Opened Tty %s : %d", tty, dev->tty.fd); */

   /* save tty name */
   dev->tty.name = eina_stringshare_add(tty);

   if (!_ecore_drm_tty_setup(dev))
     {
        close(dev->tty.fd);
        dev->tty.fd = -1;
        if (dev->tty.name)
          {
             eina_stringshare_del(dev->tty.name);
             dev->tty.name = NULL;
          }
        return EINA_FALSE;
     }

   dev->tty.event_hdlr =
     ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                             _ecore_drm_tty_cb_vt_signal, dev);

   /* set current tty into env */
   setenv("ECORE_DRM_TTY", tty, 1);

   return EINA_TRUE;
}

void
_ecore_drm_tty_restore(Ecore_Drm_Device *dev)
{
   int fd = dev->tty.fd;
   struct vt_mode mode = { 0, 0, SIGUSR1, SIGUSR2, 0 };

   if (fd < 0) return;

   if (ioctl(fd, KDSETMODE, KD_TEXT))
     ERR("Could not set KD_TEXT mode on tty: %m\n");

   if (ioctl(dev->tty.fd, KDSKBMUTE, 0) &&
       ioctl(dev->tty.fd, KDSKBMODE, kbd_mode))
     {
        ERR("Could not restore keyboard mode: %m");
     }

   ecore_drm_device_master_drop(dev);

   mode.mode = VT_AUTO;
   if (ioctl(fd, VT_SETMODE, &mode) < 0)
     ERR("Could not reset VT handling\n");
}

EAPI Eina_Bool
ecore_drm_tty_close(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (!dev->drm.name), EINA_FALSE);

   _ecore_drm_tty_restore(dev);

   close(dev->tty.fd);
   dev->tty.fd = -1;

   if (dev->tty.event_hdlr)
     ecore_event_handler_del(dev->tty.event_hdlr);

   /* clear the tty name */
   if (dev->tty.name) eina_stringshare_del(dev->tty.name);
   dev->tty.name = NULL;

   unsetenv("ECORE_DRM_TTY");

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_drm_tty_release(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (!dev->drm.name) ||
                                  (dev->tty.fd < 0), EINA_FALSE);

   /* send ioctl for vt release */
   if (ioctl(dev->tty.fd, VT_RELDISP, 1) < 0)
     {
        ERR("Could not release VT: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_drm_tty_acquire(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (!dev->drm.name) ||
                                  (dev->tty.fd < 0), EINA_FALSE);

   /* send ioctl for vt acquire */
   if (ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ) < 0)
     {
        ERR("Could not acquire VT: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI int
ecore_drm_tty_get(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (!dev->drm.name) ||
                                  (dev->tty.fd < 0), -1);

   return dev->tty.fd;
}
