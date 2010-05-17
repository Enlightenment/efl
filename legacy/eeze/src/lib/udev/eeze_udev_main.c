#include "eeze_udev_private.h"
#include <Eeze_Udev.h>

int _e_eeze_udev_log_dom = -1;
int _e_eeze_udev_init_count = 0;

EAPI int
eeze_udev_init(void)
{
   if (++_e_eeze_udev_init_count != 1)
     return _e_eeze_udev_init_count;

   if (!eina_init())
     return --_e_eeze_udev_init_count;

   _e_eeze_udev_log_dom = eina_log_domain_register
     ("eeze_udev", E_EEZE_COLOR_DEFAULT);
   if (_e_eeze_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_udev' log domain.");
        goto fail;
     }

   return _e_eeze_udev_init_count;

fail:
     eina_log_domain_unregister(_e_eeze_udev_log_dom);
     _e_eeze_udev_log_dom = -1;
     eina_shutdown();
   return _e_eeze_udev_init_count;
}

EAPI int
eeze_udev_shutdown(void)
{
   if (--_e_eeze_udev_init_count != 0)
     return _e_eeze_udev_init_count;

   eina_log_domain_unregister(_e_eeze_udev_log_dom);
   _e_eeze_udev_log_dom = -1;
   eina_shutdown();

   return _e_eeze_udev_init_count;
}
