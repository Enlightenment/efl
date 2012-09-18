#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <system/sensors.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Eina.h>
#include <Eeze_Sensor.h>
#include "eeze_sensor_private.h"

Eeze_Sensor_Module *esensor_module;
sensor_h sensor_handle; // Tizen sensor handle

static sensor_type_e
eeze_to_tizen(Eeze_Sensor_Type type)
{
   switch (type)
     {
      case EEZE_SENSOR_TYPE_ACCELEROMETER:
        return SENSOR_ACCELEROMETER;

      case EEZE_SENSOR_TYPE_MAGNETIC:
        return SENSOR_MAGNETIC;

      case EEZE_SENSOR_TYPE_ORIENTATION:
        return SENSOR_ORIENTATION;

      case EEZE_SENSOR_TYPE_GYROSCOPE:
        return SENSOR_GYROSCOPE;

      case EEZE_SENSOR_TYPE_LIGHT:
        return SENSOR_LIGHT;

      case EEZE_SENSOR_TYPE_PROXIMITY:
        return SENSOR_PROXIMITY;

      case EEZE_SENSOR_TYPE_MOTION_SNAP:
        return SENSOR_MOTION_SNAP;

      case EEZE_SENSOR_TYPE_MOTION_SHAKE:
        return SENSOR_MOTION_SHAKE;

      case EEZE_SENSOR_TYPE_MOTION_DOUBLETAP:
        return SENSOR_MOTION_DOUBLETAP;

      case EEZE_SENSOR_TYPE_MOTION_PANNING:
        return SENSOR_MOTION_PANNING;

      case EEZE_SENSOR_TYPE_MOTION_FACEDOWN:
        return SENSOR_MOTION_FACEDOWN;

      default:
        ERR("No matching Tizen sensor type available.");
        return -1;
     }
}

void
accelerometer_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_ACCELEROMETER);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_ACCELEROMETER, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ACCELEROMETER));
}

void
magnetic_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MAGNETIC);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_MAGNETIC, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MAGNETIC));
}

void
orientation_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float azimuth, float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_ORIENTATION);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = azimuth;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_ORIENTATION, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ORIENTATION));
}

void
gyroscope_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_GYROSCOPE);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_GYROSCOPE, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GYROSCOPE));
}

void
light_cb(unsigned long long timestamp, float lux, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_LIGHT);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = 0;
   obj->data[0] = lux;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_LIGHT, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LIGHT));
}

void
proximity_cb(unsigned long long timestamp, float distance, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_PROXIMITY);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = 0;
   obj->data[0] = distance;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_PROXIMITY, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_PROXIMITY));
}

void
snap_cb(unsigned long long timestamp, sensor_motion_snap_e snap, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_SNAP);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = snap;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_SNAP, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SNAP));
}

void
shake_cb(unsigned long long timestamp, sensor_motion_shake_e shake, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_SHAKE);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = shake;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_SHAKE, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SHAKE));
}

void
panning_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_PANNING);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING));
}

void
facedown_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_FACEDOWN);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_FACEDOWN, obj, NULL, NULL);
}

void
doubletap_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_DOUBLETAP);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_DOUBLETAP, obj, NULL, NULL);
}

void
accelerometer_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_accelerometer_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_ACCELEROMETER);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_ACCELEROMETER, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ACCELEROMETER));
}

void
magnetic_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_magnetic_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MAGNETIC);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_MAGNETIC, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MAGNETIC));
}

void
orientation_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float azimuth, float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_orientation_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_ORIENTATION);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = azimuth;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_ORIENTATION, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ORIENTATION));
}

void
gyroscope_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_gyroscope_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_GYROSCOPE);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_GYROSCOPE, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GYROSCOPE));
}

void
light_read_cb(unsigned long long timestamp, float lux, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_light_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_LIGHT);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = 0;
   obj->data[0] = lux;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_LIGHT, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LIGHT));
}

void
proximity_read_cb(unsigned long long timestamp, float distance, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_proximity_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_PROXIMITY);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->accuracy = 0;
   obj->data[0] = distance;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_PROXIMITY, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_PROXIMITY));
}

void
snap_read_cb(unsigned long long timestamp, sensor_motion_snap_e snap, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_motion_snap_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_SNAP);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = snap;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_SNAP, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SNAP));
}

void
shake_read_cb(unsigned long long timestamp, sensor_motion_shake_e shake, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_motion_shake_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_SHAKE);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = shake;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_SHAKE, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SHAKE));
}

void
panning_read_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = NULL;

   sensor_motion_panning_unset_cb(sensor_handle);

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_PANNING);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return;
     }
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = timestamp;
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING));
}

void
facedown_read_cb(unsigned long long timestamp, void *user_data)
{
   sensor_motion_facedown_unset_cb(sensor_handle);
   ecore_event_add(EEZE_SENSOR_EVENT_FACEDOWN, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_MOTION_TYPE_FACEDOWN));
}

void
doubletap_read_cb(unsigned long long timestamp, void *user_data)
{
   sensor_motion_doubletap_unset_cb(sensor_handle);
   ecore_event_add(EEZE_SENSOR_EVENT_DOUBLETAP, obj, NULL, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_MOTION_TYPE_DOUBLETAP));
}

Eina_Bool
eeze_sensor_tizen_read(Eeze_Sensor_Type sensor_type, Eeze_Sensor_Obj *lobj)
{
   sensor_data_accuracy_e accuracy;
   float x, y, z;
   float azimuth, pitch, roll, lux, distance;
   bool supported;
   sensor_type_e type;
   Eeze_Sensor_Obj *obj;

   type = eeze_to_tizen(sensor_type);

   sensor_is_supported(type, &supported);
   if (!supported)
     {
        ERR("Sensor type %d not available on this device.", type);
        return EINA_FALSE;
     }

   sensor_start(handle->sensor_handle, type);
   obj = eeze_sensor_obj_get(sensor_type);
   if (obj == NULL)
     {
        ERR("No matching sensor object found in list.");
        return EINA_FALSE;
     }

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_read_data(sensor_handle, &accuracy, &azimuth, &pitch, &roll);
        obj->accuracy = accuracy;
        obj->data[0] = azimuth;
        obj->data[1] = pitch;
        obj->data[2] = roll;
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        break;

      case SENSOR_LIGHT:
        sensor_light_read_data(sensor_handle, &lux);
        obj->accuracy = 0;
        obj->data[0] = lux;
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_read_data(sensor_handle, &distance);
        obj->accuracy = 0;
        obj->data[0] = distance;
        break;

      default:
        ERR("Not possible to read from this sensor type.");
        return EINA_FALSE;
     }

   memcpy(lobj, obj, sizeof(Eeze_Sensor_Obj));

   sensor_stop(sensor_handle, type);
   return EINA_TRUE;
}

// FIXME ho to handle this without explicit callback setting
#if 0
Eina_Bool
eeze_sensor_tizen_cb_set(Eeze_Sensor *handle, Eeze_Sensor_Type sensor_type, void *cb_function, void *user_data)
{
   sensor_type_e type;

   type = eeze_to_tizen(sensor_type);

   handle->cb_function = cb_function;

   sensor_start(handle->sensor_handle, type);

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_set_cb(handle->sensor_handle, 0, accelerometer_cb, handle);
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_set_cb(handle->sensor_handle, 0, magnetic_cb, handle);
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_set_cb(handle->sensor_handle, 0, orientation_cb, handle);
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_set_cb(handle->sensor_handle, 0, gyroscope_cb, handle);
        break;

      case SENSOR_LIGHT:
        sensor_light_set_cb(handle->sensor_handle, 0, light_cb, handle);
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_set_cb(handle->sensor_handle, 0, proximity_cb, handle);
        break;

      case SENSOR_MOTION_SNAP:
        sensor_motion_snap_set_cb(handle->sensor_handle, snap_cb, handle);
        break;

      case SENSOR_MOTION_SHAKE:
        sensor_motion_shake_set_cb(handle->sensor_handle, shake_cb, handle);
        break;

      case SENSOR_MOTION_PANNING:
        sensor_motion_panning_set_cb(handle->sensor_handle, panning_cb, handle);
        break;

      default:
        ERR("Not possible to set a callback for this sensor type.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}
#endif

Eina_Bool
eeze_sensor_tizen_async_read(Eeze_Sensor_Type sensor_type, void *user_data)
{
   sensor_type_e type;

   type = eeze_to_tizen(sensor_type);

   sensor_start(sensor_handle, type);

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_set_cb(sensor_handle, 0, accelerometer_read_cb, NULL);
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_set_cb(sensor_handle, 0, magnetic_read_cb, NULL);
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_set_cb(sensor_handle, 0, orientation_read_cb, NULL);
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_set_cb(sensor_handle, 0, gyroscope_read_cb, NULL);
        break;

      case SENSOR_LIGHT:
        sensor_light_set_cb(sensor_handle, 0, light_read_cb, NULL);
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_set_cb(sensor_handle, 0, proximity_read_cb, NULL);
        break;

      case SENSOR_MOTION_SNAP:
        sensor_motion_snap_set_cb(sensor_handle, snap_read_cb, NULL);
        break;

      case SENSOR_MOTION_SHAKE:
        sensor_motion_shake_set_cb(sensor_handle, shake_read_cb, NULL);
        break;

      case SENSOR_MOTION_DOUBLETAP:
        sensor_motion_doubletap_set_cb(sensor_handle, doubletap_read_cb, NULL);
        break;

      case SENSOR_MOTION_PANNING:
        sensor_motion_panning_set_cb(sensor_handle, panning_read_cb, NULL);
        break;

      case SENSOR_MOTION_FACEDOWN:
        sensor_motion_facedown_set_cb(sensor_handle, facedown_read_cb, NULL);
        break;

      default:
        ERR("Not possible to set a callback for this sensor type.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
eeze_sensor_tizen_sensors_find(void)
{
   sensor_type_e type;
   bool supported = 0;

   /* FIXME: Make this safe against changes in the enum. But how? */
   for (type = SENSOR_ACCELEROMETER; type <= SENSOR_MOTION_FACEDOWN; type++)
     {
        sensor_is_supported(type, &supported);
        if (supported)
          {
             Eeze_Sensor_Obj *obj = calloc(1, sizeof(Eeze_Sensor_Obj));
             obj->type = type;
             esensor_module->sensor_list = eina_list_append(esensor_module->sensor_list, obj);
          }
     }
}

Eina_Bool
eeze_sensor_tizen_shutdown(void)
{
   Eeze_Sensor_Obj *obj;

   EINA_LIST_FREE(esensor_module->sensor_list, obj)
     free(obj);

   sensor_start(sensor_handle, SENSOR_MOTION_FACEDOWN);
   sensor_start(sensor_handle, SENSOR_MOTION_DOUBLETAP);
   sensor_start(sensor_handle, SENSOR_MOTION_SHAKE);
   if (sensor_destroy(sensor_handle) != 0)
     {
        ERR("Failing to destroy sensor handle.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
eeze_sensor_tizen_init(void)
{

   if (sensor_create(&sensor_handle) != 0)
     {
        ERR("Failing to create sensor handle.");
        return EINA_FALSE;
     }

   eeze_sensor_tizen_sensors_find();

   /* FIXME add other motion events in here */
   sensor_start(sensor_handle, SENSOR_MOTION_FACEDOWN);
   sensor_start(sensor_handle, SENSOR_MOTION_DOUBLETAP);
   sensor_motion_doubletap_set_cb(sensor_handle, doubletap_cb, handle);
   sensor_motion_facedown_set_cb(sensor_handle, facedown_cb, handle);

   return EINA_TRUE;
}

Eina_Bool
sensor_tizen_init(void)
{
   /* Check to avoid multi-init */
   if (esensor_module) return EINA_FALSE;

   /* Set module function pointer to allow calls into the module */
   esensor_module = calloc(1, sizeof(Eeze_Sensor_Module));
   if (!esensor_module) return EINA_FALSE;

   esensor_module->init = eeze_sensor_tizen_init;
   esensor_module->shutdown = eeze_sensor_tizen_shutdown;
   esensor_module->read = eeze_sensor_tizen_read;
   esensor_module->async_read = eeze_sensor_tizen_async_read;

   if (!eeze_sensor_module_register("tizen", esensor_module))
     {
        ERR("Failed to register tizen module");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
sensor_tizen_shutdown(void)
{
   sensor_stop(sensor_handle, SENSOR_MOTION_FACEDOWN);
   sensor_stop(sensor_handle, SENSOR_MOTION_DOUBLETAP);
   eeze_sensor_module_unregister("tizen");
   free(esensor_module);
   esensor_module = NULL;
}

EINA_MODULE_INIT(sensor_tizen_init);
EINA_MODULE_SHUTDOWN(sensor_tizen_shutdown);

