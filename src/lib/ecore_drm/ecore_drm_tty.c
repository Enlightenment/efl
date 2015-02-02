#include "ecore_drm_private.h"

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static int kbd_mode = 0;

static Eina_Bool 
_ecore_drm_tty_cb_vt_signal(void *data, int type EINA_UNUSED, void *event)
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
        ecore_drm_device_master_drop(dev);
        ioctl(dev->tty.fd, VT_RELDISP, 1);
        break;
      case 2:
        ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ);
        ecore_drm_device_master_set(dev);
        break;
      default:
        break;
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_ecore_drm_tty_switch(Ecore_Drm_Device *dev, int activate_vt)
{
   if (!ioctl(dev->tty.fd, VT_ACTIVATE, activate_vt) < 0)
     return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool 
_ecore_drm_tty_setup(Ecore_Drm_Device *dev)
{
   struct stat st;
   int kmode;
   struct vt_mode vtmode = { 0 };

   if ((fstat(dev->tty.fd, &st) == -1) || 
       (major(st.st_rdev) != TTY_MAJOR) || (minor(st.st_rdev) == 0))
     {
        ERR("Failed to get stats for tty: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, KDGETMODE, &kmode))
     {
        ERR("Could not get tty mode: %m");
        return EINA_FALSE;
     }

   if (kmode != KD_TEXT)
     {
        WRN("Virtual Terminal already in KD_GRAPHICS mode");
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

   if (ioctl(dev->tty.fd, KDGKBMODE, &kbd_mode))
     {
        ERR("Could not get curent kbd mode: %m");
        return EINA_FALSE;
     }

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
   return EINA_FALSE;
}

/**
 * @defgroup Ecore_Drm_Tty_Group Tty manipulation functions
 * 
 * Functions that deal with opening, closing, and otherwise using a tty
 */

/**
 * Open a tty for use
 * 
 * @param dev  The Ecore_Drm_Device that this tty will belong to.
 * @param name The name of the tty to try and open. 
 *             If NULL, /dev/tty0 will be used.
 * 
 * @return     EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool 
ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name)
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

        dev->tty.fd = open(tty, (O_RDWR | O_CLOEXEC)); //O_RDWR | O_NOCTTY);
        if (dev->tty.fd < 0)
          {
             DBG("Failed to Open Tty: %m");
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

static void
_ecore_drm_tty_restore(Ecore_Drm_Device *dev)
{
   int fd = dev->tty.fd;
   struct vt_mode mode = { 0 };

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

/**
 * Close an already opened tty
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool 
ecore_drm_tty_close(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (!dev->drm.name)) return EINA_FALSE;

   _ecore_drm_tty_restore(dev);

   close(dev->tty.fd);

   dev->tty.fd = -1;

   /* clear the tty name */
   if (dev->tty.name) eina_stringshare_del(dev->tty.name);
   dev->tty.name = NULL;

   unsetenv("ECORE_DRM_TTY");

   return EINA_TRUE;
}

/**
 * Release a virtual terminal
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool 
ecore_drm_tty_release(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (!dev->drm.name) || (dev->tty.fd < 0)) return EINA_FALSE;

   /* send ioctl for vt release */
   if (ioctl(dev->tty.fd, VT_RELDISP, 1) < 0) 
     {
        ERR("Could not release VT: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/**
 * Acquire a virtual terminal
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool 
ecore_drm_tty_acquire(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (!dev->drm.name) || (dev->tty.fd < 0)) return EINA_FALSE;

   /* send ioctl for vt acquire */
   if (ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ) < 0) 
     {
        ERR("Could not acquire VT: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/**
 * Get the opened virtual terminal file descriptor
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    The tty fd opened from previous call to ecore_drm_tty_open
 * 
 * @ingroup Ecore_Drm_Tty_Group
 * 
 * @since 1.10
 */
EAPI int 
ecore_drm_tty_get(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (!dev->drm.name) || (dev->tty.fd < 0)) return -1;

   return dev->tty.fd;
}
