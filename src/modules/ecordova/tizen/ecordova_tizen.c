#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <dlfcn.h>

int _ecordova_log_dom;

int (*system_info_get_platform_string)(const char *key, char **value);
static void* system_info_lib = 0;

void ecordova_contacts_service_init();
void ecordova_contacts_service_shutdown();
void ecordova_sensor_init();
void ecordova_sensor_shutdown();

static Eina_Bool
_ecordova_tizen_init(void)
{
   _ecordova_log_dom = eina_log_domain_register("ecordova_tizen", EINA_COLOR_CYAN);
   if (_ecordova_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ecordova' log domain");
        return EINA_FALSE;
     }

   ecordova_contacts_service_init();

   if(!system_info_lib)
     {
       system_info_lib = dlopen("capi-system-info", RTLD_NOW);
       if(system_info_lib)
         {
           system_info_get_platform_string = dlsym(system_info_lib, "system_info_get_platform_string");
           if(!system_info_get_platform_string)
               dlclose(system_info_lib);
         }
     }

   ecordova_sensor_init();
  
   return EINA_TRUE;
}

static void
_ecordova_tizen_shutdown(void)
{
   ecordova_sensor_shutdown();

   if(system_info_lib)
     dlclose(system_info_lib);

   system_info_get_platform_string = 0;

   ecordova_contacts_service_shutdown();
  
   eina_log_domain_unregister(_ecordova_log_dom);
   _ecordova_log_dom = -1;
}

EINA_MODULE_INIT(_ecordova_tizen_init);
EINA_MODULE_SHUTDOWN(_ecordova_tizen_shutdown);
