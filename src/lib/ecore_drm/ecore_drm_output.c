/* Portions of this code have been derived from Weston
 *
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2010-2011 Benjamin Franzke
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2010 Red Hat <mjg@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ecore_drm_private.h"
#include <ctype.h>

#define EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING 0xfe
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME 0xfc
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER 0xff
#define EDID_OFFSET_DATA_BLOCKS 0x36
#define EDID_OFFSET_LAST_BLOCK 0x6c
#define EDID_OFFSET_PNPID 0x08
#define EDID_OFFSET_SERIAL 0x0c

static const char *conn_types[] =
{
   "None", "VGA", "DVI-I", "DVI-D", "DVI-A",
   "Composite", "S-Video", "LVDS", "Component", "DIN",
   "DisplayPort", "HDMI-A", "HDMI-B", "TV", "eDP", "Virtual",
   "DSI",
};

EAPI int ECORE_DRM_EVENT_OUTPUT = 0;

static void
_ecore_drm_output_event_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Drm_Event_Output *e = event;

   eina_stringshare_del(e->make);
   eina_stringshare_del(e->model);
   eina_stringshare_del(e->name);
   free(event);
}

static void
_ecore_drm_output_event_send(const Ecore_Drm_Output *output, Eina_Bool plug)
{
   Ecore_Drm_Event_Output *e;

   if (!(e = calloc(1, sizeof(Ecore_Drm_Event_Output)))) return;
   e->plug = plug;
   e->id = output->crtc_id;

   if (output->current_mode)
     {
        e->w = output->current_mode->width;
        e->h = output->current_mode->height;
        e->refresh = output->current_mode->refresh;
     }
   else if (output->crtc)
     {
        e->w = output->crtc->width;
        e->h = output->crtc->height;
     }

   e->x = output->x;
   e->y = output->y;
   e->phys_width = output->phys_width;
   e->phys_height = output->phys_height;
   e->subpixel_order = output->subpixel;
   e->make = eina_stringshare_ref(output->make);
   e->model = eina_stringshare_ref(output->model);
   e->name = eina_stringshare_ref(output->name);
   e->transform = 0;
   ecore_event_add(ECORE_DRM_EVENT_OUTPUT, e,
                   _ecore_drm_output_event_free, NULL);
}

static drmModePropertyPtr
_ecore_drm_output_property_get(int fd, drmModeConnectorPtr conn, const char *name)
{
   drmModePropertyPtr prop;
   int i = 0;

   for (; i < conn->count_props; i++)
     {
        if (!(prop = drmModeGetProperty(fd, conn->props[i])))
          continue;

        if (!strcmp(prop->name, name)) return prop;

        drmModeFreeProperty(prop);
     }

   return NULL;
}

static void
_ecore_drm_output_edid_parse_string(const uint8_t *data, char text[])
{
   int i = 0, rep = 0;

   strncpy(text, (const char *)data, 12);

   for (; text[i] != '\0'; i++)
     {
        if ((text[i] == '\n') || (text[i] == '\r'))
          {
             text[i] = '\0';
             break;
          }
     }

   for (i = 0; text[i] != '\0'; i++)
     {
        if (!isprint(text[i]))
          {
             text[i] = '-';
             rep++;
          }
     }

   if (rep > 4) text[0] = '\0';
}

static int
_ecore_drm_output_edid_parse(Ecore_Drm_Output *output, const uint8_t *data, size_t len)
{
   int i = 0;
   uint32_t serial;

   if (len < 128) return -1;
   if ((data[0] != 0x00) || (data[1] != 0xff)) return -1;

   output->edid.pnp[0] = 'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x7c) / 4) - 1;
   output->edid.pnp[1] =
     'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x3) * 8) +
     ((data[EDID_OFFSET_PNPID + 1] & 0xe0) / 32) - 1;
   output->edid.pnp[2] = 'A' + (data[EDID_OFFSET_PNPID + 1] & 0x1f) - 1;
   output->edid.pnp[3] = '\0';

   serial = (uint32_t) data[EDID_OFFSET_SERIAL + 0];
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 1] * 0x100;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 2] * 0x10000;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 3] * 0x1000000;
   if (serial > 0)
     sprintf(output->edid.serial, "%lu", (unsigned long)serial);

   for (i = EDID_OFFSET_DATA_BLOCKS; i <= EDID_OFFSET_LAST_BLOCK; i += 18)
     {
        if (data[i] != 0) continue;
        if (data[i + 2] != 0) continue;

        if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME)
          _ecore_drm_output_edid_parse_string(&data[i + 5], output->edid.monitor);
        else if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER)
          _ecore_drm_output_edid_parse_string(&data[i + 5], output->edid.serial);
        else if (data[i + 3] == EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING)
          _ecore_drm_output_edid_parse_string(&data[i + 5], output->edid.eisa);
     }

   return 0;
}

static void
_ecore_drm_output_edid_find(Ecore_Drm_Output *output, drmModeConnector *conn)
{
   drmModePropertyBlobPtr blob = NULL;
   drmModePropertyPtr prop;
   int i = 0, ret = 0;

   for (; i < conn->count_props && !blob; i++)
     {
        if (!(prop = drmModeGetProperty(output->dev->drm.fd, conn->props[i])))
          continue;
        if ((prop->flags & DRM_MODE_PROP_BLOB) &&
            (!strcmp(prop->name, "EDID")))
          {
             blob = drmModeGetPropertyBlob(output->dev->drm.fd,
                                           conn->prop_values[i]);
          }
        drmModeFreeProperty(prop);
        if (blob) break;
     }

   if (!blob) return;

   output->edid_blob = eina_memdup(blob->data, blob->length, 1);

   ret = _ecore_drm_output_edid_parse(output, blob->data, blob->length);
   if (!ret)
     {
        if (output->edid.pnp[0] != '\0')
          eina_stringshare_replace(&output->make, output->edid.pnp);
        if (output->edid.monitor[0] != '\0')
          eina_stringshare_replace(&output->model, output->edid.monitor);
        /* if (output->edid.serial[0] != '\0') */
        /*   eina_stringshare_replace(&output->serial, output->edid.serial); */
     }

   drmModeFreePropertyBlob(blob);
}

static void
_ecore_drm_output_software_render(Ecore_Drm_Output *output)
{
   if (!output) return;
   if (!output->current_mode) return;
}

static int
_ecore_drm_output_crtc_find(Ecore_Drm_Device *dev, drmModeRes *res, drmModeConnector *conn)
{
   drmModeEncoder *enc;
   unsigned int p;
   int i, j;

   /* We did not find an existing encoder + crtc combination. Loop through all of them until we
    * find the first working combination */
   for (j = 0; j < conn->count_encoders; j++)
     {
        /* get the encoder on this connector */
        if (!(enc = drmModeGetEncoder(dev->drm.fd, conn->encoders[j])))
          {
             WRN("Failed to get encoder");
             continue;
          }

        p = enc->possible_crtcs;
        drmModeFreeEncoder(enc);

	/* Walk over all CRTCs */
        for (i = 0; i < res->count_crtcs; i++)
          {
             /* Does the CRTC match the list of possible CRTCs from the encoder? */
             if ((p & (1 << i)) &&
                 (!(dev->crtc_allocator & (1 << res->crtcs[i]))))
               {
                  return i;
               }
          }
     }

   return -1;
}

static Ecore_Drm_Output_Mode *
_ecore_drm_output_mode_add(Ecore_Drm_Output *output, drmModeModeInfo *info)
{
   Ecore_Drm_Output_Mode *mode;
   uint64_t refresh;

   /* try to allocate space for mode */
   if (!(mode = malloc(sizeof(Ecore_Drm_Output_Mode))))
     {
        ERR("Could not allocate space for mode");
        return NULL;
     }

   mode->flags = 0;
   mode->width = info->hdisplay;
   mode->height = info->vdisplay;

   refresh = (info->clock * 1000LL / info->htotal + info->vtotal / 2) / info->vtotal;
   if (info->flags & DRM_MODE_FLAG_INTERLACE)
     refresh *= 2;
   if (info->flags & DRM_MODE_FLAG_DBLSCAN)
     refresh /= 2;
   if (info->vscan > 1)
     refresh /= info->vscan;

   mode->refresh = refresh;
   mode->info = *info;

   if (info->type & DRM_MODE_TYPE_PREFERRED)
     mode->flags |= DRM_MODE_TYPE_PREFERRED;

   output->modes = eina_list_append(output->modes, mode);

   return mode;
}

/* XXX: this code is currently unused comment out until needed
static double
_ecore_drm_output_brightness_get(Ecore_Drm_Backlight *backlight)
{
   const char *brightness = NULL;
   double ret;

   if (!(backlight) || !(backlight->device))
     return 0;

   brightness = eeze_udev_syspath_get_sysattr(backlight->device, "brightness");
   if (!brightness) return 0;

   ret = strtod(brightness, NULL);
   if (ret < 0) ret = 0;

   return ret;
}

static double
_ecore_drm_output_actual_brightness_get(Ecore_Drm_Backlight *backlight)
{
   const char *brightness = NULL;
   double ret;

   if (!(backlight) || !(backlight->device))
     return 0;

   brightness = eeze_udev_syspath_get_sysattr(backlight->device, "actual_brightness");
   if (!brightness) return 0;

   ret = strtod(brightness, NULL);
   if (ret < 0) ret = 0;

   return ret;
}

static double
_ecore_drm_output_max_brightness_get(Ecore_Drm_Backlight *backlight)
{
   const char *brightness = NULL;
   double ret;

   if (!(backlight) || !(backlight->device))
     return 0;

   brightness = eeze_udev_syspath_get_sysattr(backlight->device, "max_brightness");
   if (!brightness) return 0;

   ret = strtod(brightness, NULL);
   if (ret < 0) ret = 0;

   return ret;
}

static double
_ecore_drm_output_brightness_set(Ecore_Drm_Backlight *backlight, double brightness_val)
{
   Eina_Bool ret = EINA_FALSE;

   if (!(backlight) || !(backlight->device))
     return ret;

   ret = eeze_udev_syspath_set_sysattr(backlight->device, "brightness", brightness_val);

   return ret;
}
*/

static Ecore_Drm_Backlight *
_ecore_drm_output_backlight_init(Ecore_Drm_Output *output, uint32_t conn_type)
{
   Ecore_Drm_Backlight *backlight = NULL;
   Ecore_Drm_Backlight_Type type = 0;
   Eina_List *devs, *l;
   Eina_Bool found = EINA_FALSE;
   const char *device, *devtype;

   if (!(devs = eeze_udev_find_by_filter("backlight", NULL, output->dev->drm.path)))
     devs = eeze_udev_find_by_filter("leds", NULL, output->dev->drm.path);

   if (!devs) return NULL;

   EINA_LIST_FOREACH(devs, l, device)
     {
        if (!(devtype = eeze_udev_syspath_get_sysattr(device, "type")))
          continue;

        if (!strcmp(devtype, "raw"))
          type = ECORE_DRM_BACKLIGHT_RAW;
        else if (!strcmp(devtype, "platform"))
          type = ECORE_DRM_BACKLIGHT_PLATFORM;
        else if (!strcmp(devtype, "firmware"))
          type = ECORE_DRM_BACKLIGHT_FIRMWARE;

        if ((conn_type == DRM_MODE_CONNECTOR_LVDS) ||
            (conn_type == DRM_MODE_CONNECTOR_eDP) ||
            (type == ECORE_DRM_BACKLIGHT_RAW))
          found = EINA_TRUE;

        eina_stringshare_del(devtype);
        if (found) break;
     }

   if (found)
     {
        if ((backlight = calloc(1, sizeof(Ecore_Drm_Backlight))))
          {
             backlight->type = type;
             backlight->device = eina_stringshare_add(device);
          }
     }

   EINA_LIST_FREE(devs, device)
     eina_stringshare_del(device);

   return backlight;
}

static void
_ecore_drm_output_backlight_shutdown(Ecore_Drm_Backlight *backlight)
{
   if (!backlight) return;

   if (backlight->device)
     eina_stringshare_del(backlight->device);

   free(backlight);
}

static int
_ecore_drm_output_subpixel_get(int subpixel)
{
   switch (subpixel)
     {
      case DRM_MODE_SUBPIXEL_UNKNOWN:
        return 0; // WL_OUTPUT_SUBPIXEL_UNKNOWN;
      case DRM_MODE_SUBPIXEL_NONE:
        return 1; //WL_OUTPUT_SUBPIXEL_NONE;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
        return 2; //WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
        return 3; // WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR;
      case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
        return 4; // WL_OUTPUT_SUBPIXEL_VERTICAL_RGB;
      case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
        return 5; //WL_OUTPUT_SUBPIXEL_VERTICAL_BGR;
      default:
        return 0; // WL_OUTPUT_SUBPIXEL_UNKNOWN;
     }
}

static void
_ecore_drm_output_planes_get(Ecore_Drm_Output *output)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Plane *eplane;
   drmModePlaneRes *pres;
   unsigned int i = 0, j = 0;
   int k = 0;

   dev = output->dev;
   pres = drmModeGetPlaneResources(dev->drm.fd);
   if (!pres) return;

   for (; i < pres->count_planes; i++)
     {
        drmModePlane *plane;
        drmModeObjectPropertiesPtr props;
        int type = -1;

        plane = drmModeGetPlane(dev->drm.fd, pres->planes[i]);
        if (!plane) continue;

        if (!(plane->possible_crtcs & (1 << output->crtc_index)))
          goto free_plane;

        props =
          drmModeObjectGetProperties(dev->drm.fd, plane->plane_id,
                                     DRM_MODE_OBJECT_PLANE);
        if (!props) goto free_plane;

        eplane = calloc(1, sizeof(Ecore_Drm_Plane));
        if (!eplane) goto free_plane;

        eplane->id = plane->plane_id;

        for (j = 0; type == -1 && j < props->count_props; j++)
          {
             drmModePropertyPtr prop;

             prop = drmModeGetProperty(dev->drm.fd, props->props[j]);
             if (!prop) continue;

             if (!strcmp(prop->name, "type"))
               {
                  eplane->type = props->prop_values[j];
                  if (eplane->type == ECORE_DRM_PLANE_TYPE_PRIMARY)
                    output->primary_plane_id = eplane->id;
               }
             else if (!strcmp(prop->name, "rotation"))
               {
                  output->rotation_prop_id = props->props[j];
                  eplane->rotation = props->prop_values[j];

                  for (k = 0; k < prop->count_enums; k++)
                    {
                       int r = -1;

                       if (!strcmp(prop->enums[k].name, "rotate-0"))
                         r = ECORE_DRM_PLANE_ROTATION_NORMAL;
                       else if (!strcmp(prop->enums[k].name, "rotate-90"))
                         r = ECORE_DRM_PLANE_ROTATION_90;
                       else if (!strcmp(prop->enums[k].name, "rotate-180"))
                         r = ECORE_DRM_PLANE_ROTATION_180;
                       else if (!strcmp(prop->enums[k].name, "rotate-270"))
                         r = ECORE_DRM_PLANE_ROTATION_270;
                       else if (!strcmp(prop->enums[k].name, "reflect-x"))
                         r = ECORE_DRM_PLANE_ROTATION_REFLECT_X;
                       else if (!strcmp(prop->enums[k].name, "reflect-y"))
                         r = ECORE_DRM_PLANE_ROTATION_REFLECT_Y;

                       if (r != -1)
                         {
                            eplane->supported_rotations |= r;
                            eplane->rotation_map[ffs(r)] =
                              1 << prop->enums[k].value;
                         }
                    }
               }

             drmModeFreeProperty(prop);
          }

        drmModeFreeObjectProperties(props);

        output->planes = eina_list_append(output->planes, eplane);

free_plane:
        drmModeFreePlane(plane);
     }
}

static Ecore_Drm_Output *
_ecore_drm_output_create(Ecore_Drm_Device *dev, drmModeRes *res, drmModeConnector *conn, int x, int y, Eina_Bool cloned)
{
   Ecore_Drm_Output *output;
   int i = -1;
   char name[DRM_CONNECTOR_NAME_LEN];
   const char *type;
   drmModeCrtc *crtc;
   drmModeEncoder *enc;
   drmModeModeInfo crtc_mode;
   Ecore_Drm_Output_Mode *mode, *current = NULL, *preferred = NULL, *best = NULL;
   Eina_List *l;

   /* try to find a crtc for this connector */
   i = _ecore_drm_output_crtc_find(dev, res, conn);
   if (i < 0) return NULL;

   /* try to allocate space for new output */
   if (!(output = calloc(1, sizeof(Ecore_Drm_Output)))) return NULL;

   output->x = x;
   output->y = y;
   output->dev = dev;
   output->cloned = cloned;
   output->phys_width = conn->mmWidth;
   output->phys_height = conn->mmHeight;
   output->subpixel = _ecore_drm_output_subpixel_get(conn->subpixel);

   output->make = eina_stringshare_add("UNKNOWN");
   output->model = eina_stringshare_add("UNKNOWN");
   output->name = eina_stringshare_add("UNKNOWN");

   output->connected = (conn->connection == DRM_MODE_CONNECTED);
   output->enabled = output->connected;
   output->conn_type = conn->connector_type;
   if (conn->connector_type < ALEN(conn_types))
     type = conn_types[conn->connector_type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->connector_type_id);
   eina_stringshare_replace(&output->name, name);

   output->crtc_index = i;
   output->crtc_id = res->crtcs[i];
   output->pipe = i;
   dev->crtc_allocator |= (1 << output->crtc_id);
   output->conn_id = conn->connector_id;
   dev->conn_allocator |= (1 << output->conn_id);

   /* store original crtc so we can restore VT settings */
   output->crtc = drmModeGetCrtc(dev->drm.fd, output->crtc_id);

   /* get if dpms is supported */
   output->dpms = _ecore_drm_output_property_get(dev->drm.fd, conn, "DPMS");

   memset(&crtc_mode, 0, sizeof(crtc_mode));

   /* get the encoder currently driving this connector */
   if ((enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id)))
     {
        crtc = drmModeGetCrtc(dev->drm.fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) goto err;
        if (crtc->mode_valid) crtc_mode = crtc->mode;
        drmModeFreeCrtc(crtc);
     }

   for (i = 0; i < conn->count_modes; i++)
     {
        if (!(mode = _ecore_drm_output_mode_add(output, &conn->modes[i])))
          goto err;
     }

   EINA_LIST_REVERSE_FOREACH(output->modes, l, mode)
     {
        if (!memcmp(&crtc_mode, &mode->info, sizeof(crtc_mode)))
          current = mode;
        if (mode->flags & DRM_MODE_TYPE_PREFERRED)
          preferred = mode;
        best = mode;
     }

   if ((!current) && (crtc_mode.clock != 0))
     {
        if (!(current = _ecore_drm_output_mode_add(output, &crtc_mode)))
          goto err;
     }

   if (current) output->current_mode = current;
   else if (preferred) output->current_mode = preferred;
   else if (best) output->current_mode = best;

   if (!output->current_mode) goto err;

   output->current_mode->flags |= DRM_MODE_TYPE_DEFAULT;

   /* try to init backlight */
   output->backlight =
     _ecore_drm_output_backlight_init(output, conn->connector_type);

   /* parse edid */
   _ecore_drm_output_edid_find(output, conn);

   /* TODO: implement support for LCMS ? */
   output->gamma = output->crtc->gamma_size;

   dev->outputs = eina_list_append(dev->outputs, output);

   /* NB: 'primary' output property is not supported in HW, so we need to
    * implement it via software. As such, the First output which gets
    * listed via libdrm will be assigned 'primary' until user changes
    * it via config */
   if (eina_list_count(dev->outputs) == 1)
     output->primary = EINA_TRUE;

   DBG("Created New Output At %d,%d", output->x, output->y);
   DBG("\tCrtc Pos: %d %d", output->crtc->x, output->crtc->y);
   DBG("\tCrtc: %d", output->crtc_id);
   DBG("\tConn: %d", output->conn_id);
   DBG("\tMake: %s", output->make);
   DBG("\tModel: %s", output->model);
   DBG("\tName: %s", output->name);
   DBG("\tCloned: %d", output->cloned);
   DBG("\tPrimary: %d", output->primary);

   EINA_LIST_FOREACH(output->modes, l, mode)
     {
        DBG("\tAdded Mode: %dx%d@%.1f%s%s%s",
            mode->width, mode->height, (mode->refresh / 1000.0),
            (mode->flags & DRM_MODE_TYPE_PREFERRED) ? ", preferred" : "",
            (mode->flags & DRM_MODE_TYPE_DEFAULT) ? ", current" : "",
            (conn->count_modes == 0) ? ", built-in" : "");
     }

   _ecore_drm_output_planes_get(output);

   return output;

err:
   EINA_LIST_FREE(output->modes, mode)
     free(mode);
   drmModeFreeProperty(output->dpms);
   drmModeFreeCrtc(output->crtc);
   dev->crtc_allocator &= ~(1 << output->crtc_id);
   dev->conn_allocator &= ~(1 << output->conn_id);
   eina_stringshare_del(output->name);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->make);
   free(output);
   return NULL;
}

static void
_ecore_drm_output_free(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;
   Ecore_Drm_Plane *plane;

   /* check for valid output */
   if (!output) return;

   if (output->pending_flip)
     {
        output->pending_destroy = EINA_TRUE;
        return;
     }

   /* delete the backlight struct */
   if (output->backlight)
     _ecore_drm_output_backlight_shutdown(output->backlight);

   /* turn off hardware cursor */
   drmModeSetCursor(output->dev->drm.fd, output->crtc_id, 0, 0, 0);

   /* restore crtc state */
   if (output->crtc)
     {
        if (drmModeSetCrtc(output->dev->drm.fd, output->crtc->crtc_id,
                           output->crtc->buffer_id, output->crtc->x, output->crtc->y,
                           &output->conn_id, 1, &output->crtc->mode))
          {
             ERR("Failed to restore Crtc state for output %s: %m", output->name);
          }
     }

   /* free modes */
   EINA_LIST_FREE(output->modes, mode)
     free(mode);

   EINA_LIST_FREE(output->planes, plane)
     free(plane);

   /* free strings */
   if (output->name) eina_stringshare_del(output->name);
   if (output->model) eina_stringshare_del(output->model);
   if (output->make) eina_stringshare_del(output->make);

   if (output->dpms) drmModeFreeProperty(output->dpms);
   if (output->crtc) drmModeFreeCrtc(output->crtc);

   free(output);
}

void
_ecore_drm_output_frame_finish(Ecore_Drm_Output *output)
{
   if (!output) return;

   if (output->need_repaint) ecore_drm_output_repaint(output);

   output->repaint_scheduled = EINA_FALSE;
}

void
_ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb)
{
   if ((!output) || (!fb)) return;

   if ((fb->mmap) &&
       (fb != output->dev->dumb[0]) && (fb != output->dev->dumb[1]))
     ecore_drm_fb_destroy(fb);
}

void
_ecore_drm_output_repaint_start(Ecore_Drm_Output *output)
{
   unsigned int fb;

   /* DBG("Output Repaint Start"); */

   if (!output) return;
   if (output->pending_destroy) return;

   if (!output->dev->current)
     {
        /* DBG("\tNo Current FB"); */
        goto finish;
     }

   fb = output->dev->current->id;
   if (drmModePageFlip(output->dev->drm.fd, output->crtc_id, fb,
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        ERR("Could not schedule output page flip event");
        goto finish;
     }

   return;

finish:
   _ecore_drm_output_frame_finish(output);
}

void
_ecore_drm_outputs_update(Ecore_Drm_Device *dev)
{
   drmModeRes *res;
   drmModeConnector *conn;
   int i = 0, x = 0, y = 0;
   Ecore_Drm_Output *output;
   uint32_t connected = 0, disconnects = 0;

   /* try to get drm resources */
   if (!(res = drmModeGetResources(dev->drm.fd))) return;

   /* find any new connects */
   for (; i < res->count_connectors; i++)
     {
        int conn_id;

        conn_id = res->connectors[i];

        /* try to get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, conn_id)))
          continue;

        /* test if connected */
        if (conn->connection != DRM_MODE_CONNECTED) goto next;

        connected |= (1 << conn_id);

        if (!(dev->conn_allocator & (1 << conn_id)))
          {
             if (dev->outputs)
               {
                  Ecore_Drm_Output *last;

                  if ((last = eina_list_last_data_get(dev->outputs)))
                    x = last->x + last->current_mode->width;
                  else
                    x = 0;
               }
             else
               x = 0;

             /* try to create a new output */
             /* NB: hotplugged outputs will be set to cloned by default */
             if (!(output =
                   _ecore_drm_output_create(dev, res, conn, x, y, EINA_TRUE)))
               goto next;
          }
next:
        drmModeFreeConnector(conn);
     }

   drmModeFreeResources(res);

   /* find any disconnects */
   disconnects = (dev->conn_allocator & ~connected);
   if (disconnects)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(dev->outputs, l, output)
          {
             if (disconnects & (1 << output->conn_id))
               {
                  disconnects &= ~(1 << output->conn_id);
                  _ecore_drm_output_event_send(output, EINA_FALSE);
               }
          }
     }
}

void
_ecore_drm_output_render_enable(Ecore_Drm_Output *output)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Output_Mode *mode;
   /* int x = 0, y = 0; */

   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(output->dev);
   EINA_SAFETY_ON_NULL_RETURN(output->current_mode);

   if (!output->enabled) return;

   dev = output->dev;

   if (!dev->current)
     {
        /* schedule repaint */
        /* NB: this will trigger a redraw at next idle */
        output->need_repaint = EINA_TRUE;
        return;
     }

   ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_ON);

   /* if (!output->cloned) */
   /*   { */
   /*      x = output->x; */
   /*      y = output->y; */
   /*   } */

   mode = output->current_mode;
   if (drmModeSetCrtc(dev->drm.fd, output->crtc_id, dev->current->id,
                      output->x, output->y,
                      &output->conn_id, 1, &mode->info) < 0)
     {
        ERR("Failed to set Mode %dx%d for Output %s: %m",
            mode->width, mode->height, output->name);
     }
}

void
_ecore_drm_output_render_disable(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   output->need_repaint = EINA_FALSE;
   if (!output->enabled) return;
   ecore_drm_output_cursor_size_set(output, 0, 0, 0);
   ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_OFF);
}

/* public functions */

/**
 * @defgroup Ecore_Drm_Output_Group Ecore DRM Output
 *
 * Functions to manage DRM outputs.
 *
 */

EAPI Eina_Bool
ecore_drm_outputs_create(Ecore_Drm_Device *dev)
{
   Eina_Bool ret = EINA_TRUE;
   Ecore_Drm_Output *output = NULL;
   drmModeConnector *conn;
   drmModeRes *res;
   int i = 0, x = 0, y = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dev->drm.fd < 0, EINA_FALSE);

   /* DBG("Create outputs for %d", dev->drm.fd); */

   /* get the resources */
   if (!(res = drmModeGetResources(dev->drm.fd)))
     {
        ERR("Could not get resources for drm card");
        return EINA_FALSE;
     }

   if (!(dev->crtcs = calloc(res->count_crtcs, sizeof(unsigned int))))
     {
        ERR("Could not allocate space for crtcs");
        /* free resources */
        drmModeFreeResources(res);
        return EINA_FALSE;
     }

   dev->crtc_count = res->count_crtcs;
   memcpy(dev->crtcs, res->crtcs, sizeof(unsigned int) * res->count_crtcs);

   dev->min_width = res->min_width;
   dev->min_height = res->min_height;
   dev->max_width = res->max_width;
   dev->max_height = res->max_height;

   /* DBG("Dev Size"); */
   /* DBG("\tMin Width: %u", res->min_width); */
   /* DBG("\tMin Height: %u", res->min_height); */
   /* DBG("\tMax Width: %u", res->max_width); */
   /* DBG("\tMax Height: %u", res->max_height); */

   for (i = 0; i < res->count_connectors; i++)
     {
        /* get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, res->connectors[i])))
          continue;

        /* if (conn->connection != DRM_MODE_CONNECTED) goto next; */

        /* create output for this connector */
        if (!(output =
              _ecore_drm_output_create(dev, res, conn, x, y, EINA_FALSE)))
          goto next;

        x += output->current_mode->width;

next:
        /* free the connector */
        drmModeFreeConnector(conn);
     }

   ret = EINA_TRUE;
   if (!dev->outputs)
     ret = EINA_FALSE;

   /* free resources */
   drmModeFreeResources(res);

   return ret;
}

EAPI void
ecore_drm_output_free(Ecore_Drm_Output *output)
{
   _ecore_drm_output_free(output);
}

EAPI void
ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   if (!output->enabled) return;
   drmModeSetCursor(output->dev->drm.fd, output->crtc_id, handle, w, h);
}

EAPI Eina_Bool
ecore_drm_output_enable(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);

   output->enabled = EINA_TRUE;
   ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_ON);

   _ecore_drm_output_event_send(output, EINA_TRUE);

   return EINA_TRUE;
}

EAPI void
ecore_drm_output_disable(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   output->enabled = EINA_FALSE;
   ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_OFF);

   _ecore_drm_output_event_send(output, EINA_FALSE);
}

EAPI void
ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(fb);
   _ecore_drm_output_fb_release(output, fb);
}

EAPI void
ecore_drm_output_repaint(Ecore_Drm_Output *output)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Sprite *sprite;
   Eina_List *l;
   int ret = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(output->dev);
   EINA_SAFETY_ON_TRUE_RETURN(output->pending_destroy);

   if (!output->enabled) return;

   dev = output->dev;

   /* DBG("Output Repaint: %d %d", output->crtc_id, output->conn_id); */

   /* TODO: assign planes ? */

   if (!dev->next)
     _ecore_drm_output_software_render(output);
   if (!dev->next) return;

   output->need_repaint = EINA_FALSE;

   if ((!dev->current) ||
       (dev->current->stride != dev->next->stride))
     {
        Ecore_Drm_Output_Mode *mode;

        mode = output->current_mode;
        ret = drmModeSetCrtc(dev->drm.fd, output->crtc_id, dev->next->id,
                             0, 0, &output->conn_id, 1, &mode->info);
        if (ret) goto err;

        ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_ON);
     }

   if (drmModePageFlip(dev->drm.fd, output->crtc_id, dev->next->id,
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        ERR("Could not schedule pageflip: %m");
        DBG("\tCrtc: %d\tConn: %d\tFB: %d",
            output->crtc_id, output->conn_id, dev->next->id);
        goto err;
     }

   output->pending_flip = EINA_TRUE;

   /* TODO: output_cursor_set */

   EINA_LIST_FOREACH(dev->sprites, l, sprite)
     {
        unsigned int flags = 0, id = 0;
        drmVBlank vbl =
          {
             .request.type = (DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT),
             .request.sequence = 1,
          };

        if (((!sprite->current_fb) && (!sprite->next_fb)) ||
            (!ecore_drm_sprites_crtc_supported(output, sprite->crtcs)))
          continue;

        if ((sprite->next_fb) && (!dev->cursors_broken))
          id = sprite->next_fb->id;

        ecore_drm_sprites_fb_set(sprite, id, flags);

        vbl.request.signal = (unsigned long)sprite;
        ret = drmWaitVBlank(dev->drm.fd, &vbl);
        if (ret) ERR("Error Wait VBlank");

        sprite->output = output;
        output->pending_vblank = EINA_TRUE;
     }

   return;

err:
   if (dev->next)
     {
        _ecore_drm_output_fb_release(output, dev->next);
        dev->next = NULL;
     }
}

EAPI void
ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h)
{
   drmModeFB *fb;

   if (w) *w = 0;
   if (h) *h = 0;
   EINA_SAFETY_ON_NULL_RETURN(dev);

   if (!(fb = drmModeGetFB(dev->drm.fd, output))) return;
   if (w) *w = fb->width;
   if (h) *h = fb->height;
   drmModeFreeFB(fb);
}

EAPI void
ecore_drm_outputs_geometry_get(Ecore_Drm_Device *dev, int *x, int *y, int *w, int *h)
{
   Ecore_Drm_Output *output;
   Eina_List *l;
   int ox = 0, oy = 0, ow = 0, oh = 0;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   EINA_SAFETY_ON_NULL_RETURN(dev);

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        if ((!output->connected) || (!output->enabled)) continue;
        if (output->cloned) continue;
        ow += MAX(ow, output->current_mode->width);
        oh = MAX(oh, output->current_mode->height);
     }

   if (x) *x = ox;
   if (y) *y = oy;
   if (w) *w = ow;
   if (h) *h = oh;
}

EAPI void
ecore_drm_output_position_get(Ecore_Drm_Output *output, int *x, int *y)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   if (x) *x = output->x;
   if (y) *y = output->y;
}

EAPI void
ecore_drm_output_current_resolution_get(Ecore_Drm_Output *output, int *w, int *h, unsigned int *refresh)
{
   if (w) *w = 0;
   if (h) *h = 0;
   if (refresh) *refresh = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);

   if (!output->current_mode) return;

   if (w) *w = output->current_mode->width;
   if (h) *h = output->current_mode->height;
   if (refresh) *refresh = output->current_mode->refresh;
}

EAPI void
ecore_drm_output_physical_size_get(Ecore_Drm_Output *output, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   if (w) *w = output->phys_width;
   if (h) *h = output->phys_height;
}

EAPI unsigned int
ecore_drm_output_subpixel_order_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);

   return output->subpixel;
}

EAPI Eina_Stringshare *
ecore_drm_output_model_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);

   return output->model;
}

EAPI Eina_Stringshare *
ecore_drm_output_make_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);

   return output->make;
}

EAPI void
ecore_drm_output_dpms_set(Ecore_Drm_Output *output, int level)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(output->dev);
   EINA_SAFETY_ON_NULL_RETURN(output->dpms);

   drmModeConnectorSetProperty(output->dev->drm.fd, output->conn_id,
                               output->dpms->prop_id, level);
}

EAPI void
ecore_drm_output_gamma_set(Ecore_Drm_Output *output, uint16_t size, uint16_t *r, uint16_t *g, uint16_t *b)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(output->dev);
   EINA_SAFETY_ON_NULL_RETURN(output->crtc);

   if (output->gamma != size) return;

   if (drmModeCrtcSetGamma(output->dev->drm.fd, output->crtc_id, size, r, g, b))
     ERR("Failed to set output gamma: %m");
}

EAPI unsigned int
ecore_drm_output_crtc_id_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);

   return output->crtc_id;
}

EAPI unsigned int
ecore_drm_output_crtc_buffer_get(Ecore_Drm_Output *output)
{
   drmModeCrtc *crtc;
   unsigned int id = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->dev, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->crtc, 0);

   if (!(crtc = drmModeGetCrtc(output->dev->drm.fd, output->crtc_id)))
     return 0;

   id = crtc->buffer_id;
   drmModeFreeCrtc(crtc);

   return id;
}

EAPI unsigned int
ecore_drm_output_connector_id_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);

   return output->conn_id;
}

EAPI char *
ecore_drm_output_name_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);

   return strdup(output->name);
}

EAPI Eina_Bool
ecore_drm_output_connected_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);

   return output->connected;
}

EAPI unsigned int
ecore_drm_output_connector_type_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);

   return output->conn_type;
}

EAPI Eina_Bool
ecore_drm_output_backlight_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return (output->backlight != NULL);
}

EAPI char *
ecore_drm_output_edid_get(Ecore_Drm_Output *output)
{
   char *edid_str = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->edid_blob, NULL);

   edid_str = malloc((128 * 2) + 1);
   if (edid_str)
     {
        unsigned int k, kk;
        const char *hexch = "0123456789abcdef";

        for (kk = 0, k = 0; k < 128; k++)
          {
             edid_str[kk] = hexch[(output->edid_blob[k] >> 4) & 0xf];
             edid_str[kk + 1] = hexch[output->edid_blob[k] & 0xf];
             kk += 2;
          }
        edid_str[kk] = 0;
     }

   return edid_str;
}

EAPI Eina_List *
ecore_drm_output_modes_get(Ecore_Drm_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->modes, NULL);

   return output->modes;
}

EAPI Ecore_Drm_Output *
ecore_drm_output_primary_get(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Output *ret;
   const Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);

   EINA_LIST_FOREACH(dev->outputs, l, ret)
     if (ret->primary) return ret;

   return NULL;
}

EAPI void
ecore_drm_output_primary_set(Ecore_Drm_Output *output)
{
   const Eina_List *l;
   Ecore_Drm_Output *out;

   EINA_SAFETY_ON_NULL_RETURN(output);

   /* unmark all outputs as primary */
   EINA_LIST_FOREACH(output->dev->outputs, l, out)
     out->primary = EINA_FALSE;

   /* mark this output as primary */
   output->primary = EINA_TRUE;
}

EAPI void
ecore_drm_output_crtc_size_get(Ecore_Drm_Output *output, int *width, int *height)
{
   if (width) *width = 0;
   if (height) *height = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);

   if (width) *width = output->crtc->width;
   if (height) *height = output->crtc->height;
}

EAPI Eina_Bool
ecore_drm_output_possible_crtc_get(Ecore_Drm_Output *output, unsigned int crtc)
{
   Ecore_Drm_Device *dev;
   drmModeRes *res;
   drmModeConnector *conn;
   drmModeEncoder *enc;
   int i, j, k;
   unsigned int p;
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->dev, EINA_FALSE);

   dev = output->dev;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dev->drm.fd < 0, EINA_FALSE);

   /* get the resources */
   if (!(res = drmModeGetResources(dev->drm.fd)))
     {
        ERR("Could not get resources for drm card");
        return EINA_FALSE;
     }

   for (i = 0; i < res->count_connectors; i++)
     {
        /* get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, res->connectors[i])))
          continue;

        for (j = 0; j < conn->count_encoders; j++)
          {
             /* get the encoder on this connector */
             if (!(enc = drmModeGetEncoder(dev->drm.fd, conn->encoders[j])))
               {
                  WRN("Failed to get encoder");
                  continue;
               }

             /* get the encoder for given crtc */
             if (enc->crtc_id != crtc) goto next;

             p = enc->possible_crtcs;

             for (k = 0; k < res->count_crtcs; k++)
               {
                  if (res->crtcs[k] != output->crtc_id) continue;
                  if (p & (1 << k))
                    {
                       ret = EINA_TRUE;
                       break;
                    }
               }

next:
             drmModeFreeEncoder(enc);
             if (ret) break;
          }

        /* free the connector */
        drmModeFreeConnector(conn);
        if (ret) break;
     }

   /* free resources */
   drmModeFreeResources(res);

   return ret;
}

EAPI Eina_Bool
ecore_drm_output_mode_set(Ecore_Drm_Output *output, Ecore_Drm_Output_Mode *mode, int x, int y)
{
   Ecore_Drm_Device *dev;
   Eina_Bool ret = EINA_TRUE;
   unsigned int buffer = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->dev, EINA_FALSE);

   dev = output->dev;

   output->x = x;
   output->y = y;
   output->current_mode = mode;

   if (mode)
     {
        if (dev->current)
          buffer = dev->current->id;
        else if (dev->next)
          buffer = dev->next->id;
        else
          buffer = output->crtc->buffer_id;

        if (drmModeSetCrtc(dev->drm.fd, output->crtc_id, buffer,
                           output->x, output->y,
                           &output->conn_id, 1, &mode->info) < 0)
          {
             ERR("Failed to set Mode %dx%d for Output %s: %m",
                 mode->width, mode->height, output->name);
             ret = EINA_FALSE;
          }
     }
   else
     {
        if (drmModeSetCrtc(dev->drm.fd, output->crtc_id,
                           0, 0, 0, 0, 0, NULL) < 0)
          {
             ERR("Failed to turn off Output %s: %m", output->name);
             ret = EINA_FALSE;
          }
     }

   return ret;
}

EAPI unsigned int
ecore_drm_output_supported_rotations_get(Ecore_Drm_Output *output, Ecore_Drm_Plane_Type type)
{
   Ecore_Drm_Plane *plane;
   Eina_List *l;
   unsigned int rot = -1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, rot);

   EINA_LIST_FOREACH(output->planes, l, plane)
     {
        if (plane->type != type) continue;
        rot = plane->supported_rotations;
        break;
     }

   return rot;
}

EAPI Eina_Bool
ecore_drm_output_rotation_set(Ecore_Drm_Output *output, Ecore_Drm_Plane_Type type, unsigned int rotation)
{
   Ecore_Drm_Plane *plane;
   Eina_List *l;
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);

   EINA_LIST_FOREACH(output->planes, l, plane)
     {
        if (plane->type != type) continue;
        if ((plane->supported_rotations & rotation) == 0)
          {
             WRN("Unsupported rotation");
             return EINA_FALSE;
          }

        if (drmModeObjectSetProperty(output->dev->drm.fd,
                                     output->primary_plane_id,
                                     DRM_MODE_OBJECT_PLANE,
                                     output->rotation_prop_id,
                                     plane->rotation_map[ffs(rotation)]) < 0)
          {
             WRN("Failed to set Rotation");
             return EINA_FALSE;
          }
        ret = EINA_TRUE;
        break;
     }

   return ret;
}
