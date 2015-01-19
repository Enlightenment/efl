#include "ecore_drm_private.h"
#include "ecore_drm_logind.h"

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
          ERR("Failed to activate vt: %m");
     }

   return ECORE_CALLBACK_PASS_ON;
}


static Eina_Bool
_ecore_drm_launcher_cb_vt_signal(void *data, int type EINA_UNUSED, void *event)
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
        if (!logind)
          {
             Ecore_Drm_Input *input;
             Ecore_Drm_Output *output;
             Ecore_Drm_Sprite *sprite;
             Eina_List *l;

             /* disable inputs (suspends) */
             EINA_LIST_FOREACH(dev->inputs, l, input)
                ecore_drm_inputs_disable(input);

             /* disable hardware cursor */
             EINA_LIST_FOREACH(dev->outputs, l, output)
                ecore_drm_output_cursor_size_set(output, 0, 0, 0);

             /* disable sprites */
             EINA_LIST_FOREACH(dev->sprites, l, sprite)
                ecore_drm_sprites_fb_set(sprite, 0, 0);

             /* drop drm master */
             ecore_drm_device_master_drop(dev);

             _ecore_drm_event_activate_send(EINA_FALSE);
          }

        /* issue ioctl to release vt */
        if (!ecore_drm_tty_release(dev))
          ERR("Could not release VT: %m");
     }
   else if (ev->number == 2)
     {
        if (!logind)
          {
             Ecore_Drm_Output *output;
             Ecore_Drm_Input *input;
             Eina_List *l;

             /* set drm master */
             if (!ecore_drm_device_master_set(dev))
               ERR("Could not set drm master: %m");

             /* set output mode */
             EINA_LIST_FOREACH(dev->outputs, l, output)
                ecore_drm_output_enable(output);

             /* enable inputs */
             EINA_LIST_FOREACH(dev->inputs, l, input)
                ecore_drm_inputs_enable(input);

             if (ecore_drm_tty_acquire(dev))
               _ecore_drm_event_activate_send(EINA_TRUE);
             else
               ERR("Could not acquire VT: %m");
          }
        else
          {
             if (!ecore_drm_tty_acquire(dev))
               ERR("Could not acquire VT: %m");
          }
     }

   return ECORE_CALLBACK_RENEW;
}

EAPI Eina_Bool
ecore_drm_launcher_connect(Ecore_Drm_Device *dev)
{
   if (!(logind = _ecore_drm_logind_connect(dev)))
     {
        DBG("Launcher: Not Support logind\n");
        if (geteuid() == 0)
          DBG("Launcher: Try to keep going with root privilege\n");
        else
          {
             ERR("Launcher: Need Root Privilege or logind\n");
             return EINA_FALSE;
          }
     }

   /* NB: Hmmm, appears we don't need to open a tty if we are running 
    * with systemd support */
   if (!logind)
     {
        if (!ecore_drm_tty_open(dev, NULL))
          {
             ERR("Launcher: failed to open tty\n");
             return EINA_FALSE;
          }
     }

   /* setup handler for signals */
   dev->tty.event_hdlr =
      ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                              _ecore_drm_launcher_cb_vt_signal, dev);

   /* setup handler for key event of vt switch */
   dev->tty.switch_hdlr =
      ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                              _ecore_drm_launcher_cb_vt_switch, dev);

   DBG("Launcher: Success Connect\n");

   return EINA_TRUE;
}

EAPI void
ecore_drm_launcher_disconnect(Ecore_Drm_Device *dev)
{
   if (dev->tty.event_hdlr) ecore_event_handler_del(dev->tty.event_hdlr);
   dev->tty.event_hdlr = NULL;

   if (dev->tty.switch_hdlr) ecore_event_handler_del(dev->tty.switch_hdlr);
   dev->tty.switch_hdlr = NULL;

   /* NB: Hmmm, appears we don't need to open a tty if we are running 
    * with systemd support */
   if (!logind)
     {
        if (!ecore_drm_tty_close(dev))
          ERR("Launcher: failed to close tty\n");
     }

   if (logind)
     {
        logind = EINA_FALSE;
        _ecore_drm_logind_disconnect(dev);
     }
}

static int
_device_flags_set(int fd, int flags)
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
        if ((fd = _device_flags_set(fd, flags)) == -1)
          {
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

   DBG("Device opened %s", device);
   return fd;
}

void
_ecore_drm_launcher_device_close(const char *device, int fd)
{
   if ((logind) && (device))
     return _ecore_drm_logind_device_close(device);

   close(fd);
}
