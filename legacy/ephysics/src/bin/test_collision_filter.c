#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   static const char *colors[] = {"blue-ball", "big-red-ball"};
   static const int sizes[] = {54, 70};
   EPhysics_Body *fall_body;
   Evas_Object *sphere;
   int i, column, row;

   for (i = 0; i < 4; i++)
     {
        column = i % 2;
        row = i / 2;

        sphere = elm_image_add(test_data->win);
        elm_image_file_set(
           sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
           colors[row]);
        evas_object_move(sphere, (1 + column) * WIDTH / 4 +
                         (70 - sizes[row]) / 2,
                         100 + row * 60 + row / 2 * 20);
        evas_object_resize(sphere, sizes[row], sizes[row]);
        evas_object_show(sphere);
        test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

        fall_body = ephysics_body_circle_add(test_data->world);
        ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
        ephysics_body_restitution_set(fall_body, 0.95);
        ephysics_body_friction_set(fall_body, 0.1);
        ephysics_body_collision_group_add(fall_body, colors[row]);
        if (column + row == 1)
          ephysics_body_central_impulse_apply(fall_body, 600 * row - 300, 0);
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
test_collision_filter(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Collision Filter", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 60, 40, WIDTH - 120, FLOOR_Y - 32);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 2);

   boundary = ephysics_body_box_add(test_data->world);
   ephysics_body_geometry_set(boundary, 60, FLOOR_Y - 16, WIDTH - 120, 10);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 2);
   ephysics_body_mass_set(boundary, 0);
   ephysics_body_collision_group_add(boundary, "blue-ball");

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_right_boundary_add(test_data->world);

   _world_populate(test_data);
}
