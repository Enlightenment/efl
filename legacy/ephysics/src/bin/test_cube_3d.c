#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

#define BOX_SIZE (70)

static void
_mouse_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   EPhysics_Quaternion quat_prev, quat_delta, quat;
   Evas_Event_Mouse_Move *mmove = event_info;
   EPhysics_Body *body = data;
   double rx, ry;

   if (mmove->buttons != 1) return;

   rx = mmove->cur.output.y - mmove->prev.output.y;
   ry = mmove->cur.output.x - mmove->prev.output.x;

   ephysics_body_rotation_get(body, &quat_prev);
   ephysics_quaternion_euler_set(&quat_delta, -ry * 0.06, - rx * 0.04, 0);
   ephysics_quaternion_multiply(&quat_prev, &quat_delta, &quat);
   ephysics_body_rotation_set(body, &quat);
}

static void
_obj_face_add(Test_Data *test_data, EPhysics_Body *body, EPhysics_Body_Face face, const char *group, Evas_Coord x, Evas_Coord y)
{
   Evas_Object *obj;

   obj = elm_image_add(test_data->win);
   elm_image_file_set(obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                      group);
   evas_object_move(obj, x, y);
   evas_object_resize(obj, BOX_SIZE, BOX_SIZE);
   evas_object_show(obj);

   ephysics_body_face_evas_object_set(body, face, obj, EINA_TRUE);

   test_data->evas_objs = eina_list_append(test_data->evas_objs, obj);
}

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *box;
   Evas_Object *rect;
   Evas_Coord x, y;
   Evas *evas;

   x = (WIDTH - BOX_SIZE) / 2;
   y = (HEIGHT - BOX_SIZE) / 2;

   box = ephysics_body_box_add(test_data->world);

   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_FRONT, "cube_front",
                 x, y);
   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_BACK, "cube_back",
                 x, y);
   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_TOP, "cube_top",
                 x, y);
   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_BOTTOM, "cube_bottom",
                 x, y);
   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_RIGHT, "cube_right",
                 x, y);
   _obj_face_add(test_data, box, EPHYSICS_BODY_BOX_FACE_LEFT, "cube_left",
                 x, y);

   ephysics_body_damping_set(box, 0, 0.2);
   ephysics_body_linear_movement_enable_set(box, EINA_FALSE, EINA_FALSE,
                                            EINA_FALSE);
   ephysics_body_angular_movement_enable_set(box, EINA_TRUE, EINA_TRUE,
                                             EINA_TRUE);
   test_data->bodies = eina_list_append(test_data->bodies, box);

   evas = evas_object_evas_get(test_data->win);
   rect = evas_object_rectangle_add(evas);
   evas_object_resize(rect, 120, 120);
   evas_object_move(rect, (WIDTH - 120) / 2, (HEIGHT - 120) / 2);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, box);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   test_clean(data);
   _world_populate(data);
}

void
test_cube_3d(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Cube 3D", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH * 2);
   ephysics_world_gravity_set(world, 0, 0, 0);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);
   test_data->world = world;

   _world_populate(test_data);
}
