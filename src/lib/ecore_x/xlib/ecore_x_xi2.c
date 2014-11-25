#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <string.h>
#include <math.h>

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
#ifndef XITouchEmulatingPointer
#define XITouchEmulatingPointer (1 << 17)
#endif

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
        if (info->slot) free(info->slot);
        free(info);
     }

   _ecore_x_xi2_touch_info_list = NULL;
}
#endif /* ifdef ECORE_XI2_2 */
#endif /* ifdef ECORE_XI2 */

#ifdef ECORE_XI2
static Atom
_ecore_x_input_get_axis_label(char *axis_name)
{
   static Atom *atoms = NULL;
   static char *names[] =
     {
        "Abs X", "Abs Y", "Abs Pressure",
        "Abs Distance", "Abs Rotary Z",
        "Abs Wheel", "Abs Tilt X", "Abs Tilt Y"
     };
   int n = sizeof(names) / sizeof(names[0]);
   int i;

   if (atoms == NULL)
     {
        atoms = calloc(n, sizeof(Atom));
        if (!atoms) return 0;

        if (!XInternAtoms(_ecore_x_disp, names, n, 1, atoms))
          {
             free(atoms);
             atoms = NULL;
             return 0;
          }
     }

   for (i = 0; i < n; i++)
     {
        if (!strcmp(axis_name, names[i])) return atoms[i];
     }

   return 0;
}
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

static Ecore_X_Touch_Device_Info *
_ecore_x_input_touch_info_get(XIDeviceInfo *dev)
{
   int k;
   int *slot = NULL;
   XITouchClassInfo *t = NULL;
   Ecore_X_Touch_Device_Info *info = NULL;

   if (!dev)
     return NULL;

   for (k = 0; k < dev->num_classes; k++)
     {
        XIAnyClassInfo *clas = dev->classes[k];

        if (clas && (clas->type == XITouchClass))
          {
             t = (XITouchClassInfo *)clas;
             break;
          }
     }

   if (t && (t->type == XITouchClass))
     {
        info = calloc(1, sizeof(Ecore_X_Touch_Device_Info));
        if (!info) return NULL;

        slot = malloc(sizeof(int) * (t->num_touches + 1));
        if (!slot)
          {
             free(info);
             return NULL;
          }

        info->devid = dev->deviceid;
        info->max_touch = t->num_touches + 1;
        info->mode = t->mode;
        info->name = dev->name;
        memset(slot, -1, sizeof(int) * info->max_touch);
        info->slot = slot;
     }

   return info;
}
#endif /* ifdef ECORE_XI2_2 */
#endif

void
_ecore_x_input_raw_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;

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
#endif /* ifdef ECORE_XI2 */
}

void
_ecore_x_input_mouse_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;

   XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
   int devid = evd->deviceid;

   switch (xevent->xcookie.evtype)
     {
      case XI_Motion:
        INF("Handling XI_Motion");
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
        INF("ButtonEvent:multi press time=%u x=%d y=%d devid=%d", (unsigned int)evd->time, (int)evd->event_x, (int)evd->event_y, devid);
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
        INF("ButtonEvent:multi release time=%u x=%d y=%d devid=%d", (unsigned int)evd->time, (int)evd->event_x, (int)evd->event_y, devid);
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
      }
#endif /* ifdef ECORE_XI2 */
}

void
_ecore_x_input_multi_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;
   XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
   int devid = evd->deviceid;

   switch (xevent->xcookie.evtype)
     {
#ifdef XI_TouchUpdate
      case XI_TouchUpdate:
          {
#ifdef ECORE_XI2_2
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchUpdate);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer)) return;
#endif /* #ifdef ECORE_XI2_2 */
             INF("Handling XI_TouchUpdate");
             _ecore_mouse_move(evd->time,
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
#endif
          }
        break;

#ifdef XI_TouchBegin
      case XI_TouchBegin:
          {
#ifdef ECORE_XI2_2
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchBegin);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer)) return;
#endif /* #ifdef ECORE_XI2_2 */
             INF("Handling XI_TouchBegin");
             _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_DOWN,
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
#endif
          }
        break;

#ifdef XI_TouchEnd
      case XI_TouchEnd:
          {
#ifdef ECORE_XI2_2
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchEnd);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer))
               {
                  _ecore_x_input_touch_index_clear(devid,  i);
                  return;
               }
#endif /* #ifdef ECORE_XI2_2 */
             INF("Handling XI_TouchEnd");
             _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_UP,
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
#endif
          }
        break;
      default:
        break;
      }
#endif /* ifdef ECORE_XI2 */
}

#ifdef ECORE_XI2
static unsigned int
_ecore_x_count_bits(unsigned long n)
{
   unsigned int c; /* c accumulates the total bits set in v */
   for (c = 0; n; c++) n &= n - 1; /* clear the least significant bit set */
   return c;
}
#endif

#ifdef ECORE_XI2
void
_ecore_x_input_axis_handler(XEvent *xevent, XIDeviceInfo *dev)
{
   if (xevent->type != GenericEvent) return;
   XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
   unsigned int n = _ecore_x_count_bits(*evd->valuators.mask);
   int i;
   int j = 0;
   double tiltx = 0, tilty = 0;
   Eina_Bool compute_tilt = EINA_FALSE;
   Ecore_Axis *axis = calloc(n, sizeof(Ecore_Axis));
   if (!axis) return;
   Ecore_Axis *axis_ptr = axis;

   for (i = 0; i < dev->num_classes; i++)
     {
        if (dev->classes[i]->type == XIValuatorClass)
          {
             XIValuatorClassInfo *inf = ((XIValuatorClassInfo *)dev->classes[i]);

             if (*evd->valuators.mask & (1 << inf->number))
               {
                  if (inf->label == _ecore_x_input_get_axis_label("Abs X"))
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_X;
                       axis_ptr->value = evd->valuators.values[j];
                       axis_ptr++;
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Y"))
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_Y;
                       axis_ptr->value = evd->valuators.values[j];
                       axis_ptr++;
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Pressure"))
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_PRESSURE;
                       axis_ptr->value = (evd->valuators.values[j] - inf->min) / (inf->max - inf->min);
                       axis_ptr++;
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Distance"))
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_DISTANCE;
                       axis_ptr->value = (evd->valuators.values[j] - inf->min) / (inf->max - inf->min);
                       axis_ptr++;
                    }
                  else if ((inf->label == _ecore_x_input_get_axis_label("Abs Rotary Z")) ||
                           (inf->label == _ecore_x_input_get_axis_label("Abs Wheel")))
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_TWIST;
                       if (inf->resolution == 1)
                         {
                            /* some wacom drivers do not correctly report resolution, so pre-normalize */
                            axis_ptr->value = 2*((evd->valuators.values[j] - inf->min) / (inf->max - inf->min)) - 1;
                            axis_ptr->value *= M_PI;
                         }
                       else
                         {
                            axis_ptr->value = evd->valuators.values[j] / inf->resolution;
                         }
                       axis_ptr++;
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Tilt X"))
                    {
                       tiltx = evd->valuators.values[j] / inf->resolution;
                       compute_tilt = EINA_TRUE;
                       /* don't increment axis_ptr */
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Tilt Y"))
                    {
                       tilty = -evd->valuators.values[j] / inf->resolution;
                       compute_tilt = EINA_TRUE;
                       /* don't increment axis_ptr */
                    }
                  else
                    {
                       axis_ptr->label = ECORE_AXIS_LABEL_UNKNOWN;
                       axis_ptr->value = evd->valuators.values[j];
                       axis_ptr++;
                    }
                  j++;
               }
          }
     }

   if ((compute_tilt) && ((axis_ptr + 2) <= (axis + n)))
     {
        double x = sin(tiltx);
        double y = sin(tilty);
        axis_ptr->label = ECORE_AXIS_LABEL_TILT;
        axis_ptr->value = asin(sqrt((x * x) + (y * y)));
        axis_ptr++;

        /* note: the value of atan2(0,0) is implementation-defined */
        axis_ptr->label = ECORE_AXIS_LABEL_AZIMUTH;
        axis_ptr->value = atan2(y, x);
        axis_ptr++;
     }

   /* update n to reflect actual count and realloc array to free excess */
   n = (axis_ptr - axis);
   Ecore_Axis *shrunk_axis = realloc(axis, n * sizeof(Ecore_Axis));
   if (shrunk_axis != NULL) axis = shrunk_axis;

   if (n > 0)
     _ecore_x_axis_update(evd->child ? evd->child : evd->event,
                          evd->event, evd->root, evd->time, evd->deviceid,
                          evd->detail, n, axis);
}
#endif /* ifdef ECORE_XI2 */

#ifdef ECORE_XI2
static XIDeviceInfo *
_ecore_x_input_device_lookup(int deviceid)
{
   XIDeviceInfo *dev;
   int i;

   if (_ecore_x_xi2_devs)
     {
        for (i = 0; i < _ecore_x_xi2_num; i++)
          {
             dev = &(_ecore_x_xi2_devs[i]);
             if (deviceid == dev->deviceid) return dev;
          }
     }
   return NULL;
}
#endif

void
_ecore_x_input_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;

   switch (xevent->xcookie.evtype)
     {
      case XI_RawMotion:
      case XI_RawButtonPress:
      case XI_RawButtonRelease:
        _ecore_x_input_raw_handler(xevent);
        break;

      case XI_Motion:
      case XI_ButtonPress:
      case XI_ButtonRelease:
      case XI_TouchUpdate:
      case XI_TouchBegin:
      case XI_TouchEnd:
          {
             XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
             XIDeviceInfo *dev = _ecore_x_input_device_lookup(evd->deviceid);

             if ((dev->use == XISlavePointer) &&
                 !(evd->flags & XIPointerEmulated))
               _ecore_x_input_multi_handler(xevent);
             else if (dev->use == XIFloatingSlave)
               _ecore_x_input_mouse_handler(xevent);

             if (dev->use != XIMasterPointer)
               _ecore_x_input_axis_handler(xevent, dev);
          }
        break;
      default:
        break;
     }
#endif /* ifdef ECORE_XI2 */
}

EAPI Eina_Bool
ecore_x_input_select(Ecore_X_Window win)
{
#ifdef ECORE_XI2
   int i;
   Eina_Bool find = EINA_FALSE;

   if (!_ecore_x_xi2_devs)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   for (i = 0; i < _ecore_x_xi2_num; i++)
     {
        XIDeviceInfo *dev = &(_ecore_x_xi2_devs[i]);
        XIEventMask eventmask;
        unsigned char mask[4] = { 0 };
        int update = 0;

        eventmask.deviceid = dev->deviceid;
        eventmask.mask_len = sizeof(mask);
        eventmask.mask = mask;

        if ((dev->use == XIFloatingSlave) || (dev->use == XISlavePointer))
          {
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
                  update = 1;

                  l = eina_inlist_append(l, (Eina_Inlist *)info);
                  _ecore_x_xi2_touch_info_list = l;
               }
#endif /* #ifdef ECORE_XI2_2 */

#if !defined (ECORE_XI2_2) && defined (XI_TouchUpdate) && defined (XI_TouchBegin) && defined (XI_TouchEnd)
             XISetMask(mask, XI_TouchUpdate);
             XISetMask(mask, XI_TouchBegin);
             XISetMask(mask, XI_TouchEnd);
#endif

             update = 1;
          }

        if (update)
          {
             XISelectEvents(_ecore_x_disp, win, &eventmask, 1);
             if (_ecore_xlib_sync) ecore_x_sync();
             find = EINA_TRUE;
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
   if (_ecore_xlib_sync) ecore_x_sync();

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

