#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <Eina.h>
#include <Ecore.h>
#include <Eeze_Sensor.h>
#include "eeze_sensor_private.h"

Eeze_Sensor *g_handle;

/* Priority order for modules. The one with the highest order of the available ones will be used.
 * This in good enough for now as we only have two modules and one is a test harness anyway. If the
 * number of modules grows we might re-think the priority handling, but we should do this when the
 * need arise.
 */
static const char *_module_priority[] = {
   "tizen",
   "fake",
   NULL
};

Eeze_Sensor_Module *
_highest_priority_module_get(void)
{
   Eeze_Sensor_Module *module = NULL;
   int i = 0;

   while (_module_priority[i] != NULL)
     {
        module = eina_hash_find(g_handle->modules, _module_priority[i]);
        if (module) return module;
        i++;
     }
   return NULL;
}

EAPI Eeze_Sensor_Obj *
eeze_sensor_obj_get(Eeze_Sensor_Type sensor_type)
{
   Eina_List *l;
   Eeze_Sensor_Obj *obj;
   Eeze_Sensor_Module *module;

   module = _highest_priority_module_get();

   if (!module) return NULL;

   EINA_LIST_FOREACH(module->sensor_list, l, obj)
     {
        if (obj->type == sensor_type)
          {
             return obj;
          }
     }
   return NULL;
}

static void
eeze_sensor_modules_load(void)
{
   /* Check for available runtime modules and load them */
   g_handle->modules_array = eina_module_list_get(NULL, PACKAGE_LIB_DIR "/eeze-sensor/", 0, NULL, NULL);
   /* FIXME check for modules also in HOME and other locations */

   if (!g_handle->modules_array)
     {
        ERR("No modules found!");
        return;
     }

   eina_module_list_load(g_handle->modules_array);
}

static void
eeze_sensor_modules_unload(void)
{
   if (!g_handle->modules_array) return;
   eina_module_list_unload(g_handle->modules_array);
   eina_module_list_free(g_handle->modules_array);
   eina_array_free(g_handle->modules_array);
   g_handle->modules_array = NULL;
}

Eina_Bool
eeze_sensor_module_register(const char *name, Eeze_Sensor_Module *mod)
{
   Eeze_Sensor_Module *module = NULL;

   if (!mod) return EINA_FALSE;

   module = calloc(1, sizeof(Eeze_Sensor_Module));
   if (!module) return EINA_FALSE;

   module = mod;

   if (!module->init) return EINA_FALSE;
   if (!(module->init())) return EINA_FALSE;

   INF("Registered module %s", name);

   return eina_hash_add(g_handle->modules, name, module);
}

Eina_Bool
eeze_sensor_module_unregister(const char *name)
{
   DBG("Unregister module %s", name);

   Eeze_Sensor_Module *module = NULL;

   module = eina_hash_find(g_handle->modules, name);
   if (module->shutdown)
     module->shutdown();

   return eina_hash_del(g_handle->modules, name, NULL);
}

EAPI Eeze_Sensor_Obj *
eeze_sensor_new(Eeze_Sensor_Type type)
{
   Eeze_Sensor_Obj *sens;
   Eeze_Sensor_Module *module = NULL;

   sens = calloc(1, sizeof(Eeze_Sensor_Obj));
   if (!sens) return NULL;

   sens = eeze_sensor_obj_get(type);
   if (!sens) return NULL;

   module = _highest_priority_module_get();
   if (!module) return EINA_FALSE;

   if (!module->read) return NULL;

   if (module->read(sens->type, sens))
     {
        return sens;
     }
   else
      return NULL;
}

EAPI void
eeze_sensor_free(Eeze_Sensor_Obj *sens)
{
   if (!sens) return;
   free(sens);
}

EAPI Eina_Bool
eeze_sensor_accuracy_get(Eeze_Sensor_Obj *sens, int *accuracy)
{
   if (!sens) return EINA_FALSE;

   *accuracy = sens->accuracy;
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_sensor_xyz_get(Eeze_Sensor_Obj *sens, float *x, float *y, float *z)
{
   if (!sens) return EINA_FALSE;

   *x = sens->data[0];
   *y = sens->data[1];
   *z = sens->data[2];
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_sensor_xy_get(Eeze_Sensor_Obj *sens, float *x, float *y)
{
   if (!sens) return EINA_FALSE;

   *x = sens->data[0];
   *y = sens->data[1];
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_sensor_x_get(Eeze_Sensor_Obj *sens, float *x)
{
   if (!sens) return EINA_FALSE;

   *x = sens->data[0];
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_sensor_timestamp_get(Eeze_Sensor_Obj *sens, unsigned long long *timestamp)
{
   if (!sens) return EINA_FALSE;

   *timestamp = sens->timestamp;
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_sensor_read(Eeze_Sensor_Obj *sens)
{
   Eeze_Sensor_Module *module = NULL;

   if (!sens) return EINA_FALSE;

   module = _highest_priority_module_get();
   if (!module) return EINA_FALSE;

   if (module->read)
     return module->read(sens->type, sens);

   return EINA_FALSE;
}

EAPI Eina_Bool
eeze_sensor_async_read(Eeze_Sensor_Obj *sens, void *user_data)
{
   Eeze_Sensor_Module *module = NULL;

   module = _highest_priority_module_get();
   if (!module) return EINA_FALSE;
   if (module->async_read)
     return module->async_read(sens->type, user_data);

   return EINA_FALSE;
}

void
eeze_sensor_shutdown(void)
{
   eeze_sensor_modules_unload();
   free(g_handle);
   g_handle = NULL;

   eina_shutdown();
}

Eina_Bool
eeze_sensor_init(void)
{
   if (!eina_init()) return EINA_FALSE;

   g_handle = calloc(1, sizeof(Eeze_Sensor));
   if (!g_handle) return EINA_FALSE;

   g_handle->modules_array = NULL;
   g_handle->modules = eina_hash_string_small_new(NULL);
   if (!g_handle->modules) return EINA_FALSE;

   EEZE_SENSOR_EVENT_SNAP = ecore_event_type_new();
   EEZE_SENSOR_EVENT_SHAKE = ecore_event_type_new();
   EEZE_SENSOR_EVENT_DOUBLETAP = ecore_event_type_new();
   EEZE_SENSOR_EVENT_PANNING = ecore_event_type_new();
   EEZE_SENSOR_EVENT_FACEDOWN = ecore_event_type_new();
   EEZE_SENSOR_EVENT_ACCELEROMETER = ecore_event_type_new();

   eeze_sensor_modules_load();

   return EINA_TRUE;
}

