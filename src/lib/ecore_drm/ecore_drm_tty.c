#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

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
     }
   else if (ev->number == 2)
     {
        DBG("Acquire VT");
     }

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
          snprintf(tty, sizeof(tty), "%s", "/dev/tty0");
     }
   else // FIXME: NB: This should Really check for format of name (/dev/xyz)
     snprintf(tty, sizeof(tty), "%s", name);

   if (dev->tty.fd < 0)
     {
        void *data;

        /* try to open the tty */
        _ecore_drm_message_send(ECORE_DRM_OP_TTY_OPEN, tty, strlen(tty));

        /* get the result of the open operation */
        ret = _ecore_drm_message_receive(ECORE_DRM_OP_TTY_OPEN, &data, sizeof(int));
        if (!ret)
          {
             DBG("Failed to receive Open Tty Message");
             return EINA_FALSE;
          }

        dev->tty.fd = *((int *)data);
     }

   if (dev->tty.fd < 0)
     {
        DBG("Failed to open tty %s", tty);
        return EINA_FALSE;
     }

   DBG("Opened Tty %s : %d", tty, dev->tty.fd);

   /* save tty name */
   dev->tty.name = eina_stringshare_add(tty);

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
   void *data;

   /* check for valid device */
   if ((!dev) || (!dev->devname)) return EINA_FALSE;

   /* try to close the tty */
   _ecore_drm_message_send(ECORE_DRM_OP_TTY_CLOSE, &dev->tty.fd, sizeof(int));

   /* get the result of the close operation */
   ret = _ecore_drm_message_receive(ECORE_DRM_OP_TTY_CLOSE, &data, sizeof(int));
   if (!ret) return EINA_FALSE;

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
