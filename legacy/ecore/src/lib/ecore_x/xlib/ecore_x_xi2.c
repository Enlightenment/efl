#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

#ifdef ECORE_XI2
#include "Ecore_Input.h"
#endif /* ifdef ECORE_XI2 */

int _ecore_x_xi2_opcode = -1;

#ifdef ECORE_XI2
static XIDeviceInfo *_ecore_x_xi2_devs = NULL;
static int _ecore_x_xi2_num = 0;
#endif /* ifdef ECORE_XI2 */

void
_ecore_x_input_init(void)
{
#ifdef ECORE_XI2
   int event, error;
   int major = 2, minor = 0;

   if (!XQueryExtension(_ecore_x_disp, "XInputExtension",
                        &_ecore_x_xi2_opcode, &event, &error))
     {
        _ecore_x_xi2_opcode = -1;
        return;
     }

   if (XIQueryVersion(_ecore_x_disp, &major, &minor) == BadRequest)
     {
        _ecore_x_xi2_opcode = -1;
        return;
     }

   _ecore_x_xi2_devs = XIQueryDevice(_ecore_x_disp, XIAllDevices,
                                     &_ecore_x_xi2_num);
#endif /* ifdef ECORE_XI2 */
} /* _ecore_x_input_init */

void
_ecore_x_input_shutdown(void)
{
#ifdef ECORE_XI2
   if (_ecore_x_xi2_devs)
     {
        XIFreeDeviceInfo(_ecore_x_xi2_devs);
        _ecore_x_xi2_devs = NULL;
     }

   _ecore_x_xi2_num = 0;
   _ecore_x_xi2_opcode = -1;
#endif /* ifdef ECORE_XI2 */
} /* _ecore_x_input_shutdown */

void
_ecore_x_input_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
   int devid = evd->deviceid;
   int i;
   
   if (_ecore_x_xi2_devs)
     {
        for (i = 0; i < _ecore_x_xi2_num; i++)
          {
             XIDeviceInfo *dev = &(_ecore_x_xi2_devs[i]);
             
             if (devid == dev->deviceid)
               {
                  if (dev->use == XIMasterPointer) return;
                  if ((dev->use == XISlavePointer) &&
                      (evd->flags & XIPointerEmulated)) return;
               }
          }
     }
   switch (xevent->xcookie.evtype)
     {
      case XI_Motion:
         _ecore_mouse_move
            (evd->time,
            0, // state
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
         break;

      case XI_ButtonPress:
         _ecore_mouse_button
            (ECORE_EVENT_MOUSE_BUTTON_DOWN,
            evd->time,
            0, // state
            0, // button
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
         break;

      case XI_ButtonRelease:
         _ecore_mouse_button
            (ECORE_EVENT_MOUSE_BUTTON_UP,
            evd->time,
            0, // state
            0, // button
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
         break;

#ifdef XI_TouchUpdate        
      case XI_TouchUpdate:
         _ecore_mouse_move
            (evd->time,
            0, // state
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
        break;
#endif
#ifdef XI_TouchBegin
      case XI_TouchBegin:
         _ecore_mouse_button
            (ECORE_EVENT_MOUSE_BUTTON_DOWN,
            evd->time,
            0, // state
            0, // button
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
        break;
#endif
#ifdef XI_TouchEnd
      case XI_TouchEnd:
         _ecore_mouse_button
            (ECORE_EVENT_MOUSE_BUTTON_UP,
            evd->time,
            0, // state
            0, // button
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y,
            evd->event,
            (evd->child ? evd->child : evd->event),
            evd->root,
            1, // same_screen
            devid, 1, 1,
            1.0, // pressure
            0.0, // angle
            evd->event_x, evd->event_y,
            evd->root_x, evd->root_y);
        break;
#endif
      default:
        break;
     } /* switch */
#endif /* ifdef ECORE_XI2 */
} /* _ecore_x_input_handler */

EAPI Eina_Bool
ecore_x_input_multi_select(Ecore_X_Window win)
{
#ifdef ECORE_XI2
   int i;
   Eina_Bool find = EINA_FALSE;

   if (!_ecore_x_xi2_devs)
      return 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   for (i = 0; i < _ecore_x_xi2_num; i++)
     {
        XIDeviceInfo *dev = &(_ecore_x_xi2_devs[i]);

        if (dev->use == XIFloatingSlave)
          {
             XIEventMask eventmask;
             unsigned char mask[4] = { 0 };

             eventmask.deviceid = dev->deviceid;
             eventmask.mask_len = sizeof(mask);
             eventmask.mask = mask;
             XISetMask(mask, XI_ButtonPress);
             XISetMask(mask, XI_ButtonRelease);
             XISetMask(mask, XI_Motion);
             XISelectEvents(_ecore_x_disp, win, &eventmask, 1);
             find = EINA_TRUE;
          }
        else if (dev->use == XISlavePointer)
          {
             XIEventMask eventmask;
             unsigned char mask[4] = { 0 };

             eventmask.deviceid = dev->deviceid;
             eventmask.mask_len = sizeof(mask);
             eventmask.mask = mask;
             XISetMask(mask, XI_ButtonPress);
             XISetMask(mask, XI_ButtonRelease);
             XISetMask(mask, XI_Motion);
#ifdef XI_TouchUpdate
             XISetMask(mask, XI_TouchUpdate);
#endif             
#ifdef XI_TouchBegin
             XISetMask(mask, XI_TouchBegin);
#endif             
#ifdef XI_TouchEnd
             XISetMask(mask, XI_TouchEnd);
#endif             
             XISelectEvents(_ecore_x_disp, win, &eventmask, 1);
             find = EINA_TRUE;
          }
     }

   return find;
#else /* ifdef ECORE_XI2 */
   return EINA_FALSE;
#endif /* ifdef ECORE_XI2 */
} /* ecore_x_input_multi_select */

