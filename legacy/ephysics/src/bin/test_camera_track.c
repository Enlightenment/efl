#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Track_Data Track_Data;

struct _Track_Data {
     Test_Data base;
     EPhysics_Body *body;
     Evas_Object *sp;
};

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
   Evas_Object *cube, *sphere, *shadow;
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
        ephysics_body_friction_set(body, 0.1);
        ephysics_body_mass_set(body, 0);
        track_data->base.bodies = eina_list_append(
           track_data->base.bodies, body);
     }

   shadow = elm_layout_add(track_data->base.win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "shadow-ball");
   evas_object_move(shadow, 0, FLOOR_Y);
   evas_object_resize(shadow, 54, 3);
   evas_object_show(shadow);
   track_data->base.evas_objs = eina_list_append(track_data->base.evas_objs,
                                                 shadow);

   sphere = elm_image_add(track_data->base.win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "green-ball");
   evas_object_move(sphere, 0, 80);
   evas_object_resize(sphere, 54, 54);
   evas_object_show(sphere);
   track_data->base.evas_objs = eina_list_append(track_data->base.evas_objs,
                                                 sphere);

   body = ephysics_body_circle_add(track_data->base.world);
   ephysics_body_evas_object_set(body, sphere, EINA_TRUE);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_restitution_set(body, 0.95);
   ephysics_body_friction_set(body, 0.1);
   ephysics_body_central_impulse_apply(body, 3, 0);
   track_data->body = body;
   track_data->base.bodies = eina_list_append(track_data->base.bodies, body);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Track_Data *track_data = data;
   EPhysics_Camera *camera;

   DBG("Restart pressed");

   test_clean((Test_Data *)track_data);
   _world_populate(track_data);

   camera = ephysics_world_camera_get(track_data->base.world);
   ephysics_camera_position_set(camera, 50, 40);
   _track_apply(track_data);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
   EPhysics_World *world;
   Track_Data *track_data;
   Evas_Object *sp;

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
   evas_object_smart_callback_add(track_data->base.win,
                                  "delete,request", _win_del, track_data);

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

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   track_data->base.world = world;

   boundary = ephysics_body_box_add(track_data->base.world);
   ephysics_body_mass_set(boundary, 0);
   ephysics_body_geometry_set(boundary, 0, FLOOR_Y, WIDTH * 4, 10);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   _world_populate(track_data);
}
