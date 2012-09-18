#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <time.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eeze_Sensor.h>
#include "eeze_sensor_private.h"

Eeze_Sensor_Module *esensor_module;

Eina_Bool
fake_init(void)
{
   /* Set a list with fake sensors */
   Eeze_Sensor_Type type;

   for (type = 0; type <= EEZE_SENSOR_TYPE_LAST; type++)
     {
        Eeze_Sensor_Obj *obj = calloc(1, sizeof(Eeze_Sensor_Obj));
        obj->type = type;
        esensor_module->sensor_list = eina_list_append(esensor_module->sensor_list, obj);
     }

   return EINA_TRUE;
}

Eina_Bool
fake_shutdown(void)
{
   return EINA_TRUE;
}

Eina_Bool
fake_read(Eeze_Sensor_Type sensor_type, Eeze_Sensor_Obj *lobj)
{
   Eeze_Sensor_Obj *obj;

   obj = eeze_sensor_obj_get(sensor_type);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list");
        return EINA_FALSE;
     }

   switch (sensor_type)
     {
      case EEZE_SENSOR_TYPE_ACCELEROMETER:
      case EEZE_SENSOR_TYPE_MAGNETIC:
      case EEZE_SENSOR_TYPE_ORIENTATION:
      case EEZE_SENSOR_TYPE_GYROSCOPE:
        obj->accuracy = 0;
        obj->data[0] = 7;
        obj->data[1] = 23;
        obj->data[2] = 42;
        obj->timestamp = time(NULL);
        break;

      case EEZE_SENSOR_TYPE_LIGHT:
      case EEZE_SENSOR_TYPE_PROXIMITY:
      case EEZE_SENSOR_TYPE_BAROMETER:
      case EEZE_SENSOR_TYPE_TEMPERATURE:
        obj->accuracy = 0;
        obj->data[0] = 7;
        obj->timestamp = time(NULL);
        break;

      default:
        ERR("Not possible to read from this sensor type.");
        return EINA_FALSE;
     }

   memcpy(lobj, obj, sizeof(Eeze_Sensor_Obj));

   return EINA_TRUE;
}

Eina_Bool
fake_async_read(Eeze_Sensor_Type sensor_type, void *user_data EINA_UNUSED)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(sensor_type);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return EINA_FALSE;
     }

   switch (sensor_type)
     {
      case EEZE_SENSOR_TYPE_ACCELEROMETER:
         ecore_event_add(EEZE_SENSOR_EVENT_ACCELEROMETER, obj, NULL, NULL);
      case EEZE_SENSOR_TYPE_MAGNETIC:
      case EEZE_SENSOR_TYPE_ORIENTATION:
      case EEZE_SENSOR_TYPE_GYROSCOPE:
        obj->accuracy = 0;
        obj->data[0] = 7;
        obj->data[1] = 23;
        obj->data[2] = 42;
        obj->timestamp = time(NULL);
        break;

      case EEZE_SENSOR_TYPE_LIGHT:
      case EEZE_SENSOR_TYPE_PROXIMITY:
      case EEZE_SENSOR_TYPE_BAROMETER:
      case EEZE_SENSOR_TYPE_TEMPERATURE:
        obj->accuracy = 0;
        obj->data[0] = 7;
        obj->timestamp = time(NULL);
        break;

      case EEZE_SENSOR_TYPE_MOTION_SNAP:
        break;

      case EEZE_SENSOR_TYPE_MOTION_SHAKE:
        break;

      case EEZE_SENSOR_TYPE_MOTION_PANNING:
        break;

      default:
        ERR("Not possible to set a callback for this sensor type.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
sensor_fake_init(void)
{
   /* Check to avoid multi-init */
   if (esensor_module) return EINA_FALSE;

   /* Set module function pointer to allow calls into the module */
   esensor_module = calloc(1, sizeof(Eeze_Sensor_Module));
   if (!esensor_module) return EINA_FALSE;

   esensor_module->init = fake_init;
   esensor_module->shutdown = fake_shutdown;
   esensor_module->read = fake_read;
   esensor_module->async_read = fake_async_read;

   if (!eeze_sensor_module_register("fake", esensor_module))
     {
        ERR("Failed to register fake module.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
sensor_fake_shutdown(void)
{
   eeze_sensor_module_unregister("fake");
   free(esensor_module);
   esensor_module = NULL;
}

EINA_MODULE_INIT(sensor_fake_init);
EINA_MODULE_SHUTDOWN(sensor_fake_shutdown);
