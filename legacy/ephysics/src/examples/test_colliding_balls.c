#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   static const char *colors[] = {"blue-ball", "red-ball", "green-ball"};
   Evas_Object *sphere, *shadow;
   EPhysics_Body *fall_body;
   int i, column, row;

   for (i = 0; i < 9; i++)
     {
        column = i % 3;
        row = i / 3;

        shadow = elm_layout_add(test_data->win);
        elm_layout_file_set(
           shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
           "shadow-ball");
        evas_object_move(shadow, (1 + column) * WIDTH / 4, FLOOR_Y);
        evas_object_resize(shadow, 54, 3);
        evas_object_show(shadow);
        test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

        sphere = elm_image_add(test_data->win);
        elm_image_file_set(
           sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
           colors[column]);
        evas_object_move(sphere, (1 + column) * WIDTH / 4,
                         100 + row * 60 + row / 2 * 20);
        evas_object_resize(sphere, 54, 54);
        evas_object_show(sphere);
        test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

        fall_body = ephysics_body_sphere_add(test_data->world);
        ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
        ephysics_body_event_callback_add(fall_body,
                                         EPHYSICS_CALLBACK_BODY_UPDATE,
                                         update_object_cb, shadow);
        ephysics_body_restitution_set(fall_body, 0.95);
        ephysics_body_friction_set(fall_body, 1);
        test_data->bodies = eina_list_append(test_data->bodies, fall_body);
     }
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
test_colliding_balls(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Colliding Balls", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 2);

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_right_boundary_add(test_data->world);

   _world_populate(test_data);
}
