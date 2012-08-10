#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Shape *pentagon_shape, *hexagon_shape;
   EPhysics_Body *pentagon_body, *hexagon_body;
   EPhysics_Constraint *constraint;
   Evas_Object *pentagon, *hexagon;

   pentagon = elm_image_add(test_data->win);
   elm_image_file_set(
      pentagon, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "pentagon");
   evas_object_move(pentagon, WIDTH / 3, HEIGHT / 2 - 30);
   evas_object_resize(pentagon, 70, 66);
   evas_object_show(pentagon);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, pentagon);

   pentagon_shape = ephysics_shape_new();
   ephysics_shape_point_add(pentagon_shape, 0/70., 24/66.);
   ephysics_shape_point_add(pentagon_shape, 35/70., 0/66.);
   ephysics_shape_point_add(pentagon_shape, 70/70., 24/66.);
   ephysics_shape_point_add(pentagon_shape, 56/70., 66/66.);
   ephysics_shape_point_add(pentagon_shape, 14/70., 66/66.);

   pentagon_body = ephysics_body_shape_add(test_data->world, pentagon_shape);
   ephysics_body_evas_object_set(pentagon_body, pentagon, EINA_TRUE);
   ephysics_body_restitution_set(pentagon_body, 1);
   ephysics_body_friction_set(pentagon_body, 0);
   test_data->bodies = eina_list_append(test_data->bodies, pentagon_body);

   hexagon = elm_image_add(test_data->win);
   elm_image_file_set(
      hexagon, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "hexagon");
   evas_object_move(hexagon, WIDTH / 3 + 80, HEIGHT / 2 - 30 + 35);
   evas_object_resize(hexagon, 70, 60);
   evas_object_show(hexagon);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, hexagon);

   hexagon_shape = ephysics_shape_new();
   ephysics_shape_point_add(hexagon_shape, 0, 0.5);
   ephysics_shape_point_add(hexagon_shape, 18/70., 0);
   ephysics_shape_point_add(hexagon_shape, 52/70., 0);
   ephysics_shape_point_add(hexagon_shape, 1, 0.5);
   ephysics_shape_point_add(hexagon_shape, 52/70., 1);
   ephysics_shape_point_add(hexagon_shape, 18/70., 1);

   hexagon_body = ephysics_body_shape_add(test_data->world, hexagon_shape);
   ephysics_body_mass_set(hexagon_body, 5);
   ephysics_body_evas_object_set(hexagon_body, hexagon, EINA_TRUE);
   ephysics_body_restitution_set(hexagon_body, 1);
   ephysics_body_friction_set(hexagon_body, 0);
   test_data->bodies = eina_list_append(test_data->bodies, hexagon_body);

   constraint = ephysics_constraint_p2p_add(pentagon_body, NULL, 8, 0,
                                            0, 0);
   test_data->constraints = eina_list_append(test_data->constraints,
                                             constraint);

   constraint = ephysics_constraint_p2p_add(hexagon_body, NULL, 0, 0, 0, 0);
   test_data->constraints = eina_list_append(test_data->constraints,
                                             constraint);

   ephysics_body_torque_impulse_apply(pentagon_body, 2);

   ephysics_shape_del(pentagon_shape);
   ephysics_shape_del(hexagon_shape);

   ephysics_world_serialize(test_data->world, "/tmp/test.bullet");
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

   _world_populate(test_data);
}
