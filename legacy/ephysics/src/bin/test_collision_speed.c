#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Speed_Data Speed_Data;

struct _Speed_Data {
     Test_Data base;
     Evas_Object *bt;
     Evas_Object *sp;
     EPhysics_Body *ball;
};

static void
_shoot(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Speed_Data *speed_data = data;
   double impulse, rate;

   rate = ephysics_world_rate_get(ephysics_body_world_get(speed_data->ball));
   impulse = elm_spinner_value_get(speed_data->sp);

   elm_object_disabled_set(obj, EINA_TRUE);
   ephysics_body_central_impulse_apply(speed_data->ball, impulse * rate, 0);
}

static void
_world_populate(Speed_Data *speed_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *ball;

   shadow = elm_layout_add(speed_data->base.win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, 80, FLOOR_Y);
   evas_object_resize(shadow, 54, 3);
   evas_object_show(shadow);
   speed_data->base.evas_objs = eina_list_append(speed_data->base.evas_objs,
                                                 shadow);

   sphere = elm_image_add(speed_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "red-ball");
   evas_object_move(sphere, 80, FLOOR_Y - 54);
   evas_object_resize(sphere, 54, 54);
   evas_object_show(sphere);
   speed_data->base.evas_objs = eina_list_append(speed_data->base.evas_objs,
                                                 sphere);

   ball = ephysics_body_circle_add(speed_data->base.world);
   ephysics_body_evas_object_set(ball, sphere, EINA_TRUE);
   ephysics_body_restitution_set(ball, 0.8);
   ephysics_body_friction_set(ball, 0.2);
   ephysics_body_event_callback_add(ball, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   speed_data->base.bodies = eina_list_append(speed_data->base.bodies, ball);
   speed_data->ball = ball;
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Speed_Data *speed_data = data;

   DBG("Restart pressed");
   test_clean((Test_Data *) speed_data);
   _world_populate(speed_data);
   elm_object_disabled_set(speed_data->bt, EINA_FALSE);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Speed_Data *speed_data = data;

   test_clean((Test_Data *)speed_data);
   evas_object_del(speed_data->base.layout);
   ephysics_world_del(speed_data->base.world);
   free(speed_data);
   ephysics_shutdown();
}

void
test_collision_speed(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Speed_Data *speed_data;
   Evas_Object *bt, *sp;

   if (!ephysics_init())
     return;

   speed_data = calloc(1, sizeof(Speed_Data));
   if (!speed_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *) speed_data, "Collision with High Speed",
                EINA_FALSE);
   elm_object_signal_emit(speed_data->base.layout, "borders,show",
                          "ephysics_test");
   evas_object_smart_callback_add(speed_data->base.win,
                                  "delete,request", _win_del, speed_data);
   elm_layout_signal_callback_add(speed_data->base.layout, "restart",
                                  "test-theme", _restart, speed_data);

   bt = elm_button_add(speed_data->base.win);
   elm_object_style_set(bt, "ephysics-test-shoot");
   elm_layout_content_set(speed_data->base.layout, "extra_button", bt);
   evas_object_smart_callback_add(bt, "clicked", _shoot, speed_data);
   speed_data->bt = bt;

   sp = elm_spinner_add(speed_data->base.win);
   elm_spinner_min_max_set(sp, 0, 300);
   elm_spinner_step_set(sp, 5);
   elm_spinner_value_set(sp, 100);
   elm_object_style_set(sp, "ephysics-test");
   elm_layout_content_set(speed_data->base.layout, "extra_input", sp);
   speed_data->sp = sp;

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   ephysics_world_simulation_set(world, 1/260.f, 5);
   speed_data->base.world = world;

   boundary = ephysics_body_bottom_boundary_add(world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   boundary = ephysics_body_right_boundary_add(world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   boundary = ephysics_body_left_boundary_add(world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   ephysics_body_top_boundary_add(world);

   _world_populate(speed_data);
}
