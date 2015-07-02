#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_device_private.h"

#include <system/system_info.h>

#define MY_CLASS ECORDOVA_DEVICE_CLASS
#define MY_CLASS_NAME "Ecordova_Device"

static Eo_Base *
_ecordova_device_eo_base_constructor(Eo *obj, Ecordova_Device_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->uuid = NULL;
   pd->version = NULL;
   pd->model = NULL;
   pd->platform = NULL;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_device_constructor(Eo *obj EINA_UNUSED,
                             Ecordova_Device_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_device_eo_base_destructor(Eo *obj, Ecordova_Device_Data *pd)
{
   DBG("(%p)", obj);

   free(pd->uuid);
   free(pd->version);
   free(pd->model);
   free(pd->platform);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static const char *
_ecordova_device_ecordova_get(Eo *obj EINA_UNUSED,
                              Ecordova_Device_Data *pd EINA_UNUSED)
{
   ERR("Not implemented.");
   return NULL;
}

static const char *
_ecordova_device_model_get(Eo *obj EINA_UNUSED, Ecordova_Device_Data *pd)
{
   if (!pd->model)
     {
        int ret = system_info_get_platform_string("tizen.org/system/model_name",
                                                  &pd->model);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret == SYSTEM_INFO_ERROR_NONE, NULL);
     }

   return pd->model;
}

static const char *
_ecordova_device_platform_get(Eo *obj EINA_UNUSED, Ecordova_Device_Data *pd)
{
   if (!pd->model)
     {
        int ret = system_info_get_platform_string("tizen.org/system/platform.name",
                                                  &pd->model);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret == SYSTEM_INFO_ERROR_NONE, NULL);
     }

   return pd->model;
}

static const char *
_ecordova_device_uuid_get(Eo *obj EINA_UNUSED, Ecordova_Device_Data *pd)
{
   if (!pd->uuid)
     {
        int ret = system_info_get_platform_string("tizen.org/system/tizenid",
                                                  &pd->uuid);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret == SYSTEM_INFO_ERROR_NONE, NULL);
     }

   return pd->uuid;
}

static const char *
_ecordova_device_version_get(Eo *obj EINA_UNUSED, Ecordova_Device_Data *pd)
{
   if (!pd->version)
     {
        // requires http://tizen.org/privilege/system  (and not "systeminfo")  privileges to be added in config.xml
        int ret = system_info_get_platform_string("tizen.org/feature/platform.version",
                                                  &pd->version);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret == SYSTEM_INFO_ERROR_NONE, NULL);
     }

   return pd->version;
}

#include "ecordova_device.eo.c"
