#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_door_create(Test_Data *test_data, Evas_Object *front, Evas_Object *back, Evas_Object *right, Evas_Object *left, Evas_Object *top)
{
   EPhysics_Body *body;
   EPhysics_Constraint *constraint;
   Evas_Coord x, y, z, w, h, d;

   body = ephysics_body_box_add(test_data->world);
   test_data->bodies = eina_list_append(test_data->bodies, body);

   ephysics_body_face_evas_object_set(body, EPHYSICS_BODY_BOX_FACE_TOP, top,
                                      EINA_TRUE);

   ephysics_body_face_evas_object_set(body, EPHYSICS_BODY_BOX_FACE_RIGHT, right,
                                      EINA_TRUE);

   ephysics_body_face_evas_object_set(body, EPHYSICS_BODY_BOX_FACE_LEFT, left,
                                      EINA_TRUE);

   ephysics_body_face_evas_object_set(body, EPHYSICS_BODY_BOX_FACE_FRONT, front,
                                      EINA_TRUE);

   ephysics_body_face_evas_object_set(body, EPHYSICS_BODY_BOX_FACE_BACK, back,
                                      EINA_TRUE);

   ephysics_body_angular_movement_enable_set(body, EINA_FALSE, EINA_TRUE,
                                             EINA_FALSE);

   ephysics_body_linear_movement_enable_set(body, EINA_TRUE, EINA_TRUE,
                                            EINA_TRUE);

   ephysics_body_geometry_get(body, &x, &y, &z, &w, &h, &d);
   ephysics_body_resize(body, w, h, 6);
   ephysics_body_move(body, x, y, 150);
   ephysics_body_geometry_get(body, &x, &y, &z, &w, &h, &d);

   constraint = ephysics_constraint_add(body);
   ephysics_constraint_angular_limit_set(constraint, 0, 0, 90, 0, 0, 0);
   ephysics_constraint_anchor_set(constraint, x - 2, y + h / 2, z + d / 2, 0, 0,
                                  0);
   test_data->constraints = eina_list_append(test_data->constraints, constraint);
   ephysics_body_impulse_apply(body, 0, 0, -10, x + w, y + h / 2, z);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *front, *back, *right, *left, *top;

   front = elm_image_add(test_data->win);
   elm_image_file_set(
      front, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "door-front");
   evas_object_resize(front, 139, 270);
   evas_object_move(front, WIDTH / 3, FLOOR_Y - 290);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, front);

   back = elm_image_add(test_data->win);
   elm_image_file_set(
      back, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "door-back");
   evas_object_resize(back, 139, 270);
   evas_object_move(back, WIDTH / 3, FLOOR_Y - 290);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, back);

   right = evas_object_rectangle_add(evas_object_evas_get(test_data->win));
   evas_object_color_set(right, 117, 75, 35, 255);
   evas_object_resize(right, 6, 270);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, right);

   left = evas_object_rectangle_add(evas_object_evas_get(test_data->win));
   evas_object_color_set(left, 117, 75, 35, 255);
   evas_object_resize(left, 6, 270);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, left);

   top = evas_object_rectangle_add(evas_object_evas_get(test_data->win));
   evas_object_color_set(top, 117, 75, 35, 255);
   evas_object_resize(top, 139, 6);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, top);

   _door_create(test_data, front, back, right, left, top);

   evas_object_show(front);
   evas_object_show(back);
   evas_object_show(right);
   evas_object_show(left);
   evas_object_show(top);
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
test_hinge(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Camera *cam;
   EPhysics_World *world;
   Test_Data *test_data;
   Evas_Object *floor_obj, *door_post;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Hinge", EINA_TRUE);
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   floor_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      floor_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "floor");
   evas_object_move(floor_obj, 0, FLOOR_Y - 20);
   evas_object_resize(floor_obj, FLOOR_WIDTH, 224);
   evas_object_show(floor_obj);

   door_post = elm_image_add(test_data->win);
   elm_image_file_set(
      door_post, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "door-post");
   evas_object_resize(door_post, 147, 278);
   evas_object_move(door_post, WIDTH / 3 - 5, FLOOR_Y - 295);
   evas_object_show(door_post);

   world = ephysics_world_new();
   cam = ephysics_world_camera_get(world);
   ephysics_world_render_geometry_set(world, 0, 0, 0, WIDTH - 100, FLOOR_Y - 300,
                                      300);
   ephysics_camera_perspective_enabled_set(cam, EINA_TRUE);

   test_data->world = world;
   _world_populate(test_data);
}
