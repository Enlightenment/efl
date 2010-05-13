#include <E_Udev.h>
#include "e_udev_private.h"

int _e_eeze_udev_log_dom = -1;
int _e_eeze_udev_init_count = 0;

EAPI int
e_udev_init(void)
{
   if (++_e_eeze_udev_init_count != 1)
     return _e_eeze_udev_init_count;

   if (!eina_init())
     return --_e_eeze_udev_init_count;

   _e_eeze_udev_log_dom = eina_log_domain_register
     ("e_udev", E_EEZE_COLOR_DEFAULT);
   if (_e_eeze_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'e_udev' log domain.");
        goto shutdown_eina;
     }

   return _e_eeze_udev_init_count;

   unregister_log_domain:
     eina_log_domain_unregister(_e_eeze_udev_log_dom);
     _e_eeze_udev_log_dom = -1;
   shutdown_eina:
     eina_shutdown();

   return _e_eeze_udev_init_count;
}

EAPI int
e_udev_shutdown(void)
{
   if (--_e_eeze_udev_init_count != 0)
     return _e_eeze_udev_init_count;

   eina_log_domain_unregister(_e_eeze_udev_log_dom);
   _e_eeze_udev_log_dom = -1;
   eina_shutdown();

   return _e_eeze_udev_init_count;
}
