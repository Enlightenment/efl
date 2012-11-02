#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>

#include "efreetd.h"
#include "efreetd_dbus.h"
#include "efreetd_cache.h"

int efreetd_log_dom = -1;

int
main(void)
{
   if (!eina_init()) return 1;
   efreetd_log_dom = eina_log_domain_register("efreetd", EFREETD_DEFAULT_LOG_COLOR);
   if (efreetd_log_dom < 0)
     {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreetd.");
        goto ecore_error;
     }
   if (!ecore_init()) goto ecore_error;
   if (!ecore_file_init()) goto ecore_file_error;

   if (!dbus_init()) goto dbus_error;
   if (!cache_init()) goto cache_error;

   ecore_main_loop_begin();

   cache_shutdown();
   dbus_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(efreetd_log_dom);
   efreetd_log_dom = -1;
   eina_shutdown();
   return 0;

cache_error:
   dbus_shutdown();
dbus_error:
   ecore_file_shutdown();
ecore_file_error:
   ecore_shutdown();
ecore_error:
   if (efreetd_log_dom >= 0)
     eina_log_domain_unregister(efreetd_log_dom);
   efreetd_log_dom = -1;
   eina_shutdown();
   return 1;
}
