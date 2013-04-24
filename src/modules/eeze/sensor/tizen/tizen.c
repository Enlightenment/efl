#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <system/sensors.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eeze_Sensor.h>
#include "eeze_sensor_private.h"

static int _eeze_sensor_tizen_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...)  EINA_LOG_DOM_ERR(_eeze_sensor_tizen_log_dom, __VA_ARGS__)

static Eeze_Sensor_Module *esensor_module;
/* Tizen sensor handle */
static sensor_h sensor_handle;

static void
_dummy_free(void *user_data EINA_UNUSED, void *func_data EINA_UNUSED)
{
/* Don't free the event data after dispatching the event. We keep track of
 * it on our own
 */
}


/* The Tizen sensor type ENUM has shown to not be stable regarding its
 * numbering scheme so we better translate between the Tizen types and the
 * ones we use here.
 */
static sensor_type_e
eeze_to_tizen(Eeze_Sensor_Type type)
{
   switch (type)
     {
      case EEZE_SENSOR_TYPE_ACCELEROMETER:
        return SENSOR_ACCELEROMETER;

      case EEZE_SENSOR_TYPE_GRAVITY:
        return SENSOR_GRAVITY;

      case EEZE_SENSOR_TYPE_LINEAR_ACCELERATION:
        return SENSOR_LINEAR_ACCELERATION;

      case EEZE_SENSOR_TYPE_DEVICE_ORIENTATION:
        return SENSOR_DEVICE_ORIENTATION;

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

      case EEZE_SENSOR_TYPE_MOTION_PANNING_BROWSE:
        return SENSOR_MOTION_PANNING_BROWSE;

      case EEZE_SENSOR_TYPE_MOTION_TILT:
        return SENSOR_MOTION_TILT;

      case EEZE_SENSOR_TYPE_MOTION_FACEDOWN:
        return SENSOR_MOTION_FACEDOWN;

      case EEZE_SENSOR_TYPE_MOTION_DIRECT_CALL:
        return SENSOR_MOTION_DIRECTCALL;

      case EEZE_SENSOR_TYPE_MOTION_SMART_ALERT:
        return SENSOR_MOTION_SMART_ALERT;

      case EEZE_SENSOR_TYPE_MOTION_NO_MOVE:
        return SENSOR_MOTION_NO_MOVE;

      default:
        ERR("No matching Tizen sensor type available.");
        return -1;
     }
}

/* Reverse mapping from Tizen to eeze sensor types */
static Eeze_Sensor_Type
tizen_to_eeze(sensor_type_e type)
{
   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        return EEZE_SENSOR_TYPE_ACCELEROMETER;

      case SENSOR_GRAVITY:
        return EEZE_SENSOR_TYPE_GRAVITY;

      case SENSOR_LINEAR_ACCELERATION:
        return EEZE_SENSOR_TYPE_LINEAR_ACCELERATION;

      case SENSOR_DEVICE_ORIENTATION:
        return EEZE_SENSOR_TYPE_DEVICE_ORIENTATION;

      case SENSOR_MAGNETIC:
        return EEZE_SENSOR_TYPE_MAGNETIC;

      case SENSOR_ORIENTATION:
        return EEZE_SENSOR_TYPE_ORIENTATION;

      case SENSOR_GYROSCOPE:
        return EEZE_SENSOR_TYPE_GYROSCOPE;

      case SENSOR_LIGHT:
        return EEZE_SENSOR_TYPE_LIGHT;

      case SENSOR_PROXIMITY:
        return EEZE_SENSOR_TYPE_PROXIMITY;

      case SENSOR_MOTION_SNAP:
        return EEZE_SENSOR_TYPE_MOTION_SNAP;

      case SENSOR_MOTION_SHAKE:
        return EEZE_SENSOR_TYPE_MOTION_SHAKE;

      case SENSOR_MOTION_DOUBLETAP:
        return EEZE_SENSOR_TYPE_MOTION_DOUBLETAP;

      case SENSOR_MOTION_PANNING:
        return EEZE_SENSOR_TYPE_MOTION_PANNING;

      case SENSOR_MOTION_PANNING_BROWSE:
        return EEZE_SENSOR_TYPE_MOTION_PANNING_BROWSE;

      case SENSOR_MOTION_TILT:
        return EEZE_SENSOR_TYPE_MOTION_TILT;

      case SENSOR_MOTION_FACEDOWN:
        return EEZE_SENSOR_TYPE_MOTION_FACEDOWN;

      case SENSOR_MOTION_DIRECTCALL:
        return EEZE_SENSOR_TYPE_MOTION_DIRECT_CALL;

      case SENSOR_MOTION_SMART_ALERT:
        return EEZE_SENSOR_TYPE_MOTION_SMART_ALERT;

      case SENSOR_MOTION_NO_MOVE:
        return EEZE_SENSOR_TYPE_MOTION_NO_MOVE;

      default:
        ERR("No matching eeze sensor type available.");
        return -1;
     }
}

/* We receive the timestamp in nanoseconds from the Tizen system lib. Convert
 * it to seconds as floating point value which is used in our public API.
  */
static double
clock_convert(unsigned long long timestamp)
{
   return ((double)timestamp) / 1000000000.0;
}

/* All following callback function work with the same scheme.
 * They are callbacks coming in from the tizen system sensor library. With the
 * data we receive we update the matching sensor object to always have the
 * latest data available. That includes updating the timestamp to show when the
 * data was measured from the underlying system.
 * After that we send out an ecore event to let all interested parties now that
 * new data is available and then stop the sensor server to safe power. It will be
 * started again the next time data gets requested.
 */
static void
accelerometer_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_ACCELEROMETER, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ACCELEROMETER));
}

static void
gravity_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_GRAVITY, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GRAVITY));
}

static void
linear_acceleration_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_LINEAR_ACCELERATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LINEAR_ACCELERATION));
}

static void
device_orientation_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float yaw,
                      float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = yaw;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DEVICE_ORIENTATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_DEVICE_ORIENTATION));
}

static void
magnetic_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_MAGNETIC, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MAGNETIC));
}

static void
orientation_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float azimuth, float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = azimuth;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_ORIENTATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ORIENTATION));
}

static void
gyroscope_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_GYROSCOPE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GYROSCOPE));
}

static void
light_cb(unsigned long long timestamp, float lux, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = lux;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_LIGHT, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LIGHT));
}

static void
proximity_cb(unsigned long long timestamp, float distance, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = distance;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PROXIMITY, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_PROXIMITY));
}

static void
snap_cb(unsigned long long timestamp, sensor_motion_snap_e snap, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = snap;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SNAP, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SNAP));
}

static void
shake_cb(unsigned long long timestamp, sensor_motion_shake_e shake, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = shake;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SHAKE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SHAKE));
}

static void
panning_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING));
}

static void
panning_browse_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING_BROWSE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING_BROWSE));
}

static void
tilt_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_TILT, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_TILT));
}

static void
facedown_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_FACEDOWN, obj, _dummy_free, NULL);
   /* We are not stopping the sensor here because we want to keep it as a motion
    * event coming in at any time.
    */
}

static void
directcall_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DIRECT_CALL, obj, _dummy_free, NULL);
   /* We are not stopping the sensor here because we want to keep it as a motion
    * event coming in at any time.
    */
}

static void
smart_alert_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SMART_ALERT, obj, _dummy_free, NULL);
   /* We are not stopping the sensor here because we want to keep it as a motion
    * event coming in at any time.
    */
}

static void
no_move_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_NO_MOVE, obj, _dummy_free, NULL);
   /* We are not stopping the sensor here because we want to keep it as a motion
    * event coming in at any time.
    */
}

static void
doubletap_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DOUBLETAP, obj, _dummy_free, NULL);
   /* We are not stopping the sensor here because we want to keep it as a motion
    * event coming in at any time.
    */
}

static void
accelerometer_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_accelerometer_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_ACCELEROMETER, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ACCELEROMETER));
}

static void
gravity_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_gravity_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_GRAVITY, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GRAVITY));
}

static void
linear_acceleration_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_linear_acceleration_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_LINEAR_ACCELERATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LINEAR_ACCELERATION));
}

static void
device_orientation_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float yaw,
                           float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_device_orientation_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = yaw;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DEVICE_ORIENTATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_DEVICE_ORIENTATION));
}

static void
magnetic_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_magnetic_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_MAGNETIC, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MAGNETIC));
}

static void
orientation_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float azimuth, float pitch, float roll, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_orientation_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = azimuth;
   obj->data[1] = pitch;
   obj->data[2] = roll;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_ORIENTATION, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_ORIENTATION));
}

static void
gyroscope_read_cb(unsigned long long timestamp, sensor_data_accuracy_e accuracy, float x, float y, float z, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_gyroscope_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   obj->accuracy = accuracy;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->data[2] = z;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_GYROSCOPE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_GYROSCOPE));
}

static void
light_read_cb(unsigned long long timestamp, float lux, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_light_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = lux;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_LIGHT, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_LIGHT));
}

static void
proximity_read_cb(unsigned long long timestamp, float distance, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_proximity_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = distance;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PROXIMITY, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_PROXIMITY));
}

static void
snap_read_cb(unsigned long long timestamp, sensor_motion_snap_e snap, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_snap_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = snap;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SNAP, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SNAP));
}

static void
shake_read_cb(unsigned long long timestamp, sensor_motion_shake_e shake, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_shake_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = shake;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SHAKE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SHAKE));
}

static void
doubletap_read_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_doubletap_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DOUBLETAP, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_DOUBLETAP));
}

static void
panning_read_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_panning_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING));
}

static void
panning_browse_read_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_panning_browse_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_PANNING_BROWSE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_PANNING_BROWSE));
}

static void
tilt_read_cb(unsigned long long timestamp, int x, int y, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_tilt_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->data[0] = x;
   obj->data[1] = y;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_TILT, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_TILT));
}

static void
facedown_read_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_facedown_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_FACEDOWN, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_FACEDOWN));
}

static void
directcall_read_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_directcall_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_DIRECT_CALL, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_DIRECT_CALL));
}

static void
smart_alert_read_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_smart_alert_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_SMART_ALERT, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_SMART_ALERT));
}

static void
no_move_read_cb(unsigned long long timestamp, void *user_data)
{
   Eeze_Sensor_Obj *obj = user_data;

   sensor_motion_no_move_unset_cb(sensor_handle);

   if (obj == NULL)
     {
        ERR("No matching sensor object given.");
        return;
     }
   /* We have to set this ourselves because we don't get it for this type */
   obj->accuracy = -1;
   obj->timestamp = clock_convert(timestamp);
   ecore_event_add(EEZE_SENSOR_EVENT_NO_MOVE, obj, _dummy_free, NULL);
   sensor_stop(sensor_handle, eeze_to_tizen(EEZE_SENSOR_TYPE_MOTION_NO_MOVE));
}

/* Synchronous read function for sensor data. It uses the blocking calls to read
 * out the data and returns after it finishes the readout. Be aware that this
 * might take quite some time depending on the sensor and how it is connected to
 * the system. Normally it is better to use the asynchronous reading functions.
 */
static Eina_Bool
eeze_sensor_tizen_read(Eeze_Sensor_Obj *obj)
{
   sensor_data_accuracy_e accuracy;
   float x, y, z;
   float azimuth, pitch, roll, lux, distance, yaw;
   bool supported;
   sensor_type_e type;

   type = eeze_to_tizen(obj->type);

   /* Don't attempt to do anything if the sensor is not available on the system
    * we are running on.
    */
   sensor_is_supported(type, &supported);
   if (!supported)
     {
        ERR("Sensor type %d not available on this device.", type);
        return EINA_FALSE;
     }

   sensor_start(sensor_handle, type);

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_GRAVITY:
        sensor_gravity_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_LINEAR_ACCELERATION:
        sensor_linear_acceleration_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_DEVICE_ORIENTATION:
        sensor_device_orientation_read_data(sensor_handle, &accuracy, &yaw, &pitch, &roll);
        obj->accuracy = accuracy;
        obj->data[0] = yaw;
        obj->data[1] = pitch;
        obj->data[2] = roll;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_read_data(sensor_handle, &accuracy, &azimuth, &pitch, &roll);
        obj->accuracy = accuracy;
        obj->data[0] = azimuth;
        obj->data[1] = pitch;
        obj->data[2] = roll;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_read_data(sensor_handle, &accuracy, &x, &y, &z);
        obj->accuracy = accuracy;
        obj->data[0] = x;
        obj->data[1] = y;
        obj->data[2] = z;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_LIGHT:
        sensor_light_read_data(sensor_handle, &lux);
        /* As we do not get any accuracy value from the system we go with -1 */
        obj->accuracy = -1;
        obj->data[0] = lux;
        obj->timestamp = ecore_time_get();
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_read_data(sensor_handle, &distance);
        /* As we do not get any accuracy value from the system we go with -1 */
        obj->accuracy = -1;
        obj->data[0] = distance;
        obj->timestamp = ecore_time_get();
        break;

      default:
        ERR("Not possible to read from this sensor type.");
        return EINA_FALSE;
     }

   sensor_stop(sensor_handle, type);
   return EINA_TRUE;
}

// FIXME ho to handle this without explicit callback setting
#if 0
static Eina_Bool
eeze_sensor_tizen_cb_set(Eeze_Sensor *handle, Eeze_Sensor_Type sensor_type, void *cb_function, void *user_data)
{
   sensor_type_e type;

   type = eeze_to_tizen(sensor_type);

   handle->cb_function = cb_function;

   sensor_start(sensor_handle, type);

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_set_cb(sensor_handle, 0, accelerometer_cb, handle);
        break;

      case SENSOR_GRAVITY:
        sensor_gravity_set_cb(sensor_handle, 0, gravity_cb, handle);
        break;

      case SENSOR_LINEAR_ACCELERATION:
        sensor_linear_acceleration_set_cb(sensor_handle, 0, linear_acceleration_cb, handle);
        break;

      case SENSOR_DEVICE_ORIENTATION:
        sensor_device_orientation_set_cb(sensor_handle, 0, device_orientation_cb, handle);
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_set_cb(sensor_handle, 0, magnetic_cb, handle);
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_set_cb(sensor_handle, 0, orientation_cb, handle);
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_set_cb(sensor_handle, 0, gyroscope_cb, handle);
        break;

      case SENSOR_LIGHT:
        sensor_light_set_cb(sensor_handle, 0, light_cb, handle);
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_set_cb(sensor_handle, 0, proximity_cb, handle);
        break;

      case SENSOR_MOTION_SNAP:
        sensor_motion_snap_set_cb(sensor_handle, snap_cb, handle);
        break;

      case SENSOR_MOTION_SHAKE:
        sensor_motion_shake_set_cb(sensor_handle, shake_cb, handle);
        break;

      case SENSOR_MOTION_DOUBLETAP:
        sensor_motion_doubletap_set_cb(sensor_handle, doubletap_cb, handle);
        break;

      case SENSOR_MOTION_PANNING:
        sensor_motion_panning_set_cb(sensor_handle, panning_cb, handle);
        break;

      case SENSOR_MOTION_PANNING_BROWSE:
        sensor_motion_panning_browse_set_cb(sensor_handle, panning_browse_cb, handle);
        break;

      case SENSOR_MOTION_TILT:
        sensor_motion_tilt_set_cb(sensor_handle, tilt_cb, handle);
        break;

      case SENSOR_MOTION_FACEDOWN:
        sensor_motion_facedown_set_cb(sensor_handle, facedown_cb, handle);
        break;

      case SENSOR_MOTION_DIRECTCALL:
        sensor_motion_directcall_set_cb(sensor_handle, directcall_cb, handle);
        break;

      case SENSOR_MOTION_SMART_ALERT:
        sensor_motion_smart_alert_set_cb(sensor_handle, smart_alert_cb, handle);
        break;

      case SENSOR_MOTION_NO_MOVE:
        sensor_motion_no_move_set_cb(sensor_handle, no_move_cb, handle);
        break;

      default:
        ERR("Not possible to set a callback for this sensor type.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}
#endif

/* For the asynchronous reads we only start the sensor her and trigger the
 * readout. The callbacks above are actually taking care about putting the data
 * into the matching sensor objects and informing all subscribers with an ecore
 * event. The public API function does actually return right away with the cached
 * data. This is handled in the core and not in the different modules though.
 */
static Eina_Bool
eeze_sensor_tizen_async_read(Eeze_Sensor_Obj *obj, void *user_data)
{
   sensor_type_e type;

   if (user_data)
     obj->user_data = user_data;

   type = eeze_to_tizen(obj->type);

   sensor_start(sensor_handle, type);

   switch (type)
     {
      case SENSOR_ACCELEROMETER:
        sensor_accelerometer_set_cb(sensor_handle, 0, accelerometer_read_cb, obj);
        break;

      case SENSOR_GRAVITY:
        sensor_gravity_set_cb(sensor_handle, 0, gravity_read_cb, obj);
        break;

      case SENSOR_LINEAR_ACCELERATION:
        sensor_linear_acceleration_set_cb(sensor_handle, 0, linear_acceleration_read_cb, obj);
        break;

      case SENSOR_DEVICE_ORIENTATION:
        sensor_device_orientation_set_cb(sensor_handle, 0, device_orientation_read_cb, obj);
        break;

      case SENSOR_MAGNETIC:
        sensor_magnetic_set_cb(sensor_handle, 0, magnetic_read_cb, obj);
        break;

      case SENSOR_ORIENTATION:
        sensor_orientation_set_cb(sensor_handle, 0, orientation_read_cb, obj);
        break;

      case SENSOR_GYROSCOPE:
        sensor_gyroscope_set_cb(sensor_handle, 0, gyroscope_read_cb, obj);
        break;

      case SENSOR_LIGHT:
        sensor_light_set_cb(sensor_handle, 0, light_read_cb, obj);
        break;

      case SENSOR_PROXIMITY:
        sensor_proximity_set_cb(sensor_handle, 0, proximity_read_cb, obj);
        break;

      case SENSOR_MOTION_SNAP:
        sensor_motion_snap_set_cb(sensor_handle, snap_read_cb, obj);
        break;

      case SENSOR_MOTION_SHAKE:
        sensor_motion_shake_set_cb(sensor_handle, shake_read_cb, obj);
        break;

      case SENSOR_MOTION_DOUBLETAP:
        sensor_motion_doubletap_set_cb(sensor_handle, doubletap_read_cb, obj);
        break;

      case SENSOR_MOTION_PANNING:
        sensor_motion_panning_set_cb(sensor_handle, panning_read_cb, obj);
        break;

      case SENSOR_MOTION_PANNING_BROWSE:
        sensor_motion_panning_browse_set_cb(sensor_handle, panning_browse_read_cb, obj);
        break;

      case SENSOR_MOTION_TILT:
        sensor_motion_tilt_set_cb(sensor_handle, tilt_read_cb, obj);
        break;

      case SENSOR_MOTION_FACEDOWN:
        sensor_motion_facedown_set_cb(sensor_handle, facedown_read_cb, obj);
        break;

      case SENSOR_MOTION_DIRECTCALL:
        sensor_motion_directcall_set_cb(sensor_handle, directcall_read_cb, obj);
        break;

      case SENSOR_MOTION_SMART_ALERT:
        sensor_motion_smart_alert_set_cb(sensor_handle, smart_alert_read_cb, obj);
        break;

      case SENSOR_MOTION_NO_MOVE:
        sensor_motion_no_move_set_cb(sensor_handle, no_move_read_cb, obj);
        break;

      default:
        ERR("Not possible to set a callback for this sensor type.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* Go through all potential Tizen sensor and test if they are available on the
 * system we are running on. If yes, create a matching sensor object and put it
 * the list of available sensor for the core.
 */
static void
eeze_sensor_tizen_sensors_find(void)
{
   sensor_type_e type;
   bool supported = 0;

   for (type = SENSOR_ACCELEROMETER; type <= SENSOR_LAST; type++)
     {
        sensor_is_supported(type, &supported);
        if (supported)
          {
             Eeze_Sensor_Obj *obj = calloc(1, sizeof(Eeze_Sensor_Obj));
             obj->type = tizen_to_eeze(type);
             esensor_module->sensor_list = eina_list_append(esensor_module->sensor_list, obj);
          }
     }
}

/* Cleanup when getting the shutdown callback from core */
static Eina_Bool
eeze_sensor_tizen_shutdown(void)
{
   Eeze_Sensor_Obj *obj;

   EINA_LIST_FREE(esensor_module->sensor_list, obj)
     free(obj);

   sensor_stop(sensor_handle, SENSOR_MOTION_FACEDOWN);
   sensor_stop(sensor_handle, SENSOR_MOTION_DOUBLETAP);
   if (sensor_destroy(sensor_handle) != 0)
     {
        ERR("Failing to destroy sensor handle.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* Callback from core once we registered as a module. Create the Tizen sensor
 * handle and find all available sensors.
 */
static Eina_Bool
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
   sensor_motion_doubletap_set_cb(sensor_handle, doubletap_cb,
                                  eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_DOUBLETAP));
   sensor_motion_facedown_set_cb(sensor_handle, facedown_cb,
                                 eeze_sensor_obj_get(EEZE_SENSOR_TYPE_MOTION_FACEDOWN));

   return EINA_TRUE;
}

/* This function gets called when the module is loaded from the disk. Its the
 * entry point to anything in this module. After setting ourself up we register
 * into the core of eeze sensor to make our functionality available.
 */
static Eina_Bool
sensor_tizen_init(void)
{

   _eeze_sensor_tizen_log_dom = eina_log_domain_register("eeze_sensor_tizen", EINA_COLOR_BLUE);
   if (_eeze_sensor_tizen_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_sensor_tizen' log domain.");
        return EINA_FALSE;
     }

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

/* Cleanup when the module gets unloaded. Unregister ourself from the core to
 * avoid calls into a not loaded module.
 */
static void
sensor_tizen_shutdown(void)
{
   Eeze_Sensor_Obj *sens;

   eeze_sensor_module_unregister("tizen");
   EINA_LIST_FREE(esensor_module->sensor_list, sens)
      free(sens);

   eina_log_domain_unregister(_eeze_sensor_tizen_log_dom);

   free(esensor_module);
   esensor_module = NULL;

   _eeze_sensor_tizen_log_dom = -1;
}

EINA_MODULE_INIT(sensor_tizen_init);
EINA_MODULE_SHUTDOWN(sensor_tizen_shutdown);

