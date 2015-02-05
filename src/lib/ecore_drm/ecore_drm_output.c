#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

#define ALEN(array) (sizeof(array) / sizeof(array)[0])

static const char *conn_types[] = 
{
   "None", "VGA", "DVI", "DVI", "DVI",
   "Composite", "TV", "LVDS", "CTV", "DIN",
   "DP", "HDMI", "HDMI", "TV", "eDP",
};

/* local functions */

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
                  return res->crtcs[i];
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

   /* DBG("Added Mode: %dx%d@%d to Output %d",  */
   /*     mode->width, mode->height, mode->refresh, output->crtc_id); */

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
_ecore_drm_output_create(Ecore_Drm_Device *dev, drmModeRes *res, drmModeConnector *conn, int x, int y)
{
   Ecore_Drm_Output *output;
   Ecore_Drm_Output_Mode *mode;
   const char *conn_name;
   char name[32];
   int i = 0;
   drmModeEncoder *enc;
   drmModeModeInfo crtc_mode;
   drmModeCrtc *crtc;
   Eina_List *l;

   i = _ecore_drm_output_crtc_find(dev, res, conn);
   if (i < 0)
     {
        ERR("Could not find crtc or encoder for connector");
        return NULL;
     }

   /* try to allocate space for output */
   if (!(output = calloc(1, sizeof(Ecore_Drm_Output))))
     {
        ERR("Could not allocate space for output");
        return NULL;
     }

   output->dev = dev;
   output->x = x;
   output->y = y;

   output->subpixel = conn->subpixel;
   output->make = eina_stringshare_add("unknown");
   output->model = eina_stringshare_add("unknown");

   if (conn->connector_type < ALEN(conn_types))
     conn_name = conn_types[conn->connector_type];
   else
     conn_name = "UNKNOWN";

   snprintf(name, sizeof(name), "%s%d", conn_name, conn->connector_type_id);
   output->name = eina_stringshare_add(name);

   output->crtc_id = res->crtcs[i];
   dev->crtc_allocator |= (1 << output->crtc_id);
   output->conn_id = conn->connector_id;
   dev->conn_allocator |= (1 << output->conn_id);
   output->crtc = drmModeGetCrtc(dev->drm.fd, output->crtc_id);

   memset(&mode, 0, sizeof(mode));
   if ((enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id)))
     {
        crtc = drmModeGetCrtc(dev->drm.fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) goto mode_err;
        if (crtc->mode_valid) crtc_mode = crtc->mode;
        drmModeFreeCrtc(crtc);
     }

   for (i = 0; i < conn->count_modes; i++)
     {
        if (!(mode = _ecore_drm_output_mode_add(output, &conn->modes[i])))
          {
             ERR("Failed to add mode to output");
             goto mode_err;
          }
     }

   EINA_LIST_REVERSE_FOREACH(output->modes, l, mode)
     {
        if (!memcmp(&crtc_mode, &mode->info, sizeof(crtc_mode)))
          {
             output->current_mode = mode;
             break;
          }
     }

   if ((!output->current_mode) && (crtc_mode.clock != 0))
     {
        output->current_mode = _ecore_drm_output_mode_add(output, &crtc_mode);
        if (!output->current_mode) goto mode_err;
     }

   dev->use_hw_accel = EINA_FALSE;
   if (!_ecore_drm_output_software_setup(dev, output))
     goto mode_err;
   else
     DBG("Setup Output %d for Software Rendering", output->crtc_id);

   output->backlight = 
     _ecore_drm_output_backlight_init(output, conn->connector_type);

   return output;

mode_err:
   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->name);
   EINA_LIST_FREE(output->modes, mode)
     free(mode);
   drmModeFreeCrtc(output->crtc);
   dev->crtc_allocator &= ~(1 << output->crtc_id);
   dev->conn_allocator &= ~(1 << output->conn_id);
   free(output);
   return NULL;
}

static void 
_ecore_drm_output_free(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   /* check for valid output */
   if (!output) return;

   /* delete any added udev watch */
   if (output->watch) eeze_udev_watch_del(output->watch);

   /* delete the backlight struct */
   if (output->backlight) 
     _ecore_drm_output_backlight_shutdown(output->backlight);

   /* turn off hardware cursor */
   drmModeSetCursor(output->drm_fd, output->crtc_id, 0, 0, 0);

   /* restore crtc state */
   if (output->crtc)
     drmModeSetCrtc(output->drm_fd, output->crtc->crtc_id, 
                    output->crtc->buffer_id, output->crtc->x, output->crtc->y, 
                    &output->conn_id, 1, &output->crtc->mode);

   /* free modes */
   EINA_LIST_FREE(output->modes, mode)
     free(mode);

   /* free strings */
   if (output->name) eina_stringshare_del(output->name);
   if (output->model) eina_stringshare_del(output->model);
   if (output->make) eina_stringshare_del(output->make);

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

static Eina_Bool
_ecore_drm_output_device_is_hotplug(Ecore_Drm_Output *output)
{
   const char *syspath;
   const char *val;
   int sysnum;

   syspath = output->dev->drm.path;
   sysnum = eeze_udev_syspath_get_sysnum(syspath);

   if ((sysnum == -1) || (sysnum != output->dev->id))
     return EINA_FALSE;

   val = eeze_udev_syspath_get_property(syspath, "HOTPLUG");
   if (!val)
     return EINA_FALSE;

   return (strcmp(val, "1") == 0);
}

static void
_ecore_drm_update_outputs(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output *new_output;
   drmModeConnector *connector;
   drmModeRes *res;
   drmModeCrtc *crtc;
   int x = 0, y = 0;
   uint32_t connected = 0, disconnects = 0;
   int i;
   Eina_List *l;

   res = drmModeGetResources(output->drm_fd);
   if (!res)
     {
        ERR("Could not get resources for drm card: %m");
        return;
     }

   for (i = 0; i < res->count_connectors; i++)
     {
        int connector_id = res->connectors[i];

        connector = drmModeGetConnector(output->drm_fd, connector_id);
        if (connector == NULL)
          continue;

        if (connector->connection != DRM_MODE_CONNECTED)
          {
             drmModeFreeConnector(connector);
             continue;
          }

        connected |= (1 << connector_id);

        if (!(output->dev->conn_allocator & (1 << connector_id)))
          {
             drmModeEncoder *enc;

             if (!(new_output = _ecore_drm_output_create(output->dev, res, connector, x, y)))
               {
                  drmModeFreeConnector(connector);
                  _ecore_drm_output_free(new_output);
                  continue;
               }

             new_output->drm_fd = output->dev->drm.fd;

             if (!(enc = drmModeGetEncoder(new_output->dev->drm.fd, connector->encoder_id)))
               {
                  drmModeFreeConnector(connector);
                  _ecore_drm_output_free(new_output);
                  continue;
               }

             if (!(crtc = drmModeGetCrtc(new_output->dev->drm.fd, enc->crtc_id)))
               {
                  drmModeFreeEncoder(enc);
                  drmModeFreeConnector(connector);
                  _ecore_drm_output_free(new_output);
                  continue;
               }

             x += crtc->width;

             drmModeFreeCrtc(crtc);
             drmModeFreeEncoder(enc);

             output->dev->outputs = 
               eina_list_append(output->dev->outputs, new_output);
          }
          drmModeFreeConnector(connector);
     }

   disconnects = output->dev->conn_allocator & ~connected;
   if (disconnects)
     {
        EINA_LIST_FOREACH(output->dev->outputs, l, new_output)
          {
             if (disconnects & (1 << new_output->conn_id))
               {
                  disconnects &= ~(1 << new_output->conn_id);
                  ecore_drm_output_free(new_output);
               }
          }
     }
}

static void
_ecore_drm_output_event(const char *device EINA_UNUSED, Eeze_Udev_Event event EINA_UNUSED, void *data, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Ecore_Drm_Output *output;

   if (!(output = data)) return;

   if (_ecore_drm_output_device_is_hotplug(output))
     _ecore_drm_update_outputs(output);
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
   drmModeCrtc *crtc;
   int i = 0, x = 0, y = 0;

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
        int events = 0;

        /* get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, res->connectors[i])))
          continue;

        if ((conn->connection == DRM_MODE_CONNECTED) && 
            (conn->count_modes > 0))
          {
             drmModeEncoder *enc;

             /* create output for this connector */
             if (!(output = _ecore_drm_output_create(dev, res, conn, x, y)))
               {
                  /* free the connector */
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             output->drm_fd = dev->drm.fd;

             if (!(enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id)))
               {
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             if (!(crtc = drmModeGetCrtc(dev->drm.fd, enc->crtc_id)))
               {
                  drmModeFreeEncoder(enc);
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             x += crtc->width;

             drmModeFreeCrtc(crtc);
             drmModeFreeEncoder(enc);

             events = (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE);

             output->watch =
               eeze_udev_watch_add(EEZE_UDEV_TYPE_DRM, events,
                                   _ecore_drm_output_event, output);

             if (!output->watch)
               ERR("Could not create Eeze_Udev_Watch for drm output");

             dev->outputs = eina_list_append(dev->outputs, output);
          }

        /* free the connector */
        drmModeFreeConnector(conn);
     }

   ret = EINA_TRUE;
   if (eina_list_count(dev->outputs) < 1) 
     {
        ret = EINA_FALSE;
        free(dev->crtcs);
     }

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
   drmModeSetCursor(output->drm_fd, output->crtc_id, handle, w, h);
}

EAPI Eina_Bool 
ecore_drm_output_enable(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   if ((!output) || (!output->current)) return EINA_FALSE;

   mode = output->current_mode;
   if (drmModeSetCrtc(output->drm_fd, output->crtc_id, output->current->id, 
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
