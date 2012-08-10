#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Sleeping_Data Sleeping_Data;

struct _Sleeping_Data {
     Test_Data base;
     EPhysics_Body *sphere;
     EPhysics_Body *sphere2;
};

static void
_world_populate(Sleeping_Data *sleeping_data)
{
   Evas_Object *sphere1, *sphere2, *sh1, *sh2;
   EPhysics_Body *sphere_body1, *sphere_body2;

   sh1 = elm_layout_add(sleeping_data->base.win);
   elm_layout_file_set(
      sh1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh1, WIDTH / 2, FLOOR_Y);
   evas_object_resize(sh1, 70, 3);
   evas_object_show(sh1);
   sleeping_data->base.evas_objs = eina_list_append(
      sleeping_data->base.evas_objs, sh1);

   sphere1 = elm_image_add(sleeping_data->base.win);
   elm_image_file_set(
      sphere1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "big-blue-ball");
   evas_object_move(sphere1, 20, FLOOR_Y - 70 + 1);
   evas_object_resize(sphere1, 70, 70);
   evas_object_show(sphere1);
   sleeping_data->base.evas_objs = eina_list_append(
      sleeping_data->base.evas_objs, sphere1);

   evas_object_name_set(sphere1, "sphere1");

   sphere_body1 = ephysics_body_circle_add(sleeping_data->base.world);
   ephysics_body_evas_object_set(sphere_body1, sphere1, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body1,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh1);
   ephysics_body_restitution_set(sphere_body1, 0.8);
   ephysics_body_friction_set(sphere_body1, 0.4);
   ephysics_body_sleeping_threshold_set(sphere_body1, 200, 328);
   ephysics_body_linear_velocity_set(sphere_body1, 1, 0);
   ephysics_body_damping_set(sphere_body1, 0.5, 0.5);

   sleeping_data->base.bodies = eina_list_append(
      sleeping_data->base.bodies, sphere_body1);
   sleeping_data->sphere = sphere_body1;

   sh2 = elm_layout_add(sleeping_data->base.win);
   elm_layout_file_set(
      sh2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh2, WIDTH / 8, FLOOR_Y);
   evas_object_resize(sh2, 70, 3);
   evas_object_show(sh2);
   sleeping_data->base.evas_objs = eina_list_append(
      sleeping_data->base.evas_objs, sh2);

   sphere2 = elm_image_add(sleeping_data->base.win);
   elm_image_file_set(
      sphere2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "big-red-ball");
   evas_object_move(sphere2, WIDTH - 100, FLOOR_Y - 70 + 1);
   evas_object_resize(sphere2, 70, 70);
   evas_object_show(sphere2);
   sleeping_data->base.evas_objs = eina_list_append(
      sleeping_data->base.evas_objs, sphere2);

   sphere_body2 = ephysics_body_circle_add(sleeping_data->base.world);
   ephysics_body_evas_object_set(sphere_body2, sphere2, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body2,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh2);
   ephysics_body_restitution_set(sphere_body2, 1);
   ephysics_body_friction_set(sphere_body2, 0.4);
   ephysics_body_sleeping_threshold_set(sphere_body2, 0, 0);
   ephysics_body_linear_velocity_set(sphere_body2, -1, 0);
   ephysics_body_damping_set(sphere_body2, 0.5, 0.5);
   sleeping_data->sphere2 = sphere_body2;
   sleeping_data->base.bodies = eina_list_append(
      sleeping_data->base.bodies, sphere_body2);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Sleeping_Data *sleeping_data = data;

   DBG("Restart pressed");
   test_clean((Test_Data *)sleeping_data);
   _world_populate(sleeping_data);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Sleeping_Data *sleeping_data = data;

   test_clean((Test_Data *)sleeping_data);
   evas_object_del(sleeping_data->base.layout);
   ephysics_world_del(sleeping_data->base.world);
   free(sleeping_data);
   ephysics_shutdown();
}

void
test_sleeping(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Sleeping_Data *sleeping_data;
   EPhysics_Body *boundary;
   EPhysics_World *world;

   if (!ephysics_init())
     return;

   sleeping_data = calloc(1, sizeof(Sleeping_Data));
   if (!sleeping_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *)sleeping_data, "Sleeping Threshold", EINA_FALSE);
   elm_object_signal_emit(sleeping_data->base.layout, "borders,show",
                          "ephysics_test");
   evas_object_smart_callback_add(sleeping_data->base.win,
                                  "delete,request", _win_del, sleeping_data);
   elm_layout_signal_callback_add(sleeping_data->base.layout, "restart",
                                  "test-theme", _restart, sleeping_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   sleeping_data->base.world = world;
   ephysics_world_max_sleeping_time_set(world, 0.5);

   boundary = ephysics_body_bottom_boundary_add(sleeping_data->base.world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 20);

   boundary = ephysics_body_right_boundary_add(sleeping_data->base.world);
   ephysics_body_restitution_set(boundary, 0.8);

   ephysics_body_left_boundary_add(sleeping_data->base.world);
   ephysics_body_top_boundary_add(sleeping_data->base.world);

   _world_populate(sleeping_data);
}
