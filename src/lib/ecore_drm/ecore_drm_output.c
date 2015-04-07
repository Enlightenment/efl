#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <ctype.h>

#define ALEN(array) (sizeof(array) / sizeof(array)[0])

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
   free(event);
}

static void
_ecore_drm_output_event_send(const Ecore_Drm_Output *output, Eina_Bool plug)
{
   Ecore_Drm_Event_Output *e;

   if (!(e = calloc(1, sizeof(Ecore_Drm_Event_Output)))) return;
   e->plug = plug;
   e->id = output->crtc_id;
   e->w = output->current_mode->width;
   e->h = output->current_mode->height;
   e->x = output->x;
   e->y = output->y;
   e->phys_width = output->phys_width;
   e->phys_height = output->phys_height;
   e->refresh = output->current_mode->refresh;
   e->subpixel_order = output->subpixel;
   e->make = eina_stringshare_ref(output->make);
   e->model = eina_stringshare_ref(output->model);
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

   if (rep > 4) text[i] = '\0';
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
     }

   if (!blob) return;

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

static Eina_Bool 
_ecore_drm_output_software_setup(Ecore_Drm_Device *dev, Ecore_Drm_Output *output)
{
   unsigned int i = 0;
   int w = 0, h = 0;

   if ((!dev) || (!output)) return EINA_FALSE;

   if (output->current_mode)
     {
        w = output->current_mode->width;
        h = output->current_mode->height;
     }
   else
     {
        w = 1024;
        h = 768;
     }

   for (i = 0; i < NUM_FRAME_BUFFERS; i++)
     {
        if (!(output->dumb[i] = ecore_drm_fb_create(dev, w, h)))
          {
             ERR("Could not create dumb framebuffer %d", i);
             goto err;
          }
     }

   return EINA_TRUE;

err:
   for (i = 0; i < NUM_FRAME_BUFFERS; i++)
     {
        if (output->dumb[i]) ecore_drm_fb_destroy(output->dumb[i]);
        output->dumb[i] = NULL;
     }

   return EINA_FALSE;
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
             WRN("Failed to get encoder: %m");
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

   refresh = (info->clock * 1000000LL / info->htotal + info->vtotal / 2) / info->vtotal;
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
_ecore_drm_output_backlight_init(Ecore_Drm_Output *output EINA_UNUSED, uint32_t conn_type)
{
   Ecore_Drm_Backlight *backlight = NULL;
   Ecore_Drm_Backlight_Type type = 0;
   Eina_List *devs, *l;
   Eina_Bool found = EINA_FALSE;
   const char *device, *devtype;

   if (!(devs = eeze_udev_find_by_type(EEZE_UDEV_TYPE_BACKLIGHT, NULL)))
     devs = eeze_udev_find_by_type(EEZE_UDEV_TYPE_LEDS, NULL);

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

        if ((conn_type != DRM_MODE_CONNECTOR_LVDS) && 
            (conn_type != DRM_MODE_CONNECTOR_eDP))
          {
             if (type != ECORE_DRM_BACKLIGHT_RAW) goto cont;
          }

        found = EINA_TRUE;
cont:
        eina_stringshare_del(devtype);
        if (found) break;
     }

   if (!found) goto out;

   if ((backlight = calloc(1, sizeof(Ecore_Drm_Backlight))))
     {
        backlight->type = type;
        backlight->device = eina_stringshare_add(device);
     }

out:
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
   Ecore_Drm_Output_Mode *mode, *current, *preferred, *best;
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
   output->subpixel = conn->subpixel;

   output->make = eina_stringshare_add("UNKNOWN");
   output->model = eina_stringshare_add("UNKNOWN");
   output->name = eina_stringshare_add("UNKNOWN");

   if (conn->connector_type < ALEN(conn_types))
     type = conn_types[conn->connector_type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->connector_type_id);
   eina_stringshare_replace(&output->name, name);

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

   if (drmModeSetCrtc(output->dev->drm.fd, output->crtc_id,
                      output->crtc->buffer_id, 0, 0,
                      &output->conn_id, 1, &output->current_mode->info) < 0)
     {
        ERR("Failed to set Mode %dx%d for Output %s: %m",
            output->current_mode->width, output->current_mode->height,
            output->name);
        goto err;
     }

   /* try to init backlight */
   output->backlight = 
     _ecore_drm_output_backlight_init(output, conn->connector_type);

   /* parse edid */
   _ecore_drm_output_edid_find(output, conn);

   /* TODO: implement support for LCMS ? */
   output->gamma = output->crtc->gamma_size;

   if (!_ecore_drm_output_software_setup(dev, output))
     goto err;

   dev->outputs = eina_list_append(dev->outputs, output);

   DBG("Created New Output At %d,%d", output->x, output->y);
   DBG("\tCrtc Pos: %d %d", output->crtc->x, output->crtc->y);
   DBG("\tCrtc: %d", output->crtc_id);
   DBG("\tConn: %d", output->conn_id);
   DBG("\tMake: %s", output->make);
   DBG("\tModel: %s", output->model);
   DBG("\tName: %s", output->name);
   DBG("\tCloned: %d", output->cloned);

   EINA_LIST_FOREACH(output->modes, l, mode)
     {
        DBG("\tAdded Mode: %dx%d@%.1f%s%s%s",
            mode->width, mode->height, (mode->refresh / 1000.0),
            (mode->flags & DRM_MODE_TYPE_PREFERRED) ? ", preferred" : "",
            (mode->flags & DRM_MODE_TYPE_DEFAULT) ? ", current" : "",
            (conn->count_modes == 0) ? ", built-in" : "");
     }

   _ecore_drm_output_event_send(output, EINA_TRUE);

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

   /* check for valid output */
   if (!output) return;

   /* delete the backlight struct */
   if (output->backlight) 
     _ecore_drm_output_backlight_shutdown(output->backlight);

   /* turn off hardware cursor */
   drmModeSetCursor(output->dev->drm.fd, output->crtc_id, 0, 0, 0);

   /* restore crtc state */
   if (output->crtc)
     drmModeSetCrtc(output->dev->drm.fd, output->crtc->crtc_id,
                    output->crtc->buffer_id, output->crtc->x, output->crtc->y, 
                    &output->conn_id, 1, &output->crtc->mode);

   /* free modes */
   EINA_LIST_FREE(output->modes, mode)
     free(mode);

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

   if ((fb->mmap) && (fb != output->dumb[0]) && (fb != output->dumb[1]))
     ecore_drm_fb_destroy(fb);
}

void 
_ecore_drm_output_repaint_start(Ecore_Drm_Output *output)
{
   unsigned int fb;

   /* DBG("Output Repaint Start"); */

   if (!output) return;

   if (!output->current)
     {
        /* DBG("\tNo Current FB"); */
        goto finish;
     }

   fb = output->current->id;

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
             if (eina_list_count(dev->outputs) > 0)
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
                  ecore_drm_output_free(output);
               }
          }
     }
}

/* public functions */

/**
 * @defgroup Ecore_Drm_Output_Group Ecore DRM Output
 * 
 * Functions to manage DRM outputs.
 * 
 */

/**
 * Create outputs for a drm device
 *
 * This function will create outputs for Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device device for which outputs
 *            needs to be created   
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Ecore_Drm_Output_Group
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

   /* DBG("Create outputs for %d", dev->drm.fd); */

   /* get the resources */
   if (!(res = drmModeGetResources(dev->drm.fd)))
     {
        ERR("Could not get resources for drm card: %m");
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

   for (i = 0; i < res->count_connectors; i++)
     {
        /* get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, res->connectors[i])))
          continue;

        if (conn->connection != DRM_MODE_CONNECTED) goto next;

        /* create output for this connector */
        if (!(output =
              _ecore_drm_output_create(dev, res, conn, x, y, EINA_FALSE)))
          goto next;

        x += output->current_mode->width;

next:
        /* free the connector */
        drmModeFreeConnector(conn);
     }

   /* TODO: Planes */

   ret = EINA_TRUE;
   if (eina_list_count(dev->outputs) < 1) 
     ret = EINA_FALSE;

   /* free resources */
   drmModeFreeResources(res);

   return ret;
}

/**
 * Free an Ecore_Drm_Output 
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to free
 * 
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void 
ecore_drm_output_free(Ecore_Drm_Output *output)
{
   _ecore_drm_output_free(output);
}

/**
 * Set the cursor size for Ecore_Drm_Output 
 *
 * This function will set the cursor size of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to set
 * @param handle The Drm handle
 * @param w The width of cursor
 * @param h The height of cursor
 *
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void 
ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h)
{
   if (!output) return;
   drmModeSetCursor(output->dev->drm.fd, output->crtc_id, handle, w, h);
}

EAPI Eina_Bool 
ecore_drm_output_enable(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   if ((!output) || (!output->current)) return EINA_FALSE;

   ecore_drm_output_dpms_set(output, DRM_MODE_DPMS_ON);

   mode = output->current_mode;
   if (drmModeSetCrtc(output->dev->drm.fd, output->crtc_id, output->current->id, 
                      0, 0, &output->conn_id, 1, &mode->info) < 0)
     {
        ERR("Could not set output crtc: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI void 
ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb)
{
   if ((!output) || (!fb)) return;

   if ((fb->mmap) && (fb != output->dumb[0]) && (fb != output->dumb[1]))
     ecore_drm_fb_destroy(fb);
}

EAPI void 
ecore_drm_output_repaint(Ecore_Drm_Output *output)
{
   Eina_List *l;
   Ecore_Drm_Sprite *sprite;
   int ret = 0;

   if (!output) return;

   /* DBG("Output Repaint: %d %d", output->crtc_id, output->conn_id); */

   /* TODO: assign planes ? */

   if (!output->next)
     _ecore_drm_output_software_render(output);

   if (!output->next) return;

   output->need_repaint = EINA_FALSE;

   if (!output->current)
     {
        Ecore_Drm_Output_Mode *mode;

        mode = output->current_mode;

        ret = drmModeSetCrtc(output->dev->drm.fd, output->crtc_id, 
                             output->next->id, 0, 0, &output->conn_id, 1, 
                             &mode->info);
        if (ret) goto err;
     }

   /* TODO: set dpms to on */

   if (drmModePageFlip(output->dev->drm.fd, output->crtc_id, output->next->id,
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        /* ERR("Scheduling pageflip failed"); */
        goto err;
     }

   output->pending_flip = EINA_TRUE;

   EINA_LIST_FOREACH(output->dev->sprites, l, sprite)
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

        if ((sprite->next_fb) && (!output->dev->cursors_broken))
          id = sprite->next_fb->id;

        ecore_drm_sprites_fb_set(sprite, id, flags);

        vbl.request.signal = (unsigned long)sprite;
        ret = drmWaitVBlank(output->dev->drm.fd, &vbl);
        if (ret) ERR("Error Wait VBlank: %m");

        sprite->output = output;
        output->pending_vblank = EINA_TRUE;
     }

   return;

err:
   if (output->next)
     {
        ecore_drm_output_fb_release(output, output->next);
        output->next = NULL;
     }
}

/**
 * Get the output size of Ecore_Drm_Device
 *
 * This function will give the output size of Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device to get output size
 * @param output The output id whose information needs to be retrived 
 * @param *w The parameter in which output width is stored
 * @param *h The parameter in which output height is stored
 *
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void 
ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h)
{
   drmModeFB *fb;

   if (w) *w = 0;
   if (h) *h = 0;
   if (!dev) return;

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
   if (!dev) return;

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        ox += output->x;
        oy += output->y;
        ow += MAX(ow, output->current_mode->width);
        oh += MAX(oh, output->current_mode->height);
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
   EINA_SAFETY_ON_NULL_RETURN(output);

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
   EINA_SAFETY_ON_NULL_RETURN(output->dpms);

   drmModeConnectorSetProperty(output->dev->drm.fd, output->conn_id,
                               output->dpms->prop_id, level);
}

EAPI void
ecore_drm_output_gamma_set(Ecore_Drm_Output *output, uint16_t size, uint16_t *r, uint16_t *g, uint16_t *b)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_NULL_RETURN(output->crtc);

   if (output->gamma != size) return;

   if (drmModeCrtcSetGamma(output->dev->drm.fd, output->crtc_id, size, r, g, b))
     ERR("Failed to set output gamma: %m");
}
