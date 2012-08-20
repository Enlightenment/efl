#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"


static void
_update_object_cb(void *data __UNUSED__, EPhysics_Body *body, void *event_info __UNUSED__)
{
   double rot, vrot;

   rot = ephysics_body_rotation_get(body);
   vrot = ephysics_body_angular_velocity_get(body);
   ephysics_body_evas_object_update(body);

   DBG("rot: %lf, vrot :%lf", rot, vrot);
}

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *body;
   Evas_Object *cube;

   cube = elm_image_add(test_data->win);
   elm_image_file_set(
      cube, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(cube, WIDTH / 3, HEIGHT / 2 - 30);
   evas_object_resize(cube, 70, 70);
   evas_object_show(cube);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, cube);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, cube, EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, body);
   ephysics_body_event_callback_add(body,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, NULL);

   ephysics_body_torque_impulse_apply(body, 1);
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
test_rotating_forever(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Rotating Forever", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;
   ephysics_world_gravity_set(world, 0, 0);

   _world_populate(test_data);
}
