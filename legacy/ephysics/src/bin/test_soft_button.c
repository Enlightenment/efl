#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_mouse_down_cb(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   EPhysics_Body *body = data;
   Evas_Event_Mouse_Down *mdown = event_info;
   Evas_Coord w, h, x, y, z, zz, d;
   Eina_List *triangles;
   int *ldata;

   ephysics_body_geometry_get(body, NULL, NULL, &z, NULL, NULL, &d);

   w = 70;
   h = 70;
   x = mdown->output.x - (w / 2);
   y = mdown->output.y - (h / 2);
   zz = z + (d / 2);
   d = z;

   triangles = ephysics_body_soft_body_triangles_inside_get(body, x, y, zz,
                                                            w, h, d);
   ephysics_body_soft_body_triangle_list_impulse_apply(body, triangles, 0, 0,
                                                       -100);
   EINA_LIST_FREE(triangles, ldata)
     free(ldata);
}

static void
_soft_ellipsoid_add(Test_Data *test_data, Evas_Object *front_face, Evas_Object *back_face)
{
   EPhysics_Body *body;
   Evas_Coord w, h;


   body = ephysics_body_soft_ellipsoid_add(test_data->world, 100);

   ephysics_body_face_evas_object_set(body,
                 EPHYSICS_BODY_SOFT_ELLIPSOID_FACE_FRONT, front_face, EINA_TRUE);

   ephysics_body_face_evas_object_set(body,
                   EPHYSICS_BODY_SOFT_ELLIPSOID_FACE_BACK, back_face, EINA_TRUE);

   ephysics_body_geometry_get(body, NULL, NULL, NULL, &w, &h, NULL);
   ephysics_body_resize(body, w, h, h);
   test_data->bodies = eina_list_append(test_data->bodies, body);

   ephysics_body_linear_movement_enable_set(body, EINA_FALSE, EINA_FALSE,
                                            EINA_FALSE);
   ephysics_body_angular_movement_enable_set(body, EINA_FALSE, EINA_FALSE,
                                             EINA_FALSE);

   evas_object_event_callback_add(front_face, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, body);

   evas_object_event_callback_add(back_face, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, body);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *front_face, *back_face;

   front_face = elm_image_add(test_data->win);
   elm_image_file_set(
      front_face, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(front_face, WIDTH / 2 - 35, HEIGHT / 2 - 35);
   evas_object_resize(front_face, 70, 70);
   evas_object_show(front_face);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, front_face);

   back_face = elm_image_add(test_data->win);
   elm_image_file_set(
      back_face, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "blue-cube");
   evas_object_move(back_face, WIDTH / 2 - 35, HEIGHT / 2 - 35);
   evas_object_resize(back_face, 70, 70);
   evas_object_show(back_face);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, back_face);

   _soft_ellipsoid_add(test_data, front_face, back_face);
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
test_soft_button(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Soft Button", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_simulation_set(world, 1/150.f, 5);
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   ephysics_world_point_light_position_set(world, 300, 50, -200);

   _world_populate(test_data);
}
