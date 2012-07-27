#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static Eina_Bool
_camera_move_cb(void *data)
{
   Test_Data *test_data = data;
   EPhysics_Camera *camera;
   int x, y, w;

   ephysics_world_render_geometry_get(test_data->world, NULL, NULL, &w, NULL);

   camera = ephysics_world_camera_get(test_data->world);
   ephysics_camera_position_get(camera, &x, &y);

   if (x + w > WIDTH * 2)
     {
        test_data->data = NULL;
        return EINA_FALSE;
     }

   ephysics_camera_position_set(camera, x + 2, y);
   return EINA_TRUE;
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
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(sphere, WIDTH / 3, FLOOR_Y - 70);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.2);
   ephysics_body_friction_set(fall_body, 0.5);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);
   ephysics_body_central_impulse_apply(fall_body, 10, 0);

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH * 4 / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-red-ball");
   evas_object_move(sphere, WIDTH * 4 / 3, FLOOR_Y - 70);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_mass_set(fall_body, 3.2);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.2);
   ephysics_body_friction_set(fall_body, 1);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);

   test_data->data = ecore_animator_add(_camera_move_cb, test_data);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Test_Data *test_data = data;

   if (test_data->data)
     ecore_animator_del(test_data->data);

   test_data_del(test_data);
   ephysics_shutdown();
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;
   EPhysics_Camera *camera;

   DBG("Restart pressed");

   if (test_data->data)
     ecore_animator_del(test_data->data);

   camera = ephysics_world_camera_get(test_data->world);
   ephysics_camera_position_set(camera, 50, 40);

   test_clean(test_data);
   _world_populate(test_data);
}

void
test_camera(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Camera", EINA_FALSE);
   evas_object_smart_callback_add(test_data->win, "delete,request",
                                  _win_del, test_data);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_box_add(test_data->world);
   ephysics_body_mass_set(boundary, 0);
   ephysics_body_geometry_set(boundary, 0, FLOOR_Y, WIDTH * 2, 10);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   _world_populate(test_data);
}
