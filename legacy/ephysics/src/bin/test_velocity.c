#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static Eina_Bool
_on_keydown(void *data, Evas_Object *obj __UNUSED__, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   EPhysics_Body *body = data;

   if (type != EVAS_CALLBACK_KEY_UP)
     return EINA_FALSE;

   if (strcmp(ev->keyname, "Up") == 0)
     ephysics_body_central_impulse_apply(body, 0, 10);
   else if (strcmp(ev->keyname, "Down") == 0)
     ephysics_body_central_impulse_apply(body, 0, -10);
   else if (strcmp(ev->keyname, "Right") == 0)
     ephysics_body_central_impulse_apply(body, 10, 0);
   else if (strcmp(ev->keyname, "Left") == 0)
     ephysics_body_central_impulse_apply(body, -10, 0);

   return EINA_TRUE;
}

static void
_update_vel_cb(void *data, EPhysics_Body *body, void *event_info __UNUSED__)
{
   char linear_vel[64], angular_vel[64];
   Test_Data *test_data = data;
   double vx, vy, vaz;

   vaz = ephysics_body_angular_velocity_get(body);
   ephysics_body_linear_velocity_get(body, &vx, &vy);
   vx = (vx > 0 || vx <= -0.01) ? vx : 0;
   vy = (vy > 0 || vy <= -0.01) ? vy : 0;

   snprintf(linear_vel, sizeof(linear_vel),
            "Linear velocity: x = %.2f, y = %.2f", vx, vy);
   snprintf(angular_vel, sizeof(angular_vel),
            "Angular velocity: z = %.2f", vaz);

   elm_layout_text_set(test_data->layout, "linear_vel", linear_vel);
   elm_layout_text_set(test_data->layout, "angular_vel", angular_vel);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *fall_body;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 54, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "red-ball");
   evas_object_move(sphere, WIDTH / 3, FLOOR_Y - 54);
   evas_object_resize(sphere, 54, 54);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.8);
   ephysics_body_friction_set(fall_body, 0.2);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_vel_cb, test_data);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_STOPPED,
                                    _update_vel_cb, test_data);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);
   test_data->data = fall_body;
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;

   DBG("Restart pressed");
   elm_object_event_callback_del(test_data->win, _on_keydown, test_data->data);
   test_clean(test_data);
   _world_populate(test_data);
   elm_object_event_callback_add(test_data->win, _on_keydown, test_data->data);
}

void
test_velocity(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Velocity Getters", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_layout_text_set(test_data->layout, "linear_vel",
                       "Linear velocity: 0, 0");
   elm_layout_text_set(test_data->layout, "angular_vel", "Angular velocity: 0");

   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_object_signal_emit(test_data->layout, "arrows,show", "ephysics_test");
   elm_object_signal_emit(test_data->layout, "velocity,show", "ephysics_test");

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
   elm_object_event_callback_add(test_data->win, _on_keydown, test_data->data);
}
