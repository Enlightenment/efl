#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *evas_obj;
   EPhysics_Body *flag_body, *pole_body;

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "brown-pole");
   evas_object_move(evas_obj, 150, FLOOR_Y - 280);
   evas_object_resize(evas_obj, 17, 280);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   pole_body = ephysics_body_box_add(test_data->world);
   ephysics_body_mass_set(pole_body, 0);
   ephysics_body_evas_object_set(pole_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(pole_body, 0.5);
   ephysics_body_friction_set(pole_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, pole_body);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "green-flag");
   evas_object_move(evas_obj, 150 + 17, FLOOR_Y - 280 + 14);
   evas_object_resize(evas_obj, 180, 126);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   flag_body = ephysics_body_cloth_add(test_data->world, 0);
   ephysics_body_mass_set(flag_body, 10);
   ephysics_body_soft_body_hardness_set(flag_body, 1);
   evas_obj = ephysics_body_evas_object_set(flag_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(flag_body, 0.5);
   ephysics_body_friction_set(flag_body, 0.1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);
   test_data->bodies = eina_list_append(test_data->bodies, flag_body);
   ephysics_body_cloth_anchor_full_add(flag_body, pole_body,
                                       EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LEFT);
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
test_flag(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Flag", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_simulation_set(world, 1/160.f, 10);
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   boundary = ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   ephysics_body_top_boundary_add(test_data->world);
   _world_populate(test_data);
}
