#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_fb_private.h"

static Eina_List *fb_devices;

EAPI Ecore_Fb_Device *
ecore_fb_device_find(const char *name)
{
   Ecore_Fb_Device *dev = NULL;
   Eina_List *devs, *l;
   const char *device;

   /* try to get a list of fb devics */
   if (!(devs = eeze_udev_find_by_type(EEZE_UDEV_TYPE_GRAPHICS, name)))
     {
       return NULL;
     }
   DBG("Find Framebuffer Device: %s", name);

   EINA_LIST_FOREACH(devs, l, device)
     {
        const char *devpath;
        const char *devseat;
        const char *devparent;

        if (!(devpath = eeze_udev_syspath_get_devpath(device)))
          continue;

        DBG("Found Fb Device");
        DBG("\tDevice: %s", device);
        DBG("\tDevpath: %s", devpath);

        if ((name) && (!strstr(name, devpath)))
	  {
	    eina_stringshare_del(devpath);
	    if ((dev = calloc(1, sizeof(Ecore_Fb_Device))))
	      {
		dev->seat = eeze_udev_syspath_get_property(device, "ID_SEAT");
		if (!dev->seat) dev->seat = eina_stringshare_add("seat0");
		fb_devices = eina_list_append(fb_devices, dev);
		/* try to create xkb context */
		if (!(dev->xkb_ctx = xkb_context_new(0)))
		  {
		    ERR("Failed to create xkb context: %m");
		    return EINA_FALSE;
		  }

	      }

	    break;
	  }
	eina_stringshare_del(devpath);
     }

   EINA_LIST_FREE(devs, device)
     eina_stringshare_del(device);

   return dev;
}


EAPI void
ecore_fb_device_window_set(Ecore_Fb_Device *dev, unsigned int window)
{
   /* check for valid device */
   EINA_SAFETY_ON_TRUE_RETURN((!dev));

   dev->window = window;
}
