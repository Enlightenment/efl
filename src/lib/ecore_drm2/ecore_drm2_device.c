#include "ecore_drm2_private.h"

/* external variable for using atomic */
Eina_Bool _ecore_drm2_atomic_use = EINA_FALSE;

/* local functions */
static Eina_Bool
_ecore_drm2_device_atomic_capable_get(int fd)
{
   Eina_Bool ret = EINA_TRUE;

   if (sym_drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) < 0)
     ret = EINA_FALSE;
   else
     {
        if (sym_drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1) < 0)
          ret = EINA_FALSE;
     }

   return ret;
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

   /* test if this device can do Atomic Modesetting */
   _ecore_drm2_atomic_use = _ecore_drm2_device_atomic_capable_get(dev->fd);
   if (!_ecore_drm2_atomic_use)
     {
        WRN("Could not enable Atomic Modesetting support");
        goto atomic_err;
     }

   /* try to allocate space for Atomic State */
   dev->atomic_state = calloc(1, sizeof(Ecore_Drm2_Atomic_State));
   if (!dev->atomic_state)
     {
        ERR("Failed to allocate device atomic state");
        goto atomic_err;
     }

   /* try to fill atomic state */
   if (!_ecore_drm2_atomic_state_fill(dev->atomic_state, dev->fd))
     {
        ERR("Failed to fill Atomic State");
        goto atomic_fill_err;
     }

   /* TODO: event handlers for session_active & device_change */

   /* cleanup path variable */
   eina_stringshare_del(path);

   return dev;

atomic_fill_err:
   free(dev->atomic_state);
atomic_err:
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

   _ecore_drm2_atomic_state_free(dev->atomic_state);

   elput_input_shutdown(dev->em);
   elput_manager_close(dev->em, dev->fd);
   elput_manager_disconnect(dev->em);

   free(dev);
}

EAPI void
ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *dev, int *width, int *height)
{
   uint64_t caps;
   int ret = -1;

   EINA_SAFETY_ON_NULL_RETURN(dev);

   if (width)
     {
        *width = 64;
        ret = sym_drmGetCap(dev->fd, DRM_CAP_CURSOR_WIDTH, &caps);
        if (ret == 0) *width = caps;
     }

   if (height)
     {
        *height = 64;
        ret = sym_drmGetCap(dev->fd, DRM_CAP_CURSOR_HEIGHT, &caps);
        if (ret == 0) *height = caps;
     }
}
