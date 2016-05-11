#include "ecore_drm2_private.h"

#define INSIDE(x, y, xx, yy, ww, hh) \
   (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && \
       ((x) >= (xx)) && ((y) >= (yy)))

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
   "DisplayPort", "HDMI-A", "HDMI-B", "TV", "eDP", "Virtual", "DSI",
};

static void
_output_debug(Ecore_Drm2_Output *output, const drmModeConnector *conn)
{
   Eina_List *l;
   Ecore_Drm2_Output_Mode *omode;

   DBG("Created New Output At %d,%d", output->x, output->y);
   DBG("\tCrtc Pos: %d %d", output->ocrtc->x, output->ocrtc->y);
   DBG("\tCrtc: %d", output->crtc_id);
   DBG("\tConn: %d", output->conn_id);
   DBG("\tName: %s", output->name);
   DBG("\tMake: %s", output->make);
   DBG("\tModel: %s", output->model);
   DBG("\tSerial: %s", output->serial);
   DBG("\tCloned: %d", output->cloned);
   DBG("\tPrimary: %d", output->primary);
   DBG("\tConnected: %d", output->connected);
   DBG("\tEnabled: %d", output->enabled);

   if (output->backlight.path)
     {
        DBG("\tBacklight");
        switch (output->backlight.type)
          {
           case ECORE_DRM2_BACKLIGHT_RAW:
             DBG("\t\tType: Raw");
             break;
           case ECORE_DRM2_BACKLIGHT_PLATFORM:
             DBG("\t\tType: Platform");
             break;
           case ECORE_DRM2_BACKLIGHT_FIRMWARE:
             DBG("\t\tType: Firmware");
             break;
          }
        DBG("\t\tPath: %s", output->backlight.path);
     }

   EINA_LIST_FOREACH(output->modes, l, omode)
     {
        DBG("\tAdded Mode: %dx%d@%.1f%s%s%s",
            omode->width, omode->height, (omode->refresh / 1000.0),
            (omode->flags & DRM_MODE_TYPE_PREFERRED) ? ", preferred" : "",
            (omode->flags & DRM_MODE_TYPE_DEFAULT) ? ", current" : "",
            (conn->count_modes == 0) ? ", built-in" : "");
     }
}

static void
_cb_output_event_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Drm2_Event_Output_Changed *ev;

   ev = event;
   eina_stringshare_del(ev->make);
   eina_stringshare_del(ev->model);
   eina_stringshare_del(ev->name);
   free(ev);
}

static void
_output_event_send(Ecore_Drm2_Output *output)
{
   Ecore_Drm2_Event_Output_Changed *ev;

   ev = calloc(1, sizeof(Ecore_Drm2_Event_Output_Changed));
   if (!ev) return;

   ev->id = output->crtc_id;

   ev->x = output->x;
   ev->y = output->y;
   ev->w = output->current_mode->width;
   ev->h = output->current_mode->height;
   ev->phys_width = output->pw;
   ev->phys_height = output->ph;
   ev->refresh = output->current_mode->refresh;

   ev->scale = output->scale;
   ev->subpixel = output->subpixel;
   ev->transform = output->transform;
   ev->connected = output->connected;
   ev->enabled = output->enabled;

   ev->name = eina_stringshare_ref(output->name);
   ev->make = eina_stringshare_ref(output->make);
   ev->model = eina_stringshare_ref(output->model);

   ecore_event_add(ECORE_DRM2_EVENT_OUTPUT_CHANGED, ev,
                   _cb_output_event_free, NULL);
}

static void
_output_edid_parse_string(const uint8_t *data, char text[])
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
_output_edid_parse(Ecore_Drm2_Output *output, const uint8_t *data, size_t len)
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
          _output_edid_parse_string(&data[i + 5], output->edid.monitor);
        else if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER)
          _output_edid_parse_string(&data[i + 5], output->edid.serial);
        else if (data[i + 3] == EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING)
          _output_edid_parse_string(&data[i + 5], output->edid.eisa);
     }

   return 0;
}

static void
_output_edid_find(Ecore_Drm2_Output *output, const drmModeConnector *conn)
{
   drmModePropertyBlobPtr blob = NULL;
   drmModePropertyPtr prop;
   int i = 0, ret = 0;

   for (; i < conn->count_props && !blob; i++)
     {
        if (!(prop = drmModeGetProperty(output->fd, conn->props[i])))
          continue;
        if ((prop->flags & DRM_MODE_PROP_BLOB) &&
            (!strcmp(prop->name, "EDID")))
          {
             blob = drmModeGetPropertyBlob(output->fd, conn->prop_values[i]);
          }
        drmModeFreeProperty(prop);
        if (blob) break;
     }

   if (!blob) return;

   output->edid.blob = eina_memdup(blob->data, blob->length, 1);

   ret = _output_edid_parse(output, blob->data, blob->length);
   if (!ret)
     {
        if (output->edid.pnp[0] != '\0')
          eina_stringshare_replace(&output->make, output->edid.pnp);
        if (output->edid.monitor[0] != '\0')
          eina_stringshare_replace(&output->model, output->edid.monitor);
        if (output->edid.serial[0] != '\0')
          eina_stringshare_replace(&output->serial, output->edid.serial);
     }

   drmModeFreePropertyBlob(blob);
}

static int
_output_crtc_find(const drmModeRes *res, const drmModeConnector *conn, Ecore_Drm2_Device *dev)
{
   drmModeEncoder *enc;
   uint32_t pcrtcs;
   int i = 0, j = 0;

   for (j = 0; j < conn->count_encoders; j++)
     {
        enc = drmModeGetEncoder(dev->fd, conn->encoders[j]);
        if (!enc) continue;

        pcrtcs = enc->possible_crtcs;
        drmModeFreeEncoder(enc);

        for (i = 0; i < res->count_crtcs; i++)
          {
             if ((pcrtcs & (1 << i)) &&
                 (!(dev->alloc.crtc & (1 << res->crtcs[i]))))
               return i;
          }
     }

   return -1;
}

static char *
_output_name_get(const drmModeConnector *conn)
{
   char name[DRM_CONNECTOR_NAME_LEN];
   const char *type = NULL;

   if (conn->connector_type < EINA_C_ARRAY_LENGTH(conn_types))
     type = conn_types[conn->connector_type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->connector_type_id);
   return strdup(name);
}

static Ecore_Drm2_Output_Mode *
_output_mode_add(Ecore_Drm2_Output *output, const drmModeModeInfo *info)
{
   Ecore_Drm2_Output_Mode *mode;
   uint64_t refresh;

   mode = calloc(1, sizeof(Ecore_Drm2_Output_Mode));
   if (!mode) return NULL;

   mode->flags = 0;
   mode->width = info->hdisplay;
   mode->height = info->vdisplay;

   refresh = (info->clock * 1000LL / info->htotal + info->vtotal / 2) /
     info->vtotal;

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

static void
_output_modes_create(Ecore_Drm2_Device *dev, Ecore_Drm2_Output *output, const drmModeConnector *conn)
{
   int i = 0;
   drmModeCrtc *crtc;
   drmModeEncoder *enc;
   drmModeModeInfo crtc_mode;
   Ecore_Drm2_Output_Mode *omode;
   Ecore_Drm2_Output_Mode *current = NULL, *preferred = NULL, *best = NULL;
   Eina_List *l = NULL;

   memset(&crtc_mode, 0, sizeof(crtc_mode));

   enc = drmModeGetEncoder(dev->fd, conn->encoder_id);
   if (enc)
     {
        crtc = drmModeGetCrtc(dev->fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) return;
        if (crtc->mode_valid) crtc_mode = crtc->mode;
        drmModeFreeCrtc(crtc);
     }

   for (i = 0; i < conn->count_modes; i++)
     {
        omode = _output_mode_add(output, &conn->modes[i]);
        if (!omode) continue;
     }

   EINA_LIST_REVERSE_FOREACH(output->modes, l, omode)
     {
        if (!memcmp(&crtc_mode, &omode->info, sizeof(crtc_mode)))
          current = omode;
        if (omode->flags & DRM_MODE_TYPE_PREFERRED)
          preferred = omode;
        best = omode;
     }

   if ((!current) && (crtc_mode.clock != 0))
     {
        current = _output_mode_add(output, &crtc_mode);
        if (!current) goto err;
     }

   if (current) output->current_mode = current;
   else if (preferred) output->current_mode = preferred;
   else if (best) output->current_mode = best;

   if (!output->current_mode) goto err;

   output->current_mode->flags |= DRM_MODE_TYPE_DEFAULT;

   return;

err:
   EINA_LIST_FREE(output->modes, omode)
     free(omode);
}

static drmModePropertyPtr
_output_dpms_property_get(int fd, const drmModeConnector *conn)
{
   drmModePropertyPtr prop;
   int i = 0;

   for (; i < conn->count_props; i++)
     {
        prop = drmModeGetProperty(fd, conn->props[i]);
        if (!prop) continue;

        if (!strcmp(prop->name, "DPMS")) return prop;

        drmModeFreeProperty(prop);
     }

   return NULL;
}

static void
_output_backlight_init(Ecore_Drm2_Output *output, unsigned int conn_type)
{
   Eina_List *devs, *l;
   const char *dev, *t;
   Eina_Bool found = EINA_FALSE;
   Ecore_Drm2_Backlight_Type type = 0;

   devs = eeze_udev_find_by_filter("backlight", NULL, NULL);

   EINA_LIST_FOREACH(devs, l, dev)
     {
        t = eeze_udev_syspath_get_sysattr(dev, "type");
        if (!t) continue;

        if (!strcmp(t, "raw"))
          type = ECORE_DRM2_BACKLIGHT_RAW;
        else if (!strcmp(t, "platform"))
          type = ECORE_DRM2_BACKLIGHT_PLATFORM;
        else if (!strcmp(t, "firmware"))
          type = ECORE_DRM2_BACKLIGHT_FIRMWARE;

        if ((conn_type == DRM_MODE_CONNECTOR_LVDS) ||
            (conn_type == DRM_MODE_CONNECTOR_eDP) ||
            (type == ECORE_DRM2_BACKLIGHT_RAW))
          found = EINA_TRUE;

        eina_stringshare_del(t);
        if (found) break;
     }

   if (found)
     {
        output->backlight.type = type;
        output->backlight.path = eina_stringshare_add(dev);
     }

   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);
}

static void
_output_scale_init(Ecore_Drm2_Output *output, Ecore_Drm2_Transform transform, unsigned int scale)
{
   output->transform = transform;

   if ((output->enabled) && (output->current_mode))
     {
        switch (transform)
          {
           case ECORE_DRM2_TRANSFORM_90:
           case ECORE_DRM2_TRANSFORM_270:
           case ECORE_DRM2_TRANSFORM_FLIPPED_90:
           case ECORE_DRM2_TRANSFORM_FLIPPED_270:
             output->w = output->current_mode->height;
             output->h = output->current_mode->width;
             break;
           case ECORE_DRM2_TRANSFORM_NORMAL:
           case ECORE_DRM2_TRANSFORM_180:
           case ECORE_DRM2_TRANSFORM_FLIPPED:
           case ECORE_DRM2_TRANSFORM_FLIPPED_180:
             output->w = output->current_mode->width;
             output->h = output->current_mode->height;
             break;
           default:
             break;
          }
     }

   output->scale = scale;
   output->w /= scale;
   output->h /= scale;
}

static void
_output_matrix_rotate_xy(Eina_Matrix3 *matrix, double x, double y)
{
   Eina_Matrix4 tmp, m;

   eina_matrix4_identity(&tmp);
   eina_matrix4_values_set(&tmp, x, y, 0, 0, -y, x, 0, 0,
                           0, 0, 1, 0, 0, 0, 0, 1);

   eina_matrix3_matrix4_to(&m, matrix);
   eina_matrix4_multiply(&m, &m, &tmp);
   eina_matrix4_matrix3_to(matrix, &m);
}

static void
_output_matrix_update(Ecore_Drm2_Output *output)
{
   Eina_Matrix3 m3;

   eina_matrix4_identity(&output->matrix);
   eina_matrix4_matrix3_to(&m3, &output->matrix);
   eina_matrix3_translate(&m3, -output->x, -output->y);

   switch (output->transform)
     {
      case ECORE_DRM2_TRANSFORM_FLIPPED:
      case ECORE_DRM2_TRANSFORM_FLIPPED_90:
      case ECORE_DRM2_TRANSFORM_FLIPPED_180:
      case ECORE_DRM2_TRANSFORM_FLIPPED_270:
        eina_matrix3_translate(&m3, -output->w, 0);
        break;
      default:
        break;
     }

   switch (output->transform)
     {
      case ECORE_DRM2_TRANSFORM_NORMAL:
      case ECORE_DRM2_TRANSFORM_FLIPPED:
      default:
        break;
      case ECORE_DRM2_TRANSFORM_90:
      case ECORE_DRM2_TRANSFORM_FLIPPED_90:
        eina_matrix3_translate(&m3, 0, -output->h);
        _output_matrix_rotate_xy(&m3, 0, 1);
        break;
      case ECORE_DRM2_TRANSFORM_180:
      case ECORE_DRM2_TRANSFORM_FLIPPED_180:
        eina_matrix3_translate(&m3, -output->w, -output->h);
        _output_matrix_rotate_xy(&m3, -1, 0);
        break;
      case ECORE_DRM2_TRANSFORM_270:
      case ECORE_DRM2_TRANSFORM_FLIPPED_270:
        eina_matrix3_translate(&m3, -output->w, 0);
        _output_matrix_rotate_xy(&m3, 0, -1);
        break;
     }

   if (output->scale != 1)
     eina_matrix3_scale(&m3, output->scale, output->scale);

   eina_matrix3_matrix4_to(&output->matrix, &m3);
   eina_matrix4_inverse(&output->inverse, &output->matrix);
}

static Eina_Bool
_output_create(Ecore_Drm2_Device *dev, const drmModeRes *res, const drmModeConnector *conn, int x, int y, int *w, Eina_Bool cloned)
{
   Ecore_Drm2_Output *output;
   int i = 0;

   if (w) *w = 0;

   i = _output_crtc_find(res, conn, dev);
   if (i < 0) return EINA_FALSE;

   output = calloc(1, sizeof(Ecore_Drm2_Output));
   if (!output) return EINA_FALSE;

   output->fd = dev->fd;
   output->x = x;
   output->y = y;
   output->cloned = cloned;
   output->pw = conn->mmWidth;
   output->ph = conn->mmHeight;

   switch (conn->subpixel)
     {
      case DRM_MODE_SUBPIXEL_UNKNOWN:
        output->subpixel = 0; // WL_OUTPUT_SUBPIXEL_UNKNOWN
        break;
      case DRM_MODE_SUBPIXEL_NONE:
        output->subpixel = 1; // WL_OUTPUT_SUBPIXEL_NONE
        break;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
        output->subpixel = 2; // WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB
        break;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
        output->subpixel = 3; // WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR
        break;
      case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
        output->subpixel = 4; // WL_OUTPUT_SUBPIXEL_VERTICAL_RGB
        break;
      case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
        output->subpixel = 5; // WL_OUTPUT_SUBPIXEL_VERTICAL_BGR
        break;
      default:
        output->subpixel = 0;
        break;
     }

   output->name = eina_stringshare_add(_output_name_get(conn));
   output->make = eina_stringshare_add("unknown");
   output->model = eina_stringshare_add("unknown");
   output->serial = eina_stringshare_add("unknown");

   output->pipe = i;
   output->crtc_id = res->crtcs[i];
   output->conn_id = conn->connector_id;
   output->conn_type = conn->connector_type;

   output->connected = (conn->connection == DRM_MODE_CONNECTED);

   output->ocrtc = drmModeGetCrtc(dev->fd, output->crtc_id);

   output->dpms = _output_dpms_property_get(dev->fd, conn);

   _output_backlight_init(output, conn->connector_type);

   /* TODO: gamma */

   _output_modes_create(dev, output, conn);

   _output_edid_find(output, conn);

   if (output->connected) output->enabled = EINA_TRUE;

   _output_scale_init(output, ECORE_DRM2_TRANSFORM_NORMAL, 1);
   _output_matrix_update(output);

   if (!eina_list_count(dev->outputs))
     output->primary = EINA_TRUE;

   dev->alloc.crtc |= (1 << output->crtc_id);
   dev->alloc.conn |= (1 << output->conn_id);
   dev->outputs = eina_list_append(dev->outputs, output);

   _output_debug(output, conn);

   if ((output->enabled) && (output->current_mode))
     {
        if (w) *w = output->current_mode->width;
     }

   return EINA_TRUE;
}

static void
_outputs_update(Ecore_Drm2_Device *dev)
{
   drmModeRes *res;
   drmModeConnector *conn;
   uint32_t connected = 0, disconnected = 0;
   int i = 0, x = 0, y = 0;

   res = drmModeGetResources(dev->fd);
   if (!res) return;

   for (i = 0; i < res->count_connectors; i++)
     {
        conn = drmModeGetConnector(dev->fd, res->connectors[i]);
        if (!conn) continue;

        if (conn->connection != DRM_MODE_CONNECTED) goto next;

        connected |= (1 << res->connectors[i]);

        if (!(dev->alloc.conn & (1 << res->connectors[i])))
          {
             if (dev->outputs)
               {
                  Ecore_Drm2_Output *last;

                  last = eina_list_last_data_get(dev->outputs);
                  if (last) x = last->x + last->current_mode->width;
                  else x = 0;
               }
             else
               x = 0;

             if (!_output_create(dev, res, conn, x, y, NULL, EINA_TRUE))
               goto next;
          }

next:
        drmModeFreeConnector(conn);
     }

   drmModeFreeResources(res);

   disconnected = (dev->alloc.conn & ~connected);
   if (disconnected)
     {
        Ecore_Drm2_Output *output;
        Eina_List *l;

        EINA_LIST_FOREACH(dev->outputs, l, output)
          {
             if (disconnected & (1 << output->conn_id))
               {
                  disconnected &= ~(1 << output->conn_id);
                  output->connected = EINA_FALSE;
                  output->enabled = EINA_FALSE;
                  _output_event_send(output);
               }
          }
     }
}

static void
_cb_output_event(const char *device EINA_UNUSED, Eeze_Udev_Event event EINA_UNUSED, void *data, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Ecore_Drm2_Device *dev;

   dev = data;
   _outputs_update(dev);
}

static void
_output_destroy(Ecore_Drm2_Device *dev, Ecore_Drm2_Output *output)
{
   dev->alloc.crtc &= ~(1 << output->crtc_id);
   dev->alloc.conn &= ~(1 << output->conn_id);

   eina_stringshare_del(output->backlight.path);
   eina_stringshare_del(output->name);
   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->serial);

   drmModeFreeProperty(output->dpms);
   free(output->edid.blob);

   free(output);
}

EAPI Eina_Bool
ecore_drm2_outputs_create(Ecore_Drm2_Device *device)
{
   drmModeConnector *conn;
   drmModeRes *res;
   int i = 0, x = 0, y = 0, w = 0;
   int events = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((device->fd < 0), EINA_FALSE);

   res = drmModeGetResources(device->fd);
   if (!res) return EINA_FALSE;

   device->crtcs = calloc(res->count_crtcs, sizeof(uint32_t));
   if (!device->crtcs) goto err;

   device->min.width = res->min_width;
   device->min.height = res->min_height;
   device->max.width = res->max_width;
   device->max.height = res->max_height;

   device->num_crtcs = res->count_crtcs;
   memcpy(device->crtcs, res->crtcs, sizeof(uint32_t) * res->count_crtcs);

   for (i = 0; i < res->count_connectors; i++)
     {
        conn = drmModeGetConnector(device->fd, res->connectors[i]);
        if (!conn) continue;

        if (!_output_create(device, res, conn, x, y, &w, EINA_FALSE))
          goto next;

        x += w;

next:
        drmModeFreeConnector(conn);
     }

   if (eina_list_count(device->outputs) < 1) goto err;

   drmModeFreeResources(res);

   events = (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE |
             EEZE_UDEV_EVENT_CHANGE);

   device->watch =
     eeze_udev_watch_add(EEZE_UDEV_TYPE_DRM, events,
                         _cb_output_event, device);

   return EINA_TRUE;

err:
   drmModeFreeResources(res);
   return EINA_FALSE;
}

EAPI void
ecore_drm2_outputs_destroy(Ecore_Drm2_Device *device)
{
   Ecore_Drm2_Output *output;

   EINA_SAFETY_ON_NULL_RETURN(device);

   EINA_LIST_FREE(device->outputs, output)
     _output_destroy(device, output);
}

EAPI const Eina_List *
ecore_drm2_outputs_get(Ecore_Drm2_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);
   return device->outputs;
}

EAPI int
ecore_drm2_output_dpms_get(Ecore_Drm2_Output *output)
{
   drmModeObjectProperties *props;
   drmModePropertyRes *prop;
   int val = -1;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, -1);

   props =
     drmModeObjectGetProperties(output->fd, output->conn_id,
                                DRM_MODE_OBJECT_CONNECTOR);
   if (!props) return -1;

   for (i = 0; i < props->count_props; i++)
     {
        prop = drmModeGetProperty(output->fd, props->props[i]);
        if (!prop) continue;

        if (!strcmp(prop->name, "DPMS"))
          val = props->prop_values[i];

        drmModeFreeProperty(prop);
     }

   drmModeFreeObjectProperties(props);

   return val;
}

EAPI void
ecore_drm2_output_dpms_set(Ecore_Drm2_Output *output, int level)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_TRUE_RETURN(!output->enabled);

   drmModeConnectorSetProperty(output->fd, output->conn_id,
                               output->dpms->prop_id, level);
}

EAPI char *
ecore_drm2_output_edid_get(Ecore_Drm2_Output *output)
{
   char *edid_str = NULL;
   unsigned char *blob;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->edid.blob, NULL);

   blob = output->edid.blob;

   edid_str = malloc((128 * 2) + 1);
   if (edid_str)
     {
        unsigned int k, kk;
        const char *hexch = "0123456789abcdef";

        for (kk = 0, k = 0; k < 128; k++)
          {
             edid_str[kk] = hexch[(blob[k] >> 4) & 0xf];
             edid_str[kk + 1] = hexch[blob[k] & 0xf];
             kk += 2;
          }
        edid_str[kk] = 0;
     }

   return edid_str;
}

EAPI Eina_Bool
ecore_drm2_output_backlight_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return (output->backlight.path != NULL);
}

EAPI Ecore_Drm2_Output *
ecore_drm2_output_find(Ecore_Drm2_Device *device, int x, int y)
{
   Eina_List *l;
   Ecore_Drm2_Output *output;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);

   EINA_LIST_FOREACH(device->outputs, l, output)
     {
        int ox, oy, ow, oh;

        if (!output->enabled) continue;

        ox = output->x;
        oy = output->y;
        ow = output->current_mode->width;
        oh = output->current_mode->height;

        if (INSIDE(x, y, ox, oy, ow, oh))
          return output;
     }

   return NULL;
}

EAPI void
ecore_drm2_output_geometry_get(Ecore_Drm2_Output *output, int *x, int *y, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_TRUE_RETURN(!output->enabled);

   if (x) *x = output->x;
   if (y) *y = output->y;
   if (w) *w = output->current_mode->width;
   if (h) *h = output->current_mode->height;
}

EAPI unsigned int
ecore_drm2_output_crtc_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);
   return output->crtc_id;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_output_next_fb_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   return output->next;
}

EAPI void
ecore_drm2_output_next_fb_set(Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   output->next = fb;
}

EAPI void
ecore_drm2_output_crtc_size_get(Ecore_Drm2_Output *output, int *w, int *h)
{
   drmModeCrtcPtr crtc;

   if (w) *w = 0;
   if (h) *h = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);

   crtc = drmModeGetCrtc(output->fd, output->crtc_id);
   if (!crtc) return;

   if (w) *w = crtc->width;
   if (h) *h = crtc->height;

   drmModeFreeCrtc(crtc);
}

EAPI Eina_Bool
ecore_drm2_output_primary_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return output->primary;
}

EAPI void
ecore_drm2_output_primary_set(Ecore_Drm2_Output *output, Eina_Bool primary)
{
   EINA_SAFETY_ON_NULL_RETURN(output);
   output->primary = primary;
}

EAPI Eina_Bool
ecore_drm2_output_enabled_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return output->enabled;
}

EAPI void
ecore_drm2_output_enabled_set(Ecore_Drm2_Output *output, Eina_Bool enabled)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   if (output->enabled == enabled) return;
   output->enabled = enabled;

   if (output->enabled)
     ecore_drm2_output_dpms_set(output, DRM_MODE_DPMS_ON);
   else
     ecore_drm2_output_dpms_set(output, DRM_MODE_DPMS_OFF);

   _output_event_send(output);
}

EAPI void
ecore_drm2_output_physical_size_get(Ecore_Drm2_Output *output, int *w, int *h)
{
   if (w) *w = 0;
   if (h) *h = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);

   if (w) *w = output->pw;
   if (h) *h = output->ph;
}

EAPI const Eina_List *
ecore_drm2_output_modes_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   return output->modes;
}

EAPI void
ecore_drm2_output_mode_info_get(Ecore_Drm2_Output_Mode *mode, int *w, int *h, unsigned int *refresh, unsigned int *flags)
{
   if (w) *w = 0;
   if (h) *h = 0;
   if (refresh) *refresh = 0;
   if (flags) *flags = 0;

   EINA_SAFETY_ON_NULL_RETURN(mode);

   if (w) *w = mode->width;
   if (h) *h = mode->height;
   if (refresh) *refresh = mode->refresh;
   if (flags) *flags = mode->flags;
}

EAPI Eina_Bool
ecore_drm2_output_mode_set(Ecore_Drm2_Output *output, Ecore_Drm2_Output_Mode *mode, int x, int y)
{
   Eina_Bool ret = EINA_TRUE;
   unsigned int buffer = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((output->fd < 0), EINA_FALSE);

   output->x = x;
   output->y = y;
   output->current_mode = mode;

   if (mode)
     {
        if (output->current)
          buffer = output->current->id;
        else if (output->next)
          buffer = output->next->id;
        else
          buffer = output->ocrtc->buffer_id;

        if (drmModeSetCrtc(output->fd, output->crtc_id, buffer,
                           x, y, &output->conn_id, 1, &mode->info) < 0)
          {
             ERR("Failed to set Mode %dx%d for Output %s: %m",
                 mode->width, mode->height, output->name);
             ret = EINA_FALSE;
          }
     }
   else
     {
        if (drmModeSetCrtc(output->fd, output->crtc_id, 0,
                           0, 0, 0, 0, NULL) < 0)
          {
             ERR("Failed to turn off Output %s: %m", output->name);
             ret = EINA_FALSE;
          }
     }

   return ret;
}

EAPI char *
ecore_drm2_output_name_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->name, NULL);
   return strdup(output->name);
}

EAPI char *
ecore_drm2_output_model_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->model, NULL);
   return strdup(output->model);
}

EAPI Eina_Bool
ecore_drm2_output_connected_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return output->connected;
}

EAPI Eina_Bool
ecore_drm2_output_cloned_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   return output->cloned;
}

EAPI unsigned int
ecore_drm2_output_connector_type_get(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);
   return output->conn_type;
}

EAPI void
ecore_drm2_output_resolution_get(Ecore_Drm2_Output *output, int *w, int *h, unsigned int *refresh)
{
   if (w) *w = 0;
   if (h) *h = 0;
   if (refresh) *refresh = 0;

   EINA_SAFETY_ON_NULL_RETURN(output);
   EINA_SAFETY_ON_TRUE_RETURN(!output->current_mode);

   if (w) *w = output->current_mode->width;
   if (h) *h = output->current_mode->height;
   if (refresh) *refresh = output->current_mode->refresh;
}
