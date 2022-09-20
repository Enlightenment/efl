#include "ecore_drm2_private.h"

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

static char *
_ecore_drm2_display_name_get(Ecore_Drm2_Connector *conn)
{
   char name[DRM_CONNECTOR_NAME_LEN];
   const char *type = NULL;

   if (conn->type < EINA_C_ARRAY_LENGTH(conn_types))
     type = conn_types[conn->type];
   else
     type = "UNKNOWN";

   snprintf(name, sizeof(name), "%s-%d", type, conn->conn->connector_type_id);
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

   cstate = disp->conn->state;

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
   DBG("Display Atomic State Fill Complete");

   DBG("\tName: %s", disp->name);
   DBG("\tMake: %s", disp->make);
   DBG("\tModel: %s", disp->model);
   DBG("\tSerial: %s", disp->serial);

   DBG("\tCrtc: %d", disp->crtc->id);
   DBG("\tCrtc Pos: %d %d", disp->crtc->dcrtc->x, disp->crtc->dcrtc->y);
   DBG("\tConnector: %d", disp->conn->id);

   /* DBG("\tCloned: %d", disp->cloned); */
   DBG("\tPrimary: %d", disp->primary);
   DBG("\tEnabled: %d", disp->enabled);
   DBG("\tConnected: %d", disp->connected);

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
}

static void
_ecore_drm2_display_state_fill(Ecore_Drm2_Display *disp)
{
   char *name = NULL;

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
   disp->pw = disp->conn->conn->mmWidth;
   disp->ph = disp->conn->conn->mmHeight;

   /* get subpixel */
   switch (disp->conn->conn->subpixel)
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

   /* get connected state */
   disp->connected = (disp->conn->conn->connection == DRM_MODE_CONNECTED);
}

static void
_ecore_drm2_display_state_thread(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Display *disp;

   disp = data;
   /* TODO: FIXME: Should this check for disp->state ? */
   if (!disp->name)
     _ecore_drm2_display_state_fill(disp);
   else
     {
        /* TODO: update atomic state for commit */
     }
}

static void
_ecore_drm2_display_state_thread_end(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Display *disp;

   disp = data;
   /* disp->thread = NULL; */
   _ecore_drm2_display_state_debug(disp);
}

static void
_ecore_drm2_display_state_thread_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Display *disp;

   disp = data;
   disp->thread = NULL;
}

Eina_Bool
_ecore_drm2_displays_create(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Display *disp;
   Ecore_Drm2_Connector *c;
   Ecore_Drm2_Crtc *crtc;
   Eina_List *l = NULL, *ll = NULL;

   /* go through list of connectors and create displays */
   EINA_LIST_FOREACH(dev->conns, l, c)
     {
        drmModeEncoder *encoder;
        drmModeCrtc *dcrtc;

        /* try to get the encoder from drm */
        encoder = sym_drmModeGetEncoder(dev->fd, c->conn->encoder_id);
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

        disp->fd = dev->fd;
        disp->conn = c;

        /* append this display to the list */
        dev->displays = eina_list_append(dev->displays, disp);

        disp->thread =
          ecore_thread_feedback_run(_ecore_drm2_display_state_thread,
                                    NULL, // _ecore_drm2_display_state_thread_notify
                                    _ecore_drm2_display_state_thread_end,
                                    _ecore_drm2_display_state_thread_cancel,
                                    disp, EINA_TRUE);

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
        eina_stringshare_del(disp->serial);
        eina_stringshare_del(disp->model);
        eina_stringshare_del(disp->make);
        eina_stringshare_del(disp->name);
        free(disp);
     }
}
