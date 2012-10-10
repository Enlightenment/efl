#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_ball_add(Test_Data *test_data, int x)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *ball;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(sphere, x, HEIGHT / 8);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   ball = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(ball, sphere, EINA_TRUE);
   ephysics_body_restitution_set(ball, 0.95);
   ephysics_body_friction_set(ball, 0.1);
   ephysics_body_event_callback_add(ball, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, ball);
}

static void
_world_populate(Test_Data *test_data)
{
    _ball_add(test_data, WIDTH / 3);
    _ball_add(test_data, WIDTH / 3 + 160);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   test_clean(data);
   _world_populate(data);
}

void
test_light(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Light", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world, 10);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   ephysics_world_light_set(world, 200, 300, -120, 255, 120, 120, 40, 40, 40);
   ephysics_world_light_all_bodies_set(world, EINA_TRUE);

   _world_populate(test_data);
}

