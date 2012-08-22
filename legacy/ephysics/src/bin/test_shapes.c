#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Shape *pentagon_shape, *hexagon_shape;
   EPhysics_Body *pentagon_body, *hexagon_body;
   Evas_Object *pentagon, *hexagon;

   pentagon = elm_image_add(test_data->win);
   elm_image_file_set(
      pentagon, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "pentagon");
   evas_object_move(pentagon, WIDTH / 3, HEIGHT / 2 - 30);
   evas_object_resize(pentagon, 70, 68);
   evas_object_show(pentagon);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, pentagon);

   pentagon_shape = ephysics_shape_new();
   ephysics_shape_point_add(pentagon_shape, -1, -9/33.);
   ephysics_shape_point_add(pentagon_shape, 0, -1);
   ephysics_shape_point_add(pentagon_shape, 1, -9/33.);
   ephysics_shape_point_add(pentagon_shape, -21/35., 1);
   ephysics_shape_point_add(pentagon_shape, 21/35., 1);

   pentagon_body = ephysics_body_shape_add(test_data->world, pentagon_shape);
   ephysics_body_evas_object_set(pentagon_body, pentagon, EINA_TRUE);
   ephysics_body_restitution_set(pentagon_body, 1);
   test_data->bodies = eina_list_append(test_data->bodies, pentagon_body);

   hexagon = elm_image_add(test_data->win);
   elm_image_file_set(
      hexagon, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "hexagon");
   evas_object_move(hexagon, WIDTH / 3 + 80, HEIGHT / 2 - 30);
   evas_object_resize(hexagon, 70, 60);
   evas_object_show(hexagon);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, hexagon);

   hexagon_shape = ephysics_shape_new();
   ephysics_shape_point_add(hexagon_shape, 0, 30);
   ephysics_shape_point_add(hexagon_shape, 18, 0);
   ephysics_shape_point_add(hexagon_shape, 52, 0);
   ephysics_shape_point_add(hexagon_shape, 70, 30);
   ephysics_shape_point_add(hexagon_shape, 52, 60);
   ephysics_shape_point_add(hexagon_shape, 18, 60);

   hexagon_body = ephysics_body_shape_add(test_data->world, hexagon_shape);
   ephysics_body_evas_object_set(hexagon_body, hexagon, EINA_TRUE);
   ephysics_body_restitution_set(hexagon_body, 1);
   test_data->bodies = eina_list_append(test_data->bodies, hexagon_body);

   ephysics_shape_del(pentagon_shape);
   ephysics_shape_del(hexagon_shape);
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
test_shapes(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Shapes", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_right_boundary_add(test_data->world);

   _world_populate(test_data);
}
