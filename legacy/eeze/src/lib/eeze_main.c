#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_private.h"
#include "eeze_udev_private.h"

_udev *udev;

int _eeze_log_dom = -1;
int _eeze_init_count = 0;

EAPI int
eeze_init(void)
{
   if (++_eeze_init_count != 1)
     return _eeze_init_count;

   if (!eina_init())
     return --_eeze_init_count;

   _eeze_log_dom = eina_log_domain_register
                       ("eeze_udev", eeze_COLOR_DEFAULT);

   if (_eeze_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_udev' log domain.");
        goto fail;
     }

   if (!((udev) = udev_new()))
     {
      EINA_LOG_ERR("Could not initialize udev library!");
      goto fail;
     }

   return _eeze_init_count;
fail:
   eina_log_domain_unregister(_eeze_log_dom);
   _eeze_log_dom = -1;
   eina_shutdown();
   return _eeze_init_count;
}

EAPI int
eeze_shutdown(void)
{
   if (--_eeze_init_count != 0)
     return _eeze_init_count;

   udev_unref(udev);
   eina_log_domain_unregister(_eeze_log_dom);
   _eeze_log_dom = -1;
   eina_shutdown();
   return _eeze_init_count;
}
