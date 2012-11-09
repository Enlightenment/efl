#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static Evas_Object *
_obj_face_add(Test_Data *test_data, const char *group, Evas_Coord x, Evas_Coord y)
{
   Evas_Object *obj;

   obj = elm_image_add(test_data->win);
   elm_image_file_set(obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                      group);
   evas_object_move(obj, x, y);
   evas_object_resize(obj, 70, 70);
   evas_object_show(obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, obj);

   return obj;
}

static EPhysics_Body *
_box_add(Test_Data *test_data, Evas_Coord x, Evas_Coord y)
{
   Evas_Object *front, *back, *top, *bottom, *right, *left, *shadow;
   EPhysics_Body *box;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, x, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   front = _obj_face_add(test_data, "dice1", x, y);
   back = _obj_face_add(test_data, "dice6", x, y);
   top = _obj_face_add(test_data, "dice2", x, y);
   bottom = _obj_face_add(test_data, "dice5", x, y);
   right = _obj_face_add(test_data, "dice3", x, y);
   left = _obj_face_add(test_data, "dice4", x, y);

   box = ephysics_body_box_add(test_data->world);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_FRONT,
                                      front, EINA_TRUE);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_BACK,
                                      back, EINA_TRUE);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_TOP,
                                      top, EINA_TRUE);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_BOTTOM,
                                      bottom, EINA_TRUE);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_RIGHT,
                                      right, EINA_TRUE);
   ephysics_body_face_evas_object_set(box, EPHYSICS_BODY_BOX_FACE_LEFT,
                                      left, EINA_TRUE);
   ephysics_body_restitution_set(box, 0.7);
   ephysics_body_sleeping_threshold_set(box, 0, 0);
   ephysics_body_damping_set(box, 0, 0);
   ephysics_body_linear_movement_enable_set(box, EINA_TRUE, EINA_TRUE,
                                            EINA_TRUE);
   ephysics_body_angular_movement_enable_set(box, EINA_TRUE, EINA_TRUE,
                                             EINA_TRUE);
   ephysics_body_event_callback_add(box, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, box);

   return box;
}

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *box;

   box = _box_add(test_data, WIDTH / 2, HEIGHT / 4);
   ephysics_body_torque_impulse_apply(box, 6, -2, 0);
   box = _box_add(test_data, 80, 80);
   ephysics_body_torque_impulse_apply(box, 0, 0, 3);
   ephysics_body_central_impulse_apply(box, 80, 0, 0);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   test_clean(data);
   _world_populate(data);
}

void
test_dices(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Dices", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH * 2);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);

   ephysics_world_point_light_position_set(world, 0, 100, -400);
   ephysics_world_ambient_light_color_set(world, 60, 60, 60);
   ephysics_world_light_all_bodies_set(world, EINA_TRUE);

   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.7);

   ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_back_boundary_add(test_data->world);
   ephysics_body_top_boundary_add(test_data->world);

   _world_populate(test_data);
}
