#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *box_body1, *box_body2;
   Evas_Object *box1, *box2, *sh1, *sh2;
   EPhysics_Constraint *constraint;

   sh1 = elm_layout_add(test_data->win);
   elm_layout_file_set(
      sh1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-cube");
   evas_object_move(sh1, WIDTH / 3, FLOOR_Y);
   evas_object_resize(sh1, 70, 3);
   evas_object_show(sh1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sh1);

   box1 = elm_image_add(test_data->win);
   elm_image_file_set(
      box1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(box1, WIDTH / 3, HEIGHT / 8);
   evas_object_resize(box1, 70, 70);
   evas_object_show(box1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, box1);

   box_body1 = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(box_body1, box1, EINA_TRUE);
   ephysics_body_event_callback_add(box_body1, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh1);
   ephysics_body_restitution_set(box_body1, 0.3);
   ephysics_body_friction_set(box_body1, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, box_body1);

   sh2 = elm_layout_add(test_data->win);
   elm_layout_file_set(
      sh2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-cube");
   evas_object_move(sh2, WIDTH / 3 + 110, FLOOR_Y);
   evas_object_resize(sh2, 70, 3);
   evas_object_show(sh2);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sh2);

   box2 = elm_image_add(test_data->win);
   elm_image_file_set(
      box2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
   evas_object_move(box2, WIDTH / 3 + 110, HEIGHT / 8);
   evas_object_resize(box2, 70, 70);
   evas_object_show(box2);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, box2);

   box_body2 = ephysics_body_box_add(test_data->world);
   ephysics_body_mass_set(box_body2, 5);
   ephysics_body_evas_object_set(box_body2, box2, EINA_TRUE);
   ephysics_body_event_callback_add(box_body2, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh2);
   ephysics_body_restitution_set(box_body2, 0.5);
   ephysics_body_friction_set(box_body2, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, box_body2);

   constraint = ephysics_constraint_p2p_add(box_body1, box_body2, 0, 100, 0,
                                            0);
   test_data->constraints = eina_list_append(test_data->constraints,
                                             constraint);
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
test_constraint(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Constraint", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 3);

   _world_populate(test_data);
}
