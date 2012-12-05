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

#ifndef XIPointerEmulated
#define XIPointerEmulated (1 << 16)
#endif

#ifdef ECORE_XI2
#ifdef ECORE_XI2_2
typedef struct _Ecore_X_Touch_Device_Info
{
   EINA_INLIST;
   int devid;
   int mode;
   const char *name;
   int max_touch;
   int *slot;
} Ecore_X_Touch_Device_Info;
#endif /* ifdef ECORE_XI2_2 */

static XIDeviceInfo *_ecore_x_xi2_devs = NULL;
static int _ecore_x_xi2_num = 0;
#ifdef ECORE_XI2_2
static Eina_Inlist *_ecore_x_xi2_touch_info_list = NULL;
#endif /* ifdef ECORE_XI2_2 */
#endif /* ifdef ECORE_XI2 */

void
_ecore_x_input_init(void)
{
#ifdef ECORE_XI2
   int event, error;
   int major = XI_2_Major, minor = XI_2_Minor;

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
}

#ifdef ECORE_XI2
#ifdef ECORE_XI2_2
static void
_ecore_x_input_touch_info_clear(void)
{
   Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
   Ecore_X_Touch_Device_Info *info = NULL;

   while (l)
     {
        info = EINA_INLIST_CONTAINER_GET(l, Ecore_X_Touch_Device_Info);
        l = eina_inlist_remove(l, l);
        free(info);
     }

   _ecore_x_xi2_touch_info_list = NULL;
}
#endif /* ifdef ECORE_XI2_2 */
#endif /* ifdef ECORE_XI2 */

void
_ecore_x_input_shutdown(void)
{
#ifdef ECORE_XI2
   if (_ecore_x_xi2_devs)
     {
        XIFreeDeviceInfo(_ecore_x_xi2_devs);
        _ecore_x_xi2_devs = NULL;
#ifdef ECORE_XI2_2
        _ecore_x_input_touch_info_clear();
#endif /* ifdef ECORE_XI2_2 */
     }

   _ecore_x_xi2_num = 0;
   _ecore_x_xi2_opcode = -1;
#endif /* ifdef ECORE_XI2 */
}

#ifdef ECORE_XI2
#ifdef ECORE_XI2_2
static int
_ecore_x_input_touch_index_get(int devid, int detail, int event_type)
{
   int i;
   Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
   Ecore_X_Touch_Device_Info *info = NULL;

   if ((!_ecore_x_xi2_devs) || (!_ecore_x_xi2_touch_info_list))
     return 0;

   EINA_INLIST_FOREACH(l, info)
     if (info->devid == devid) break;

   if ((!info) || (!info->slot)) return 0;

   for (i = 0; i < info->max_touch ; i++)
     {
        int *p = &(info->slot[i]);

        if ((event_type == XI_TouchBegin) && (*p < 0))
          {
             *p = detail;
             return i;
          }
       else if (*p == detail)
         {
            return i;
         }
     }

   return 0;
}

static void
_ecore_x_input_touch_index_clear(int devid, int idx)
{
   Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
   Ecore_X_Touch_Device_Info *info = NULL;

   if ((!_ecore_x_xi2_devs) || (!_ecore_x_xi2_touch_info_list))
     return;

   EINA_INLIST_FOREACH(l, info)
     {
        if ((info->devid == devid) && (info->slot))
          {
             info->slot[idx] = -1;
             return;
          }
     }
}

static Ecore_X_Touch_Device_Info*
_ecore_x_input_touch_info_get(XIDeviceInfo* dev)
{
   int k;
   int *slot = NULL;
   XITouchClassInfo *t = NULL;
   Ecore_X_Touch_Device_Info *info = NULL;

   if (!dev)
     return NULL;

   for (k = 0; k < dev->num_classes; k++)
     {
        XIAnyClassInfo *class = dev->classes[k];

        if (class && (class->type == XITouchClass))
          {
             t = (XITouchClassInfo*)class;
             break;
          }
     }

   if (t && (t->type == XITouchClass))
     {
        info = calloc(1, sizeof(Ecore_X_Touch_Device_Info));
        if (!info) return NULL;

        slot = (int*)malloc(sizeof(int)*(t->num_touches + 1));
        if (!slot)
          {
             free(info);
             return NULL;
          }

        info->devid = dev->deviceid;
        info->max_touch = t->num_touches + 1;
        info->mode = t->mode;
        info->name = dev->name;
        memset(slot, -1, sizeof(int)*info->max_touch);
        info->slot = slot;
     }

   return info;
}
#endif /* ifdef ECORE_XI2_2 */
#endif

void
_ecore_x_input_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
   /* XIRawEvent *evr = (XIRawEvent *)(xevent->xcookie.data); */
   int devid = evd->deviceid;
   int i;

   /* No filter for this events */
   switch (xevent->xcookie.evtype)
     {
#ifdef XI_RawButtonPress
      case XI_RawButtonPress:
         ecore_event_add(ECORE_X_RAW_BUTTON_PRESS, NULL, NULL, NULL);
         break;
#endif
#ifdef XI_RawButtonRelease
      case XI_RawButtonRelease:
         ecore_event_add(ECORE_X_RAW_BUTTON_RELEASE, NULL, NULL, NULL);
         break;
#endif
#ifdef XI_RawMotion
      case XI_RawMotion:
         ecore_event_add(ECORE_X_RAW_MOTION, NULL, NULL, NULL);
         break;
#endif
     }

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
          0,   // state
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
          devid, 1, 1,
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
        break;

      case XI_ButtonPress:
        _ecore_mouse_button
          (ECORE_EVENT_MOUSE_BUTTON_DOWN,
          evd->time,
          0,   // state
          0,   // button
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
          devid, 1, 1,
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
        break;

      case XI_ButtonRelease:
        _ecore_mouse_button
          (ECORE_EVENT_MOUSE_BUTTON_UP,
          evd->time,
          0,   // state
          0,   // button
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
          devid, 1, 1,
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
        break;

#ifdef XI_TouchUpdate
      case XI_TouchUpdate:
#ifdef ECORE_XI2_2
        i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchUpdate);
#endif /* #ifdef ECORE_XI2_2 */
        _ecore_mouse_move
          (evd->time,
          0,   // state
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
#ifdef ECORE_XI2_2
          i, 1, 1,
#else
          devid, 1, 1,
#endif /* #ifdef ECORE_XI2_2 */
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
        break;
#endif

#ifdef XI_TouchBegin
      case XI_TouchBegin:
#ifdef ECORE_XI2_2
        i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchBegin);
#endif /* #ifdef ECORE_XI2_2 */
        _ecore_mouse_button
          (ECORE_EVENT_MOUSE_BUTTON_DOWN,
          evd->time,
          0,   // state
          0,   // button
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
#ifdef ECORE_XI2_2
          i, 1, 1,
#else
          devid, 1, 1,
#endif /* #ifdef ECORE_XI2_2 */
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
        break;
#endif

#ifdef XI_TouchEnd
      case XI_TouchEnd:
#ifdef ECORE_XI2_2
        i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchEnd);
#endif /* #ifdef ECORE_XI2_2 */
        _ecore_mouse_button
          (ECORE_EVENT_MOUSE_BUTTON_UP,
          evd->time,
          0,   // state
          0,   // button
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y,
          evd->event,
          (evd->child ? evd->child : evd->event),
          evd->root,
          1,   // same_screen
#ifdef ECORE_XI2_2
          i, 1, 1,
#else
          devid, 1, 1,
#endif /* #ifdef ECORE_XI2_2 */
          1.0,   // pressure
          0.0,   // angle
          evd->event_x, evd->event_y,
          evd->root_x, evd->root_y);
#ifdef ECORE_XI2_2
        _ecore_x_input_touch_index_clear(devid,  i);
#endif /* #ifdef ECORE_XI2_2 */
        break;
#endif

      default:
        break;
     }
#endif /* ifdef ECORE_XI2 */
}

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
             XIDeviceInfo *atdev = NULL;
             int j;

             for (j = 0; j < _ecore_x_xi2_num; j++)
               {
                  if (_ecore_x_xi2_devs[j].deviceid == dev->attachment)
                    atdev = &(_ecore_x_xi2_devs[j]);
               }
             if (((atdev) && (atdev->use != XIMasterPointer)) ||
                 (!atdev))
               {
                  XIEventMask eventmask;
                  unsigned char mask[4] = { 0 };

                  eventmask.deviceid = dev->deviceid;
                  eventmask.mask_len = sizeof(mask);
                  eventmask.mask = mask;
                  XISetMask(mask, XI_ButtonPress);
                  XISetMask(mask, XI_ButtonRelease);
                  XISetMask(mask, XI_Motion);
#ifdef ECORE_XI2_2
                  Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
                  Ecore_X_Touch_Device_Info *info;
                  info = _ecore_x_input_touch_info_get(dev);

                  if (info)
                    {
                       XISetMask(mask, XI_TouchUpdate);
                       XISetMask(mask, XI_TouchBegin);
                       XISetMask(mask, XI_TouchEnd);

                       l = eina_inlist_append(l, (Eina_Inlist *)info);
                       _ecore_x_xi2_touch_info_list = l;
                    }
#else
# ifdef XI_TouchUpdate
                  XISetMask(mask, XI_TouchUpdate);
# endif
# ifdef XI_TouchBegin
                  XISetMask(mask, XI_TouchBegin);
# endif
# ifdef XI_TouchEnd
                  XISetMask(mask, XI_TouchEnd);
# endif
#endif /* #ifdef ECORE_XI2_2 */

                  XISelectEvents(_ecore_x_disp, win, &eventmask, 1);
                  find = EINA_TRUE;
               }
#ifdef ECORE_XI2_2
             else if ((atdev) && (atdev->use == XIMasterPointer))
               {
                  Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
                  Ecore_X_Touch_Device_Info *info;
                  info = _ecore_x_input_touch_info_get(dev);

                  if (info)
                    {
                       XIEventMask eventmask;
                       unsigned char mask[4] = { 0 };

                       eventmask.deviceid = dev->deviceid;
                       eventmask.mask_len = sizeof(mask);
                       eventmask.mask = mask;

                       XISetMask(mask, XI_TouchUpdate);
                       XISetMask(mask, XI_TouchBegin);
                       XISetMask(mask, XI_TouchEnd);
                       XISelectEvents(_ecore_x_disp, win, &eventmask, 1);

                       l = eina_inlist_append(l, (Eina_Inlist *)info);
                       _ecore_x_xi2_touch_info_list = l;

                       find = EINA_TRUE;
                    }
               }
#endif /* #ifdef ECORE_XI2_2 */
          }
     }

   return find;
#else /* ifdef ECORE_XI2 */
   return EINA_FALSE;
#endif /* ifdef ECORE_XI2 */
}

EAPI Eina_Bool
ecore_x_input_raw_select(Ecore_X_Window win)
{
#ifdef ECORE_XI2
   XIEventMask emask;
   unsigned char mask[4] = { 0 };

   if (!_ecore_x_xi2_devs)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   emask.deviceid = XIAllMasterDevices;
   emask.mask_len = sizeof(mask);
   emask.mask = mask;
#ifdef XI_RawButtonPress
   XISetMask(emask.mask, XI_RawButtonPress);
#endif
#ifdef XI_RawButtonRelease
   XISetMask(emask.mask, XI_RawButtonRelease);
#endif
#ifdef XI_RawMotion
   XISetMask(emask.mask, XI_RawMotion);
#endif

   XISelectEvents(_ecore_x_disp, win, &emask, 1);

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

