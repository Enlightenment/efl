#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static Eina_Bool
_grow_cb(void *data)
{
   Test_Data *test_data = data;
   Evas_Object *obj;
   Eina_List *l;
   int size, i = -1;

   EINA_LIST_FOREACH(test_data->evas_objs, l, obj)
     {
        evas_object_geometry_get(obj, NULL, NULL, &size, NULL);
        size += i * 8;
        i++;

        if ((size < 20) || (size > 120))
          continue;

        evas_object_resize(obj, size, size);
     }

   return EINA_TRUE;
}

static void
_add_sphere(Test_Data *test_data, const char *group, int size, int x, int y)
{
   EPhysics_Body *sphere_body;
   Evas_Object *sphere;

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
                      group);
   evas_object_move(sphere, x, y);
   evas_object_resize(sphere, size, size);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   sphere_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(sphere_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(sphere_body, 0.5);
   ephysics_body_central_impulse_apply(sphere_body, 300, 0);
   test_data->bodies = eina_list_append(test_data->bodies, sphere_body);
}

static void
_world_populate(Test_Data *test_data)
{
   _add_sphere(test_data, "big-red-ball", 60, 100, 100);
   _add_sphere(test_data, "big-blue-ball", 60, 150, 150);
   _add_sphere(test_data, "big-green-ball", 60, 200, 200);
   test_data->data = ecore_timer_add(1, _grow_cb, test_data);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Test_Data *test_data = data;
   Ecore_Timer *timer = test_data->data;

   if (timer)
     ecore_timer_del(timer);

   test_data_del(test_data);
   ephysics_shutdown();
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;
   Ecore_Timer *timer = test_data->data;

   if (timer)
     ecore_timer_del(timer);

   DBG("Restart pressed");
   test_clean(test_data);
   _world_populate(test_data);
}

void
test_growing_balls(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Growing Balls", EINA_FALSE);
   evas_object_smart_callback_add(test_data->win, "delete,request", _win_del,
                                  test_data);

   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.8);

   boundary = ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.8);
   boundary = ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.8);
   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.8);

   _world_populate(test_data);
}
