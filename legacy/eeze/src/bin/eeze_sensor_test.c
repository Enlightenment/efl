#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <Eeze.h>
#include <Ecore.h>
#include <stdio.h>
#include <unistd.h>

#include <Eeze_Sensor.h>

/*
   TODO
  if you want a continual flow maybe eeze_sensor_flow_set(sens, EINA_TRUE);  (by
  default they are alll EINA_FALSE - ie dont provide a flow of events).
*/

static Eina_Bool
event_cb(void *data EINA_UNUSED, int ev_type, void *event)
{
   Eeze_Sensor_Obj *sens = NULL;
   float x, y, z;
   int acc;
   unsigned long long timestamp;

   sens = event;
   if (!sens) printf("can't find sensor!\n");
   if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
   if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");

   if (ev_type == EEZE_SENSOR_EVENT_ACCELEROMETER)
     {
        if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
        printf("Accelerometer callback: accuracy %i, x %f, y %f, z %f at time: %lli\n", acc, x, y, z, timestamp);
     }
   else if (ev_type == EEZE_SENSOR_EVENT_FACEDOWN)
     printf("Facedown callback at time: %lli\n", timestamp);
   else if (ev_type == EEZE_SENSOR_EVENT_DOUBLETAP)
     printf("Doubletap callback at time: %lli\n", timestamp);
   else if (ev_type == EEZE_SENSOR_EVENT_SHAKE)
     {
        if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
        printf("Shake callback: accuracy %i, x %f at time: %lli\n", acc, x, timestamp);
     }

   return ECORE_CALLBACK_PASS_ON;
}

int
main(void)
{
   Eeze_Sensor_Obj *sens;
   float x, y, z;
   int acc;
   unsigned long long timestamp;

   ecore_init();
   eeze_init();

   printf("=== Test sync reads: ===\n");
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_MAGNETIC);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Magnetic output: accuracy %i, x %f, y %f, z %f at time: %lli\n", acc, x, y, z, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a magnetic sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ACCELEROMETER);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Acceleromter output: accuracy %i, x %f, y %f, z %f at time: %lli\n", acc, x, y, z, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find an accelerometer sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ORIENTATION);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Orientation output: accuracy %i, x %f, y %f, z %f at time: %lli\n", acc, x, y, z, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find an orientation sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_GYROSCOPE);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Gyroscope output: accuracy %i, x %f, y %f, z %f at time: %lli\n", acc, x, y, z, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a gyroscope sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_PROXIMITY);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Proximity output: accuracy %i, distance %f at time: %lli\n", acc, x, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a proximity sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   if (sens)
     {
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Light output: accuracy %i, lux %f at time: %lli\n", acc, x, timestamp);
     }
   else
     printf("Could not find a light sensor!\n");

   /* Get updated values on a sensor. Read out is synchronous */
   eeze_sensor_read(sens);
   if (!sens) printf("can't find an light sensor!\n");
   if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
   if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
   if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
   printf("Light output: accuracy %i, lux %f at time: %lli\n", acc, x, timestamp);
   eeze_sensor_free(sens);

   printf("=== Test async reads / events: ===\n");
   /* Async read request for sensors. You have to register an event handler for it first and then
    * request the read out */
   ecore_event_handler_add(EEZE_SENSOR_EVENT_ACCELEROMETER, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ACCELEROMETER);
   eeze_sensor_async_read(sens, NULL);

   /* Set callbacks for motion events coming in */
   ecore_event_handler_add(EEZE_SENSOR_EVENT_FACEDOWN, event_cb, NULL);
   ecore_event_handler_add(EEZE_SENSOR_EVENT_DOUBLETAP, event_cb, NULL);

   ecore_main_loop_begin();

   eeze_sensor_free(sens);
   eeze_shutdown();
   ecore_shutdown();

   return 0;
}

