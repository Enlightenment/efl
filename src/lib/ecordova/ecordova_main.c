#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_private.h"
#include "ecordova_systeminfo.eo.h"

#include <Ecore_File.h>

static int _ecordova_init_count = 0;
int _ecordova_log_dom = -1;
Eo* _ecordova_systeminfo = NULL;

EAPI int
ecordova_init(void)
{
   if (_ecordova_init_count++ > 0)
     return _ecordova_init_count;

   if (!eina_init())
     {
        fputs("Ecordova: Unable to initialize eina\n", stderr);
        return 0;
     }

   _ecordova_log_dom = eina_log_domain_register("ecordova", EINA_COLOR_CYAN);
   if (_ecordova_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ecordova' log domain");
        goto on_error_1;
     }

   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        goto on_error_2;
     }

   if (!ecore_file_init())
     {
        ERR("Unable to initialize ecore_file");
        goto on_error_3;
     }

   if (!eio_init())
     {
        ERR("Unable to initialize eio");
        goto on_error_4;
     }

   _ecordova_systeminfo = eo_add(ECORDOVA_SYSTEMINFO_CLASS, NULL,
                                 ecordova_systeminfo_constructor());
   if (!_ecordova_systeminfo)
     {
        ERR("Unable to initialize systeminfo service");
        goto on_error_4;
     }

   return _ecordova_init_count;

on_error_4:
   ecore_file_shutdown();

on_error_3:
   ecore_shutdown();

on_error_2:
   eina_log_domain_unregister(_ecordova_log_dom);

on_error_1:
   _ecordova_log_dom = -1;
   eina_shutdown();
   return 0;
}

EAPI int
ecordova_shutdown(void)
{
   if (_ecordova_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _ecordova_init_count = 0;
        return 0;
     }

   if (--_ecordova_init_count)
     return _ecordova_init_count;

   eo_unref(_ecordova_systeminfo);
   eio_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_ecordova_log_dom);
   _ecordova_log_dom = -1;
   eina_shutdown();
   return 0;
}

