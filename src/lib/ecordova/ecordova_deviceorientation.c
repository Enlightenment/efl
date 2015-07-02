#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_deviceorientation_private.h"

#define MY_CLASS ECORDOVA_DEVICEORIENTATION_CLASS
#define MY_CLASS_NAME "Ecordova_DeviceOrientation"

static void _on_sensor_event(sensor_h, sensor_event_s *, void *);
static void _hash_data_free(sensor_listener_h);
static sensor_listener_h _create_listener(Ecordova_DeviceOrientation_Data *);

static Eo_Base *
_ecordova_deviceorientation_eo_base_constructor(Eo *obj,
                                                Ecordova_DeviceOrientation_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->sensor = NULL;
   pd->listeners = eina_hash_int32_new(EINA_FREE_CB(_hash_data_free));

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_deviceorientation_constructor(Eo *obj EINA_UNUSED,
                                        Ecordova_DeviceOrientation_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_deviceorientation_eo_base_destructor(Eo *obj,
                                               Ecordova_DeviceOrientation_Data *pd)
{
   DBG("(%p)", obj);

   eina_hash_free(pd->listeners);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_deviceorientation_current_heading_get(Eo *obj,
                                                Ecordova_DeviceOrientation_Data *pd)
{
   sensor_listener_h listener = _create_listener(pd);
   if (!listener)
     {
        Ecordova_DeviceOrientation_Error error = ECORDOVA_DEVICEORIENTATION_ERROR_COMPASS_INTERNAL_ERR;
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_ERROR,
                                          &error));
        return;
     }

   int ret = sensor_listener_start(listener);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   sensor_event_s event;
   ret = sensor_listener_read_data(listener, &event);
   EINA_SAFETY_ON_FALSE_GOTO(SENSOR_ERROR_NONE == ret, on_error);

   // TODO: check the correct values
   Ecordova_DeviceOrientation_Heading orientation = {
     .magnetic_heading = event.values[0],
     .true_heading = event.values[0],
     .heading_accuracy = 0,
     .timestamp = event.timestamp
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_CURRENT_SUCCESS,
                                     &orientation));

on_error:
   ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN(SENSOR_ERROR_NONE == ret);
}

static Ecordova_DeviceOrientation_WatchID
_ecordova_deviceorientation_heading_watch(Eo *obj,
                                          Ecordova_DeviceOrientation_Data *pd,
                                          const Ecordova_DeviceOrientation_OrientationOptions *options)
{
   sensor_listener_h listener = _create_listener(pd);
   if (!listener)
     {
        Ecordova_DeviceOrientation_Error error = ECORDOVA_DEVICEORIENTATION_ERROR_COMPASS_INTERNAL_ERR;
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_ERROR,
                                          &error));
        return 0;
     }

   const Ecordova_DeviceOrientation_OrientationOptions default_options = {
     .frequency = 100
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

   static Ecordova_DeviceOrientation_WatchID id = 0;
   ++id;
   eina_hash_add(pd->listeners, &id, listener);
   return id;

on_error:
   ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(SENSOR_ERROR_NONE == ret, 0);
   return 0;
}

static void
_ecordova_deviceorientation_watch_clear(Eo *obj,
                                        Ecordova_DeviceOrientation_Data *pd,
                                        Ecordova_DeviceOrientation_WatchID watch_id)
{
   sensor_listener_h listener = eina_hash_find(pd->listeners, &watch_id);
   if (!listener)
     {
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_ERROR,
                                          NULL));
        return;
     }

   Eina_Bool ret = eina_hash_del(pd->listeners, &watch_id, NULL);
   EINA_SAFETY_ON_FALSE_RETURN(ret);
}

static sensor_listener_h
_create_listener(Ecordova_DeviceOrientation_Data *pd)
{
   int ret = SENSOR_ERROR_NONE;

   if (!pd->sensor)
     {
        ret = sensor_get_default_sensor(SENSOR_ORIENTATION, &pd->sensor);
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

   // TODO: check the correct values
   Ecordova_DeviceOrientation_Heading orientation = {
     .magnetic_heading = event->values[0],
     .true_heading = event->values[0],
     .heading_accuracy = 0,
     .timestamp = event->timestamp
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_WATCH_SUCCESS,
                                     &orientation));
}

static void
_hash_data_free(sensor_listener_h listener)
{
   int ret = sensor_destroy_listener(listener);
   EINA_SAFETY_ON_FALSE_RETURN(SENSOR_ERROR_NONE == ret);
}

#include "ecordova_deviceorientation.eo.c"
