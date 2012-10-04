#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_add_sphere(Test_Data *test_data, int i)
{
   EPhysics_Body *body;
   Evas_Object *sphere;
   static const char *colors[] = {"blue-ball", "red-ball", "green-ball"};

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", colors[i % 3]);
   evas_object_move(sphere, 50 + (i % 34) * 12, 40 + i / 34 * 12);
   evas_object_resize(sphere, 10, 10);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   body = ephysics_body_circle_add(test_data->world);
   ephysics_body_restitution_set(body, 0.85);
   ephysics_body_evas_object_set(body, sphere, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
}

static void
_world_populate(Test_Data *test_data)
{
   int i;
   for (i = 0; i < 612; i++)
     _add_sphere(test_data, i);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   test_clean(data);
   _world_populate(data);
}

void
test_heavy(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Heavy", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_layout_signal_emit(test_data->layout, "loading_bar,show",
                          "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 1);

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_right_boundary_add(test_data->world);

   _world_populate(test_data);
}
