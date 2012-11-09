#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Dragging_Data
{
  EPhysics_Body *body;
  int node;
} Dragging_Data;

static void
_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_light_all_bodies_set(world, elm_check_state_get(obj));
}

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

  if (mdown->button != 1) return;

  evas_object_geometry_get(obj, &x, &y, NULL, NULL);
  dragging->node = ephysics_body_soft_body_triangle_index_get(
     dragging->body, mdown->output.x - x, mdown->output.y - y);

  ephysics_body_soft_body_dragging_set(dragging->body,
                                       dragging->node);

  DBG("Mouse down on %i, %i", mdown->output.x - x, mdown->output.y - y);
  DBG("Selected node: %i", dragging->node);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
  Evas_Event_Mouse_Up *mup = event_info;
  Dragging_Data *dragging = data;
  if (mup->button != 1) return;
  ephysics_body_soft_body_dragging_unset(dragging->body);
  dragging->node = -1;
}

static void
_mouse_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
  Dragging_Data *dragging = data;
  Evas_Event_Mouse_Move *mmove = event_info;
  Evas_Coord nx, ny;
  Evas_Coord x, y;

  evas_object_geometry_get(obj, &x, &y, NULL, NULL);
  printf("canvas = (%i, %i), output = (%i, %i)\n", mmove->cur.canvas.x,
          mmove->cur.canvas.y, mmove->cur.output.x, mmove->cur.output.y);

  if ((mmove->buttons != 1) || (dragging->node < 0)) return;

  nx = mmove->cur.canvas.x;
  ny = mmove->cur.canvas.y;

  DBG("Node: %d, nx: %d, ny: %d", dragging->node, nx, ny);
  ephysics_body_soft_body_triangle_move(dragging->body,
                                        dragging->node, nx, ny, 10);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *evas_obj;
   EPhysics_Body *flag_body, *pole_body;
   Dragging_Data *dragging;

   dragging = calloc(1, sizeof(Dragging_Data));
   dragging->node = -1;

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
   ephysics_body_soft_body_position_iterations_set(flag_body, 10);
   ephysics_body_mass_set(flag_body, 10);
   ephysics_body_soft_body_hardness_set(flag_body, 1);
   ephysics_body_evas_object_set(flag_body, evas_obj, EINA_TRUE);
   ephysics_body_restitution_set(flag_body, 0.5);
   ephysics_body_friction_set(flag_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, flag_body);
   ephysics_body_cloth_anchor_full_add(flag_body, pole_body,
                                       EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LEFT);

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
   EPhysics_World *world;
   Test_Data *test_data;
   Evas_Object *tg;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Flag", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   tg = elm_check_add(test_data->win);
   elm_object_style_set(tg, "ephysics-test");
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, 0.5);
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_show(tg);
   elm_layout_content_set(test_data->layout, "extra_input", tg);

   world = ephysics_world_new();
   ephysics_world_gravity_set(world, 100, 0, 0);
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   test_data->world = world;

   evas_object_smart_callback_add(tg, "changed", _changed_cb, world);

   ephysics_world_point_light_position_set(world, 300, 50, -200);
   ephysics_world_light_all_bodies_set(world, EINA_TRUE);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);

   ephysics_body_top_boundary_add(test_data->world);
   _world_populate(test_data);
}
