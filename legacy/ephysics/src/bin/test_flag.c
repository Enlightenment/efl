#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Dragging_Data
{
  int mouse_status; // 0, up, 1, down
  EPhysics_Body *body;
  struct {
    int x;
    int y;
    int node;
  } click_data;
} Dragging_Data;

static void
_on_delete(void *data __UNUSED__, EPhysics_Body *body, void *event_info __UNUSED__)
{
   Dragging_Data *dragging = ephysics_body_data_get(body);
   free(dragging);
}

static void
_mouse_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
  Dragging_Data *dragging = data;
  Evas_Event_Mouse_Down *mdown = event_info;
  Evas_Coord x, y;

  evas_object_geometry_get(obj, &x, &y, NULL, NULL);
  dragging->mouse_status = 1;
  dragging->click_data.x = mdown->output.x - x;
  dragging->click_data.y = mdown->output.y - y;
  dragging->click_data.node = ephysics_body_soft_body_triangle_index_get(
                               dragging->body, mdown->output.x, mdown->output.y);

  ephysics_body_soft_body_dragging_set(dragging->body,
                                       dragging->click_data.node);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
  Dragging_Data *dragging = data;
  ephysics_body_soft_body_dragging_unset(dragging->body);
  dragging->mouse_status = 0;
}

static void
_mouse_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
  Dragging_Data *dragging = data;
  Evas_Event_Mouse_Move *mmove = event_info;
  Evas_Coord nx, ny;

  if (!dragging->mouse_status) return;

  nx = mmove->cur.output.x;
  ny = mmove->cur.output.y;

  if (nx < 0 || ny < 0 || dragging->click_data.node < 0) return;

  DBG("node: %d, nx: %d, ny: %d\n", dragging->click_data.node, nx, ny);
  ephysics_body_soft_body_triangle_move(dragging->body,
                                        dragging->click_data.node, nx, ny, 10);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *evas_obj;
   EPhysics_Body *flag_body, *pole_body;
   Dragging_Data *dragging;

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "brown-pole");
   evas_object_move(evas_obj, 150, FLOOR_Y - 280);
   evas_object_resize(evas_obj, 17, 280);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   pole_body = ephysics_body_box_add(test_data->world);
   ephysics_body_mass_set(pole_body, 0);
   ephysics_body_evas_object_set(pole_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(pole_body, 0.5);
   ephysics_body_friction_set(pole_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, pole_body);

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "green-flag");
   evas_object_move(evas_obj, 150 + 12, FLOOR_Y - 280 + 14);
   evas_object_resize(evas_obj, 180, 126);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   flag_body = ephysics_body_cloth_add(test_data->world, 0, 0);
   ephysics_body_mass_set(flag_body, 10);
   ephysics_body_soft_body_hardness_set(flag_body, 1);
   evas_obj = ephysics_body_evas_object_set(flag_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(flag_body, 0.5);
   ephysics_body_friction_set(flag_body, 0.1);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);
   test_data->bodies = eina_list_append(test_data->bodies, flag_body);
   ephysics_body_cloth_anchor_full_add(flag_body, pole_body,
                                       EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LEFT);

   dragging = calloc(1, sizeof(Dragging_Data));
   dragging->body = flag_body;
   ephysics_body_data_set(flag_body, dragging);

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, dragging);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, dragging);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, dragging);

  ephysics_body_event_callback_add(flag_body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _on_delete, NULL);
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
test_flag(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Flag", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   world = ephysics_world_new();
   ephysics_world_simulation_set(world, 1/160.f, 10);
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   ephysics_world_light_set(world, 300, 50, -200, 255, 255, 255, 0, 0, 0);
   ephysics_world_light_all_bodies_set(world, EINA_TRUE);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);

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
