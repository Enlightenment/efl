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
   double timestamp;

   sens = event;
   if (!sens) printf("can't find sensor!\n");
   if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
   if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");

   if (ev_type == EEZE_SENSOR_EVENT_ACCELEROMETER)
     {
        if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
        printf("Accelerometer callback: accuracy %i, x %f, y %f, z %f at time: %f\n", acc, x, y, z, timestamp);
     }
   else if (ev_type == EEZE_SENSOR_EVENT_FACEDOWN)
     printf("Facedown callback at time: %f\n", timestamp);
   else if (ev_type == EEZE_SENSOR_EVENT_DOUBLETAP)
     printf("Doubletap callback at time: %f\n", timestamp);
   else if (ev_type == EEZE_SENSOR_EVENT_SHAKE)
     {
        if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
        printf("Shake callback: accuracy %i, x %f at time: %f\n", acc, x, timestamp);
     }

   return ECORE_CALLBACK_PASS_ON;
}

int
main(void)
{
   Eeze_Sensor_Obj *sens;
   float x, y, z;
   int acc;
   double timestamp;

   /* Init the subsystems we use to make sure they are ready to use. */
   ecore_init();
   eeze_init();

   printf("=== Test sync reads: ===\n");
   /* Create a new sensor object from the type MAGNETIC. We will operate on this
    * object to get data out of the sensor */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_MAGNETIC);
   if (sens)
     {
         /* Request the actual sensor data from the object. A magnetic sensor
          * has three data points so we have to use the xyz function. The data
          * could be cached from the last read so better also get the timestamp
          * when the data got read form the physical sensor. */
         if (!eeze_sensor_xyz_get(sens, &x, &y, &z)) printf("fail get xyz\n");
         /* Get the accurancy of the reading. Not all sensors support this. */
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         /* Here we read the timestamp the data was read out from the physical
          * sensor. Can be used to detect how old the readout it.*/
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Magnetic output: accuracy %i, x %f, y %f, z %f at time: %f\n", acc, x, y, z, timestamp);
         /* Free the sensor when we do not longer use it. */
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
         printf("Acceleromter output: accuracy %i, x %f, y %f, z %f at time: %f\n", acc, x, y, z, timestamp);
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
         printf("Orientation output: accuracy %i, x %f, y %f, z %f at time: %f\n", acc, x, y, z, timestamp);
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
         printf("Gyroscope output: accuracy %i, x %f, y %f, z %f at time: %f\n", acc, x, y, z, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a gyroscope sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_PROXIMITY);
   if (sens)
     {
         if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Proximity output: accuracy %i, distance %f at time: %f\n", acc, x, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a proximity sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   if (sens)
     {
         if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Light output: accuracy %i, lux %f at time: %f\n", acc, x, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a light sensor!\n");

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_TEMPERATURE);
   if (sens)
     {
         if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Temperature output: accuracy %i, temperature %f °C at time: %f\n", acc, x, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find a temperature sensor!\n");

   printf("=== Test sync read update: ===\n");
   /* Get updated values on a sensor. This readout is synchronous. This way we
    * can force a read out of the physical sensor instead of using the cached
    * values. Depending on the hardware this can take a long time. If you don't have special
    * requirements the cached values should be used. */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   if (sens)
     {
         eeze_sensor_read(sens);
         if (!eeze_sensor_x_get(sens, &x)) printf("fail get x\n");
         if (!eeze_sensor_accuracy_get(sens, &acc)) printf("fail get accuracy\n");
         if (!eeze_sensor_timestamp_get(sens, &timestamp)) printf("fail get timestamp\n");
         printf("Light output: accuracy %i, lux %f at time: %f\n", acc, x, timestamp);
         eeze_sensor_free(sens);
     }
   else
     printf("Could not find an light sensor!\n");

   printf("=== Test async reads / events: ===\n");
   /* Async read request for sensors. You have to register an event handler for
    * it first and then request the readout */
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
