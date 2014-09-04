#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>

#ifndef KDSKBMUTE
# define KDSKBMUTE 0x4B51
#endif

static Eina_Bool 
_ecore_drm_tty_cb_signal(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Drm_Device *dev;
   Ecore_Event_Signal_User *ev;
   siginfo_t sigdata;

   dev = data;
   ev = event;

   sigdata = ev->data;
   if (sigdata.si_pid != getpid()) return ECORE_CALLBACK_RENEW;

   if (ev->number == 1)
     {
        Ecore_Drm_Input *input;
        Ecore_Drm_Output *output;
        Ecore_Drm_Sprite *sprite;
        Eina_List *l;

        DBG("Release VT");

        /* disable inputs (suspends) */
        EINA_LIST_FOREACH(dev->inputs, l, input)
          ecore_drm_inputs_disable(input);

        /* disable hardware cursor */
        EINA_LIST_FOREACH(dev->outputs, l, output)
          ecore_drm_output_cursor_size_set(output, 0, 0, 0);

        /* disable sprites */
        EINA_LIST_FOREACH(dev->sprites, l, sprite)
          ecore_drm_sprites_fb_set(sprite, 0, 0);

        /* close input fds ?? */

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
        Ecore_Drm_Output *output;
        Ecore_Drm_Input *input;
        Eina_List *l;

        DBG("Acquire VT");

        /* issue ioctl to acquire vt */
        if (ecore_drm_tty_acquire(dev))
          {
             /* set drm master */
             if (!ecore_drm_device_master_set(dev))
               ERR("Could not set drm master: %m");

             /* set output mode */
             EINA_LIST_FOREACH(dev->outputs, l, output)
               ecore_drm_output_enable(output);

             /* enable inputs */
             EINA_LIST_FOREACH(dev->inputs, l, input)
               ecore_drm_inputs_enable(input);
          }
        else
          ERR("Could not acquire VT: %m");
     }

   return ECORE_CALLBACK_RENEW;
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

   if (ioctl(dev->tty.fd, KDSETMODE, KD_GRAPHICS))
     {
        ERR("Could not set graphics mode: %m");
        return EINA_FALSE;
     }

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
          dev->tty.fd = STDIN_FILENO;
     }
   else // FIXME: NB: This should Really check for format of name (/dev/xyz)
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
   /* check for valid device */
   if ((!dev) || (!dev->drm.name)) return EINA_FALSE;

   close(dev->tty.fd);

   dev->tty.fd = -1;

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
   if ((!dev) || (!dev->drm.name) || (dev->tty.fd < 0)) return EINA_FALSE;

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
   if ((!dev) || (!dev->drm.name) || (dev->tty.fd < 0)) return EINA_FALSE;

   /* send ioctl for vt acquire */
   if (ioctl(dev->tty.fd, VT_RELDISP, VT_ACKACQ) < 0) return EINA_FALSE;

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
