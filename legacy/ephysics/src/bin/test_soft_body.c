#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *evas_obj, *shadow;
   EPhysics_Body *fall_body;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-red-ball");
   evas_object_move(evas_obj, WIDTH / 3, HEIGHT / 2);
   evas_object_resize(evas_obj, 70, 70);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   fall_body = ephysics_body_soft_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.95);
   ephysics_body_friction_set(fall_body, 0.1);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(evas_obj, WIDTH / 3, HEIGHT / 4);
   evas_object_resize(evas_obj, 70, 70);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, evas_obj, EINA_TRUE);
   ephysics_body_mass_set(fall_body, 1200);
   ephysics_body_restitution_set(fall_body, 0.95);
   ephysics_body_friction_set(fall_body, 0.1);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 4, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(evas_obj, WIDTH / 4, HEIGHT / 4);
   evas_object_resize(evas_obj, 70, 70);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, evas_obj, EINA_TRUE);
   ephysics_body_mass_set(fall_body, 600);
   ephysics_body_restitution_set(fall_body, 0.95);
   ephysics_body_friction_set(fall_body, 0.1);
   ephysics_body_central_impulse_apply(fall_body, 500, 150);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-cube");
   evas_object_move(shadow, WIDTH / 6 + 60, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
   evas_object_move(evas_obj, WIDTH / 6 + 60, HEIGHT / 8);
   evas_object_resize(evas_obj, 70, 70);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   fall_body = ephysics_body_soft_box_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, evas_obj, EINA_TRUE);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_restitution_set(fall_body, 0.5);
   ephysics_body_friction_set(fall_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);
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
test_soft_body(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Soft Body", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_object_signal_emit(test_data->layout, "arrows,show", "ephysics_test");

   world = ephysics_world_new();
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
