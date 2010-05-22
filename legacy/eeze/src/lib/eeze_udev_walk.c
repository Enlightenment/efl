#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_private.h"
#include "eeze_udev_private.h"

extern _udev *udev;

/**
 * Walks up the device chain starting at @p syspath,
 * checking each device for @p sysattr with (optional) @p value.
 *
 * @param syspath The /sys/ path of the device to start at, with or without the /sys/
 * @param sysattr The attribute to find
 * @param value OPTIONAL: The value that @p sysattr should have, or NULL
 *
 * @return If the sysattr (with value) is found, returns TRUE.  Else, false.
 */
EAPI Eina_Bool
eeze_udev_walk_check_sysattr(const char *syspath, const char *sysattr,
                             const char *value)
{
   _udev_device *device, *child, *parent;
   Eina_Strbuf *sbuf;
   const char *test = NULL;

   if (!udev)
     return 0;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   for (parent = device; parent;
       child = parent, parent = udev_device_get_parent(child))
     {
        if ((test = udev_device_get_sysattr_value(parent, sysattr)))
          {
             if (value)
               {
                  if (!strcmp(test, value))
                    {
                      eina_strbuf_free(sbuf);
                      udev_device_unref(device);
                      return 1;
                    }
               }
             else
               {
                  eina_strbuf_free(sbuf);
                  udev_device_unref(device);
                  return 1;
               }
          }
     }

   eina_strbuf_free(sbuf);
   udev_device_unref(device);
   return 0;
}

/**
 * Walks up the device chain starting at @p syspath,
 * checking each device for @p sysattr, and returns the value if found.
 *
 * @param syspath The /sys/ path of the device to start at, with or without the /sys/
 * @param sysattr The attribute to find
 *
 * @return The value of @p sysattr if found, or NULL
 */
EAPI const char *
eeze_udev_walk_get_sysattr(const char *syspath, const char *sysattr)
{
   _udev_device *device, *child, *parent;
   Eina_Strbuf *sbuf;
   const char *test = NULL;

   if (!syspath)
     return NULL;

   sbuf = eina_strbuf_new();

   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));

   for (parent = device; parent;
       child = parent, parent = udev_device_get_parent(child))
     {
        if ((test = udev_device_get_sysattr_value(parent, sysattr)))
          {
             eina_strbuf_free(sbuf);
             test = eina_stringshare_add(test);
             udev_device_unref(device);
             return test;
          }
     }

   eina_strbuf_free(sbuf);
   udev_device_unref(device);
   return NULL;
}
