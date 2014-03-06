#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <dlfcn.h>

/* #ifdef HAVE_GBM */
/* static Eina_Bool  */
/* _ecore_drm_device_egl_config_get(Ecore_Drm_Device *dev, const EGLint *attribs, const EGLint *visual) */
/* { */
/*    EGLint c = 0, m = 0; */
/*    EGLConfig *cfgs; */
/*    int i = 0; */

/*    if (!eglGetConfigs(dev->egl.disp, NULL, 0, &c) || (c < 1)) */
/*      return EINA_FALSE; */

/*    if (!(cfgs = calloc(c, sizeof(*cfgs)))) return EINA_FALSE; */

/*    if (!eglChooseConfig(dev->egl.disp, attribs, cfgs, c, &m)) */
/*      { */
/*         free(cfgs); */
/*         return EINA_FALSE; */
/*      } */

/*    for (i = 0; i < m; i++) */
/*      { */
/*         EGLint id; */

/*         if (visual) */
/*           { */
/*              if (!eglGetConfigAttrib(dev->egl.disp, cfgs[i],  */
/*                                      EGL_NATIVE_VISUAL_ID, &id)) */
/*                continue; */

/*              if ((id != 0) && (id != *visual)) */
/*                continue; */
/*           } */

/*         dev->egl.cfg = cfgs[i]; */
/*         free(cfgs); */
/*         return EINA_TRUE; */
/*      } */

/*    free(cfgs); */
/*    return EINA_FALSE; */
/* } */
/* #endif */

static void 
_ecore_drm_device_cb_page_flip(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   Ecore_Drm_Output *output;

   DBG("Drm Page Flip Event");

   if (!(output = data)) return;

   if (output->pending_flip)
     {
        ecore_drm_output_fb_release(output, output->current);
        output->current = output->next;
        output->next = NULL;
     }

   output->pending_flip = EINA_FALSE;
   if (!output->pending_vblank) ecore_drm_output_repaint(output);
}

static void 
_ecore_drm_device_cb_vblank(int fd EINA_UNUSED, unsigned int frame EINA_UNUSED, unsigned int sec EINA_UNUSED, unsigned int usec EINA_UNUSED, void *data)
{
   Ecore_Drm_Sprite *sprite;
   Ecore_Drm_Output *output;

   DBG("Drm VBlank Event");

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
   drmEventContext ctx;

   if (!(dev = data)) return ECORE_CALLBACK_RENEW;

   DBG("Drm Device Event");

   memset(&ctx, 0, sizeof(ctx));

   ctx.version = DRM_EVENT_CONTEXT_VERSION;
   ctx.page_flip_handler = _ecore_drm_device_cb_page_flip;
   ctx.vblank_handler = _ecore_drm_device_cb_vblank;

   drmHandleEvent(dev->drm.fd, &ctx);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool 
_ecore_drm_device_cb_idle(void *data)
{
   Ecore_Drm_Device *dev;
   Ecore_Drm_Output *output;
   Eina_List *l;

   if (!(dev = data)) return ECORE_CALLBACK_CANCEL;

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        output->need_repaint = EINA_TRUE;
        if (output->repaint_scheduled) continue;
        _ecore_drm_output_repaint_start(output);
     }

   return ECORE_CALLBACK_RENEW;
}

/**
 * @defgroup Ecore_Drm_Device_Group Device manipulation functions
 * 
 * Functions that deal with finding, opening, closing, and otherwise using 
 * the DRM device itself.
 */

/**
 * Find a drm device in the system.
 *
 * @param name The name of the device to find. If NULL, this function will 
 *             search for the default drm device.
 * @param seat The name of the seat where this device may be found. If NULL, 
 *             this function will use a default seat name 'seat0'.
 * 
 * @return An opaque Ecore_Drm_Device structure representing the card.
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Ecore_Drm_Device *
ecore_drm_device_find(const char *name, const char *seat)
{
   Ecore_Drm_Device *dev = NULL;
   struct udev_enumerate *uenum;
   struct udev_list_entry *uentry;
   struct udev_device *udevice, *tmpdevice = NULL, *pcidevice;
   const char *path = NULL, *devseat = NULL;

   /* check for existing udev reference */
   if (!udev) return NULL;

   /* setup udev enumerator */
   uenum = udev_enumerate_new(udev);
   udev_enumerate_add_match_subsystem(uenum, "drm");
   udev_enumerate_add_match_subsystem(uenum, "card[0-9]*");

   /* ask udev for list of drm devices */
   udev_enumerate_scan_devices(uenum);

   /* loop list of returned devices */
   udev_list_entry_foreach(uentry, udev_enumerate_get_list_entry(uenum))
     {
        /* get device path */
        path = udev_list_entry_get_name(uentry);

        /* get udev device */
        if (!(udevice = udev_device_new_from_syspath(udev, path)))
          continue;

        /* if we are looking for a certain device, then compare names */
        if (name)
          {
             if (strcmp(name, udev_device_get_devnode(udevice)))
               {
                  udev_device_unref(udevice);
                  continue;
               }
          }

        /* get this devices' seat */
        devseat = udev_device_get_property_value(udevice, "ID_SEAT");
        if (!devseat) devseat = "seat0";

        /* if we are looking for a device on a certain seat, compare it */
        if (seat)
          {
             if (strcmp(seat, devseat))
               {
                  udev_device_unref(udevice);
                  continue;
               }
          }
        else
          {
             /* no seat name passed to use. check default */
             if (strcmp(devseat, "seat0"))
               {
                  udev_device_unref(udevice);
                  continue;
               }
          }

        /* try to find the boot_vga attribute */
        if ((pcidevice = 
             udev_device_get_parent_with_subsystem_devtype(udevice, "pci", NULL)))
          {
             const char *id;

             if ((id = udev_device_get_sysattr_value(pcidevice, "boot_vga")))
               {
                  if (!strcmp(id, "1"))
                    {
                       if (tmpdevice) udev_device_unref(tmpdevice);
                       tmpdevice = udevice;
                       break;
                    }
               }
          }

        if (!tmpdevice) 
          tmpdevice = udevice;
        else
          udev_device_unref(udevice);
     }

   /* destroy the enumerator */
   udev_enumerate_unref(uenum);

   if (tmpdevice)
     {
        DBG("Found Drm Device");
        DBG("\tFilename: %s", udev_device_get_devnode(tmpdevice));
        DBG("\tDriver: %s", udev_device_get_driver(tmpdevice));
        DBG("\tDevpath: %s", udev_device_get_devpath(tmpdevice));
        DBG("\tSyspath: %s", udev_device_get_syspath(tmpdevice));
        DBG("\tSysname: %s", udev_device_get_sysname(tmpdevice));

        /* try to allocate space for return device structure */
        if ((dev = calloc(1, sizeof(Ecore_Drm_Device))))
          {
             const char *id, *seat;

             /* set device name */
             dev->drm.name = 
               eina_stringshare_add(udev_device_get_devnode(tmpdevice));

             /* set device path */
             dev->drm.path = 
               eina_stringshare_add(udev_device_get_syspath(tmpdevice));

             /* store id for this device */
             if ((id = udev_device_get_sysnum(tmpdevice)))
               dev->id = atoi(id);

             /* set dev seat */
             seat = udev_device_get_property_value(tmpdevice, "ID_SEAT");
             if (!seat) seat = "seat0";

             dev->seat = eina_stringshare_add(seat);

             dev->format = GBM_FORMAT_XRGB8888;
             dev->use_hw_accel = EINA_FALSE;
          }
     }

   /* release device reference */
   udev_device_unref(tmpdevice);

   return dev;
}

/**
 * Free an Ecore_Drm_Device
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Device.
 * 
 * @param dev The Ecore_Drm_Device to free
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI void 
ecore_drm_device_free(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Output *output;

   /* check for valid device */
   if (!dev) return;

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

   /* free structure */
   free(dev);
}

/**
 * Open an Ecore_Drm_Device
 *
 * This function will open an existing Ecore_Drm_Device for use.
 * 
 * @param dev The Ecore_Drm_Device to try and open
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool 
ecore_drm_device_open(Ecore_Drm_Device *dev)
{
   uint64_t caps;

   /* check for valid device */
   if ((!dev) || (!dev->drm.name)) return EINA_FALSE;

   dev->drm.fd = open(dev->drm.name, O_RDWR);
   if (dev->drm.fd < 0) return EINA_FALSE;

   DBG("Opened Device %s : %d", dev->drm.name, dev->drm.fd);

   if (!drmGetCap(dev->drm.fd, DRM_CAP_TIMESTAMP_MONOTONIC, &caps))
     {
        if (caps == 1)
          dev->drm.clock = CLOCK_MONOTONIC;
        else
          dev->drm.clock = CLOCK_REALTIME;
     }
   else
     {
        ERR("Could not get device capabilities: %m");
     }

/* #ifdef HAVE_GBM */
/*    if (getenv("ECORE_DRM_HW_ACCEL")) */
/*      { */
        /* Typically, gbm loads the dri driver However some versions of Mesa 
         * do not have libglapi symbols linked in the driver. Because of this, 
         * using hardware accel for our drm code Could fail with a 
         * message that the driver could not load. Let's be proactive and 
         * work around this for the user by preloading the glapi library */
        /* dlopen("libglapi.so.0", (RTLD_LAZY | RTLD_GLOBAL)); */

        /* if ((dev->gbm = gbm_create_device(dev->drm.fd))) */
        /*   { */
        /*      EGLint major, minor, visual; */
        /*      const EGLint attribs[] =  */
        /*        { */
        /*           EGL_SURFACE_TYPE, EGL_WINDOW_BIT,  */
        /*           EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1,  */
        /*           EGL_BLUE_SIZE, 1, EGL_ALPHA_SIZE, 0,  */
        /*           EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE */
        /*        }; */

        /*      dev->use_hw_accel = EINA_TRUE; */
        /*      dev->format = GBM_FORMAT_XRGB8888; */

        /*      dev->egl.disp = eglGetDisplay(dev->gbm); */
        /*      if (dev->egl.disp == EGL_NO_DISPLAY) */
        /*        { */
        /*           ERR("Could not get egl display"); */
        /*           goto init_software; */
        /*        } */

        /*      if (!eglInitialize(dev->egl.disp, &major, &minor)) */
        /*        { */
        /*           ERR("Could not initialize egl"); */
        /*           goto init_software; */
        /*        } */

        /*      visual = dev->format; */
        /*      if (!_ecore_drm_device_egl_config_get(dev, attribs, &visual)) */
        /*        { */
        /*           ERR("Could not get egl config"); */
        /*           goto init_software; */
        /*        } */
        /*   } */
        /* else */
        /*   { */
        /*      WRN("Failed to create gbm device"); */
        /*      goto init_software; */
        /*   } */
     /* } */
   /* else */
/* #endif */
     /* { */
        /* TODO: init software */
/* init_software: */
/*         DBG("Init Software Engine"); */
/* #ifdef HAVE_GBM */
/*         if (dev->egl.disp)  */
/*           { */
/*              eglMakeCurrent(dev->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE,  */
/*                             EGL_NO_CONTEXT); */
/*              eglTerminate(dev->egl.disp); */
/*              eglReleaseThread(); */
/*           } */

/*         if (dev->gbm) gbm_device_destroy(dev->gbm); */
/*         dev->gbm = NULL; */
/* #endif */
/*      } */

   dev->drm.hdlr = 
     ecore_main_fd_handler_add(dev->drm.fd, ECORE_FD_READ, 
                               _ecore_drm_device_cb_event, dev, NULL, NULL);

   dev->drm.idler = 
     ecore_idle_enterer_add(_ecore_drm_device_cb_idle, dev);

   return EINA_TRUE;
}

/**
 * Close an Ecore_Drm_Device
 *
 * This function will close a previously opened Ecore_Drm_Device
 * 
 * @param dev The Ecore_Drm_Device to free
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool 
ecore_drm_device_close(Ecore_Drm_Device *dev)
{
   /* check for valid device */
   if (!dev) return EINA_FALSE;

/* #ifdef HAVE_GBM */
/*    if (dev->use_hw_accel) */
/*      { */
/*         if (dev->egl.disp)  */
/*           { */
/*              eglMakeCurrent(dev->egl.disp, EGL_NO_SURFACE, EGL_NO_SURFACE,  */
/*                             EGL_NO_CONTEXT); */
/*              eglTerminate(dev->egl.disp); */
/*              eglReleaseThread(); */
/*           } */

/*         if (dev->gbm) gbm_device_destroy(dev->gbm); */
/*         dev->gbm = NULL; */
/*      } */
/* #endif */

   if (dev->drm.hdlr) ecore_main_fd_handler_del(dev->drm.hdlr);
   dev->drm.hdlr = NULL;

   close(dev->drm.fd);

   /* reset device fd */
   dev->drm.fd = -1;

   /* free(data); */

   return EINA_TRUE;
}

/**
 * Get if a given Ecore_Drm_Device is master
 * 
 * This function will check if the given drm device is set to master
 * 
 * @param dev The Ecore_Drm_Device to check
 * 
 * @return EINA_TRUE if device is master, EINA_FALSE otherwise
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool 
ecore_drm_device_master_get(Ecore_Drm_Device *dev)
{
   /* drm_magic_t mag; */

   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   /* FIXME: Remote this to the slave process !! */

   /* get if we are master or not */
   /* if ((drmGetMagic(dev->drm.fd, &mag) == 0) &&  */
   /*     (drmAuthMagic(dev->drm.fd, mag) == 0)) */
   /*   return EINA_TRUE; */

   return EINA_FALSE;
}

/**
 * Set a given Ecore_Drm_Device to master
 * 
 * This function will attempt to set a given drm device to be master
 * 
 * @param dev The Ecore_Drm_Device to set
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool 
ecore_drm_device_master_set(Ecore_Drm_Device *dev)
{
   Eina_Bool ret = EINA_FALSE;
   int dfd;

   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   DBG("Set Master On Fd: %d", dev->drm.fd);

   /* try to close the device */
   _ecore_drm_message_send(ECORE_DRM_OP_DEVICE_MASTER_SET, dev->drm.fd, 
                           NULL, 0);

   /* get the result of the close operation */
   ret = _ecore_drm_message_receive(ECORE_DRM_OP_DEVICE_MASTER_SET, &dfd,
                                    NULL, 0);
   if (!ret) return EINA_FALSE;

   return EINA_TRUE;
}

/**
 * Tell a given Ecore_Drm_Device to stop being master
 * 
 * This function will attempt to ask a drm device to stop being master
 * 
 * @param dev The Ecore_Drm_Device to set
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool 
ecore_drm_device_master_drop(Ecore_Drm_Device *dev)
{
   Eina_Bool ret = EINA_FALSE;
   int dfd;

   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   DBG("Drop Master On Fd: %d", dev->drm.fd);

   /* try to close the device */
   _ecore_drm_message_send(ECORE_DRM_OP_DEVICE_MASTER_DROP, dev->drm.fd,
                           NULL, 0);

   /* get the result of the close operation */
   ret = _ecore_drm_message_receive(ECORE_DRM_OP_DEVICE_MASTER_DROP, &dfd,
                                    NULL, 0);
   if (!ret) return EINA_FALSE;

   return EINA_TRUE;
}

EAPI int 
ecore_drm_device_fd_get(Ecore_Drm_Device *dev)
{
   if (!dev) return -1;
   return dev->drm.fd;
}
