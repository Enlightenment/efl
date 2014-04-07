#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>
#include <Eeze_Sensor.h>
#include "eeze_udev_private.h"
#include "eeze_net_private.h"
#include "eeze_disk_private.h"
#include "eeze_sensor_private.h"

_udev *udev;

int _eeze_udev_log_dom = -1;
int _eeze_net_log_dom = -1;
int _eeze_sensor_log_dom = -1;
int _eeze_init_count = 0;

static Eeze_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Eeze_Version *eeze_version = &_version;

EAPI int
eeze_init(void)
{
   if (++_eeze_init_count != 1)
     return _eeze_init_count;

   if (!eina_init())
     return --_eeze_init_count;

   _eeze_udev_log_dom = eina_log_domain_register("eeze_udev", EINA_COLOR_CYAN);
   if (_eeze_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_udev' log domain.");
        goto eina_fail;
     }
   _eeze_net_log_dom = eina_log_domain_register("eeze_net", EINA_COLOR_GREEN);
   if (_eeze_net_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_net' log domain.");
        goto eina_net_fail;
     }

   _eeze_sensor_log_dom = eina_log_domain_register("eeze_sensor", EINA_COLOR_BLUE);
   if (_eeze_sensor_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_sensor' log domain.");
        goto eina_sensor_fail;
     }

   if (!ecore_init())
     goto ecore_fail;
#ifdef HAVE_EEZE_MOUNT
   if (!eeze_disk_init())
     goto eeze_fail;
#endif
   if (!(udev = udev_new()))
     {
        EINA_LOG_ERR("Could not initialize udev library!");
        goto fail;
     }
   if (!eeze_net_init())
     {
        EINA_LOG_ERR("Error initializing eeze_net subsystems!");
        goto net_fail;
     }
   if (!eeze_sensor_init())
     {
        EINA_LOG_ERR("Error initializing eeze_sensor subsystems!");
        goto sensor_fail;
     }

   return _eeze_init_count;

sensor_fail:
   eeze_net_shutdown();
net_fail:
   udev_unref(udev);
fail:
#ifdef HAVE_EEZE_MOUNT
   eeze_disk_shutdown();
eeze_fail:
#endif
   ecore_shutdown();
ecore_fail:
   eina_log_domain_unregister(_eeze_sensor_log_dom);
   _eeze_sensor_log_dom = -1;
eina_sensor_fail:
   eina_log_domain_unregister(_eeze_net_log_dom);
   _eeze_net_log_dom = -1;
eina_net_fail:
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
eina_fail:
   eina_shutdown();
   return --_eeze_init_count;
}

EAPI int
eeze_shutdown(void)
{
   if (_eeze_init_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_eeze_init_count != 0)
     return _eeze_init_count;

   udev_unref(udev);
#ifdef HAVE_EEZE_MOUNT
   eeze_disk_shutdown();
#endif
   eeze_sensor_shutdown();
   eeze_net_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
   eina_log_domain_unregister(_eeze_net_log_dom);
   _eeze_net_log_dom = -1;
   eina_log_domain_unregister(_eeze_sensor_log_dom);
   _eeze_sensor_log_dom = -1;
   eina_shutdown();
   return _eeze_init_count;
}

EAPI void *
eeze_udev_get(void)
{
   return udev;
}
