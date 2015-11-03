#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_TIZEN_SENSOR
#include "ecordova_devicemotion_private.h"

#define MY_CLASS ECORDOVA_DEVICEMOTION_CLASS
#define MY_CLASS_NAME "Ecordova_DeviceMotion"

static void _on_sensor_event(sensor_h, sensor_event_s *, void *);
static void _hash_data_free(sensor_listener_h);
static sensor_listener_h _create_listener(Ecordova_DeviceMotion_Data *);

static Eo_Base *
_ecordova_devicemotion_eo_base_constructor(Eo *obj,
                                           Ecordova_DeviceMotion_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->sensor = NULL;
   pd->listeners = eina_hash_int32_new(EINA_FREE_CB(_hash_data_free));

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_devicemotion_constructor(Eo *obj EINA_UNUSED,
                                   Ecordova_DeviceMotion_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_devicemotion_eo_base_destructor(Eo *obj,
                                          Ecordova_DeviceMotion_Data *pd)
{
   DBG("(%p)", obj);

   eina_hash_free(pd->listeners);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_devicemotion_current_acceleration_get(Eo *obj,
                                                Ecordova_DeviceMotion_Data *pd)
{
   sensor_listener_h listener = _create_listener(pd);
   if (!listener)
     {
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEMOTION_EVENT_ERROR,
                                          NULL));
        return;
     }

   int ret = sensor_listener_start(listener);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   sensor_event_s event;
   ret = sensor_listener_read_data(listener, &event);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   Ecordova_DeviceMotion_Acceleration acceleration = {
     .x = event.values[0],
     .y = event.values[1],
     .z = event.values[2],
     .timestamp = event.timestamp
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEMOTION_EVENT_CURRENT_SUCCESS,
                                     &acceleration));

on_error:
   ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN(SENSOR_ERROR_NONE == ret);
}

static Ecordova_DeviceMotion_WatchID
_ecordova_devicemotion_acceleration_watch(Eo *obj,
                                          Ecordova_DeviceMotion_Data *pd,
                                          const Ecordova_DeviceMotion_AccelerometerOptions *options)
{
   sensor_listener_h listener = _create_listener(pd);
   if (!listener)
     {
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEMOTION_EVENT_ERROR,
                                          NULL));
        return 0;
     }

   const Ecordova_DeviceMotion_AccelerometerOptions default_options = {
     .frequency = 10000
   };
   if (!options)
     options = &default_options;

   int ret = sensor_listener_set_event_cb(listener,
                                          options->frequency,
                                          _on_sensor_event,
                                          pd->obj);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   ret = sensor_listener_start(listener);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   static Ecordova_DeviceMotion_WatchID id = 0;
   ++id;
   eina_hash_add(pd->listeners, &id, listener);
   return id;

on_error:
   ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(SENSOR_ERROR_NONE == ret, 0);
   return 0;
}

static void
_ecordova_devicemotion_watch_clear(Eo *obj,
                                   Ecordova_DeviceMotion_Data *pd,
                                   Ecordova_DeviceMotion_WatchID watch_id)
{
   sensor_listener_h listener = eina_hash_find(pd->listeners, &watch_id);
   if (!listener)
     {
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEMOTION_EVENT_ERROR,
                                          NULL));
        return;
     }

   Eina_Bool ret = eina_hash_del(pd->listeners, &watch_id, NULL);
   EINA_SAFETY_ON_FALSE_RETURN(ret);
}

static sensor_listener_h
_create_listener(Ecordova_DeviceMotion_Data *pd)
{
   int ret = SENSOR_ERROR_NONE;

   if (!pd->sensor)
     {
        ret = sensor_get_default_sensor(SENSOR_ACCELEROMETER, &pd->sensor);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(SENSOR_ERROR_NONE == ret, NULL);
     }

   sensor_listener_h listener = NULL;
   ret = sensor_create_listener(pd->sensor, &listener);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(SENSOR_ERROR_NONE == ret, NULL);

   return listener;
}

static void
_on_sensor_event(sensor_h sensor EINA_UNUSED,
                 sensor_event_s *event,
                 void *user_data)
{
   Eo *obj = user_data;

   Ecordova_DeviceMotion_Acceleration acceleration = {
     .x = event->values[0],
     .y = event->values[1],
     .z = event->values[2],
     .timestamp = event->timestamp
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEMOTION_EVENT_WATCH_SUCCESS,
                                     &acceleration));
}

static void
_hash_data_free(sensor_listener_h listener)
{
   int ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN(SENSOR_ERROR_NONE == ret);
}

#include "ecordova_devicemotion.eo.c"
#endif
