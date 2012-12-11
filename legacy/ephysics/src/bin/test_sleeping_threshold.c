#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *sphere1, *sphere2, *sh1, *sh2;
   EPhysics_Body *sphere_body1, *sphere_body2;
   double linear, angular;

   sh1 = elm_layout_add(test_data->win);
   elm_layout_file_set(
      sh1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh1, 50, FLOOR_Y);
   evas_object_resize(sh1, 70, 3);
   evas_object_show(sh1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sh1);

   sphere1 = elm_image_add(test_data->win);
   elm_image_file_set(sphere1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                      "big-blue-ball");
   evas_object_move(sphere1, 50, FLOOR_Y - 70);
   evas_object_resize(sphere1, 70, 70);
   evas_object_show(sphere1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere1);

   sphere_body1 = ephysics_body_sphere_add(test_data->world);
   ephysics_body_evas_object_set(sphere_body1, sphere1, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body1,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh1);
   ephysics_body_restitution_set(sphere_body1, 1);
   ephysics_body_friction_set(sphere_body1, 0.4);
   ephysics_body_sleeping_threshold_set(sphere_body1, 60, 360);
   ephysics_body_linear_velocity_set(sphere_body1, 100, 0, 0);
   ephysics_body_damping_set(sphere_body1, 0.5, 0.5);
   test_data->bodies = eina_list_append(test_data->bodies, sphere_body1);

   sh2 = elm_layout_add(test_data->win);
   elm_layout_file_set(
      sh2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh2, WIDTH - 120, FLOOR_Y);
   evas_object_resize(sh2, 70, 3);
   evas_object_show(sh2);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sh2);

   sphere2 = elm_image_add(test_data->win);
   elm_image_file_set(sphere2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                      "big-red-ball");
   evas_object_move(sphere2, WIDTH - 120, FLOOR_Y - 70);
   evas_object_resize(sphere2, 70, 70);
   evas_object_show(sphere2);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere2);

   sphere_body2 = ephysics_body_sphere_add(test_data->world);
   ephysics_body_evas_object_set(sphere_body2, sphere2, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body2,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh2);
   ephysics_body_restitution_set(sphere_body2, 1);
   ephysics_body_friction_set(sphere_body2, 0.4);
   ephysics_body_sleeping_threshold_set(sphere_body2, 10, 360);
   ephysics_body_linear_velocity_set(sphere_body2, -100, 0, 0);
   ephysics_body_damping_set(sphere_body2, 0.5, 0.5);
   test_data->bodies = eina_list_append(test_data->bodies, sphere_body2);

   ephysics_body_sleeping_threshold_get(sphere_body1, &linear, &angular);
   INF("Body 1: linear threshold: %.2f, angular: %.2f", linear, angular);
   ephysics_body_sleeping_threshold_get(sphere_body2, &linear, &angular);
   INF("Body 2: linear threshold: %.2f, angular: %.2f", linear, angular);
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
test_sleeping(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Sleeping Threshold", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart",
                                  "test-theme", _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;
   ephysics_world_max_sleeping_time_set(world, 0.3);

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 20);

   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.8);

   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_top_boundary_add(test_data->world);

   _world_populate(test_data);
}
