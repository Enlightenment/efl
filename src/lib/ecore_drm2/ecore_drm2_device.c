#include "ecore_drm2_private.h"

#ifndef DRM_CLIENT_CAP_ASPECT_RATIO
# define DRM_CLIENT_CAP_ASPECT_RATIO 4
#endif
#ifndef DRM_CLIENT_CAP_WRITEBACK_CONNECTORS
# define DRM_CLIENT_CAP_WRITEBACK_CONNECTORS 5
#endif

/* local functions */
static Eina_Bool
_ecore_drm2_device_cb_session_active(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   /* Ecore_Drm2_Device *dev; */
   Elput_Event_Session_Active *ev;

   /* dev = data; */
   ev = event;

   if (ev->active)
     {
        /* TODO: wake compositor, compositor damage all, set state_invalid = true */
        /* NB: Input enable is already done inside elput */
     }
   else
     {
        /* TODO: compositor offscreen, output->repaint_needed = false */
        /* NB: Input disable is already done inside elput */
     }

   /* TODO: raise ecore_drm2_event_active ?? */

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_drm2_device_modeset_capable_get(int fd)
{
   Eina_Bool ret = EINA_TRUE;
   drmModeRes *res;

   res = sym_drmModeGetResources(fd);
   if (!res) return EINA_FALSE;

   if ((res->count_crtcs <= 0) || (res->count_connectors <= 0) ||
       (res->count_encoders <= 0))
     ret = EINA_FALSE;

   sym_drmModeFreeResources(res);

   return ret;
}

static const char *
_ecore_drm2_device_path_get(Elput_Manager *em, const char *seat)
{
   Eina_List *devs, *l;
   const char *denv = NULL, *dev = NULL, *chosen = NULL, *ret = NULL;
   Eina_Bool found = EINA_FALSE, ms = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(em, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);

   denv = getenv("ECORE_DRM2_CARD");
   if (denv)
     devs = eeze_udev_find_by_subsystem_sysname("drm", denv);
   else
     devs = eeze_udev_find_by_subsystem_sysname("drm", "card[0-9]*");

   if (!devs) return NULL;

   EINA_LIST_FOREACH(devs, l, dev)
     {
        int fd = -1;
        const char *dpath, *dseat, *dparent;

        dpath = eeze_udev_syspath_get_devpath(dev);
        if (!dpath) continue;

        dseat = eeze_udev_syspath_get_property(dev, "ID_SEAT");
        if (!dseat) dseat = eina_stringshare_add("seat0");

        if (strcmp(seat, dseat)) goto cont;

        fd = elput_manager_open(em, dpath, -1);
        if (fd < 0) goto cont;

        ms = _ecore_drm2_device_modeset_capable_get(fd);
        elput_manager_close(em, fd);
        if (!ms) goto cont;

        chosen = dev;

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

   if (chosen)
     ret = eeze_udev_syspath_get_devpath(chosen);

   EINA_LIST_FREE(devs, dev)
     eina_stringshare_del(dev);

   return ret;
}

static Eina_Bool
_ecore_drm2_device_kms_caps_get(Ecore_Drm2_Device *dev)
{
   uint64_t cap;
   int ret = 0;

   /* get drm presentation clock */
   ret = sym_drmGetCap(dev->fd, DRM_CAP_TIMESTAMP_MONOTONIC, &cap);
   if ((ret == 0) && (cap == 1))
     dev->clock_id = CLOCK_MONOTONIC;
   else
     dev->clock_id = CLOCK_REALTIME;

   /* get drm cursor width & height */
   dev->cursor.width = 64;
   dev->cursor.height = 64;
   ret = sym_drmGetCap(dev->fd, DRM_CAP_CURSOR_WIDTH, &cap);
   if (ret == 0) dev->cursor.width = cap;
   ret = sym_drmGetCap(dev->fd, DRM_CAP_CURSOR_HEIGHT, &cap);
   if (ret == 0) dev->cursor.height = cap;

   /* try to enable universal planes ... without This, not even Atomic works */
   ret = sym_drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
   if (ret)
     {
        ERR("Drm card does not support universal planes");
        return EINA_FALSE;
     }

   /* test if crtc_in_vblank_event is supported */
   /* NB: our callbacks do not check for this yet, but it's new.
    * Very useful tho. tells us when crtc is vblank */
   /* NB: This is NOT necessarily needed for ATOMIC support */
   ret = sym_drmGetCap(dev->fd, DRM_CAP_CRTC_IN_VBLANK_EVENT, &cap);
   if (ret != 0) cap = 0;

   /* try to enable atomic modesetting support */
   ret = sym_drmSetClientCap(dev->fd, DRM_CLIENT_CAP_ATOMIC, 1);
   dev->atomic = ((ret == 0) && (cap == 1));

   /* test if gbm can do modifiers */
   /* ret = sym_drmGetCap(dev->fd, DRM_CAP_ADDFB2_MODIFIERS, &cap); */
   /* if (ret == 0) dev->gbm_mods = cap; */

   /* set writeback connector support */
   sym_drmSetClientCap(dev->fd, DRM_CLIENT_CAP_WRITEBACK_CONNECTORS, 1);

   /* test to see if aspect ratio is supported */
   ret = sym_drmSetClientCap(dev->fd, DRM_CLIENT_CAP_ASPECT_RATIO, 1);
   dev->aspect_ratio = (ret == 0);

   return EINA_TRUE;
}

/* API functions */
EAPI Ecore_Drm2_Device *
ecore_drm2_device_open(const char *seat, unsigned int tty)
{
   const char *path;
   Ecore_Drm2_Device *dev;

   /* try to allocate space for device structure */
   dev = calloc(1, sizeof(Ecore_Drm2_Device));
   if (!dev) return NULL;

   /* try to connect to Elput manager */
   dev->em = elput_manager_connect(seat, tty);
   if (!dev->em)
     {
        ERR("Could not connect to input manager");
        goto man_err;
     }

   /* try to get drm device path */
   path = _ecore_drm2_device_path_get(dev->em, seat);
   if (!path)
     {
        ERR("Could not find drm device on seat %s", seat);
        goto path_err;
     }

   /* try to open this device */
   dev->fd = elput_manager_open(dev->em, path, -1);
   if (dev->fd < 0)
     {
        ERR("Could not open drm device %s", path);
        goto open_err;
     }

   DBG("Opened DRM Device: %s", path);

   /* try to enable elput input */
   if (!elput_input_init(dev->em))
     {
        ERR("Could not initialize Elput Input");
        goto input_err;
     }

   /* try to get the kms capabilities of this device */
   if (!_ecore_drm2_device_kms_caps_get(dev))
     {
        ERR("Could not get kms capabilities for device");
        goto caps_err;
     }

   /* try to create crtcs */
   if (!_ecore_drm2_crtcs_create(dev))
     {
        ERR("Could not create crtcs");
        goto caps_err;
     }

   /* try to create planes */
   if (!_ecore_drm2_planes_create(dev))
     {
        ERR("Could not create planes");
        goto plane_err;
     }

   /* try to create connectors */
   if (!_ecore_drm2_connectors_create(dev))
     {
        ERR("Could not create connectors");
        goto conn_err;
     }

   /* try to create displays */
   if (!_ecore_drm2_displays_create(dev))
     {
        ERR("Could not create displays");
        goto disp_err;
     }

   /* TODO: check dmabuf import capable ?
    *
    * NB: This will require EGL extension: EGL_EXT_image_dma_buf_import
    * so will likely need to be done in the compositor
    */

   /* TODO: check explicit sync support
    *
    * NB: requires native_fence_sync & wait_sync
    * NB: native_fence_sync requires EGL_KHR_fence_sync
    * NB: wait_sync requires EGL_KHR_wait_sync
    *
    * NB: These need to be done in the compositor
    */

   /* TODO: enable content protection if atomic ?
    *
    * NB: This should be done in the compositor
    */

   dev->session_hdlr =
     ecore_event_handler_add(ELPUT_EVENT_SESSION_ACTIVE,
                             _ecore_drm2_device_cb_session_active, dev);

   /* TODO: event handler for device_change */

   /* cleanup path variable */
   eina_stringshare_del(path);

   return dev;

disp_err:
   _ecore_drm2_connectors_destroy(dev);
conn_err:
   _ecore_drm2_planes_destroy(dev);
plane_err:
   _ecore_drm2_crtcs_destroy(dev);
caps_err:
   elput_input_shutdown(dev->em);
input_err:
   elput_manager_close(dev->em, dev->fd);
open_err:
   eina_stringshare_del(path);
path_err:
   elput_manager_disconnect(dev->em);
man_err:
   free(dev);
   return NULL;
}

EAPI void
ecore_drm2_device_close(Ecore_Drm2_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN(dev);

   _ecore_drm2_displays_destroy(dev);
   _ecore_drm2_connectors_destroy(dev);
   _ecore_drm2_planes_destroy(dev);
   _ecore_drm2_crtcs_destroy(dev);

   ecore_event_handler_del(dev->session_hdlr);

   elput_input_shutdown(dev->em);
   elput_manager_close(dev->em, dev->fd);
   elput_manager_disconnect(dev->em);

   free(dev);
}

EAPI void
ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *dev, int *width, int *height)
{
   EINA_SAFETY_ON_NULL_RETURN(dev);

   if (width) *width = dev->cursor.width;
   if (height) *height = dev->cursor.height;
}

EAPI void
ecore_drm2_device_preferred_depth_get(Ecore_Drm2_Device *dev, int *depth, int *bpp)
{
   uint64_t caps;
   int ret = -1;

   EINA_SAFETY_ON_NULL_RETURN(dev);

   ret = sym_drmGetCap(dev->fd, DRM_CAP_DUMB_PREFERRED_DEPTH, &caps);
   if (ret == 0)
     {
        if (depth) *depth = caps;
        if (bpp) *bpp = caps;
     }
}

EAPI void
ecore_drm2_device_screen_size_range_get(Ecore_Drm2_Device *dev, int *minw, int *minh, int *maxw, int *maxh)
{
   drmModeRes *res;

   if (minw) *minw = 0;
   if (minh) *minh = 0;
   if (maxw) *maxw = 0;
   if (maxh) *maxh = 0;

   EINA_SAFETY_ON_NULL_RETURN(dev);

   res = sym_drmModeGetResources(dev->fd);
   if (!res) return;

   if (minw) *minw = res->min_width;
   if (minh) *minh = res->min_height;
   if (maxw) *maxw = res->max_width;
   if (maxh) *maxh = res->max_height;

   sym_drmModeFreeResources(res);
}

EAPI const Eina_List *
ecore_drm2_device_crtcs_get(Ecore_Drm2_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);
   return dev->crtcs;
}
