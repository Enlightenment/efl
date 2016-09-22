#include "ecore_drm2_private.h"

#ifndef DRM_CAP_CURSOR_WIDTH
# define DRM_CAP_CURSOR_WIDTH 0x8
#endif

#ifndef DRM_CAP_CURSOR_HEIGHT
# define DRM_CAP_CURSOR_HEIGHT 0x9
#endif

#ifdef HAVE_ATOMIC_DRM
# include <sys/utsname.h>
#endif

Eina_Bool _ecore_drm2_use_atomic = EINA_FALSE;

static Eina_Bool
_cb_session_active(void *data, int type EINA_UNUSED, void *event)
{
   Eina_List *l;
   Ecore_Drm2_Device *device;
   Ecore_Drm2_Output *output;
   Elput_Event_Session_Active *ev;
   Ecore_Drm2_Event_Activate *ea;

   ev = event;
   device = data;

   EINA_LIST_FOREACH(device->outputs, l, output)
     ecore_drm2_output_enabled_set(output, ev->active);

   ea = calloc(1, sizeof(Ecore_Drm2_Event_Activate));
   if (!ea) return ECORE_CALLBACK_RENEW;

   ea->active = ev->active;

   ecore_event_add(ECORE_DRM2_EVENT_ACTIVATE, ea, NULL, NULL);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_cb_device_change(void *data, int type EINA_UNUSED, void *event)
{
   Elput_Event_Device_Change *ev = event;
   Ecore_Drm2_Device *device = data;

   if (ev->type == ELPUT_DEVICE_ADDED)
     {
        Eina_List *l;
        Ecore_Drm2_Output *output;
        Eina_Stringshare *name;

        name = elput_input_device_output_name_get(ev->device);
        if (!name)
          {
             output = eina_list_data_get(device->outputs);
             if (output)
               ecore_drm2_device_calibrate(device,
                                           output->w, output->h);
          }
        else
          {
             EINA_LIST_FOREACH(device->outputs, l, output)
               {
                  if ((output->name) &&
                      (!strcmp(output->name, name)))
                    {
                       ecore_drm2_device_calibrate(device,
                                                   output->w, output->h);
                       break;
                    }
               }

             eina_stringshare_del(name);
          }
     }

   return ECORE_CALLBACK_RENEW;
}

static const char *
_drm2_device_find(const char *seat)
{
   Eina_List *devs, *l;
   const char *dev, *ret = NULL;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool platform = EINA_FALSE;

   devs = eeze_udev_find_by_subsystem_sysname("drm", "card[0-9]*");
   if (!devs) return NULL;

   EINA_LIST_FOREACH(devs, l, dev)
     {
        const char *dpath, *dseat, *dparent;

        dpath = eeze_udev_syspath_get_devpath(dev);
        if (!dpath) continue;

        dseat = eeze_udev_syspath_get_property(dev, "ID_SEAT");
        if (!dseat) dseat = eina_stringshare_add("seat0");

        if ((seat) && (strcmp(seat, dseat)))
          goto cont;
        else if (strcmp(dseat, "seat0"))
          goto cont;

        dparent = eeze_udev_syspath_get_parent_filtered(dev, "pci", NULL);
        if (!dparent)
          {
             dparent =
               eeze_udev_syspath_get_parent_filtered(dev, "platform", NULL);
             platform = EINA_TRUE;
          }

        if (dparent)
          {
             if (!platform)
               {
                  const char *id;

                  id = eeze_udev_syspath_get_sysattr(dparent, "boot_vga");
                  if (id)
                    {
                       if (!strcmp(id, "1")) found = EINA_TRUE;
                       eina_stringshare_del(id);
                    }
               }
             else
               found = EINA_TRUE;

             eina_stringshare_del(dparent);
          }

cont:
        eina_stringshare_del(dpath);
        eina_stringshare_del(dseat);
        if (found) break;
     }

   if (!found) goto out;

   ret = eeze_udev_syspath_get_devpath(dev);

out:
   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);

   return ret;
}

#ifdef HAVE_ATOMIC_DRM
static Eina_Bool
_drm2_atomic_usable(int fd)
{
   drmVersion *drmver;
   Eina_Bool ret = EINA_FALSE;

   drmver = drmGetVersion(fd);
   if (!drmver) return EINA_FALSE;

   /* detect driver */
   if ((!strcmp(drmver->name, "i915")) &&
       (!strcmp(drmver->desc, "Intel Graphics")))
     {
        FILE *fp;

        /* detect kernel version
         * NB: In order for atomic modesetting to work properly for Intel,
         * we need to be using a kernel >= 4.8.0 */

        fp = fopen("/proc/sys/kernel/osrelease", "rb");
        if (fp)
          {
             char buff[512];
             int maj = 0, min = 0;

             if (fgets(buff, sizeof(buff), fp))
               {
                  if (sscanf(buff, "%i.%i.%*s", &maj, &min) == 2)
                    {
                       if ((maj >= 4) && (min >= 8))
                         ret = EINA_TRUE;
                    }
               }
             fclose(fp);
          }
     }

   drmFreeVersion(drmver);

   return ret;
}

static void
_drm2_atomic_state_crtc_fill(Ecore_Drm2_Crtc_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Crtc Fill");

   oprops =
     drmModeObjectGetProperties(fd, cstate->obj_id, DRM_MODE_OBJECT_CRTC);
   if (!oprops) return;

   DBG("\tCrtc %d", cstate->obj_id);

   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s %d", prop->name, i);

        if (!strcmp(prop->name, "MODE_ID"))
          {
             drmModePropertyBlobPtr bp;

             cstate->mode.id = prop->prop_id;
             cstate->mode.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->mode.value);

             if (!cstate->mode.value)
               {
                  cstate->mode.len = 0;
                  goto cont;
               }

             bp = drmModeGetPropertyBlob(fd, cstate->mode.value);
             if (!bp) goto cont;

             if ((!cstate->mode.data) ||
                 memcmp(cstate->mode.data, bp->data, bp->length) != 0)
               {
                  cstate->mode.data =
                    eina_memdup(bp->data, bp->length, 1);
               }

             cstate->mode.len = bp->length;

             if (cstate->mode.value != 0)
               drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                         &cstate->mode.value);

             drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "ACTIVE"))
          {
             cstate->active.id = prop->prop_id;
             cstate->active.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->active.value);
          }

cont:
        drmModeFreeProperty(prop);
     }

   drmModeFreeObjectProperties(oprops);
}

static void
_drm2_atomic_state_conn_fill(Ecore_Drm2_Connector_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Connector Fill");

   oprops =
     drmModeObjectGetProperties(fd, cstate->obj_id, DRM_MODE_OBJECT_CONNECTOR);
   if (!oprops) return;

   DBG("\tConnector: %d", cstate->obj_id);

   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s", prop->name);

        if (!strcmp(prop->name, "CRTC_ID"))
          {
             cstate->crtc.id = prop->prop_id;
             cstate->crtc.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->crtc.value);
          }
        else if (!strcmp(prop->name, "DPMS"))
          {
             cstate->dpms.id = prop->prop_id;
             cstate->dpms.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->dpms.value);
          }
        else if (!strcmp(prop->name, "EDID"))
          {
             drmModePropertyBlobPtr bp;

             cstate->edid.id = oprops->prop_values[i];
             if (!cstate->edid.id)
               {
                  cstate->edid.len = 0;
                  goto cont;
               }

             bp = drmModeGetPropertyBlob(fd, cstate->edid.id);
             if (!bp) goto cont;

             if ((!cstate->edid.data) ||
                 memcmp(cstate->edid.data, bp->data, bp->length) != 0)
               {
                  cstate->edid.data =
                    eina_memdup(bp->data, bp->length, 1);
               }

             cstate->edid.len = bp->length;

             if (cstate->edid.id != 0)
               drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                         &cstate->edid.id);

             drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "aspect ratio"))
          {
             cstate->aspect.id = prop->prop_id;
             cstate->aspect.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->aspect.value);
          }
        else if (!strcmp(prop->name, "scaling mode"))
          {
             cstate->scaling.id = prop->prop_id;
             cstate->scaling.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %d", cstate->scaling.value);
          }

cont:
        drmModeFreeProperty(prop);
     }

   drmModeFreeObjectProperties(oprops);
}

static void
_drm2_atomic_state_fill(Ecore_Drm2_Atomic_State *state, int fd)
{
   int i = 0;
   drmModeResPtr res;

   res = drmModeGetResources(fd);
   if (!res) return;

   state->crtcs = res->count_crtcs;
   state->crtc_states = calloc(state->crtcs, sizeof(Ecore_Drm2_Crtc_State));
   if (state->crtc_states)
     {
        for (i = 0; i < state->crtcs; i++)
          {
             Ecore_Drm2_Crtc_State *cstate;

             cstate = &state->crtc_states[i];
             cstate->obj_id = res->crtcs[i];
             cstate->index = i;

             _drm2_atomic_state_crtc_fill(cstate, fd);
          }
     }

   state->conns = res->count_connectors;
   state->conn_states =
     calloc(state->conns, sizeof(Ecore_Drm2_Connector_State));
   if (state->conn_states)
     {
        for (i = 0; i < state->conns; i++)
          {
             Ecore_Drm2_Connector_State *cstate;

             cstate = &state->conn_states[i];
             cstate->obj_id = res->connectors[i];

             _drm2_atomic_state_conn_fill(cstate, fd);
          }
     }

err:
   drmModeFreeResources(res);
}
#endif

EAPI Ecore_Drm2_Device *
ecore_drm2_device_find(const char *seat, unsigned int tty)
{
   Ecore_Drm2_Device *dev;

   dev = calloc(1, sizeof(Ecore_Drm2_Device));
   if (!dev) return NULL;

   dev->path = _drm2_device_find(seat);
   if (!dev->path)
     {
        ERR("Could not find drm device on seat %s", seat);
        goto path_err;
     }

   dev->em = elput_manager_connect(seat, tty);
   if (!dev->em)
     {
        ERR("Could not connect to input manager");
        goto man_err;
     }

   return dev;

man_err:
   eina_stringshare_del(dev->path);
path_err:
   free(dev);
   return NULL;
}

EAPI int
ecore_drm2_device_open(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, -1);

   device->fd = elput_manager_open(device->em, device->path, -1);
   if (device->fd < 0) goto open_err;

   if (!elput_input_init(device->em))
     {
        ERR("Could not initialize Elput Input");
        goto input_err;
     }

   DBG("Device Path: %s", device->path);
   DBG("Device Fd: %d", device->fd);

#ifdef HAVE_ATOMIC_DRM
   /* check that this system can do atomic */
   _ecore_drm2_use_atomic = _drm2_atomic_usable(device->fd);
   if (_ecore_drm2_use_atomic)
     {
        if (drmSetClientCap(device->fd, DRM_CLIENT_CAP_ATOMIC, 1) < 0)
          {
             WRN("Could not enable Atomic Modesetting support");
             _ecore_drm2_use_atomic = EINA_FALSE;
          }
        else
          {
             if (drmSetClientCap(device->fd,
                                 DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) < 0)
               WRN("Could not enable Universal Plane support");
             else
               {
                  /* atomic & planes are usable */
                  device->state = calloc(1, sizeof(Ecore_Drm2_Atomic_State));
                  if (device->state)
                    _drm2_atomic_state_fill(device->state, device->fd);
               }
          }
     }
#endif

   device->active_hdlr =
     ecore_event_handler_add(ELPUT_EVENT_SESSION_ACTIVE,
                             _cb_session_active, device);

   device->device_change_hdlr =
     ecore_event_handler_add(ELPUT_EVENT_DEVICE_CHANGE,
                             _cb_device_change, device);

   return device->fd;

input_err:
   elput_manager_close(device->em, device->fd);
open_err:
   return -1;
}

EAPI void
ecore_drm2_device_close(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN(device);
   EINA_SAFETY_ON_TRUE_RETURN(device->fd < 0);

   elput_input_shutdown(device->em);
   elput_manager_close(device->em, device->fd);
}

EAPI void
ecore_drm2_device_free(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   ecore_event_handler_del(device->active_hdlr);
   ecore_event_handler_del(device->device_change_hdlr);
   eina_stringshare_del(device->path);
   free(device);
}

EAPI int
ecore_drm2_device_clock_id_get(Ecore_Drm2_Device *device)
{
   uint64_t caps;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, -1);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((device->fd < 0), -1);

   ret = drmGetCap(device->fd, DRM_CAP_TIMESTAMP_MONOTONIC, &caps);
   if ((ret == 0) && (caps == 1))
     return CLOCK_MONOTONIC;
   else
     return CLOCK_REALTIME;
}

EAPI void
ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *device, int *width, int *height)
{
   uint64_t caps;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN(device);
   EINA_SAFETY_ON_TRUE_RETURN((device->fd < 0));

   if (width)
     {
        *width = 64;
        ret = drmGetCap(device->fd, DRM_CAP_CURSOR_WIDTH, &caps);
        if (ret == 0) *width = caps;
     }
   if (height)
     {
        *height = 64;
        ret = drmGetCap(device->fd, DRM_CAP_CURSOR_HEIGHT, &caps);
        if (ret == 0) *height = caps;
     }
}

EAPI void
ecore_drm2_device_pointer_xy_get(Ecore_Drm2_Device *device, int *x, int *y)
{
   if (x) *x = 0;
   if (y) *y = 0;

   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_pointer_xy_get(device->em, NULL, x, y);
}

EAPI void
ecore_drm2_device_pointer_warp(Ecore_Drm2_Device *device, int x, int y)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_pointer_xy_set(device->em, NULL, x, y);
}

EAPI Eina_Bool
ecore_drm2_device_pointer_left_handed_set(Ecore_Drm2_Device *device, Eina_Bool left)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   return elput_input_pointer_left_handed_set(device->em, NULL, left);
}

EAPI void
ecore_drm2_device_window_set(Ecore_Drm2_Device *device, unsigned int window)
{
   EINA_SAFETY_ON_NULL_RETURN(device);
   EINA_SAFETY_ON_NULL_RETURN(device->em);
   elput_manager_window_set(device->em, window);
}

EAPI void
ecore_drm2_device_pointer_max_set(Ecore_Drm2_Device *device, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(device);
   EINA_SAFETY_ON_NULL_RETURN(device->em);

   elput_input_pointer_max_set(device->em, w, h);
}

EAPI void
ecore_drm2_device_keyboard_cached_context_set(Ecore_Drm2_Device *device, void *context)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_keyboard_cached_context_set(device->em, context);
}

EAPI void
ecore_drm2_device_keyboard_cached_keymap_set(Ecore_Drm2_Device *device, void *keymap)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_keyboard_cached_keymap_set(device->em, keymap);
}

EAPI unsigned int *
ecore_drm2_device_crtcs_get(Ecore_Drm2_Device *device, int *num)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);

   if (num) *num = device->num_crtcs;
   return device->crtcs;
}

EAPI void
ecore_drm2_device_screen_size_range_get(Ecore_Drm2_Device *device, int *minw, int *minh, int *maxw, int *maxh)
{
   if (minw) *minw = 0;
   if (minh) *minh = 0;
   if (maxw) *maxw = 0;
   if (maxh) *maxh = 0;

   EINA_SAFETY_ON_NULL_RETURN(device);

   if (minw) *minw = device->min.width;
   if (minh) *minh = device->min.height;
   if (maxw) *maxw = device->max.width;
   if (maxh) *maxh = device->max.height;
}

EAPI void
ecore_drm2_device_calibrate(Ecore_Drm2_Device *device, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_devices_calibrate(device->em, w, h);
}

EAPI Eina_Bool
ecore_drm2_device_vt_set(Ecore_Drm2_Device *device, int vt)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   return elput_manager_vt_set(device->em, vt);
}
