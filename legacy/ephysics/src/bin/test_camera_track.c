#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Track_Data Track_Data;

struct _Track_Data {
     Test_Data base;
     EPhysics_Body *body;
     Evas_Object *sp;
     int old_cx;
};

static void
_update_floor(Evas_Object *layout, const char *name, int delta_x, int cy, Eina_Bool ver)
{
   Evas_Object *floor_obj;
   int x, y, fx, fy;

   floor_obj = evas_object_data_get(layout, name);
   evas_object_geometry_get(floor_obj, &x, &y, NULL, NULL);
   fx = x + delta_x;
   if (fx < -FLOOR_WIDTH)
     fx += 2 * FLOOR_WIDTH;

   fy = (ver) ? FLOOR_Y - 20 - cy + 40 : y;
   evas_object_move(floor_obj, fx, fy);
}

static void
_camera_moved_cb(void *data, EPhysics_World *world __UNUSED__, void *event_info)
{
   EPhysics_Camera *camera = event_info;
   Track_Data *track_data = data;
   int cx, cy, delta_x = 0;
   Eina_Bool hor, ver;

   DBG("Camera moved");

   ephysics_camera_tracked_body_get(camera, NULL, &hor, &ver);
   ephysics_camera_position_get(camera, &cx, &cy);

   if (hor)
     delta_x = track_data->old_cx - cx;

   _update_floor(track_data->base.layout, "floor", delta_x, cy, ver);
   _update_floor(track_data->base.layout, "floor2", delta_x, cy, ver);

   track_data->old_cx = cx;
}

static void
_track_apply(Track_Data *track_data)
{
   Eina_Bool hor = EINA_FALSE;
   Eina_Bool ver = EINA_FALSE;
   EPhysics_Camera *camera;
   EPhysics_Body *body;
   int mode;

   body = track_data->body;
   camera = ephysics_world_camera_get(track_data->base.world);
   mode = (int) elm_spinner_value_get(track_data->sp);

   switch (mode)
     {
      case 1:
         hor = EINA_TRUE;
         break;
      case 3:
         hor = EINA_TRUE;
      case 2:
         ver = EINA_TRUE;
     }

   INF("Tracking mode: hor = %i, ver = %i", hor, ver);
   ephysics_camera_body_track(camera, body, hor, ver);
}

static void
_tracking_mode_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _track_apply(data);
}

static void
_world_populate(Track_Data *track_data)
{
   static const char *colors[] = {"blue-cube", "purple-cube"};
   Evas_Object *cube, *sphere;
   EPhysics_Body *body;
   int i, color, row;

   for (i = 0; i < 9; i++)
     {
        color = i % 3 % 2;
        row = i / 3;

        cube = elm_image_add(track_data->base.win);
        elm_image_file_set(
           cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
           colors[color]);
        evas_object_move(cube, i * 70, (row + 2) * 70);
        evas_object_resize(cube, 70, 70);
        evas_object_show(cube);
        track_data->base.evas_objs = eina_list_append(
           track_data->base.evas_objs, cube);

        body = ephysics_body_box_add(track_data->base.world);
        ephysics_body_evas_object_set(body, cube, EINA_TRUE);
        ephysics_body_restitution_set(body, 0.95);
        ephysics_body_friction_set(body, 1);
        ephysics_body_mass_set(body, 0);
        track_data->base.bodies = eina_list_append(
           track_data->base.bodies, body);
     }

   sphere = elm_image_add(track_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "green-ball");
   evas_object_move(sphere, 0, 80);
   evas_object_resize(sphere, 54, 54);
   evas_object_show(sphere);
   track_data->base.evas_objs = eina_list_append(track_data->base.evas_objs,
                                                 sphere);

   body = ephysics_body_sphere_add(track_data->base.world);
   ephysics_body_evas_object_set(body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(body, 0.95);
   ephysics_body_friction_set(body, 1);
   ephysics_body_damping_set(body, 0.1, 0);
   ephysics_body_central_impulse_apply(body, 90, 0, 0);
   track_data->body = body;
   track_data->base.bodies = eina_list_append(track_data->base.bodies, body);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Track_Data *track_data = data;
   EPhysics_Camera *camera;
   Evas_Object *floor_obj;

   DBG("Restart pressed");

   test_clean((Test_Data *)track_data);
   _world_populate(track_data);

   floor_obj = evas_object_data_get(track_data->base.layout, "floor");
   evas_object_move(floor_obj, - WIDTH / 2, FLOOR_Y - 20);
   floor_obj = evas_object_data_get(track_data->base.layout, "floor2");
   evas_object_move(floor_obj, FLOOR_WIDTH - WIDTH / 2, FLOOR_Y - 20);

   camera = ephysics_world_camera_get(track_data->base.world);
   ephysics_camera_position_set(camera, 50, 40);
   track_data->old_cx = 50;

   _track_apply(track_data);
}

static void
_win_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Track_Data *track_data = data;

   test_clean((Test_Data *)track_data);
   evas_object_del(track_data->base.layout);
   ephysics_world_del(track_data->base.world);
   free(track_data);
   ephysics_shutdown();
}

void
test_camera_track(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   Evas_Object *sp, *floor_obj;
   Track_Data *track_data;
   EPhysics_World *world;

   if (!ephysics_init())
     return;

   track_data = calloc(1, sizeof(Track_Data));
   if (!track_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *) track_data, "Camera Track", EINA_FALSE);
   evas_object_event_callback_add(track_data->base.win, EVAS_CALLBACK_DEL,
                                  _win_del, track_data);

   elm_layout_signal_callback_add(track_data->base.layout, "restart",
                                  "test-theme", _restart, track_data);

   sp = elm_spinner_add(track_data->base.win);
   elm_spinner_min_max_set(sp, 0, 3);
   elm_spinner_step_set(sp, 1);
   elm_spinner_wrap_set(sp, EINA_TRUE);
   elm_spinner_special_value_add(sp, 0, "No tracking");
   elm_spinner_special_value_add(sp, 1, "Horizontal tracking");
   elm_spinner_special_value_add(sp, 2, "Vertical tracking");
   elm_spinner_special_value_add(sp, 3, "Full tracking");
   elm_spinner_editable_set(sp, EINA_FALSE);
   elm_object_style_set(sp, "ephysics-test");
   evas_object_smart_callback_add(sp, "delay,changed", _tracking_mode_cb,
                                  track_data);
   elm_layout_content_set(track_data->base.layout, "extra_input", sp);
   track_data->sp = sp;

   floor_obj = elm_image_add(track_data->base.win);
   elm_image_file_set(
      floor_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "floor");
   evas_object_move(floor_obj, - WIDTH / 2, FLOOR_Y - 20);
   evas_object_resize(floor_obj, FLOOR_WIDTH, 224);
   evas_object_show(floor_obj);
   evas_object_data_set(track_data->base.layout, "floor", floor_obj);

   floor_obj = elm_image_add(track_data->base.win);
   elm_image_file_set(
      floor_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "floor");
   evas_object_move(floor_obj, FLOOR_WIDTH - WIDTH / 2, FLOOR_Y - 20);
   evas_object_resize(floor_obj, FLOOR_WIDTH, 224);
   evas_object_show(floor_obj);
   evas_object_data_set(track_data->base.layout, "floor2", floor_obj);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   track_data->base.world = world;
   ephysics_world_event_callback_add(world,
                                     EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED,
                                     _camera_moved_cb, track_data);

   boundary = ephysics_body_box_add(track_data->base.world);
   ephysics_body_mass_set(boundary, 0);
   ephysics_body_geometry_set(boundary, 0, FLOOR_Y, -15, WIDTH * 4, 10, 30);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   track_data->old_cx = 50;
   _world_populate(track_data);
}
