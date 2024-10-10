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

static Eina_Thread_Queue *thq = NULL;

typedef struct
{
   Eina_Thread_Queue_Msg head;
   Ecore_Drm2_Thread_Op_Code code;
} Thread_Msg;

static const char *conn_types[] =
{
   "None", "VGA", "DVI-I", "DVI-D", "DVI-A",
   "Composite", "S-Video", "LVDS", "Component", "DIN",
   "DisplayPort", "HDMI-A", "HDMI-B", "TV", "eDP", "Virtual", "DSI",
};

static void
_ecore_drm2_display_state_thread_send(Ecore_Drm2_Thread_Op_Code code)
{
   Thread_Msg *msg;
   void *ref;

   msg = eina_thread_queue_send(thq, sizeof(Thread_Msg), &ref);
   msg->code = code;
   eina_thread_queue_send_done(thq, ref);
}

static char *
_ecore_drm2_display_name_get(Ecore_Drm2_Connector *conn)
{
   char name[DRM_CONNECTOR_NAME_LEN];
   const char *type = NULL;

   if (conn->type < EINA_C_ARRAY_LENGTH(conn_types))
     type = conn_types[conn->type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->drmConn->connector_type_id);
   return strdup(name);
}

static void
_ecore_drm2_display_edid_parse_string(const uint8_t *data, char text[])
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
_ecore_drm2_display_edid_parse(Ecore_Drm2_Display *disp, const uint8_t *data, size_t len)
{
   int i = 0;
   uint32_t serial;

   if (len < 128) return -1;
   if ((data[0] != 0x00) || (data[1] != 0xff)) return -1;

   disp->edid.pnp[0] = 'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x7c) / 4) - 1;
   disp->edid.pnp[1] =
     'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x3) * 8) +
     ((data[EDID_OFFSET_PNPID + 1] & 0xe0) / 32) - 1;
   disp->edid.pnp[2] = 'A' + (data[EDID_OFFSET_PNPID + 1] & 0x1f) - 1;
   disp->edid.pnp[3] = '\0';

   serial = (uint32_t) data[EDID_OFFSET_SERIAL + 0];
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 1] * 0x100;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 2] * 0x10000;
   serial += (uint32_t) data[EDID_OFFSET_SERIAL + 3] * 0x1000000;
   if (serial > 0)
     sprintf(disp->edid.serial, "%lu", (unsigned long)serial);

   for (i = EDID_OFFSET_DATA_BLOCKS; i <= EDID_OFFSET_LAST_BLOCK; i += 18)
     {
        if (data[i] != 0) continue;
        if (data[i + 2] != 0) continue;

        if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME)
          _ecore_drm2_display_edid_parse_string(&data[i + 5], disp->edid.monitor);
        else if (data[i + 3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER)
          _ecore_drm2_display_edid_parse_string(&data[i + 5], disp->edid.serial);
        else if (data[i + 3] == EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING)
          _ecore_drm2_display_edid_parse_string(&data[i + 5], disp->edid.eisa);
     }

   return 0;
}

static void
_ecore_drm2_display_edid_get(Ecore_Drm2_Display *disp)
{
   Ecore_Drm2_Connector_State *cstate;
   int ret = 0;

   cstate = disp->conn->state.current;

   ret = _ecore_drm2_display_edid_parse(disp, cstate->edid.data, cstate->edid.len);
   if (!ret)
     {
        if (disp->edid.pnp[0] != '\0')
          eina_stringshare_replace(&disp->make, disp->edid.pnp);
        if (disp->edid.monitor[0] != '\0')
          eina_stringshare_replace(&disp->model, disp->edid.monitor);
        if (disp->edid.serial[0] != '\0')
          eina_stringshare_replace(&disp->serial, disp->edid.serial);
     }
}

static void
_ecore_drm2_display_state_debug(Ecore_Drm2_Display *disp)
{
   Eina_List *l = NULL;
   Ecore_Drm2_Display_Mode *mode;

   DBG("Display Atomic State Fill Complete");

   DBG("\tName: %s", disp->name);
   DBG("\tMake: %s", disp->make);
   DBG("\tModel: %s", disp->model);
   DBG("\tSerial: %s", disp->serial);

   DBG("\tCrtc: %d", disp->crtc->id);
   DBG("\tCrtc Pos: %d %d", disp->crtc->drmCrtc->x, disp->crtc->drmCrtc->y);
   DBG("\tConnector: %d", disp->conn->id);

   if (disp->backlight.path)
     {
        DBG("\tBacklight");
        switch (disp->backlight.type)
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
        DBG("\t\tPath: %s", disp->backlight.path);
     }

   EINA_LIST_FOREACH(disp->modes, l, mode)
     {
        DBG("\tAdded Mode: %dx%d@%.1f%s%s%s, %.1f MHz",
            mode->width, mode->height, mode->refresh / 1000.0,
            (mode->flags & DRM_MODE_TYPE_PREFERRED) ? ", preferred" : "",
            (mode->flags & DRM_MODE_TYPE_DEFAULT) ? ", current" : "",
            (disp->conn->drmConn->count_modes == 0) ? ", built-in" : "",
            mode->info.clock / 1000.0);
     }

   /* DBG("\tCloned: %d", disp->cloned); */
   DBG("\tPrimary: %d", disp->state.current->primary);
   DBG("\tEnabled: %d", disp->state.current->enabled);
   DBG("\tConnected: %d", disp->connected);
}

static double
_ecore_drm2_display_backlight_value_get(Ecore_Drm2_Display *disp, const char *attr)
{
   const char *b = NULL;
   double ret = 0.0;

   if ((!disp) || (!disp->backlight.path)) return 0.0;

   b = eeze_udev_syspath_get_sysattr(disp->backlight.path, attr);
   if (!b) return 0.0;

   ret = strtod(b, NULL);
   if (ret < 0) ret = 0.0;

   return ret;
}

static void
_ecore_drm2_display_backlight_get(Ecore_Drm2_Display *disp)
{
   Eina_List *devs, *l;
   const char *dev, *t;
   Ecore_Drm2_Backlight_Type type = 0;
   Eina_Bool found = EINA_FALSE;

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

        if ((disp->conn->type == DRM_MODE_CONNECTOR_LVDS) ||
            (disp->conn->type == DRM_MODE_CONNECTOR_eDP) ||
            (type == ECORE_DRM2_BACKLIGHT_RAW))
          found = EINA_TRUE;

        eina_stringshare_del(t);
        if (found) break;
     }

   if (found)
     {
        disp->backlight.type = type;
        disp->backlight.path = eina_stringshare_add(dev);
        disp->backlight.max =
          _ecore_drm2_display_backlight_value_get(disp, "max_brightness");
        disp->state.current->backlight =
          _ecore_drm2_display_backlight_value_get(disp, "brightness");
     }

   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);
}

static Ecore_Drm2_Display_Mode *
_ecore_drm2_display_mode_create(const drmModeModeInfo *info)
{
   Ecore_Drm2_Display_Mode *mode;
   uint64_t refresh;

   EINA_SAFETY_ON_NULL_RETURN_VAL(info, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((info->htotal > 0), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((info->vtotal > 0), NULL);

   mode = calloc(1, sizeof(Ecore_Drm2_Display_Mode));
   if (!mode) return NULL;

   mode->flags = 0;
   mode->width = info->hdisplay;
   mode->height = info->vdisplay;

   refresh = (info->clock * 1000000LL / info->htotal +
              info->vtotal / 2) / info->vtotal;

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

   return mode;
}

static void
_ecore_drm2_display_modes_get(Ecore_Drm2_Display *disp)
{
   int i = 0;
   drmModeModeInfo crtc_mode;
   Ecore_Drm2_Display_Mode *dmode;
   Ecore_Drm2_Display_Mode *current = NULL, *pref = NULL, *best = NULL;
   Eina_List *l = NULL;

   memset(&crtc_mode, 0, sizeof(crtc_mode));

   if (disp->crtc->drmCrtc->mode_valid)
     crtc_mode = disp->crtc->drmCrtc->mode;

   /* loop through connector modes and try to create mode */
   for (; i < disp->conn->drmConn->count_modes; i++)
     {
        dmode =
          _ecore_drm2_display_mode_create(&disp->conn->drmConn->modes[i]);
        if (!dmode) continue;

        /* append mode to display mode list */
        disp->modes = eina_list_append(disp->modes, dmode);
     }

   /* try to select current mode */
   EINA_LIST_REVERSE_FOREACH(disp->modes, l, dmode)
     {
        if (!memcmp(&crtc_mode, &dmode->info, sizeof(crtc_mode)))
          current = dmode;
        if (dmode->flags & DRM_MODE_TYPE_PREFERRED)
          pref = dmode;
        best = dmode;
     }

   if ((!current) && (crtc_mode.clock != 0))
     {
        current = _ecore_drm2_display_mode_create(&crtc_mode);
        if (!current) goto err;
        disp->modes = eina_list_append(disp->modes, current);
     }

   if (current) disp->state.current->mode = current;
   else if (pref) disp->state.current->mode = pref;
   else if (best) disp->state.current->mode = best;

   if (!disp->state.current->mode) goto err;

   disp->state.current->mode->flags |= DRM_MODE_TYPE_DEFAULT;

   return;

err:
   EINA_LIST_FREE(disp->modes, dmode)
     free(dmode);
}

static void
_ecore_drm2_display_rotation_get(Ecore_Drm2_Display *disp)
{
   Ecore_Drm2_Plane *plane;

   /* try to find primary plane for this display */
   plane = _ecore_drm2_planes_primary_find(disp->dev, disp->crtc->id);
   if (plane)
     {
        if (plane->state.current)
          disp->state.current->rotation = plane->state.current->rotation.value;
        else
          {
             drmModeObjectPropertiesPtr oprops;

             /* NB: Sadly we cannot rely on plane->state.current being already 
              * filled by the time we reach this (due to threading), 
              * so we will query the plane properties we want directly */

             /* query plane for rotations */
             oprops =
               sym_drmModeObjectGetProperties(plane->fd,
                                              plane->drmPlane->plane_id,
                                              DRM_MODE_OBJECT_PLANE);
             if (oprops)
               {
                  unsigned int i = 0;

                  for (; i < oprops->count_props; i++)
                    {
                       drmModePropertyPtr prop;

                       prop = sym_drmModeGetProperty(plane->fd, oprops->props[i]);
                       if (!prop) continue;

                       if (!strcmp(prop->name, "rotation"))
                         disp->state.current->rotation = oprops->prop_values[i];

                       sym_drmModeFreeProperty(prop);
                    }
                  sym_drmModeFreeObjectProperties(oprops);
               }
          }
     }
}

static void
_ecore_drm2_display_state_fill(Ecore_Drm2_Display *disp)
{
   char *name = NULL;

   /* try to allocate space for current Display state */
   disp->state.current = calloc(1, sizeof(Ecore_Drm2_Display_State));
   if (!disp->state.current)
     {
        ERR("Could not allocate space for Display state");
        return;
     }

   /* get display name */
   name = _ecore_drm2_display_name_get(disp->conn);
   disp->name = eina_stringshare_add(name);
   free(name);

   disp->make = eina_stringshare_add("unknown");
   disp->model = eina_stringshare_add("unknown");
   disp->serial = eina_stringshare_add("unknown");

   /* get edid and parse */
   _ecore_drm2_display_edid_get(disp);

   /* get physical dimensions */
   disp->pw = disp->conn->drmConn->mmWidth;
   disp->ph = disp->conn->drmConn->mmHeight;

   /* get subpixel */
   switch (disp->conn->drmConn->subpixel)
     {
      case DRM_MODE_SUBPIXEL_NONE:
        disp->subpixel = 1;
        break;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
        disp->subpixel = 2;
        break;
      case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
        disp->subpixel = 3;
        break;
      case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
        disp->subpixel = 4;
        break;
      case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
        disp->subpixel = 5;
        break;
      case DRM_MODE_SUBPIXEL_UNKNOWN:
      default:
          disp->subpixel = 0;
        break;
     }

   /* get current rotation value */
   _ecore_drm2_display_rotation_get(disp);

   /* get backlight values */
   _ecore_drm2_display_backlight_get(disp);

   /* get available display modes */
   _ecore_drm2_display_modes_get(disp);

   /* get gamma from crtc */
   disp->state.current->gamma.size = disp->crtc->drmCrtc->gamma_size;

   /* get connected state */
   disp->connected = (disp->conn->drmConn->connection == DRM_MODE_CONNECTED);

   /* duplicate current state into pending so we can handle changes */
   disp->state.pending = calloc(1, sizeof(Ecore_Drm2_Display_State));
   if (disp->state.pending)
     memcpy(disp->state.pending, disp->state.current, sizeof(Ecore_Drm2_Display_State));

   /* send message to thread for debug printing display state */
   _ecore_drm2_display_state_thread_send(ECORE_DRM2_THREAD_CODE_DEBUG);
}

static void
_ecore_drm2_display_state_thread(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Display *disp;
   Thread_Msg *msg;
   void *ref;

   disp = data;

   eina_thread_name_set(eina_thread_self(), "Ecore-drm2-display");

   while (!ecore_thread_check(thread))
     {
        msg = eina_thread_queue_wait(thq, &ref);
        if (msg)
          {
             switch (msg->code)
               {
                case ECORE_DRM2_THREAD_CODE_FILL:
                  _ecore_drm2_display_state_fill(disp);
                  break;
                case ECORE_DRM2_THREAD_CODE_DEBUG:
                  _ecore_drm2_display_state_debug(disp);
                  break;
                default:
                  break;
               }
             eina_thread_queue_wait_done(thq, ref);
          }
     }
}

static void
_ecore_drm2_display_state_thread_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   free(msg);
}

static unsigned int
_ecore_drm2_display_vblank_pipe(Ecore_Drm2_Display *disp)
{
   if (!disp->crtc) return 0;

   if (disp->crtc->pipe > 1)
     {
        return (disp->crtc->pipe << DRM_VBLANK_HIGH_CRTC_SHIFT) &
          DRM_VBLANK_HIGH_CRTC_MASK;
     }
   else if (disp->crtc->pipe > 0)
     return DRM_VBLANK_SECONDARY;
   else
     return 0;
}

static void
_ecore_drm2_display_clock_read(Ecore_Drm2_Display *disp, struct timespec *tsnow)
{
   int ret;

   ret = clock_gettime(disp->dev->clock_id, tsnow);
   if (ret < 0)
     {
        tsnow->tv_sec = 0;
        tsnow->tv_nsec = 0;

        WRN("Failed to read display clock %#x: '%s' (%d)",
            disp->dev->clock_id, strerror(errno), errno);
     }
}

static void
_ecore_drm2_display_msc_update(Ecore_Drm2_Display *disp, unsigned int sequence)
{
   uint32_t mh;

   mh = disp->msc >> 32;
   if (sequence < (disp->msc & 0xffffffff))
     mh++;

   disp->msc = ((uint64_t)mh << 32) + sequence;
}

Eina_Bool
_ecore_drm2_displays_create(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Display *disp;
   Ecore_Drm2_Connector *c;
   Ecore_Drm2_Crtc *crtc;
   Eina_List *l = NULL, *ll = NULL;

   thq = eina_thread_queue_new();

   /* go through list of connectors and create displays */
   EINA_LIST_FOREACH(dev->conns, l, c)
     {
        drmModeEncoder *encoder;
        drmModeCrtc *dcrtc;

        /* try to get the encoder from drm */
        encoder = sym_drmModeGetEncoder(dev->fd, c->drmConn->encoder_id);
        if (!encoder) continue;

        /* try to get the crtc from drm */
        dcrtc = sym_drmModeGetCrtc(dev->fd, encoder->crtc_id);
        if (!dcrtc) goto cont;

        /* try to allocate space for new display */
        disp = calloc(1, sizeof(Ecore_Drm2_Display));
        if (!disp)
          {
             WRN("Could not allocate space for Display");
             sym_drmModeFreeCrtc(dcrtc);
             goto cont;
          }

        disp->dev = dev;

        /* try to find crtc matching dcrtc->crtc_id and assign to display */
        EINA_LIST_FOREACH(dev->crtcs, ll, crtc)
          {
             if (crtc->id == dcrtc->crtc_id)
               {
                  disp->crtc = crtc;
                  break;
               }
          }

        sym_drmModeFreeCrtc(dcrtc);

        /* disp->fd = dev->fd; */
        disp->conn = c;

        /* append this display to the list */
        dev->displays = eina_list_append(dev->displays, disp);

        disp->thread =
          ecore_thread_feedback_run(_ecore_drm2_display_state_thread,
                                    _ecore_drm2_display_state_thread_notify,
                                    NULL, NULL, disp, EINA_TRUE);

cont:
        sym_drmModeFreeEncoder(encoder);
     }

   return EINA_TRUE;
}

void
_ecore_drm2_displays_destroy(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Display *disp;

   EINA_LIST_FREE(dev->displays, disp)
     {
        if (disp->thread) ecore_thread_cancel(disp->thread);
        eina_stringshare_del(disp->backlight.path);
        eina_stringshare_del(disp->relative.to);
        eina_stringshare_del(disp->serial);
        eina_stringshare_del(disp->model);
        eina_stringshare_del(disp->make);
        eina_stringshare_del(disp->name);
        free(disp->state.pending);
        free(disp->state.current);
        free(disp);
     }

   if (thq)
     {
        eina_thread_queue_free(thq);
        thq = NULL;
     }
}

EAPI char *
ecore_drm2_display_name_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->name, NULL);
   return strdup(disp->name);
}

EAPI char *
ecore_drm2_display_model_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->model, NULL);
   return strdup(disp->model);
}

EAPI void
ecore_drm2_display_mode_set(Ecore_Drm2_Display *disp, Ecore_Drm2_Display_Mode *mode, int x, int y)
{
   Ecore_Drm2_Display_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);
   EINA_SAFETY_ON_NULL_RETURN(mode);
   EINA_SAFETY_ON_NULL_RETURN(disp->crtc);

   cstate = disp->state.current;
   pstate = disp->state.pending;

   if ((cstate->x != x) || (cstate->y != y))
     {
        pstate->x = x;
        pstate->y = y;
        pstate->changes |= ECORE_DRM2_DISPLAY_STATE_POSITION;
     }

   if (cstate->mode != mode)
     {
        pstate->mode = mode;
        pstate->changes |= ECORE_DRM2_DISPLAY_STATE_MODE;
     }

   /* FIXME: apply changes */
}

EAPI Eina_Bool
ecore_drm2_display_backlight_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   return (disp->backlight.path != NULL);
}

EAPI Eina_Bool
ecore_drm2_display_connected_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   return disp->connected;
}

EAPI unsigned int
ecore_drm2_display_connector_type_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn, 0);
   return disp->conn->type;
}

EAPI unsigned int
ecore_drm2_display_subpixel_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, 0);
   return disp->subpixel;
}

EAPI void
ecore_drm2_display_physical_size_get(Ecore_Drm2_Display *disp, int *w, int *h)
{
   if (w) *w = 0;
   if (h) *h = 0;

   EINA_SAFETY_ON_NULL_RETURN(disp);

   if (w) *w = disp->pw;
   if (h) *h = disp->ph;
}

EAPI int
ecore_drm2_display_dpms_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn, -1);
   return disp->conn->state.current->dpms.value;
}

EAPI void
ecore_drm2_display_dpms_set(Ecore_Drm2_Display *disp, uint64_t level)
{
   Ecore_Drm2_Connector_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);
   EINA_SAFETY_ON_NULL_RETURN(disp->conn);

   cstate = disp->conn->state.current;
   if (cstate->dpms.value == level) return;

   pstate = disp->conn->state.pending;
   pstate->dpms.value = level;
   pstate->changes |= ECORE_DRM2_CONNECTOR_STATE_DPMS;

   /* FIXME: apply changes */
}

EAPI Eina_Bool
ecore_drm2_display_enabled_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   return disp->state.current->enabled;
}

EAPI void
ecore_drm2_display_enabled_set(Ecore_Drm2_Display *disp, Eina_Bool enabled)
{
   Ecore_Drm2_Display_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);

   cstate = disp->state.current;
   pstate = disp->state.pending;

   if (cstate->enabled == enabled) return;

   pstate->enabled = enabled;
   pstate->changes |= ECORE_DRM2_DISPLAY_STATE_ENABLED;

   /* FIXME: apply changes */
}

EAPI char *
ecore_drm2_display_edid_get(Ecore_Drm2_Display *disp)
{
   char *edid_str = NULL;
   unsigned char *blob;
   unsigned char fblob[128];

   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn->state.current, NULL);

   blob = disp->conn->state.current->edid.data;
   if (!blob)
     {
        memset(fblob, 0, sizeof(fblob));
        blob = fblob;
     }

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

EAPI const Eina_List *
ecore_drm2_display_modes_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   return disp->modes;
}

EAPI void
ecore_drm2_display_mode_info_get(Ecore_Drm2_Display_Mode *mode, int *w, int *h, unsigned int *refresh, unsigned int *flags)
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
ecore_drm2_display_primary_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   return disp->state.current->primary;
}

EAPI void
ecore_drm2_display_primary_set(Ecore_Drm2_Display *disp, Eina_Bool primary)
{
   Ecore_Drm2_Display_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);

   cstate = disp->state.current;
   pstate = disp->state.pending;

   if (cstate->primary == primary) return;

   pstate->primary = primary;
   pstate->changes |= ECORE_DRM2_DISPLAY_STATE_PRIMARY;

   /* FIXME: apply changes */
}

EAPI const Eina_List *
ecore_drm2_displays_get(Ecore_Drm2_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);
   return dev->displays;
}

EAPI void
ecore_drm2_display_info_get(Ecore_Drm2_Display *disp, int *x, int *y, int *w, int *h, unsigned int *refresh)
{
   Ecore_Drm2_Display_State *cstate;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   if (refresh) *refresh = 0;

   EINA_SAFETY_ON_NULL_RETURN(disp);
   EINA_SAFETY_ON_TRUE_RETURN(!disp->state.current->mode);

   if (x) *x = disp->x;
   if (y) *y = disp->y;

   cstate = disp->state.current;

   switch (cstate->rotation)
     {
      case ECORE_DRM2_ROTATION_90:
      case ECORE_DRM2_ROTATION_270:
        if (w) *w = cstate->mode->height;
        if (h) *h = cstate->mode->width;
        break;
      case ECORE_DRM2_ROTATION_NORMAL:
      case ECORE_DRM2_ROTATION_180:
      default:
        if (w) *w = cstate->mode->width;
        if (h) *h = cstate->mode->height;
        break;
     }

   if (refresh) *refresh = cstate->mode->refresh;
}

EAPI int
ecore_drm2_display_rotation_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, -1);
   return disp->state.current->rotation;
}

EAPI void
ecore_drm2_display_rotation_set(Ecore_Drm2_Display *disp, uint64_t rotation)
{
   Ecore_Drm2_Display_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);

   cstate = disp->state.current;
   pstate = disp->state.pending;

   if (cstate->rotation == rotation) return;

   pstate->rotation = rotation;
   pstate->changes |= ECORE_DRM2_DISPLAY_STATE_ROTATION;

   /* FIXME: apply changes */
}

EAPI Ecore_Drm2_Crtc *
ecore_drm2_display_crtc_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   return disp->crtc;
}

EAPI Eina_Bool
ecore_drm2_display_possible_crtc_get(Ecore_Drm2_Display *disp, Ecore_Drm2_Crtc *crtc)
{
   drmModeConnector *conn;
   drmModeEncoder *enc;
   int i = 0;
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp->conn->drmConn, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(crtc, EINA_FALSE);

   conn = disp->conn->drmConn;
   for (; i < conn->count_encoders; i++)
     {
        enc = sym_drmModeGetEncoder(disp->conn->fd, conn->encoders[i]);
        if (!enc) continue;

        if (enc->crtc_id != crtc->id) goto next;

        if (enc->possible_crtcs & (1 << crtc->pipe))
          ret = EINA_TRUE;

next:
        sym_drmModeFreeEncoder(enc);
        if (ret) break;
     }

   return ret;
}

EAPI int
ecore_drm2_display_supported_rotations_get(Ecore_Drm2_Display *disp)
{
   Ecore_Drm2_Plane *plane;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, -1);

   /* get the primary plane for this output */
   plane = _ecore_drm2_planes_primary_find(disp->dev, disp->crtc->id);
   if (!plane) return -1;

   /* if plane state has not been filled yet, bail out */
   /* NB: We could modify this to get the plane rotations directly from drm */
   if (!plane->state.current) return -1;

   /* return primary plane state supported_rotations */
   return plane->state.current->supported_rotations;
}

EAPI void
ecore_drm2_display_relative_mode_set(Ecore_Drm2_Display *disp, Ecore_Drm2_Relative_Mode mode)
{
   EINA_SAFETY_ON_NULL_RETURN(disp);
   disp->relative.mode = mode;
}

EAPI void
ecore_drm2_display_relative_to_set(Ecore_Drm2_Display *disp, const char *relative)
{
   EINA_SAFETY_ON_NULL_RETURN(disp);
   eina_stringshare_replace(&disp->relative.to, relative);
}

EAPI void
ecore_drm2_display_dpi_get(Ecore_Drm2_Display *disp, int *xdpi, int *ydpi)
{
   EINA_SAFETY_ON_NULL_RETURN(disp);
   EINA_SAFETY_ON_TRUE_RETURN(!disp->state.current->enabled);

   if (xdpi)
     *xdpi = ((25.4 * (disp->state.current->mode->width)) / disp->pw);

   if (ydpi)
     *ydpi = ((25.4 * (disp->state.current->mode->height)) / disp->ph);
}

EAPI Ecore_Drm2_Display *
ecore_drm2_display_find(Ecore_Drm2_Device *dev, int x, int y)
{
   Eina_List *l;
   Ecore_Drm2_Display *disp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);

   EINA_LIST_FOREACH(dev->displays, l, disp)
     {
        int ox, oy, ow, oh;

        if (!disp->state.current->enabled) continue;

        ecore_drm2_display_info_get(disp, &ox, &oy, &ow, &oh, NULL);
        if (INSIDE(x, y, ox, oy, ow, oh))
          return disp;
     }

   return NULL;
}

EAPI void
ecore_drm2_display_user_data_set(Ecore_Drm2_Display *disp, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(disp);
   disp->user_data = data;
}

EAPI void *
ecore_drm2_display_user_data_get(Ecore_Drm2_Display *disp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, NULL);
   return disp->user_data;
}

EAPI Eina_Bool
ecore_drm2_display_blanktime_get(Ecore_Drm2_Display *disp, int seq, long *sec, long *usec)
{
   drmVBlank vbl;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sec, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(usec, EINA_FALSE);

   memset(&vbl, 0, sizeof(vbl));
   vbl.request.type = DRM_VBLANK_RELATIVE;
   vbl.request.type |= _ecore_drm2_display_vblank_pipe(disp);
   vbl.request.sequence = seq;
   vbl.request.signal = 0;

   /* try to get timestamp from drm vblank query */
   ret = sym_drmWaitVBlank(disp->dev->fd, &vbl);

   /* ret or zero timestamp is failure to get valid timestamp */
   if ((ret == 0) && (vbl.reply.tval_sec > 0 || vbl.reply.tval_usec > 0))
     {
        struct timespec ts, tsnow, vblnow;
        int64_t nsec, rnsec;

        ts.tv_sec = vbl.reply.tval_sec;
        ts.tv_nsec = vbl.reply.tval_usec * 1000;

        /* read clock */
        _ecore_drm2_display_clock_read(disp, &tsnow);

        vblnow.tv_sec = tsnow.tv_sec - ts.tv_sec;
        vblnow.tv_nsec = tsnow.tv_nsec - ts.tv_nsec;
        if (vblnow.tv_nsec < 0)
          {
             vblnow.tv_sec--;
             vblnow.tv_nsec += 1000000000;
          }

        rnsec = (1000000000000LL / disp->state.current->mode->refresh);
        nsec = (int64_t)vblnow.tv_sec * 1000000000 + vblnow.tv_nsec;
        if (nsec < rnsec)
          {
             /* update msc */
             _ecore_drm2_display_msc_update(disp, vbl.reply.sequence);
             return EINA_TRUE;
          }
        else
          {
             /* TODO: Do we need to provide a timestamp using pageflip fallback ? */
          }
     }

   *sec = vbl.reply.tval_sec;
   *usec = vbl.reply.tval_usec;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_drm2_display_changes_apply(Ecore_Drm2_Display *disp)
{
   Ecore_Drm2_Display_State *pstate;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disp, EINA_FALSE);

   pstate = disp->state.pending;

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_GAMMA)
     {
	uint16_t *r, *g, *b;

	r = pstate->gamma.r;
	g = pstate->gamma.g;
	b = pstate->gamma.b;

	if (sym_drmModeCrtcSetGamma(disp->crtc->fd, disp->crtc->id,
				    pstate->gamma.size, r, g, b) < 0)
	  ERR("Failed to set gamma for Display %s: %m", disp->name);
	else
	  pstate->changes &= ~ECORE_DRM2_DISPLAY_STATE_GAMMA;
     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_ROTATION)
     {

     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_BACKLIGHT)
     {

     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_MODE)
     {

     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_PRIMARY)
     {

     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_ENABLED)
     {

     }

   if (pstate->changes & ECORE_DRM2_DISPLAY_STATE_POSITION)
     {

     }

   /* TODO: copy pending state to current when applying changes is successful */

   return EINA_TRUE;
}

EAPI void
ecore_drm2_display_gamma_set(Ecore_Drm2_Display *disp, uint16_t size, uint16_t *red, uint16_t *green, uint16_t *blue)
{
   Ecore_Drm2_Display_State *cstate, *pstate;

   EINA_SAFETY_ON_NULL_RETURN(disp);

   cstate = disp->state.current;
   pstate = disp->state.pending;

   if (cstate->gamma.size == size) return;
   if ((cstate->gamma.r == red) &&
       (cstate->gamma.g == green) &&
       (cstate->gamma.b == blue))
     return;

   pstate->gamma.size = size;
   pstate->gamma.r = red;
   pstate->gamma.g = green;
   pstate->gamma.b = blue;
   pstate->changes |= ECORE_DRM2_DISPLAY_STATE_GAMMA;
}
