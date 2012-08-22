#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

struct _Ball_Props {
     int color;
     int size;
     int x;
     int y;
};

static void
_jump(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_List *l, *bodies;
   EPhysics_Body *body;
   Test_Data *test_data = data;
   EPhysics_World *world = test_data->world;
   Evas_Object *bt = test_data->data;

   elm_object_disabled_set(bt, EINA_TRUE);
   bodies = ephysics_world_bodies_get(world);
   EINA_LIST_FOREACH(bodies, l, body)
      ephysics_body_central_impulse_apply(body, 0, -300);
   eina_list_free(bodies);
}

static void
_add_sphere(Test_Data *test_data, int i)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *fall_body;

   static const char *colors[] = {"big-blue-ball", "red-ball", "green-ball"};
   static const struct _Ball_Props props[] = {
        {2, 54, 220, 136},
        {0, 70, 180, 178}, {0, 70, 248, 178},
        {1, 54, 166, 244}, {1, 54, 220, 244}, {1, 54, 274, 244},
        {2, 54, 138, 290}, {2, 54, 192, 290},
        {2, 54, 246, 290}, {2, 54, 300, 290},
   };

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, props[i].x, FLOOR_Y);
   evas_object_resize(shadow, props[i].size, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      colors[props[i].color]);
   evas_object_move(sphere, props[i].x, props[i].y);
   evas_object_resize(sphere, props[i].size, props[i].size);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   ephysics_body_restitution_set(fall_body, 0.95);
   ephysics_body_friction_set(fall_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);
}

static void
_world_populate(Test_Data *test_data)
{
   int i;

   for (i = 0; i < 10; i++)
     _add_sphere(test_data, i);

   elm_object_signal_emit(test_data->data, "click", "ephysics_test");
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;
   Evas_Object *bt = test_data->data;

   DBG("Restart pressed");
   test_clean(test_data);
   _world_populate(test_data);
   elm_object_disabled_set(bt, EINA_FALSE);
}

static void
_world_stopped_cb(void *data, EPhysics_World *world __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt = data;
   elm_object_disabled_set(bt, EINA_FALSE);
   INF("World stopped\n");
}

void
test_jumping_balls(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;
   Evas_Object *bt;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Jumping Balls", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   bt = elm_button_add(test_data->win);
   elm_object_style_set(bt, "ephysics-test-jump");
   elm_layout_content_set(test_data->layout, "extra_button", bt);
   evas_object_smart_callback_add(bt, "clicked", _jump, test_data);

   test_data->data = bt;
   ephysics_world_event_callback_add(world, EPHYSICS_CALLBACK_WORLD_STOPPED,
                                     _world_stopped_cb, bt);

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 2);

   ephysics_body_top_boundary_add(test_data->world);
   ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_right_boundary_add(test_data->world);

   _world_populate(test_data);
}
