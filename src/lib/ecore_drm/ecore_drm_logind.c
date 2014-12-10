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
_ecore_drm_logind_cb_vt_switch(void *data, int type EINA_UNUSED, void *event)
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

        if (ioctl(dev->tty.fd, VT_ACTIVATE, vt) < 0)
          ERR("Failed to activate vt: %m");
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_drm_logind_cb_vt_signal(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Event_Signal_User *ev;
   siginfo_t sigdata;

   dev = data;
   ev = event;

   sigdata = ev->data;
   if (sigdata.si_code != SI_KERNEL) return ECORE_CALLBACK_RENEW;

   if (ev->number == 1)
     {
        if (!ecore_drm_tty_release(dev))
          ERR("Could not release VT: %m");
     }
   else if (ev->number == 2)
     {
        if (!ecore_drm_tty_acquire(dev))
          ERR("Could not acquire VT: %m");
     }

   return ECORE_CALLBACK_PASS_ON;
}

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

static Eina_Bool
_ecore_drm_logind_tty_setup(Ecore_Drm_Device *dev)
{
   struct stat st;
   int kmode;
   struct vt_mode vtmode = { 0 };

   if (fstat(dev->tty.fd, &st) == -1)
     {
        ERR("Failed to get stats for tty: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, KDGETMODE, &kmode))
     {
        ERR("Could not get tty mode: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, VT_ACTIVATE, minor(st.st_rdev)) < 0)
     {
        ERR("Failed to activate vt: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, VT_WAITACTIVE, minor(st.st_rdev)) < 0)
     {
        ERR("Failed to wait active: %m");
        return EINA_FALSE;
     }

   /* NB: Don't set this. This Turns OFF keyboard on the VT */
   /* if (ioctl(dev->tty.fd, KDSKBMUTE, 1) &&  */
   /*     ioctl(dev->tty.fd, KDSKBMODE, K_OFF)) */
   /*   { */
   /*      ERR("Could not set K_OFF keyboard mode: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   if (kmode != KD_GRAPHICS)
     {
        if (ioctl(dev->tty.fd, KDSETMODE, KD_GRAPHICS))
          {
             ERR("Could not set graphics mode: %m");
             goto err_kmode;
          }
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
   return EINA_FALSE;
}

static Eina_Bool
_ecore_drm_logind_vt_open(Ecore_Drm_Device *dev, const char *name)
{
   char tty[32] = "<stdin>";

   /* check for valid device */
   if ((!dev) || (!dev->drm.name)) return EINA_FALSE;

   /* assign default tty fd of -1 */
   dev->tty.fd = -1;

   if (!name)
     {
        char *env;

        if ((env = getenv("ECORE_DRM_TTY")))
          snprintf(tty, sizeof(tty), "%s", env);
        else
          dev->tty.fd = dup(STDIN_FILENO);
     }
   else
     snprintf(tty, sizeof(tty), "%s", name);

   if (dev->tty.fd < 0)
     {
        DBG("Trying to Open Tty: %s", tty);

        dev->tty.fd = open(tty, O_RDWR | O_NOCTTY);
        if (dev->tty.fd < 0)
          {
             DBG("Failed to Open Tty: %m");
             return EINA_FALSE;
          }
     }

   /* save tty name */
   dev->tty.name = eina_stringshare_add(tty);

   /* FIXME */
   if (!_ecore_drm_logind_tty_setup(dev))
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

   /* setup handler for signals */
   dev->tty.event_hdlr =
     ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                             _ecore_drm_logind_cb_vt_signal, dev);

   /* setup handler for key event of vt switch */
   dev->tty.switch_hdlr =
     ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                             _ecore_drm_logind_cb_vt_switch, dev);

   active_hdl =
     ecore_event_handler_add(ECORE_DRM_EVENT_ACTIVATE,
                             _ecore_drm_logind_cb_activate, dev);

   /* set current tty into env */
   setenv("ECORE_DRM_TTY", tty, 1);

   return EINA_TRUE;
}

static void
_ecore_drm_logind_vt_close(Ecore_Drm_Device *dev)
{
   struct vt_mode mode = { 0 };

   ioctl(dev->tty.fd, KDSETMODE, KD_TEXT);
   mode.mode = VT_AUTO;
   ioctl(dev->tty.fd, VT_SETMODE, &mode);

   if (dev->tty.event_hdlr) ecore_event_handler_del(dev->tty.event_hdlr);
   dev->tty.event_hdlr = NULL;

   if (dev->tty.switch_hdlr) ecore_event_handler_del(dev->tty.switch_hdlr);
   dev->tty.switch_hdlr = NULL;

   if (dev->tty.name) eina_stringshare_del(dev->tty.name);
   dev->tty.name = NULL;

   unsetenv("ECORE_DRM_TTY");
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
     goto dbus_err;

   if (!_ecore_drm_logind_vt_open(dev, NULL))
     goto vt_err;

   return EINA_TRUE;

vt_err:
   _ecore_drm_dbus_shutdown();
dbus_err:
   free(sid);
   return EINA_FALSE;
}

void
_ecore_drm_logind_disconnect(Ecore_Drm_Device *dev)
{
   _ecore_drm_logind_vt_close(dev);
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
