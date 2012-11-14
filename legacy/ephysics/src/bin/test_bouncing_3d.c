#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_pos_print_cb(void *data __UNUSED__, EPhysics_Body *body, void *event_info __UNUSED__)
{
   EPhysics_Quaternion quat;
   Evas_Coord x, y, z;

   ephysics_body_geometry_get(body, &x, &y, &z, NULL, NULL, NULL);
   ephysics_body_rotation_get(body, &quat);

   printf("Position X:%i Y:%i Z:%i\n", x, y, z);
   printf("Rotation X:%lf Y:%lf Z:%lf W:%lf\n", quat.x, quat.y, quat.z, quat.w);
}

static Eina_Bool
_on_keydown(void *data, Evas_Object *obj __UNUSED__, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   EPhysics_Body *body = data;

   if (type != EVAS_CALLBACK_KEY_UP)
     return EINA_FALSE;

   if (!strcmp(ev->keyname, "j"))
     ephysics_body_central_impulse_apply(body, 0, -20, 0);
   else if (!strcmp(ev->keyname, "k"))
     ephysics_body_central_impulse_apply(body, 0, 20, 0);
   else if (!strcmp(ev->keyname, "l"))
     ephysics_body_central_impulse_apply(body, 20, 0, 0);
   else if (!strcmp(ev->keyname, "h"))
     ephysics_body_central_impulse_apply(body, -20, 0, 0);
   else if (!strcmp(ev->keyname, "f"))
     ephysics_body_central_impulse_apply(body, 0, 0, -20);
   else if (!strcmp(ev->keyname, "g"))
     ephysics_body_central_impulse_apply(body, 0, 0, 20);
   else if (!strcmp(ev->keyname, "u"))
     ephysics_body_torque_impulse_apply(body, 0, -2, 0);
   else if (!strcmp(ev->keyname, "i"))
     ephysics_body_torque_impulse_apply(body, 0, 2, 0);
   else if (!strcmp(ev->keyname, "o"))
     ephysics_body_torque_impulse_apply(body, 2, 0, 0);
   else if (!strcmp(ev->keyname, "y"))
     ephysics_body_torque_impulse_apply(body, -2, 0, 0);
   else if (!strcmp(ev->keyname, "r"))
     ephysics_body_torque_impulse_apply(body, 0, 0, -2);
   else if (!strcmp(ev->keyname, "t"))
     ephysics_body_torque_impulse_apply(body, 0, 0, 2);

   return EINA_TRUE;
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *cube, *shadow;
   EPhysics_Body *box;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "purple-cube");
   evas_object_move(cube, WIDTH / 2, HEIGHT / 2);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   box = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(box, cube, EINA_TRUE);
   ephysics_body_restitution_set(box, 0);
   ephysics_body_friction_set(box, 0);
   ephysics_body_sleeping_threshold_set(box, 0, 0);
   ephysics_body_damping_set(box, 0, 0);
   ephysics_body_linear_movement_enable_set(box, EINA_TRUE, EINA_TRUE,
                                            EINA_TRUE);
   ephysics_body_angular_movement_enable_set(box, EINA_TRUE, EINA_TRUE,
                                             EINA_TRUE);
   ephysics_body_event_callback_add(box, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_event_callback_add(box, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _pos_print_cb, NULL);
   test_data->bodies = eina_list_append(test_data->bodies, box);
   test_data->data = box;
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;

   elm_object_event_callback_del(test_data->win, _on_keydown, test_data->data);
   test_clean(test_data);
   _world_populate(test_data);
   elm_object_event_callback_add(test_data->win, _on_keydown, test_data->data);
}

void
test_bouncing_3d(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Bouncing 3D", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_object_signal_emit(test_data->layout, "controls,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_gravity_set(world, 0, 0, 0);
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   boundary = ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   boundary = ephysics_body_front_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   boundary = ephysics_body_back_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 0);

   _world_populate(test_data);
   elm_object_event_callback_add(test_data->win, _on_keydown, test_data->data);
}
