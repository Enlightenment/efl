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

static Ecore_Job *update_devices_job = NULL;
static XIDeviceInfo *_ecore_x_xi2_devs = NULL;
static int _ecore_x_xi2_num = 0;
#ifdef ECORE_XI2_2
static Eina_Inlist *_ecore_x_xi2_touch_info_list = NULL;
#endif /* ifdef ECORE_XI2_2 */
static Eina_List *_ecore_x_xi2_grabbed_devices_list;
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

   // listen for device changes
   XIEventMask m;
   m.deviceid = XIAllDevices;
   m.mask_len = XIMaskLen(XI_LASTEVENT);
   m.mask = calloc( m.mask_len, 1);
   if (!m.mask) return;
   XISetMask(m.mask, XI_DeviceChanged);
   XISetMask(m.mask, XI_HierarchyChanged);
   XISetMask(m.mask, XI_PropertyEvent);
   XISelectEvents(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), &m, 1);
   free(m.mask);

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
        "Abs Wheel", "Abs Tilt X", "Abs Tilt Y",
        "Rel X", "Rel Y", "Rel Dial", "Rel Horiz Wheel", "Rel Vert Wheel"
     };
   int n = sizeof(names) / sizeof(names[0]);
   int i;

   if (EINA_UNLIKELY(atoms == NULL))
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
   XIEventMask m;
   m.deviceid = XIAllDevices;
   m.mask_len = XIMaskLen(XI_LASTEVENT);
   m.mask = calloc( m.mask_len, 1);
   if (m.mask)
     {
        XISelectEvents(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), &m, 1);
        free(m.mask);
     }
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

   if (_ecore_x_xi2_grabbed_devices_list)
     eina_list_free(_ecore_x_xi2_grabbed_devices_list);
   _ecore_x_xi2_grabbed_devices_list = NULL;
#endif /* ifdef ECORE_XI2 */
   if (update_devices_job) ecore_job_del(update_devices_job);
   update_devices_job = NULL;
}

#ifdef ECORE_XI2
#ifdef ECORE_XI2_2

# ifdef XI_TouchCancel
static Eina_Bool
_ecore_x_input_touch_device_check(int devid)
{
   Eina_Inlist *l = _ecore_x_xi2_touch_info_list;
   Ecore_X_Touch_Device_Info *info = NULL;

   if ((!_ecore_x_xi2_devs) || (!_ecore_x_xi2_touch_info_list))
     return EINA_FALSE;

   EINA_INLIST_FOREACH(l, info)
     if (info->devid == devid) return EINA_TRUE;
   return EINA_FALSE;
}
#endif

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

#ifdef ECORE_XI2_2
static Eina_Bool
_ecore_x_input_grabbed_is(int deviceId)
{
   void *id;
   Eina_List *l;

   EINA_LIST_FOREACH(_ecore_x_xi2_grabbed_devices_list, l, id)
     {
        if (deviceId == (intptr_t)id)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}
#endif /* ifdef ECORE_XI2_2 */

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

//XI_TouchUpdate, XI_TouchBegin, XI_TouchEnd only available in XI2_2
//So it is better using ECORE_XI2_2 define than XI_TouchXXX defines.
void
_ecore_x_input_multi_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;

   switch (xevent->xcookie.evtype)
     {
#ifdef ECORE_XI2_2
      case XI_TouchUpdate:
          {
             XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
             int devid = evd->deviceid;
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchUpdate);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer) && !_ecore_x_input_grabbed_is(devid)) return;
             INF("Handling XI_TouchUpdate");
             _ecore_mouse_move(evd->time,
                               0,   // state
                               evd->event_x, evd->event_y,
                               evd->root_x, evd->root_y,
                               evd->event,
                               (evd->child ? evd->child : evd->event),
                               evd->root,
                               1,   // same_screen
                               i, 1, 1,
                               1.0,   // pressure
                               0.0,   // angle
                               evd->event_x, evd->event_y,
                               evd->root_x, evd->root_y);
          }
        break;

      case XI_TouchBegin:
          {
             XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
             int devid = evd->deviceid;
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchBegin);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer) && !_ecore_x_input_grabbed_is(devid)) return;
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
                                 i, 1, 1,
                                 1.0,   // pressure
                                 0.0,   // angle
                                 evd->event_x, evd->event_y,
                                 evd->root_x, evd->root_y);
          }
        break;

      case XI_TouchEnd:
          {
             XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
             int devid = evd->deviceid;
             int i = _ecore_x_input_touch_index_get(devid, evd->detail, XI_TouchEnd);
             if ((i == 0) && (evd->flags & XITouchEmulatingPointer) && !_ecore_x_input_grabbed_is(devid))
               {
                  _ecore_x_input_touch_index_clear(devid,  i);
                  return;
               }
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
                                 i, 1, 1,
                                 1.0,   // pressure
                                 0.0,   // angle
                                 evd->event_x, evd->event_y,
                                 evd->root_x, evd->root_y);
             _ecore_x_input_touch_index_clear(devid,  i);
          }
        break;
#endif /* ifdef ECORE_XI2_2 */
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
   unsigned int n = _ecore_x_count_bits(*evd->valuators.mask) + 4;
   int i;
   int j = 0;
   double tiltx = 0, tilty = 0;
   Eina_Bool compute_tilt = EINA_FALSE;
   Ecore_Axis *axis = calloc(n, sizeof(Ecore_Axis));
   if (!axis) return;
   Ecore_Axis *axis_ptr = axis;
   Ecore_Axis *shrunk_axis;

   for (i = 0; i < dev->num_classes; i++)
     {
        if (dev->classes[i]->type == XIValuatorClass)
          {
             XIValuatorClassInfo *inf = ((XIValuatorClassInfo *)dev->classes[i]);

             if (*evd->valuators.mask & (1 << inf->number))
               {
                  if (inf->label == _ecore_x_input_get_axis_label("Abs X"))
                    {
                       int x = evd->valuators.values[j];
                       axis_ptr->label = ECORE_AXIS_LABEL_X;
                       axis_ptr->value = x;
                       axis_ptr++;
                       if (inf->max > inf->min)
                         {
                            axis_ptr->label = ECORE_AXIS_LABEL_NORMAL_X;
                            axis_ptr->value = (x - inf->min) / (inf->max - inf->min);
                            axis_ptr++;
                         }
                    }
                  else if (inf->label == _ecore_x_input_get_axis_label("Abs Y"))
                    {
                       int y = evd->valuators.values[j];
                       axis_ptr->label = ECORE_AXIS_LABEL_Y;
                       axis_ptr->value = y;
                       axis_ptr++;
                       if (inf->max > inf->min)
                         {
                            axis_ptr->label = ECORE_AXIS_LABEL_NORMAL_Y;
                            axis_ptr->value = (y - inf->min) / (inf->max - inf->min);
                            axis_ptr++;
                         }
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
                  else if ((inf->label == _ecore_x_input_get_axis_label("Rel X")) ||
                           (inf->label == _ecore_x_input_get_axis_label("Rel Y")) ||
                           (inf->label == _ecore_x_input_get_axis_label("Rel Vert Wheel")) ||
                           (inf->label == _ecore_x_input_get_axis_label("Rel Horiz Wheel")) ||
                           (inf->label == _ecore_x_input_get_axis_label("Rel Dial")))
                    {
                       /* Ignore those: mouse. Values are in fact not relative.
                        * No idea what is a "dial" event. */
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
   if (n > 0)
     {
        /* event position in the window - most useful */
        axis_ptr->label = ECORE_AXIS_LABEL_WINDOW_X;
        axis_ptr->value = evd->event_x;
        axis_ptr++;
        axis_ptr->label = ECORE_AXIS_LABEL_WINDOW_Y;
        axis_ptr->value = evd->event_y;
        axis_ptr++;
        n += 2;

        shrunk_axis = realloc(axis, n * sizeof(Ecore_Axis));
        if (shrunk_axis != NULL) axis = shrunk_axis;
        _ecore_x_axis_update(evd->child ? evd->child : evd->event,
                             evd->event, evd->root, evd->time, evd->deviceid,
                             evd->detail, n, axis);
     }
   else
     free(axis);
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

static void
_cb_update_devices(void *data EINA_UNUSED)
{
   update_devices_job = NULL;
   ecore_x_input_devices_update();
   ecore_event_add(ECORE_X_DEVICES_CHANGE, NULL, NULL, NULL);
}

void
_ecore_x_input_handler(XEvent *xevent)
{
#ifdef ECORE_XI2
   if (xevent->type != GenericEvent) return;

   switch (xevent->xcookie.evtype)
     {
      case XI_DeviceChanged:
      case XI_HierarchyChanged:
      case XI_PropertyEvent:
        if (update_devices_job) ecore_job_del(update_devices_job);
        update_devices_job = ecore_job_add(_cb_update_devices, NULL);
        // XXX: post change event
        break;

      case XI_RawMotion:
      case XI_RawButtonPress:
      case XI_RawButtonRelease:
        _ecore_x_input_raw_handler(xevent);
        break;

      case XI_Motion:
      case XI_ButtonPress:
      case XI_ButtonRelease:
#ifdef ECORE_XI2_2
      case XI_TouchUpdate:
      case XI_TouchBegin:
      case XI_TouchEnd:
#endif
          {
             XIDeviceEvent *evd = (XIDeviceEvent *)(xevent->xcookie.data);
             XIDeviceInfo *dev = _ecore_x_input_device_lookup(evd->deviceid);

             if (!dev) return;

             if ((dev->use == XISlavePointer) &&
                 !(evd->flags & XIPointerEmulated))
               {
                  _ecore_x_input_multi_handler(xevent);
               }
             else if (dev->use == XIFloatingSlave)
               _ecore_x_input_mouse_handler(xevent);

             if (dev->use != XIMasterPointer)
               _ecore_x_input_axis_handler(xevent, dev);
          }
        break;
#ifdef XI_TouchCancel
      case XI_TouchCancel:
          {
             XITouchCancelEvent *evd = (XITouchCancelEvent *)(xevent->xcookie.data);
             int devid = evd->deviceid;

             if(!_ecore_x_input_touch_device_check(devid)) return;

             INF("Handling XI_TouchCancel device(%d)", devid);

             /* Currently X sends only one cancel event according to the touch device.
                But in the future, it maybe need several cancel events according to the touch.
                So it is better use button structure instead of creating new cancel structure.
              */
             _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_CANCEL,
                                 evd->time,
                                 0,   // state
                                 0,   // button
                                 0, 0,
                                 0, 0,
                                 evd->event,
                                (evd->child ? evd->child : evd->event),
                                 evd->root,
                                 1,   // same_screen
                                 0, 1, 1,
                                 0.0,   // pressure
                                 0.0,   // angle
                                 0, 0,
                                 0, 0);
          }
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
     return EINA_FALSE;

   LOGFN;
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
#ifdef XI_TouchCancel
                  XISetMask(mask, XI_TouchCancel);
#endif
                  update = 1;

                  l = eina_inlist_append(l, (Eina_Inlist *)info);
                  _ecore_x_xi2_touch_info_list = l;
               }
#endif /* #ifdef ECORE_XI2_2 */
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

   LOGFN;
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

EAPI Eina_Bool
_ecore_x_input_touch_devices_grab(Ecore_X_Window grab_win, Eina_Bool grab)
{
#ifdef ECORE_XI2
   int i;

   if (!_ecore_x_xi2_devs)
     return EINA_FALSE;

   Eina_Bool status = EINA_FALSE;

   LOGFN;
   for (i = 0; i < _ecore_x_xi2_num; i++)
     {
        XIDeviceInfo *dev = &(_ecore_x_xi2_devs[i]);
        int update = 0;
        XIEventMask eventmask;
        unsigned char mask[4] = { 0 };

        eventmask.deviceid = XISlavePointer;
        eventmask.mask_len = sizeof(mask);
        eventmask.mask = mask;

        if (dev->use == XISlavePointer)
          {
#ifdef ECORE_XI2_2
             Ecore_X_Touch_Device_Info *info;
             info = _ecore_x_input_touch_info_get(dev);

             if (info)
               {
                  XISetMask(mask, XI_TouchUpdate);
                  XISetMask(mask, XI_TouchBegin);
                  XISetMask(mask, XI_TouchEnd);
#ifdef XI_TouchCancel
                  XISetMask(mask, XI_TouchCancel);
#endif
                  update = 1;
                  free(info);
               }
#endif /* #ifdef ECORE_XI2_2 */
          }

        if (update)
          {
             if (grab) {
                status |= (XIGrabDevice(_ecore_x_disp, dev->deviceid, grab_win, CurrentTime,
                           None, GrabModeAsync, GrabModeAsync, False, &eventmask) == GrabSuccess);
                _ecore_x_xi2_grabbed_devices_list = eina_list_append(_ecore_x_xi2_grabbed_devices_list, (void*)(intptr_t)dev->deviceid);
             }
             else {
                status |= (XIUngrabDevice(_ecore_x_disp, dev->deviceid, CurrentTime) == Success);
                _ecore_x_xi2_grabbed_devices_list = eina_list_remove(_ecore_x_xi2_grabbed_devices_list, (void*)(intptr_t)dev->deviceid);
             }
             if (_ecore_xlib_sync) ecore_x_sync();
          }
     }

   return status;
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_input_touch_devices_grab(Ecore_X_Window grab_win)
{
   return _ecore_x_input_touch_devices_grab(grab_win, EINA_TRUE);
}

EAPI Eina_Bool
ecore_x_input_touch_devices_ungrab(void)
{
   return _ecore_x_input_touch_devices_grab(0, EINA_FALSE);
}

// XXX
EAPI void
ecore_x_input_devices_update(void)
{
   if (_ecore_x_xi2_devs) XIFreeDeviceInfo(_ecore_x_xi2_devs);
   _ecore_x_xi2_num = 0;
   _ecore_x_xi2_devs = XIQueryDevice(_ecore_x_disp, XIAllDevices,
                                     &_ecore_x_xi2_num);
}

EAPI int
ecore_x_input_device_num_get(void)
{
   return _ecore_x_xi2_num;
}

EAPI int
ecore_x_input_device_id_get(int slot)
{
   if ((slot < 0) || (slot >= _ecore_x_xi2_num)) return 0;
   return _ecore_x_xi2_devs[slot].deviceid;
}

EAPI const char *
ecore_x_input_device_name_get(int slot)
{
   if ((slot < 0) || (slot >= _ecore_x_xi2_num)) return NULL;
   return _ecore_x_xi2_devs[slot].name;
}

EAPI char **
ecore_x_input_device_properties_list(int slot, int *num_ret)
{
   char **atoms = NULL;
   int num = 0, i;
   Atom *a;

   if ((slot < 0) || (slot >= _ecore_x_xi2_num)) goto err;
   a = XIListProperties(_ecore_x_disp, _ecore_x_xi2_devs[slot].deviceid, &num);
   if (!a) goto err;
   atoms = calloc(num, sizeof(char *));
   if (!atoms) goto err;
   for (i = 0; i < num; i++)
     {
        Ecore_X_Atom at = a[i];
        atoms[i] = ecore_x_atom_name_get(at);
        if (!atoms[i]) goto err;
     }
   XFree(a);
   *num_ret = num;
   return atoms;
err:
   if (atoms)
     {
        for (i = 0; i < num; i++) free(atoms[i]);
        free(atoms);
     }
   *num_ret = 0;
   return NULL;
}

EAPI void
ecore_x_input_device_properties_free(char **list, int num)
{
   int i;

   for (i = 0; i < num; i++) free(list[i]);
   free(list);
}

// unit_size_ret will be 8, 16, 32
// fromat_ret will almost be one of:
// ECORE_X_ATOM_CARDINAL
// ECORE_X_ATOM_INTEGER
// ECORE_X_ATOM_FLOAT (unit_size 32 only)
// ECORE_X_ATOM_ATOM // very rare
// ECORE_X_ATOM_STRING (unit_size 8 only - guaratee nul termination)

EAPI void *
ecore_x_input_device_property_get(int slot, const char *prop, int *num_ret,
                                  Ecore_X_Atom *format_ret, int *unit_size_ret)
{
   Atom a, a_type = 0;
   int fmt = 0;
   unsigned long num = 0, dummy;
   unsigned char *data = NULL;
   unsigned char *d = NULL;

   if ((slot < 0) || (slot >= _ecore_x_xi2_num)) goto err;
   a = XInternAtom(_ecore_x_disp, prop, False);
   // XIGetProperty returns 0 AKA `Success` if everything is good
   if (XIGetProperty(_ecore_x_disp, _ecore_x_xi2_devs[slot].deviceid,
                      a, 0, 65536, False, AnyPropertyType, &a_type, &fmt,
                      &num, &dummy, &data) != 0) goto err;
   *format_ret = a_type;
   *num_ret = num;
   *unit_size_ret = fmt;
   if ((a_type == ECORE_X_ATOM_STRING) && (fmt == 8))
     {
        d = malloc(num + 1);
        if (!d) goto err2;
        memcpy(d, data, num);
        d[num] = 0;
     }
   else
     {
        if      (fmt == 8 ) d = malloc(num);
        else if (fmt == 16) d = malloc(num * 2);
        else if (fmt == 32) d = malloc(num * 4);
        if (!d) goto err2;
        memcpy(d, data, num * (fmt / 8));
     }
   XFree(data);
   return d;
err2:
   XFree(data);
err:
   *num_ret = 0;
   *format_ret = 0;
   *unit_size_ret = 0;
   return NULL;
}

EAPI void
ecore_x_input_device_property_set(int slot, const char *prop, void *data,
                                  int num, Ecore_X_Atom format, int unit_size)
{
   Atom a, a_type = 0;
   if ((slot < 0) || (slot >= _ecore_x_xi2_num)) return;
   a = XInternAtom(_ecore_x_disp, prop, False);
   a_type = format;
   XIChangeProperty(_ecore_x_disp, _ecore_x_xi2_devs[slot].deviceid,
                    a, a_type, unit_size, XIPropModeReplace, data, num);
}

// XXX: add api's to get XIDeviceInfo->... stuff like
// use, attachement, enabled, num_classes, classes (which list number of
// buttons and their names, keycodes, valuators (mouse etc.), touch devices
// etc.
