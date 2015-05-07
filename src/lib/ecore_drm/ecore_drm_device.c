#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <dlfcn.h>

#define INSIDE(x, y, xx, yy, ww, hh) \
   (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && \
       ((x) >= (xx)) && ((y) >= (yy)))

static Eina_List *drm_devices;
static int flip_count = 0;

static void 
_ecore_drm_device_cb_page_flip(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   Ecore_Drm_Pageflip_Callback *cb;

   /* DBG("Drm Page Flip Event"); */

   if (!(cb = data)) return;

   flip_count++;
   if (flip_count < cb->count) return;

   flip_count = 0;
   if (cb->func) cb->func(cb->data);
   /* free(cb); */

   /* Ecore_Drm_Output *output; */

   /* DBG("Drm Page Flip Event"); */

   /* if (!(output = data)) return; */

   /* if (output->pending_flip) */
   /*   { */
   /*      if (output->dev->current) */
   /*        ecore_drm_output_fb_release(output, output->dev->current); */
   /*      output->dev->current = output->dev->next; */
   /*      output->dev->next = NULL; */
   /*   } */

   /* output->pending_flip = EINA_FALSE; */
   /* if (output->pending_destroy) */
   /*   { */
   /*      output->pending_destroy = EINA_FALSE; */
   /*      ecore_drm_output_free(output); */
   /*   } */
   /* else if (!output->pending_vblank) */
   /*   ecore_drm_output_repaint(output); */
}

static void 
_ecore_drm_device_cb_vblank(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   Ecore_Drm_Sprite *sprite;
   Ecore_Drm_Output *output;

   /* DBG("Drm VBlank Event"); */

   if (!(sprite = data)) return;

   output = sprite->output;
   output->pending_vblank = EINA_FALSE;

   ecore_drm_output_fb_release(output, sprite->current_fb);
   sprite->current_fb = sprite->next_fb;
   sprite->next_fb = NULL;

   if (!output->pending_flip) _ecore_drm_output_frame_finish(output);
}

static Eina_Bool 
_ecore_drm_device_cb_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Drm_Device *dev;

   if (!(dev = data)) return ECORE_CALLBACK_RENEW;

   /* DBG("Drm Device Event"); */

   drmHandleEvent(dev->drm.fd, &dev->drm_ctx);

   return ECORE_CALLBACK_RENEW;
}

#if 0
static Eina_Bool 
_ecore_drm_device_cb_idle(void *data)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Output *output;
   Eina_List *l;

   if (!(dev = data)) return ECORE_CALLBACK_CANCEL;

   if (!dev->active) return ECORE_CALLBACK_RENEW;

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        if ((!output->enabled) || (!output->need_repaint)) continue;
        if (output->repaint_scheduled) continue;
        _ecore_drm_output_repaint_start(output);
     }

   return ECORE_CALLBACK_RENEW;
}
#endif

static void
_ecore_drm_device_cb_output_event(const char *device EINA_UNUSED, Eeze_Udev_Event event EINA_UNUSED, void *data, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Ecore_Drm_Device *dev;

   if (!(dev = data)) return;
   _ecore_drm_outputs_update(dev);
}

/**
 * @defgroup Ecore_Drm_Device_Group Device manipulation functions
 * 
 * Functions that deal with finding, opening, closing, and otherwise using 
 * the DRM device itself.
 */

EAPI Ecore_Drm_Device *
ecore_drm_device_find(const char *name, const char *seat)
{
   Ecore_Drm_Device *dev = NULL;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool platform = EINA_FALSE;
   Eina_List *devs, *l;
   const char *device;

   /* try to get a list of drm devics */
   if (!(devs = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRM, name)))
     return NULL;

   DBG("Find Drm Device: %s", name);

   EINA_LIST_FOREACH(devs, l, device)
     {
        const char *devpath;
        const char *devseat;
        const char *devparent;

        if (!(devpath = eeze_udev_syspath_get_devpath(device)))
          continue;

        DBG("Found Drm Device");
        DBG("\tDevice: %s", device);
        DBG("\tDevpath: %s", devpath);

        if ((name) && (strcmp(name, devpath))) goto cont;

        if (!(devseat = eeze_udev_syspath_get_property(device, "ID_SEAT")))
          devseat = eina_stringshare_add("seat0");

        if ((seat) && (strcmp(seat, devseat))) 
          goto cont;
        else if (strcmp(devseat, "seat0"))
          goto cont;

        devparent = eeze_udev_syspath_get_parent_filtered(device, "pci", NULL);
        if (!devparent)
          {
             devparent = 
               eeze_udev_syspath_get_parent_filtered(device, "platform", NULL);
             platform = EINA_TRUE;
          }

        if (devparent)
          {
             if (!platform)
               {
                  const char *id;

                  if ((id = eeze_udev_syspath_get_sysattr(devparent, "boot_vga")))
                    {
                       if (!strcmp(id, "1")) found = EINA_TRUE;
                       eina_stringshare_del(id);
                    }
               }
             else
               found = EINA_TRUE;

             eina_stringshare_del(devparent);
          }

cont:
        eina_stringshare_del(devpath);
        if (found) break;
     }

   if (!found) goto out;

   if ((dev = calloc(1, sizeof(Ecore_Drm_Device))))
     {
        dev->drm.name = eeze_udev_syspath_get_devpath(device);
        dev->drm.path = eina_stringshare_add(device);

        dev->id = eeze_udev_syspath_get_sysnum(device);

        dev->seat = eeze_udev_syspath_get_property(device, "ID_SEAT");
        if (!dev->seat) dev->seat = eina_stringshare_add("seat0");

        dev->vt = 0;
        dev->format = 0;
        dev->use_hw_accel = EINA_FALSE;
        dev->session = NULL;

        DBG("Using Drm Device: %s", dev->drm.name);

        drm_devices = eina_list_append(drm_devices, dev);
     }

out:
   EINA_LIST_FREE(devs, device)
     eina_stringshare_del(device);

   return dev;
}

EAPI void 
ecore_drm_device_free(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Output *output;
   unsigned int i = 0;

   /* check for valid device */
   if (!dev) return;

   for (; i < ALEN(dev->dumb); i++)
     {
        if (dev->dumb[i]) ecore_drm_fb_destroy(dev->dumb[i]);
        dev->dumb[i] = NULL;
     }

   ecore_drm_inputs_destroy(dev);

   /* free outputs */
   EINA_LIST_FREE(dev->outputs, output)
     ecore_drm_output_free(output);

   /* free crtcs */
   if (dev->crtcs) free(dev->crtcs);

   /* free device name */
   if (dev->drm.name) eina_stringshare_del(dev->drm.name);

   /* free device path */
   if (dev->drm.path) eina_stringshare_del(dev->drm.path);

   /* free device seat */
   if (dev->seat) eina_stringshare_del(dev->seat);

   /* free session */
   free(dev->session);

   drm_devices = eina_list_remove(drm_devices, dev);

   /* free structure */
   free(dev);
}

EAPI Eina_Bool 
ecore_drm_device_open(Ecore_Drm_Device *dev)
{
   uint64_t caps;
   int events = 0;

   /* check for valid device */
   if ((!dev) || (!dev->drm.name)) return EINA_FALSE;

   /* DRM device node is needed immediately to keep going. */
   dev->drm.fd = 
     _ecore_drm_launcher_device_open_no_pending(dev->drm.name, O_RDWR);
   if (dev->drm.fd < 0) return EINA_FALSE;

   DBG("Opened Device %s : %d", dev->drm.name, dev->drm.fd);

   /* set client capabilities to 'universal planes' so drm core will expose
    * the full universal plane list (including primary & cursor planes) */
   drmSetClientCap(dev->drm.fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

   if (!drmGetCap(dev->drm.fd, DRM_CAP_TIMESTAMP_MONOTONIC, &caps))
     {
        if (caps == 1)
          dev->drm.clock = CLOCK_MONOTONIC;
        else
          dev->drm.clock = CLOCK_REALTIME;
     }
   else
     {
        ERR("Could not get TIMESTAMP_MONOTONIC device capabilities: %m");
     }

   /* Without DUMB_BUFFER we can't do software rendering on DRM. Fail without it
    * until we have rock solid hardware accelerated DRM on all drivers */
   if (drmGetCap(dev->drm.fd, DRM_CAP_DUMB_BUFFER, &caps) < 0 || !caps)
     {
        ERR("Could not get DUMB_BUFFER device capabilities: %m");
        return EINA_FALSE;
     }

   /* try to create xkb context */
   if (!(dev->xkb_ctx = xkb_context_new(0)))
     {
        ERR("Failed to create xkb context: %m");
        return EINA_FALSE;
     }

   memset(&dev->drm_ctx, 0, sizeof(dev->drm_ctx));
   dev->drm_ctx.version = DRM_EVENT_CONTEXT_VERSION;
   dev->drm_ctx.page_flip_handler = _ecore_drm_device_cb_page_flip;
   dev->drm_ctx.vblank_handler = _ecore_drm_device_cb_vblank;

   events = (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE |
             EEZE_UDEV_EVENT_CHANGE);

   dev->watch =
     eeze_udev_watch_add(EEZE_UDEV_TYPE_DRM, events,
                         _ecore_drm_device_cb_output_event, dev);

   dev->drm.hdlr = 
     ecore_main_fd_handler_add(dev->drm.fd, ECORE_FD_READ, 
                               _ecore_drm_device_cb_event, dev, NULL, NULL);

   /* dev->drm.idler =  */
   /*   ecore_idle_enterer_add(_ecore_drm_device_cb_idle, dev); */

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_drm_device_close(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);

   /* delete udev watch */
   if (dev->watch) eeze_udev_watch_del(dev->watch);

   /* close xkb context */
   if (dev->xkb_ctx) xkb_context_unref(dev->xkb_ctx);

   if (dev->drm.hdlr) ecore_main_fd_handler_del(dev->drm.hdlr);
   dev->drm.hdlr = NULL;

   _ecore_drm_launcher_device_close(dev->drm.name, dev->drm.fd);

   /* reset device fd */
   dev->drm.fd = -1;

   return EINA_TRUE;
}

EAPI const Eina_List *
ecore_drm_devices_get(void)
{
   return drm_devices;
}

EAPI Eina_Bool 
ecore_drm_device_master_get(Ecore_Drm_Device *dev)
{
   drm_magic_t mag;

   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   /* get if we are master or not */
   if ((drmGetMagic(dev->drm.fd, &mag) == 0) && 
       (drmAuthMagic(dev->drm.fd, mag) == 0))
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_drm_device_master_set(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   DBG("Set Master On Fd: %d", dev->drm.fd);

   drmSetMaster(dev->drm.fd);

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_drm_device_master_drop(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   DBG("Drop Master On Fd: %d", dev->drm.fd);

   drmDropMaster(dev->drm.fd);

   return EINA_TRUE;
}

EAPI int 
ecore_drm_device_fd_get(Ecore_Drm_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, -1);
   return dev->drm.fd;
}

EAPI void 
ecore_drm_device_window_set(Ecore_Drm_Device *dev, unsigned int window)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN((!dev) || (dev->drm.fd < 0));

   dev->window = window;
}

EAPI const char *
ecore_drm_device_name_get(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((!dev) || (dev->drm.fd < 0), NULL);

   return dev->drm.name;
}

EAPI void
ecore_drm_device_pointer_xy_get(Ecore_Drm_Device *dev, int *x, int *y)
{
   Ecore_Drm_Seat *seat;
   Ecore_Drm_Evdev *edev;
   Eina_List *l, *ll;

   if (x) *x = 0;
   if (y) *y = 0;

   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN((!dev) || (dev->drm.fd < 0));

   EINA_LIST_FOREACH(dev->seats, l, seat)
     {
        EINA_LIST_FOREACH(seat->devices, ll, edev)
          {
             if (!libinput_device_has_capability(edev->device, 
                                                 LIBINPUT_DEVICE_CAP_POINTER))
               continue;

             if (x) *x = edev->mouse.dx;
             if (y) *y = edev->mouse.dy;

             return;
          }
     }
}

EAPI Eina_Bool
ecore_drm_device_software_setup(Ecore_Drm_Device *dev)
{
   unsigned int i = 0;
   int w = 0, h = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);

   /* destroy any old buffers */
   for (; i < ALEN(dev->dumb); i++)
     {
        if (dev->dumb[i]) ecore_drm_fb_destroy(dev->dumb[i]);
        dev->dumb[i] = NULL;
     }

   /* get screen size */
   ecore_drm_outputs_geometry_get(dev, NULL, NULL, &w, &h);

   /* create new buffers */
   for (i = 0; i < ALEN(dev->dumb); i++)
     {
        if (!(dev->dumb[i] = ecore_drm_fb_create(dev, w, h)))
          {
             ERR("Could not create dumb framebuffer: %m");
             goto err;
          }

        DBG("Ecore_Drm_Device Created Dumb Buffer");
        DBG("\tFb: %d", dev->dumb[i]->id);
        DBG("\tHandle: %d", dev->dumb[i]->hdl);
        DBG("\tStride: %d", dev->dumb[i]->stride);
        DBG("\tSize: %d", dev->dumb[i]->size);
        DBG("\tW: %d\tH: %d", dev->dumb[i]->w, dev->dumb[i]->h);
     }

   return EINA_TRUE;

err:
   for (i = 0; i < ALEN(dev->dumb); i++)
     {
        if (dev->dumb[i]) ecore_drm_fb_destroy(dev->dumb[i]);
        dev->dumb[i] = NULL;
     }
   return EINA_FALSE;
}

EAPI Ecore_Drm_Output *
ecore_drm_device_output_find(Ecore_Drm_Device *dev, int x, int y)
{
   Ecore_Drm_Output *output;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((x < 0) || (y < 0), NULL);

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        int ox = 0, oy = 0;
        int ow = 0, oh = 0;

        if (!output->cloned)
          {
             ox = output->x;
             oy = output->y;
          }

        ow = output->current_mode->width;
        oh = output->current_mode->height;

        if (INSIDE(x, y, ox, oy, ow, oh))
          return output;
     }

   return NULL;
}

EAPI void
ecore_drm_screen_size_range_get(Ecore_Drm_Device *dev, int *minw, int *minh, int *maxw, int *maxh)
{
   EINA_SAFETY_ON_NULL_RETURN(dev);

   if (minw) *minw = dev->min_width;
   if (minh) *minh = dev->min_height;
   if (maxw) *maxw = dev->max_width;
   if (maxh) *maxh = dev->max_height;
}

EAPI Ecore_Drm_Output *
ecore_drm_device_output_name_find(Ecore_Drm_Device *dev, const char *name)
{
   Ecore_Drm_Output *output;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_LIST_FOREACH(dev->outputs, l, output)
     if ((output->name) && (!strcmp(name, output->name)))
       return output;

   return NULL;
}
