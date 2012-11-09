#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Dragging_Data {
     int mouse_status; // 0, up, 1, down
     double curr_mass;
     struct {
          int x;
          int y;
     } clicked_position;
} Dragging_Data;

static void
_on_delete(void *data __UNUSED__, EPhysics_Body *body, void *event_info __UNUSED__)
{
   Dragging_Data *dragging = ephysics_body_data_get(body);
   if (dragging)
     free(dragging);
}

static void
_mouse_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Dragging_Data *dragging = ephysics_body_data_get(data);
   Evas_Event_Mouse_Down *mdown = event_info;
   Evas_Coord x, y;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   dragging->mouse_status = 1;
   dragging->clicked_position.x = mdown->output.x - x;
   dragging->clicked_position.y = mdown->output.y - y;
   dragging->curr_mass = ephysics_body_mass_get(data);
   ephysics_body_mass_set(data, 0);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Dragging_Data *dragging = ephysics_body_data_get(data);
   dragging->mouse_status = 0;
   ephysics_body_mass_set(data, dragging->curr_mass);
   dragging->curr_mass = 0;
}

static void
_mouse_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   EPhysics_Body *body = data;
   Dragging_Data *dragging = ephysics_body_data_get(body);
   Evas_Event_Mouse_Move *mmove = event_info;
   Evas_Coord nx, ny;

   if (!dragging->mouse_status) return;

   nx = mmove->cur.output.x - dragging->clicked_position.x;
   ny = mmove->cur.output.y - dragging->clicked_position.y;

   if (nx < 0 || ny < 0) return;
   ephysics_body_move(body, nx, ny, -15);
}

static void
_box_add(Test_Data *test_data, Evas_Coord x, Evas_Coord y, const char *file)
{
   Evas_Object *evas_obj, *shadow;
   EPhysics_Body *body;
   Dragging_Data *dragging;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-cube");
   evas_object_move(shadow, x, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", file);
   evas_object_move(evas_obj, x, y - 70);
   evas_object_resize(evas_obj, 70, 70);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(body, evas_obj, EINA_TRUE);
   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, body);

   dragging = calloc(1, sizeof(Dragging_Data));
   ephysics_body_data_set(body, dragging);

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, body);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, body);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, body);
   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _on_delete, NULL);
}

static void
_world_populate(Test_Data *test_data)
{
   _box_add(test_data, WIDTH / 3, FLOOR_Y, "blue-cube");
   _box_add(test_data, WIDTH / 3 - 70, FLOOR_Y, "purple-cube");
   _box_add(test_data, WIDTH / 3 + 70, FLOOR_Y, "purple-cube");
   _box_add(test_data, WIDTH / 3 + 140, FLOOR_Y, "blue-cube");
   _box_add(test_data, WIDTH / 3 - 35, FLOOR_Y - 70, "purple-cube");
   _box_add(test_data, WIDTH / 3 + 35, FLOOR_Y - 70, "blue-cube");
   _box_add(test_data, WIDTH / 3 + 105, FLOOR_Y - 70, "purple-cube");
   _box_add(test_data, WIDTH / 3, FLOOR_Y - 140, "blue-cube");
   _box_add(test_data, WIDTH / 3 + 70, FLOOR_Y - 140, "purple-cube");
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
test_grab(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Grab", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_object_signal_emit(test_data->layout, "grab,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 4);

   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   boundary = ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.4);
   ephysics_body_friction_set(boundary, 3);

   ephysics_body_top_boundary_add(test_data->world);

   _world_populate(test_data);
}
