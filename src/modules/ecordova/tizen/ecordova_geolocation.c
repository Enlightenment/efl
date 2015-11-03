#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef TIZEN_LOCATION_MANAGER
#include "ecordova_geolocation_private.h"
#include <ecore_timer.eo.h>

#define MY_CLASS ECORDOVA_GEOLOCATION_CLASS
#define MY_CLASS_NAME "Ecordova_Geolocation"

typedef struct
{
  Eo                           *obj;
  location_manager_h            manager;
  Ecore_Timer                  *timer;
  bool                          current;
  bool                          is_ready;
} Ecordova_Geolocation_Watch;

static void _watch_free(Ecordova_Geolocation_Watch*);
static Eina_Bool _interval_cb(void *);
static void _state_changed_cb(location_service_state_e, void *);
static void _notify(Ecordova_Geolocation_Watch *);
static Ecordova_Geolocation_Watch *_create_watch(Eo *, const Ecordova_Geolocation_Options *, bool);

static Eo_Base *
_ecordova_geolocation_eo_base_constructor(Eo *obj,
                                          Ecordova_Geolocation_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->watchers = eina_hash_int32_new(EINA_FREE_CB(_watch_free));

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_geolocation_constructor(Eo *obj EINA_UNUSED,
                                  Ecordova_Geolocation_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_geolocation_eo_base_destructor(Eo *obj,
                                         Ecordova_Geolocation_Data *pd)
{
   DBG("(%p)", obj);

   eina_hash_free(pd->watchers);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_geolocation_current_position_get(Eo *obj,
                                           Ecordova_Geolocation_Data *pd EINA_UNUSED,
                                           const Ecordova_Geolocation_Options *options)
{
   Ecordova_Geolocation_Watch *watch = _create_watch(obj, options, true);
   if (!watch)
     {
        Ecordova_Geolocation_Error error = {
           .code = ECORDOVA_GEOLOCATION_ERRORCODE_PERMISSION_DENIED,
           .message = "Unknown"
        };
        eo_do(obj,
          eo_event_callback_call(ECORDOVA_GEOLOCATION_EVENT_ERROR, &error));
        return;
     }
}

static Ecordova_Geolocation_WatchID
_ecordova_geolocation_position_watch(Eo *obj,
                                     Ecordova_Geolocation_Data *pd,
                                     const Ecordova_Geolocation_Options *options)
{
   Ecordova_Geolocation_Watch *watch = _create_watch(obj, options, false);
   if (!watch)
     {
        Ecordova_Geolocation_Error error = {
           .code = ECORDOVA_GEOLOCATION_ERRORCODE_PERMISSION_DENIED,
           .message = "Unknown"
        };
        eo_do(obj,
          eo_event_callback_call(ECORDOVA_GEOLOCATION_EVENT_ERROR, &error));
        return 0;
     }

   watch->timer = eo_add(ECORE_TIMER_CLASS, NULL,
     ecore_obj_timer_constructor(options->timeout / 1000.0, _interval_cb, watch));

   static Ecordova_Geolocation_WatchID id = 0;
   ++id;
   eina_hash_add(pd->watchers, &id, watch);
   return id;
}

static void
_ecordova_geolocation_watch_clear(Eo *obj,
                                  Ecordova_Geolocation_Data *pd,
                                  Ecordova_Geolocation_WatchID watch_id)
{
   Ecordova_Geolocation_Watch *watch = eina_hash_find(pd->watchers, &watch_id);
   if (!watch)
     {
        eo_do(obj, eo_event_callback_call(ECORDOVA_DEVICEORIENTATION_EVENT_ERROR,
                                          NULL));
        return;
     }

   Eina_Bool ret = eina_hash_del(pd->watchers, &watch_id, NULL);
   EINA_SAFETY_ON_FALSE_RETURN(ret);
}

static void
_watch_free(Ecordova_Geolocation_Watch *watch)
{
   if (watch->timer)
     eo_unref(watch->timer);

   int ret = location_manager_unset_service_state_changed_cb(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE != ret, on_error);

   ret = location_manager_stop(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE != ret, on_error);

   ret = location_manager_destroy(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE != ret, on_error);

on_error:
   free(watch);
}

static Eina_Bool
_interval_cb(void *data)
{
   Ecordova_Geolocation_Watch *watch = data;
   if (!watch->is_ready)
     {
        Ecordova_Geolocation_Error error = {
           .code = ECORDOVA_GEOLOCATION_ERRORCODE_TIMEOUT,
           .message = "Timeout"
        };
        eo_do(watch->obj,
          eo_event_callback_call(ECORDOVA_GEOLOCATION_EVENT_ERROR, &error));
        return ECORE_CALLBACK_RENEW;
     }

   _notify(watch);

   return ECORE_CALLBACK_RENEW;
}

static void
_state_changed_cb(location_service_state_e state, void *data)
{
   if (LOCATIONS_SERVICE_ENABLED != state)
     return;

   Ecordova_Geolocation_Watch *watch = data;
   watch->is_ready = true;

   if (watch->current)
     {
        _notify(watch);
        _watch_free(watch);
     }
}

static void
_notify(Ecordova_Geolocation_Watch* watch)
{
   double altitude;
   double latitude;
   double longitude;
   double climb;
   double direction;
   double speed;
   location_accuracy_level_e level;
   double horizontal;
   double vertical;
   time_t timestamp;

   int ret = location_manager_get_location(watch->manager,
                                           &altitude,
                                           &latitude,
                                           &longitude,
                                           &climb,
                                           &direction,
                                           &speed,
                                           &level,
                                           &horizontal,
                                           &vertical,
                                           &timestamp);
   if (LOCATIONS_ERROR_NONE != ret)
     {
        Ecordova_Geolocation_Error error = {0};
        switch (ret)
          {
           case LOCATIONS_ERROR_INVALID_PARAMETER:
             error.code = ECORDOVA_GEOLOCATION_ERRORCODE_POSITION_UNAVAILABLE;
             error.message = "Invalid parameter";
             break;
           default:
             error.code = ECORDOVA_GEOLOCATION_ERRORCODE_POSITION_UNAVAILABLE;
             error.message = "Unknown";
             break;
          }
        eo_do(watch->obj,
          eo_event_callback_call(ECORDOVA_GEOLOCATION_EVENT_ERROR, &error));
        return;
     }

   Ecordova_Geolocation_Position position = {
     .coords = {
       .latitude = latitude,
       .longitude = longitude,
       .altitude = altitude,
       .accuracy = horizontal,
       .altitude_accuracy = vertical,
       .heading = direction,
       .speed = speed / 3.6
     },
     .timestamp = timestamp
   };

   const Eo_Event_Description * const event =
     watch->current ? ECORDOVA_GEOLOCATION_EVENT_CURRENT_SUCCESS
                    : ECORDOVA_GEOLOCATION_EVENT_WATCH_SUCCESS;
   eo_do(watch->obj, eo_event_callback_call(event, &position));
}

static Ecordova_Geolocation_Watch *
_create_watch(Eo *obj,
              const Ecordova_Geolocation_Options *options,
              bool is_current)
{
   const Ecordova_Geolocation_Options default_options = {
     .enable_high_accuracy = EINA_FALSE,
     .timeout = 1000,
     .maximum_age = 0
   };
   if (!options)
     options = &default_options;

   Ecordova_Geolocation_Watch *watch = calloc(1, sizeof(Ecordova_Geolocation_Watch));
   watch->obj = obj;
   watch->current = is_current;

   int ret = LOCATIONS_ERROR_NONE;

   if (options->enable_high_accuracy)
     ret = location_manager_create(LOCATIONS_METHOD_GPS, &watch->manager);
   else
     ret = location_manager_create(LOCATIONS_METHOD_GPS, &watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE == ret, on_error_1);

   ret = location_manager_set_service_state_changed_cb(watch->manager,
                                                       _state_changed_cb,
                                                       watch);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE == ret, on_error_2);

   ret = location_manager_start(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE == ret, on_error_3);
   return watch;

on_error_3:
   ret = location_manager_unset_service_state_changed_cb(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE == ret, on_error_2);

on_error_2:
   ret = location_manager_destroy(watch->manager);
   EINA_SAFETY_ON_FALSE_GOTO(LOCATIONS_ERROR_NONE == ret, on_error_1);

on_error_1:
   free(watch);
   return NULL;
}

#include "ecordova_geolocation.eo.c"
#endif
