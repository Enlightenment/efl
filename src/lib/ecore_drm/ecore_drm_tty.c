#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static Eina_Bool 
_ecore_drm_tty_cb_signal(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Event_Signal_User *ev;

   dev = data;
   ev = event;

   DBG("Caught user signal: %d", ev->number);

   if (ev->number == 1)
     {
        DBG("Release VT");

        /* drop drm master */
        if (ecore_drm_device_master_drop(dev))
          {
             /* issue ioctl to release vt */
             if (!ecore_drm_tty_release(dev))
               ERR("Could not release VT: %m");
          }
        else
          ERR("Could not drop drm master: %m");
     }
   else if (ev->number == 2)
     {
        DBG("Acquire VT");

        /* issue ioctl to acquire vt */
        if (ecore_drm_tty_acquire(dev))
          {
             /* set drm master */
             if (!ecore_drm_device_master_set(dev))
               ERR("Could not set drm master: %m");
          }
        else
          ERR("Could not acquire VT: %m");
     }

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_drm_tty_setup(Ecore_Drm_Device *dev)
{
   struct stat st;
   int kb_mode;
   struct vt_mode vtmode = { 0 };

   if (fstat(dev->tty.fd, &st) == -1)
     {
        ERR("Failed to get stats for tty: %m");
        return EINA_FALSE;
     }

   if (ioctl(dev->tty.fd, KDGKBMODE, &kb_mode))
     {
        ERR("Could not get tty keyboard mode: %m");
        return EINA_FALSE;
     }

   /* NB: Don't set this. This Turns OFF keyboard on the VT */
   /* if (ioctl(dev->tty.fd, KDSKBMUTE, 1) &&  */
   /*     ioctl(dev->tty.fd, KDSKBMODE, K_OFF)) */
   /*   { */
   /*      ERR("Could not set K_OFF keyboard mode: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   /* if (ioctl(dev->tty.fd, KDSETMODE, KD_GRAPHICS)) */
   /*   { */
   /*      ERR("Could not set graphics mode: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   vtmode.mode = VT_PROCESS;
   vtmode.waitv = 0;
   vtmode.relsig = SIGUSR1;
   vtmode.acqsig = SIGUSR2;
   if (ioctl(dev->tty.fd, VT_SETMODE, &vtmode) < 0)
     {
        ERR("Could not set Terminal Mode: %m");
        return EINA_FALSE;
     }

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

   return EINA_TRUE;
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
   Eina_Bool ret = EINA_FALSE;
   char tty[32] = "<stdin>";

   /* check for valid device */
   if ((!dev) || (!dev->devname)) return EINA_FALSE;

   /* assign default tty fd of -1 */
   dev->tty.fd = -1;

   if (!name)
     {
        char *env;

        if ((env = getenv("ECORE_DRM_TTY")))
          snprintf(tty, sizeof(tty), "%s", env);
        else
          dev->tty.fd = STDIN_FILENO;
     }
   else // FIXME: NB: This should Really check for format of name (/dev/xyz)
     snprintf(tty, sizeof(tty), "%s", name);

   if (dev->tty.fd < 0)
     {
        void *data = malloc(sizeof(int));

        DBG("Trying to Open Tty: %s", tty);

        /* try to open the tty */
        _ecore_drm_message_send(ECORE_DRM_OP_TTY_OPEN, -1, tty, strlen(tty));

        /* get the result of the open operation */
        ret = _ecore_drm_message_receive(ECORE_DRM_OP_TTY_OPEN, &dev->tty.fd,
                                         &data, sizeof(data));
        if (!ret)
          {
             DBG("Failed to receive Open Tty Message");
             return EINA_FALSE;
          }

        free(data);
     }

   if (dev->tty.fd < 0)
     {
        DBG("Failed to open tty %s", tty);
        return EINA_FALSE;
     }

   DBG("Opened Tty %s : %d", tty, dev->tty.fd);

  /* save tty name */
   dev->tty.name = eina_stringshare_add(tty);

   /* FIXME */
   if (!_ecore_drm_tty_setup(dev))
     return EINA_FALSE;

   /* setup handler for signals */
   dev->tty.event_hdlr = 
     ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER, 
                             _ecore_drm_tty_cb_signal, dev);

   /* set current tty into env */
   setenv("ECORE_DRM_TTY", tty, 1);

   return EINA_TRUE;
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
   Eina_Bool ret = EINA_FALSE;
   void *data = malloc(sizeof(int));

   /* check for valid device */
   if ((!dev) || (!dev->devname)) return EINA_FALSE;

   /* try to close the tty */
   _ecore_drm_message_send(ECORE_DRM_OP_TTY_CLOSE, dev->tty.fd,
                           NULL, 0);
//                           &dev->tty.fd, sizeof(int));

   /* get the result of the close operation */
   ret = _ecore_drm_message_receive(ECORE_DRM_OP_TTY_CLOSE, NULL,
                                    &data, sizeof(int));
   if (!ret) return EINA_FALSE;

   dev->tty.fd = -1;

   free(data);

   /* destroy the event handler */
   if (dev->tty.event_hdlr) ecore_event_handler_del(dev->tty.event_hdlr);
   dev->tty.event_hdlr = NULL;

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
   if ((!dev) || (!dev->devname) || (dev->tty.fd < 0)) return EINA_FALSE;

   /* send ioctl for vt release */
   if (ioctl(dev->tty.fd, VT_RELDISP, 1) < 0) return EINA_FALSE;

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
   if ((!dev) || (!dev->devname) || (dev->tty.fd < 0)) return EINA_FALSE;

   /* send ioctl for vt acquire */
   if (ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ) < 0) return EINA_FALSE;

   return EINA_TRUE;
}
