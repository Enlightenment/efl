#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Camera_Data Camera_Data;

struct _Camera_Data {
     Test_Data base;
     Ecore_Animator *animator;
     int old_x;
};

static void
_update_floor(Evas_Object *floor_obj, Evas_Coord delta)
{
   int fx, x, y;

   evas_object_geometry_get(floor_obj, &x, &y, NULL, NULL);
   fx = x + delta;
   if (fx < -FLOOR_WIDTH)
     fx += 2 * FLOOR_WIDTH;
   evas_object_move(floor_obj, fx, y);
}

static void
_camera_moved_cb(void *data, EPhysics_World *world __UNUSED__, void *event_info)
{
   EPhysics_Camera *camera = event_info;
   Camera_Data *camera_data = data;
   Evas_Object *floor_obj;
   int x;

   DBG("Camera moved");

   ephysics_camera_position_get(camera, &x, NULL);

   floor_obj = evas_object_data_get(camera_data->base.layout, "floor");
   _update_floor(floor_obj, camera_data->old_x - x);

   floor_obj = evas_object_data_get(camera_data->base.layout, "floor2");
   _update_floor(floor_obj, camera_data->old_x - x);

   camera_data->old_x = x;
}

static Eina_Bool
_camera_move_cb(void *data)
{
   Camera_Data *camera_data = data;
   EPhysics_Camera *camera;
   int x, y, w;

   ephysics_world_render_geometry_get(camera_data->base.world,
                                      NULL, NULL, NULL, &w, NULL, NULL);

   camera = ephysics_world_camera_get(camera_data->base.world);
   ephysics_camera_position_get(camera, &x, &y);

   if (x + w > WIDTH * 2)
     {
        camera_data->animator = NULL;
        return EINA_FALSE;
     }

   x += 2;
   ephysics_camera_position_set(camera, x, y);

   return EINA_TRUE;
}

static void
_world_populate(Camera_Data *camera_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *fall_body;

   shadow = elm_layout_add(camera_data->base.win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   camera_data->base.evas_objs = eina_list_append(camera_data->base.evas_objs,
                                                  shadow);

   sphere = elm_image_add(camera_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "big-blue-ball");
   evas_object_move(sphere, WIDTH / 3, FLOOR_Y - 70);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   camera_data->base.evas_objs = eina_list_append(camera_data->base.evas_objs,
                                                  sphere);

   fall_body = ephysics_body_sphere_add(camera_data->base.world);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.2);
   ephysics_body_friction_set(fall_body, 1);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   camera_data->base.bodies = eina_list_append(camera_data->base.bodies,
                                               fall_body);
   ephysics_body_central_impulse_apply(fall_body, 300, 0, 0);

   shadow = elm_layout_add(camera_data->base.win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH * 4 / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   camera_data->base.evas_objs = eina_list_append(camera_data->base.evas_objs,
                                                  shadow);

   sphere = elm_image_add(camera_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-red-ball");
   evas_object_move(sphere, WIDTH * 4 / 3, FLOOR_Y - 70);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   camera_data->base.evas_objs = eina_list_append(camera_data->base.evas_objs,
                                                  sphere);

   fall_body = ephysics_body_sphere_add(camera_data->base.world);
   ephysics_body_mass_set(fall_body, 4.6);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(fall_body, 0.12);
   ephysics_body_friction_set(fall_body, 2);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   camera_data->base.bodies = eina_list_append(camera_data->base.bodies,
                                               fall_body);

   camera_data->animator = ecore_animator_add(_camera_move_cb, camera_data);
}

static void
_win_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Camera_Data *camera_data = data;

   if (camera_data->animator)
     ecore_animator_del(camera_data->animator);

   test_clean((Test_Data *)camera_data);
   evas_object_del(camera_data->base.layout);
   ephysics_world_del(camera_data->base.world);
   free(camera_data);
   ephysics_shutdown();
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Camera_Data *camera_data = data;
   EPhysics_Camera *camera;
   Evas_Object *floor_obj;

   DBG("Restart pressed");

   if (camera_data->animator)
     ecore_animator_del(camera_data->animator);

   camera = ephysics_world_camera_get(camera_data->base.world);
   ephysics_camera_position_set(camera, 50, 40);

   camera_data->old_x = 0;
   floor_obj = evas_object_data_get(camera_data->base.layout, "floor");
   evas_object_move(floor_obj, 0, FLOOR_Y - 20);
   floor_obj = evas_object_data_get(camera_data->base.layout, "floor2");
   evas_object_move(floor_obj, FLOOR_WIDTH, FLOOR_Y - 20);

   test_clean((Test_Data *)camera_data);
   _world_populate(camera_data);
}

void
test_camera(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Camera_Data *camera_data;
   EPhysics_Body *boundary;
   Evas_Object *floor_obj;
   EPhysics_World *world;

   if (!ephysics_init())
     return;

   camera_data = calloc(1, sizeof(Camera_Data));
   if (!camera_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *)camera_data, "Camera", EINA_FALSE);
   evas_object_event_callback_add(camera_data->base.win, EVAS_CALLBACK_DEL,
                                  _win_del, camera_data);

   elm_layout_signal_callback_add(camera_data->base.layout, "restart",
                                  "test-theme", _restart, camera_data);

   floor_obj = elm_image_add(camera_data->base.win);
   elm_image_file_set(
      floor_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "floor");
   evas_object_move(floor_obj, 0, FLOOR_Y - 20);
   evas_object_resize(floor_obj, FLOOR_WIDTH, 224);
   evas_object_show(floor_obj);
   evas_object_data_set(camera_data->base.layout, "floor", floor_obj);

   floor_obj = elm_image_add(camera_data->base.win);
   elm_image_file_set(
      floor_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "floor");
   evas_object_move(floor_obj, FLOOR_WIDTH, FLOOR_Y - 20);
   evas_object_resize(floor_obj, FLOOR_WIDTH, 224);
   evas_object_show(floor_obj);
   evas_object_data_set(camera_data->base.layout, "floor2", floor_obj);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   ephysics_world_event_callback_add(world,
                                     EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED,
                                     _camera_moved_cb, camera_data);
   camera_data->base.world = world;

   boundary = ephysics_body_box_add(camera_data->base.world);
   ephysics_body_mass_set(boundary, 0);
   ephysics_body_geometry_set(boundary, 0, FLOOR_Y, -15, WIDTH * 2, 10, 30);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   _world_populate(camera_data);
}
