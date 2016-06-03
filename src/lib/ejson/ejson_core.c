#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ejson.h"
#include "ejson_private.h"

static int _ejson_init_count = 0;
int _ejson_log_dom = -1;

EAPI int
ejson_init(void)
{
   if (_ejson_init_count++ > 0)
     return _ejson_init_count;

   if (!eina_init())
     {
        fputs("Ejson: Unable to initialize eina\n", stderr);
        return 0;
     }

   _ejson_log_dom = eina_log_domain_register("ejson_model", EINA_COLOR_CYAN);
   if (_ejson_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ejson_model' log domain");
        _ejson_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   ecore_app_no_system_modules();
   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        eina_log_domain_unregister(_ejson_log_dom);
        _ejson_log_dom = -1;
        eina_shutdown();
        return 0;
     }

   // ...

   return _ejson_init_count;
}

EAPI int
ejson_shutdown(void)
{
   if (_ejson_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _ejson_init_count = 0;
        return 0;
     }

   if (--_ejson_init_count)
     return _ejson_init_count;

   // ...

   ecore_shutdown();
   eina_log_domain_unregister(_ejson_log_dom);
   _ejson_log_dom = -1;
   eina_shutdown();
   return 0;
}
