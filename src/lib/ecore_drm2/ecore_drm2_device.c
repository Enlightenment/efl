#include "ecore_drm2_private.h"

#ifndef DRM_CAP_DUMB_PREFER_SHADOW
# define DRM_CAP_DUMB_PREFER_SHADOW 0x4
#endif

#include <sys/utsname.h>

Eina_Bool _ecore_drm2_use_atomic = EINA_TRUE;

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

   if (ev->active)
     EINA_LIST_FOREACH(device->outputs, l, output)
       ecore_drm2_output_dpms_set(output, DRM_MODE_DPMS_ON);

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

        name = elput_device_output_name_get(ev->device);
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
                  if (eina_streq(output->name, name))
                    {
                       ecore_drm2_device_calibrate(device,
                                                   output->w, output->h);
                       break;
                    }
               }
          }
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_drm2_device_modeset_capable(int fd)
{
   int ret = EINA_TRUE;
   drmModeRes *res;

   res = sym_drmModeGetResources(fd);
   if (!res)
     return EINA_FALSE;

   if (res->count_crtcs <= 0 ||
       res->count_connectors <= 0 ||
       res->count_encoders <= 0)
     ret = EINA_FALSE;

   sym_drmModeFreeResources(res);

   return ret;
}

static const char *
_drm2_device_find(Elput_Manager *em, const char *seat)
{
   Eina_List *devs, *l;
   const char *dev, *ret = NULL, *chosen_dev = NULL, *d = NULL;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool modeset;
   int fd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);

   d = getenv("ECORE_DRM2_CARD");
   if (d)
     devs = eeze_udev_find_by_subsystem_sysname("drm", d);
   else
     devs = eeze_udev_find_by_subsystem_sysname("drm", "card[0-9]*");

   if (!devs) return NULL;

   EINA_LIST_FOREACH(devs, l, dev)
     {
        const char *dpath, *dseat, *dparent;

        dpath = eeze_udev_syspath_get_devpath(dev);
        if (!dpath) continue;

        dseat = eeze_udev_syspath_get_property(dev, "ID_SEAT");
        if (!dseat) dseat = eina_stringshare_add("seat0");

        if (strcmp(seat, dseat))
          goto cont;

        fd = elput_manager_open(em, dpath, -1);
        if (fd < 0)
          goto cont;
        modeset = _drm2_device_modeset_capable(fd);
        elput_manager_close(em, fd);
        if (!modeset)
          goto cont;

        chosen_dev = dev;

        dparent = eeze_udev_syspath_get_parent_filtered(dev, "pci", NULL);
        if (dparent)
          {
             const char *id;

             id = eeze_udev_syspath_get_sysattr(dparent, "boot_vga");
             if (id)
               {
                  if (!strcmp(id, "1")) found = EINA_TRUE;
                  eina_stringshare_del(id);
               }

             eina_stringshare_del(dparent);
          }

cont:
        eina_stringshare_del(dpath);
        eina_stringshare_del(dseat);
        if (found) break;
     }

   if (chosen_dev)
     ret = eeze_udev_syspath_get_devpath(chosen_dev);

   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);

   return ret;
}

# if 0
static Eina_Bool
_drm2_atomic_usable(int fd)
{
   drmVersion *drmver;
   Eina_Bool ret = EINA_FALSE;

   drmver = sym_drmGetVersion(fd);
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

   sym_drmFreeVersion(drmver);

   return ret;
}
# endif

static void
_drm2_atomic_state_crtc_fill(Ecore_Drm2_Crtc_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Crtc Fill");

   oprops =
     sym_drmModeObjectGetProperties(fd, cstate->obj_id, DRM_MODE_OBJECT_CRTC);
   if (!oprops) return;

   DBG("\tCrtc %d", cstate->obj_id);

   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
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

             bp = sym_drmModeGetPropertyBlob(fd, cstate->mode.value);
             if (!bp) goto cont;

             if ((!cstate->mode.data) ||
                 memcmp(cstate->mode.data, bp->data, bp->length) != 0)
               {
                  cstate->mode.data =
                    eina_memdup(bp->data, bp->length, 1);
               }

             cstate->mode.len = bp->length;

             if (cstate->mode.value != 0)
               sym_drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                         &cstate->mode.value);

             sym_drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "ACTIVE"))
          {
             cstate->active.id = prop->prop_id;
             cstate->active.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->active.value);
          }
        else if (!strcmp(prop->name, "BACKGROUND_COLOR"))
          {
             cstate->background.id = prop->prop_id;
             cstate->background.value = oprops->prop_values[i];
          }
cont:
        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);
}

static void
_drm2_atomic_state_conn_fill(Ecore_Drm2_Connector_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Connector Fill");

   oprops =
     sym_drmModeObjectGetProperties(fd, cstate->obj_id, DRM_MODE_OBJECT_CONNECTOR);
   if (!oprops) return;

   DBG("\tConnector: %d", cstate->obj_id);

   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s", prop->name);

        if (!strcmp(prop->name, "CRTC_ID"))
          {
             cstate->crtc.id = prop->prop_id;
             cstate->crtc.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->crtc.value);
          }
        else if (!strcmp(prop->name, "DPMS"))
          {
             cstate->dpms.id = prop->prop_id;
             cstate->dpms.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->dpms.value);
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

             bp = sym_drmModeGetPropertyBlob(fd, cstate->edid.id);
             if (!bp) goto cont;

             if ((!cstate->edid.data) ||
                 memcmp(cstate->edid.data, bp->data, bp->length) != 0)
               {
                  cstate->edid.data =
                    eina_memdup(bp->data, bp->length, 1);
               }

             cstate->edid.len = bp->length;

             if (cstate->edid.id != 0)
               sym_drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                         &cstate->edid.id);

             sym_drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "aspect ratio"))
          {
             cstate->aspect.id = prop->prop_id;
             cstate->aspect.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->aspect.value);
          }
        else if (!strcmp(prop->name, "scaling mode"))
          {
             cstate->scaling.id = prop->prop_id;
             cstate->scaling.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->scaling.value);
          }

cont:
        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);
}

static void
_drm2_atomic_state_plane_fill(Ecore_Drm2_Plane_State *pstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;
   int k = 0;

   DBG("Atomic State Plane Fill");

   oprops =
     sym_drmModeObjectGetProperties(fd, pstate->obj_id, DRM_MODE_OBJECT_PLANE);
   if (!oprops) return;

   DBG("\tPlane: %d", pstate->obj_id);

   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s", prop->name);

        if (!strcmp(prop->name, "CRTC_ID"))
          {
             pstate->cid.id = prop->prop_id;
             pstate->cid.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)pstate->cid.value);
          }
        else if (!strcmp(prop->name, "FB_ID"))
          {
             pstate->fid.id = prop->prop_id;
             pstate->fid.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)pstate->fid.value);
         }
        else if (!strcmp(prop->name, "CRTC_X"))
          {
             pstate->cx.id = prop->prop_id;
             pstate->cx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_Y"))
          {
             pstate->cy.id = prop->prop_id;
             pstate->cy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_W"))
          {
             pstate->cw.id = prop->prop_id;
             pstate->cw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_H"))
          {
             pstate->ch.id = prop->prop_id;
             pstate->ch.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_X"))
          {
             pstate->sx.id = prop->prop_id;
             pstate->sx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_Y"))
          {
             pstate->sy.id = prop->prop_id;
             pstate->sy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_W"))
          {
             pstate->sw.id = prop->prop_id;
             pstate->sw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_H"))
          {
             pstate->sh.id = prop->prop_id;
             pstate->sh.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "type"))
          {
             pstate->type.id = prop->prop_id;
             pstate->type.value = oprops->prop_values[i];
             switch (pstate->type.value)
               {
                case DRM_PLANE_TYPE_OVERLAY:
                  DBG("\t\t\tOverlay Plane");
                  break;
                case DRM_PLANE_TYPE_PRIMARY:
                  DBG("\t\t\tPrimary Plane");
                  break;
                case DRM_PLANE_TYPE_CURSOR:
                  DBG("\t\t\tCursor Plane");
                  break;
                default:
                  DBG("\t\t\tValue: %lu", (long)pstate->type.value);
                  break;
               }
          }
        else if (!strcmp(prop->name, "rotation"))
          {
             pstate->rotation.id = prop->prop_id;
             pstate->rotation.value = oprops->prop_values[i];

             for (k = 0; k < prop->count_enums; k++)
               {
                  int r = -1;

                  DBG("\t\t\tRotation: %s", prop->enums[k].name);
                  if (!strcmp(prop->enums[k].name, "rotate-0"))
                    r = ECORE_DRM2_ROTATION_NORMAL;
                  else if (!strcmp(prop->enums[k].name, "rotate-90"))
                    r = ECORE_DRM2_ROTATION_90;
                  else if (!strcmp(prop->enums[k].name, "rotate-180"))
                    r = ECORE_DRM2_ROTATION_180;
                  else if (!strcmp(prop->enums[k].name, "rotate-270"))
                    r = ECORE_DRM2_ROTATION_270;
                  else if (!strcmp(prop->enums[k].name, "reflect-x"))
                    r = ECORE_DRM2_ROTATION_REFLECT_X;
                  else if (!strcmp(prop->enums[k].name, "reflect-y"))
                    r = ECORE_DRM2_ROTATION_REFLECT_Y;

                  if (r != -1)
                    {
                       pstate->supported_rotations |= r;
                       pstate->rotation_map[ffs(r)] =
                         1 << prop->enums[k].value;
                    }
               }
          }

        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);
}

static void
_drm2_atomic_state_fill(Ecore_Drm2_Atomic_State *state, int fd)
{
   int i = 0;
   drmModeResPtr res;
   drmModePlaneResPtr pres;

   res = sym_drmModeGetResources(fd);
   if (!res) return;

   pres = sym_drmModeGetPlaneResources(fd);
   if (!pres) goto err;

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

   state->planes = pres->count_planes;
   state->plane_states = calloc(state->planes, sizeof(Ecore_Drm2_Plane_State));
   if (state->plane_states)
     {
        unsigned int f = 0;

        for (i = 0; i < state->planes; i++)
          {
             drmModePlanePtr plane;
             Ecore_Drm2_Plane_State *pstate;

             plane = sym_drmModeGetPlane(fd, pres->planes[i]);
             if (!plane) continue;

             pstate = &state->plane_states[i];
             pstate->obj_id = pres->planes[i];
             pstate->mask = plane->possible_crtcs;
             pstate->num_formats = plane->count_formats;

             pstate->formats = calloc(plane->count_formats, sizeof(uint32_t));

             for (f = 0; f < plane->count_formats; f++)
               pstate->formats[f] = plane->formats[f];

             sym_drmModeFreePlane(plane);

             _drm2_atomic_state_plane_fill(pstate, fd);
          }
     }

   sym_drmModeFreePlaneResources(pres);

err:
   sym_drmModeFreeResources(res);
}

static void
_drm2_atomic_state_free(Ecore_Drm2_Atomic_State *state)
{
   free(state->plane_states);
   free(state->conn_states);
   free(state->crtc_states);
   free(state);
}

EAPI Ecore_Drm2_Device *
ecore_drm2_device_open(const char *seat, unsigned int tty)
{
   Ecore_Drm2_Device *device;

   device = calloc(1, sizeof(Ecore_Drm2_Device));
   if (!device) return NULL;

   device->em = elput_manager_connect(seat, tty);
   if (!device->em)
     {
        ERR("Could not connect to input manager");
        goto man_err;
     }

   device->path = _drm2_device_find(device->em, seat);
   if (!device->path)
     {
        ERR("Could not find drm device on seat %s", seat);
        goto path_err;
     }

   device->fd = elput_manager_open(device->em, device->path, -1);
   if (device->fd < 0) goto path_err;

   if (!elput_input_init(device->em))
     {
        ERR("Could not initialize Elput Input");
        goto input_err;
     }

   DBG("Device Path: %s", device->path);
   DBG("Device Fd: %d", device->fd);

# if 0
   /* check that this system can do atomic */
   _ecore_drm2_use_atomic = _drm2_atomic_usable(device->fd);
# endif

   if (getenv("ECORE_DRM2_ATOMIC_DISABLE"))
     _ecore_drm2_use_atomic = EINA_FALSE;

   if (_ecore_drm2_use_atomic)
     {
        if (sym_drmSetClientCap(device->fd,
                                DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) < 0)
          {
             WRN("Could not enable Universal Plane support");
             _ecore_drm2_use_atomic = EINA_FALSE;
          }
        else
          {
             if (sym_drmSetClientCap(device->fd, DRM_CLIENT_CAP_ATOMIC, 1) < 0)
               {
                  WRN("Could not enable Atomic Modesetting support");
                  _ecore_drm2_use_atomic = EINA_FALSE;
               }
          }
     }

   if (_ecore_drm2_use_atomic)
     {
        device->state = calloc(1, sizeof(Ecore_Drm2_Atomic_State));
        if (device->state)
          _drm2_atomic_state_fill(device->state, device->fd);
     }

   device->active_hdlr =
     ecore_event_handler_add(ELPUT_EVENT_SESSION_ACTIVE,
                             _cb_session_active, device);

   device->device_change_hdlr =
     ecore_event_handler_add(ELPUT_EVENT_DEVICE_CHANGE,
                             _cb_device_change, device);

   return device;

input_err:
   elput_manager_close(device->em, device->fd);
path_err:
   elput_manager_disconnect(device->em);
man_err:
   free(device);
   return NULL;
}

EAPI void
ecore_drm2_device_close(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_shutdown(device->em);
   elput_manager_close(device->em, device->fd);
   if (_ecore_drm2_use_atomic)
     _drm2_atomic_state_free(device->state);

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

   ret = sym_drmGetCap(device->fd, DRM_CAP_TIMESTAMP_MONOTONIC, &caps);
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

   if (width)
     {
        *width = 64;
        ret = sym_drmGetCap(device->fd, DRM_CAP_CURSOR_WIDTH, &caps);
        if (ret == 0)
          {
             device->cursor.width = caps;
             *width = caps;
          }
     }
   if (height)
     {
        *height = 64;
        ret = sym_drmGetCap(device->fd, DRM_CAP_CURSOR_HEIGHT, &caps);
        if (ret == 0)
          {
             device->cursor.height = caps;
             *height = caps;
          }
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

EAPI Eina_Bool
ecore_drm2_device_pointer_rotation_set(Ecore_Drm2_Device *device, int rotation)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   return elput_input_pointer_rotation_set(device->em, rotation);
}

EAPI void
ecore_drm2_device_pointer_accel_speed_set(Ecore_Drm2_Device *device, double speed)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_pointer_accel_speed_set(device->em, NULL, speed);
}

EAPI void
ecore_drm2_device_pointer_accel_profile_set(Ecore_Drm2_Device *device, uint32_t profile)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_pointer_accel_profile_set(device->em, NULL, profile);
}

EAPI void
ecore_drm2_device_touch_tap_to_click_enabled_set(Ecore_Drm2_Device *device, Eina_Bool enabled)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_touch_tap_to_click_enabled_set(device->em, NULL, enabled);
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
ecore_drm2_device_keyboard_info_set(Ecore_Drm2_Device *device, void *context, void *keymap, int group)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_keyboard_info_set(device->em, context, keymap, group);
}

EAPI void
ecore_drm2_device_keyboard_group_set(Ecore_Drm2_Device *device, int group)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   elput_input_keyboard_group_set(device->em, group);
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

EAPI Eina_Bool
ecore_drm2_device_prefer_shadow(Ecore_Drm2_Device *device)
{
   uint64_t caps;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   ret = sym_drmGetCap(device->fd, DRM_CAP_DUMB_PREFER_SHADOW, &caps);
   if ((ret == 0) && (caps == 1))
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

EAPI int
ecore_drm2_device_fd_get(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, -1);

   return device->fd;
}

EAPI Eina_Bool
ecore_drm2_vblank_supported(Ecore_Drm2_Device *dev)
{
   drmVBlank tmp;
   int ret = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);

   memset(&tmp, 0, sizeof(drmVBlank));
   tmp.request.type = DRM_VBLANK_RELATIVE;

   ret = sym_drmWaitVBlank(dev->fd, &tmp);

   if (ret != 0) return EINA_FALSE;
   return EINA_TRUE;
}

/* prevent crashing with old apps compiled against these functions */
EAPI void ecore_drm2_device_keyboard_cached_context_set(){};
EAPI void ecore_drm2_device_keyboard_cached_keymap_set(){};
