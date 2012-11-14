#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static Eina_Bool
_rotate_cb(void *data)
{
   EPhysics_Quaternion *quat_prev, quat_delta, quat;
   EPhysics_Body *body = data;

   quat_prev = ephysics_body_rotation_get(body, NULL);
   ephysics_quaternion_set(&quat_delta, 0, 0, -0.15, 0.98);
   ephysics_quaternion_normalize(&quat_delta);
   ephysics_body_rotation_set(
      body, ephysics_quaternion_multiply(&quat_delta, quat_prev, &quat));
   free(quat_prev);

   return EINA_TRUE;
}

static Eina_Bool
_increase_torque_cb(void *data)
{
   EPhysics_Body *body = data;
   Evas_Object *obj;

   ephysics_body_torque_apply(body, 0, 0, 2);
   obj = ephysics_body_evas_object_get(body);
   evas_object_data_set(obj, "increase_timer", NULL);

   return EINA_FALSE;
}

static Eina_Bool
_stop_torque_cb(void *data)
{
   EPhysics_Body *body = data;
   Evas_Object *obj;

   ephysics_body_forces_clear(body);
   obj = ephysics_body_evas_object_get(body);
   evas_object_data_set(obj, "stop_timer", NULL);

   return EINA_FALSE;
}

static void
_del_cb(void *data, EPhysics_Body *body __UNUSED__, void *event_info __UNUSED__)
{
   ecore_timer_del(data);
}

static void
_del_torque_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Timer *timer;

   timer = evas_object_data_get(obj, "increase_timer");
   if (timer)
     ecore_timer_del(timer);

   timer = evas_object_data_get(obj, "stop_timer");
   if (timer)
     ecore_timer_del(timer);
}

static void
_update_object_cb(void *data __UNUSED__, EPhysics_Body *body, void *event_info __UNUSED__)
{
   double rx, ry, rz, rw, vrot, torque;
   EPhysics_Quaternion quat;

   ephysics_body_angular_velocity_get(body, NULL, NULL, &vrot);
   ephysics_body_torques_get(body, NULL, NULL, &torque);

   ephysics_body_rotation_get(body, &quat);
   ephysics_quaternion_get(&quat, &rx, &ry, &rz, &rw);

   ephysics_body_evas_object_update(body);

   DBG("body: %p, rot: (%lf, %lf, %lf, %lf), vrot: %lf, torque: %lf", body,
       rx, ry, rz, rw, vrot, torque);
}

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *body;
   Ecore_Timer *timer;
   Evas_Object *cube;

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(cube, WIDTH / 3, HEIGHT / 2 - 30);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, cube, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, NULL);

   ephysics_body_torque_impulse_apply(body, 0, 0, 1);

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
   evas_object_move(cube, WIDTH / 3, FLOOR_Y - 70);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, cube, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, NULL);

   ephysics_body_impulse_apply(body, 30, 0, 0, 0, -10, 0);

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
   evas_object_move(cube, WIDTH / 3, 60);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, cube, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, NULL);

   timer = ecore_timer_add(1, _rotate_cb, body);

   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_DEL,
                                    _del_cb, timer);

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(cube, WIDTH * 2 / 3, 60);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, cube, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, NULL);

   ephysics_body_torque_apply(body, 0, 0, 2);
   evas_object_event_callback_add(cube, EVAS_CALLBACK_DEL,
                                  _del_torque_cb, NULL);

   timer = ecore_timer_add(3, _increase_torque_cb, body);
   evas_object_data_set(cube, "increase_timer", timer);
   timer = ecore_timer_add(5, _stop_torque_cb, body);
   evas_object_data_set(cube, "stop_timer", timer);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;

   DBG("Restart pressed");
   test_clean(test_data);
   _world_populate(test_data);
}

void
test_rotating_forever(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Rotating Forever", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;
   ephysics_world_gravity_set(world, 0, 0, 0);

   _world_populate(test_data);
}
