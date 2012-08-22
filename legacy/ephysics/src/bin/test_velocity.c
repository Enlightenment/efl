#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Velocity_Data Velocity_Data;

struct _Velocity_Data {
     Test_Data base;
     EPhysics_Body *body;
     double old_vx;
     double old_vy;
     double old_vaz;
     double last_time;
};

static Eina_Bool
_on_keydown(void *data, Evas_Object *obj __UNUSED__, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   EPhysics_Body *body = data;

   if (type != EVAS_CALLBACK_KEY_UP)
     return EINA_FALSE;

   if (strcmp(ev->keyname, "Up") == 0)
     ephysics_body_central_impulse_apply(body, 0, -300);
   else if (strcmp(ev->keyname, "Down") == 0)
     ephysics_body_central_impulse_apply(body, 0, 300);
   else if (strcmp(ev->keyname, "Right") == 0)
     ephysics_body_central_impulse_apply(body, 300, 0);
   else if (strcmp(ev->keyname, "Left") == 0)
     ephysics_body_central_impulse_apply(body, -300, 0);

   return EINA_TRUE;
}

static void
_update_vel_cb(void *data, EPhysics_Body *body, void *event_info __UNUSED__)
{
   Velocity_Data *velocity_data = data;
   Eina_Bool first_call = EINA_FALSE;
   double vx, vy, ax, ay, vaz, aaz;
   double time_now, delta_time;
   char buff[64];

   if (!velocity_data->last_time)
     first_call = EINA_TRUE;

   time_now = ecore_time_get();
   delta_time = time_now - velocity_data->last_time;
   velocity_data->last_time = time_now;

   vaz = ephysics_body_angular_velocity_get(body);
   ephysics_body_linear_velocity_get(body, &vx, &vy);
   vx = (vx > 0 || vx <= -0.01) ? vx : 0;
   vy = (vy > 0 || vy <= -0.01) ? vy : 0;

   aaz = (vaz - velocity_data->old_vaz) / delta_time;
   velocity_data->old_vaz = vaz;
   ax = (vx - velocity_data->old_vx) / delta_time;
   velocity_data->old_vx = vx;
   ay = (vy - velocity_data->old_vy) / delta_time;
   velocity_data->old_vy = vy;

   if (first_call) return;

   DBG("Delta Time: %0.3lf", delta_time);

   snprintf(buff, sizeof(buff), "Linear velocity: %.2f, %.2f", vx, vy);
   elm_layout_text_set(velocity_data->base.layout, "linear_vel", buff);
   snprintf(buff, sizeof(buff), "Linear acceleration: %.2f, %.2f", ax, ay);
   elm_layout_text_set(velocity_data->base.layout, "linear_acc", buff);
   DBG("Vx: %0.3lf p/s Ax: %0.3lf p/s/s", vx, ax);
   DBG("Vy: %0.3lf p/s Ay: %0.3lf p/s/s", vy, ay);

   snprintf(buff, sizeof(buff), "Angular velocity: %.2f", vaz);
   elm_layout_text_set(velocity_data->base.layout, "angular_vel", buff);
   snprintf(buff, sizeof(buff), "Angular acceleration: %.2f", aaz);
   elm_layout_text_set(velocity_data->base.layout, "angular_acc", buff);
   DBG("Va: %0.3lf d/s Aa: %0.3lf d/s/s", vaz, aaz);
}

static void
_world_populate(Velocity_Data *velocity_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *sphere_body;

   shadow = elm_layout_add(velocity_data->base.win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 54, 3);
   evas_object_show(shadow);
   velocity_data->base.evas_objs = eina_list_append(
      velocity_data->base.evas_objs, shadow);

   sphere = elm_image_add(velocity_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "red-ball");
   evas_object_move(sphere, WIDTH / 3, FLOOR_Y - 54);
   evas_object_resize(sphere, 54, 54);
   evas_object_show(sphere);
   velocity_data->base.evas_objs = eina_list_append(
      velocity_data->base.evas_objs, sphere);

   sphere_body = ephysics_body_circle_add(velocity_data->base.world);
   ephysics_body_evas_object_set(sphere_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(sphere_body, 0.8);
   ephysics_body_friction_set(sphere_body, 1);
   ephysics_body_sleeping_threshold_set(sphere_body, 0, 0);
   ephysics_body_damping_set(sphere_body, 0.2, 0.2);
   ephysics_body_event_callback_add(sphere_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_event_callback_add(sphere_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_vel_cb, velocity_data);
   ephysics_body_event_callback_add(sphere_body, EPHYSICS_CALLBACK_BODY_STOPPED,
                                    _update_vel_cb, velocity_data);
   velocity_data->base.bodies = eina_list_append(velocity_data->base.bodies,
                                                 sphere_body);
   velocity_data->body = sphere_body;
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Velocity_Data *velocity_data = data;

   DBG("Restart pressed");
   test_clean((Test_Data *)velocity_data);
   _world_populate(velocity_data);
   velocity_data->old_vx = 0;
   velocity_data->old_vy = 0;
   velocity_data->old_vaz = 0;
   velocity_data->last_time = 0;
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Velocity_Data *velocity_data = data;

   test_clean((Test_Data *)velocity_data);
   evas_object_del(velocity_data->base.layout);
   ephysics_world_del(velocity_data->base.world);
   free(velocity_data);
   ephysics_shutdown();
}
void
test_velocity(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Velocity_Data *velocity_data;
   EPhysics_Body *boundary;
   EPhysics_World *world;

   if (!ephysics_init())
     return;

   velocity_data = calloc(1, sizeof(Velocity_Data));
   if (!velocity_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *)velocity_data, "Velocity Getters", EINA_FALSE);
   evas_object_smart_callback_add(velocity_data->base.win,
                                  "delete,request", _win_del, velocity_data);
   elm_layout_signal_callback_add(velocity_data->base.layout, "restart",
                                  "test-theme", _restart, velocity_data);

   elm_object_signal_emit(velocity_data->base.layout, "borders,show",
                          "ephysics_test");
   elm_object_signal_emit(velocity_data->base.layout, "arrows,show",
                          "ephysics_test");
   elm_object_signal_emit(velocity_data->base.layout, "velocity,show",
                          "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   velocity_data->base.world = world;

   boundary = ephysics_body_bottom_boundary_add(velocity_data->base.world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   boundary = ephysics_body_right_boundary_add(velocity_data->base.world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   boundary = ephysics_body_left_boundary_add(velocity_data->base.world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   ephysics_body_top_boundary_add(velocity_data->base.world);

   _world_populate(velocity_data);
   elm_object_event_callback_add(velocity_data->base.win, _on_keydown,
                                 velocity_data->body);
}
