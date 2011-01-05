#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include "eeze_udev_private.h"

/**
 * @defgroup udev udev
 *
 * These are functions which interact directly with udev.
 */

_udev *udev;

int _eeze_udev_log_dom = -1;
int _eeze_init_count = 0;

/**
 * @defgroup main main
 *
 * These are general eeze functions which include init and shutdown.
 */

static Eeze_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Eeze_Version *eeze_version = &_version;

/**
 * Initialize the eeze library.
 * @return The number of times the function has been called, or -1 on failure.
 *
 * This function should be called prior to using any eeze functions, and MUST
 * be called prior to using any udev functions to avoid a segv.
 *
 * @ingroup main
 */
EAPI int
eeze_init(void)
{
   if (++_eeze_init_count != 1)
     return _eeze_init_count;

   if (!eina_init())
     return 0;

   _eeze_udev_log_dom = eina_log_domain_register
       ("eeze_udev", EEZE_UDEV_COLOR_DEFAULT);

   if (_eeze_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_udev' log domain.");
        goto eina_fail;
     }

   if (!ecore_init())
     goto fail;

   if (!((udev) = udev_new()))
     {
        EINA_LOG_ERR("Could not initialize udev library!");
        goto ecore_fail;
     }

   return _eeze_init_count;
ecore_fail:
   ecore_shutdown();
fail:
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
eina_fail:
   eina_shutdown();
   return 0;
}

/**
 * Shut down the eeze library.
 * @return The number of times the eeze_init has been called, or -1 when
 * all occurrences of eeze have been shut down.
 *
 * This function should be called when no further eeze functions will be called.
 *
 * @ingroup main
 */
EAPI int
eeze_shutdown(void)
{
   if (--_eeze_init_count != 0)
     return _eeze_init_count;

   udev_unref(udev);
   ecore_shutdown();
   eina_log_domain_unregister(_eeze_udev_log_dom);
   _eeze_udev_log_dom = -1;
   eina_shutdown();
   return _eeze_init_count;
}

