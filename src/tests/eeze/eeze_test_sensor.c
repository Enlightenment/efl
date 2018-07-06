#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Sensor.h>

#include "eeze_suite.h"
#include "eeze_sensor_private.h"

static int cb_count = 0;

static Eina_Bool
event_cb(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Eeze_Sensor_Obj *sens = NULL;
   int acc;
   double timestamp;
   Eina_Bool rc = EINA_FALSE;

   sens = event;
   fail_if(sens == NULL);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);

   /* We expect 5 callbacks from async reads right now */
   if (++cb_count == 5)
     ecore_main_loop_quit();

   return ECORE_CALLBACK_DONE;
}

EFL_START_TEST(eeze_test_sensor_read)
{
   Eeze_Sensor_Obj *sens = NULL;
   float x, y, z;
   int acc;
   double timestamp;
   Eina_Bool rc = EINA_FALSE;

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ACCELEROMETER);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_xyz_get(sens, &x, &y, &z);
   fail_if(rc == EINA_FALSE);

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_MAGNETIC);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_xyz_get(sens, &x, &y, &z);
   fail_if(rc == EINA_FALSE);

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ORIENTATION);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_xyz_get(sens, &x, &y, &z);
   fail_if(rc == EINA_FALSE);

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_GYROSCOPE);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_xyz_get(sens, &x, &y, &z);
   fail_if(rc == EINA_FALSE);

   /* Use gyro with xy here even if it offers xzy */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_GYROSCOPE);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_xy_get(sens, &x, &y);
   fail_if(rc == EINA_FALSE);

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_PROXIMITY);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_x_get(sens, &x);
   fail_if(rc == EINA_FALSE);

   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   rc = eeze_sensor_read(sens);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_accuracy_get(sens, &acc);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_timestamp_get(sens, &timestamp);
   fail_if(rc == EINA_FALSE);
   rc = eeze_sensor_x_get(sens, &x);
   fail_if(rc == EINA_FALSE);

   /* Call non existing type */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LAST + 1);
   fail_if(sens != NULL);

   /* Give NULL as sensor object */
   rc = eeze_sensor_read(NULL);
   fail_if(rc != EINA_FALSE);

   /* Change sensor type after creation but before read */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   sens->type = EEZE_SENSOR_TYPE_LAST + 1;
   rc = eeze_sensor_read(sens);
   fail_if(rc != EINA_FALSE);

   /* Try to read from a type you can't read from */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   sens->type = EEZE_SENSOR_TYPE_LAST;
   rc = eeze_sensor_read(sens);
   fail_if(rc != EINA_FALSE);

   /* Try all getter functions with a NULL sensor object */
   rc = eeze_sensor_xyz_get(NULL, &x, &y, &z);
   fail_if(rc != EINA_FALSE);
   rc = eeze_sensor_xy_get(NULL, &x, &y);
   fail_if(rc != EINA_FALSE);
   rc = eeze_sensor_x_get(NULL, &x);
   fail_if(rc != EINA_FALSE);
   rc = eeze_sensor_accuracy_get(NULL, &acc);
   fail_if(rc != EINA_FALSE);
   rc = eeze_sensor_timestamp_get(NULL, &timestamp);
   fail_if(rc != EINA_FALSE);

   eeze_sensor_free(sens);

   /* Try free on NULL */
   eeze_sensor_free(NULL);
}
EFL_END_TEST

EFL_START_TEST(eeze_test_sensor_async_read)
{
   Ecore_Event_Handler *handler;

   Eeze_Sensor_Obj *sens = NULL;
   Eina_Bool rc = EINA_FALSE;

   handler = ecore_event_handler_add(EEZE_SENSOR_EVENT_ACCELEROMETER, event_cb, NULL);
   fail_if(handler == NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ACCELEROMETER);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   ecore_event_handler_add(EEZE_SENSOR_EVENT_MAGNETIC, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_MAGNETIC);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   ecore_event_handler_add(EEZE_SENSOR_EVENT_ORIENTATION, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_ORIENTATION);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   ecore_event_handler_add(EEZE_SENSOR_EVENT_GYROSCOPE, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_GYROSCOPE);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   ecore_event_handler_add(EEZE_SENSOR_EVENT_PROXIMITY, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_PROXIMITY);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   ecore_event_handler_add(EEZE_SENSOR_EVENT_LIGHT, event_cb, NULL);
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc == EINA_FALSE);

   /* Error case */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LAST + 1);
   fail_if(sens != NULL);
   rc = eeze_sensor_async_read(NULL, NULL);
   fail_if(rc != EINA_FALSE);

   /* Change sensor type after creation but before read */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   sens->type = EEZE_SENSOR_TYPE_LAST + 1;
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc != EINA_FALSE);

   /* Try to read from a type you can't read from */
   sens = eeze_sensor_new(EEZE_SENSOR_TYPE_LIGHT);
   fail_if(sens == NULL);
   sens->type = EEZE_SENSOR_TYPE_LAST;
   rc = eeze_sensor_async_read(sens, NULL);
   fail_if(rc != EINA_FALSE);

   ecore_main_loop_begin();
}
EFL_END_TEST

EFL_START_TEST(eeze_test_sensor_obj_get)
{
   Eeze_Sensor_Obj *obj = NULL, *obj_tmp = NULL;

   obj = calloc(1, sizeof(Eeze_Sensor_Obj));
   fail_if(obj == NULL);

   obj_tmp = obj;

   obj = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_ACCELEROMETER);
   fail_if(obj == obj_tmp);

   free(obj);

   /* Try to get non existing obj */
   obj_tmp = eeze_sensor_obj_get(EEZE_SENSOR_TYPE_LAST + 1);
   fail_if(obj_tmp != NULL);
}
EFL_END_TEST

void eeze_test_sensor(TCase *tc)
{
   tcase_add_test(tc, eeze_test_sensor_read);
   tcase_add_test(tc, eeze_test_sensor_async_read);
   tcase_add_test(tc, eeze_test_sensor_obj_get);
}
