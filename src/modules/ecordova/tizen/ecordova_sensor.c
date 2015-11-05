
#include "ecordova_sensor.h"

int (*sensor_listener_start)(sensor_listener_h listener) = 0;
int (*sensor_listener_read_data)(sensor_listener_h listener, sensor_event_s *event) = 0;
int (*sensor_destroy_listener)(sensor_listener_h listener) = 0;
int (*sensor_listener_set_event_cb)(sensor_listener_h listener, unsigned int interval_ms, sensor_event_cb callback, void *data) = 0;
int (*sensor_get_default_sensor)(sensor_type_e type, sensor_h *sensor) = 0;
int (*sensor_create_listener)(sensor_h sensor, sensor_listener_h *listener) = 0;

static void* system_sensor_lib = 0;

void ecordova_sensor_shutdown()
{
  if(sensor_lib)
    dlclose(sensor_lib);
  sensor_listener_start = NULL;
  sensor_listener_read_data = NULL;
  sensor_destroy_listener = NULL;
  sensor_listener_set_event_cb = NULL;
  sensor_get_default_sensor = NULL;
  sensor_create_listener = NULL;
}

void ecordova_sensor_init()
{
  if(!sensor_lib)
    {
       sensor_lib = dlopen("capi-system-sensor.so", RTLD_NOW);
       if(!sensor_lib)
         goto on_error;
       sensor_listener_start = dlsym(sensor_lib, "sensor_listener_start");
       if(!sensor_listener_start)
         goto on_error;
       sensor_listener_read_data = dlsym(sensor_lib, "sensor_listener_read_data");
       if(!sensor_listener_read_data)
         goto on_error;
       sensor_destroy_listener = dlsym(sensor_lib, "sensor_destroy_listener");
       if(!sensor_destroy_listener)
         goto on_error;
       sensor_listener_set_event_cb = dlsym(sensor_lib, "sensor_listener_set_event_cb");
       if(!sensor_listener_set_event_cb)
         goto on_error;
       sensor_get_default_sensor = dlsym(sensor_lib, "sensor_get_default_sensor");
       if(!sensor_get_default_sensor)
         goto on_error;
       sensor_create_listener = dlsym(sensor_lib, "sensor_create_listener");
       if(!sensor_create_listener)
         goto on_error;
    }

  return;
on_error:
  ecordova_sensor_shutdown();
}


