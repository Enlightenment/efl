#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

#define PAGE_NUM 3

typedef struct _Page_Data
{
  EPhysics_Body *body;
  Evas_Object *evas_obj;
  int order;
  struct {
    Evas_Coord x;
    Evas_Coord y;
    int node;
  } click_data;
  Evas_Coord x;
} Page_Data;

static void
_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_light_all_bodies_set(world, elm_check_state_get(obj));
}

static int
_pages_sort_cb(const void *d1, const void *d2)
{
   const Page_Data *page1, *page2;

   page1 = d1;
   page2 = d2;

   if (!page1) return -1;
   if (!page2) return 1;

   if (page1->order < page2->order) return -1;
   if (page2->order > page2->order) return 1;

   return 0;
}

static void
_pages_restack(Eina_Hash *pages, Page_Data *page_data)
{
   Page_Data *data;
   Eina_Iterator *it;
   Eina_List *l, *list = NULL;
   Eina_Bool found = EINA_FALSE;
   int i = 1;

   it = eina_hash_iterator_data_new(pages);
   while (eina_iterator_next(it, (void **)&data))
     list = eina_list_append(list, data);
   eina_iterator_free(it);

   list = eina_list_sort(list, eina_list_count(list), _pages_sort_cb);
   EINA_LIST_REVERSE_FOREACH(list, l, data)
     {
        if (found)
          {
             evas_object_layer_set(data->evas_obj, data->order);
             continue;
          }

        if (data->evas_obj == page_data->evas_obj)
          {
             found = EINA_TRUE;
             continue;
          }
        evas_object_layer_set(data->evas_obj, i);
        i++;
     }

   eina_list_free(list);
   evas_object_layer_set(page_data->evas_obj, PAGE_NUM);
}

static void
_mouse_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Test_Data *test_data = data;
   Eina_Hash *pages = test_data->data;
   Page_Data *page_data = eina_hash_find(pages, &obj);
   Evas_Event_Mouse_Down *mdown = event_info;
   Evas_Coord x, y;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   page_data->click_data.x = mdown->output.x;
   page_data->click_data.y = mdown->output.y;
   page_data->click_data.node = ephysics_body_soft_body_triangle_index_get(
                        page_data->body, mdown->canvas.x -x, mdown->canvas.y -y);

   _pages_restack(pages, page_data);
   ephysics_body_soft_body_dragging_set(page_data->body,
                                        page_data->click_data.node);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Test_Data *test_data = data;
   Eina_Hash *pages = test_data->data;
   Page_Data *page_data = eina_hash_find(pages, &obj);

   ephysics_body_soft_body_dragging_unset(page_data->body);
}

static void
_mouse_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Test_Data *test_data = data;
   Eina_Hash *pages = test_data->data;
   Page_Data *page_data = eina_hash_find(pages, &obj);
   Evas_Event_Mouse_Move *mmove = event_info;
   Evas_Coord nx, ny, nz, zl, dist;

   if (mmove->buttons != 1 || page_data->click_data.node < 0) return;

   nx = mmove->cur.output.x;
   ny = mmove->cur.output.y;

   // distance between the clicked and the evas object x coordinates
   zl = sqrt(pow(page_data->click_data.x - page_data->x, 2));

   // distance between the current cursor x and the evas object x coordnates
   dist = sqrt(pow(nx - page_data->x, 2));

   // difference between both distances, the closer the x the highter the z
   nz = zl - dist;

   DBG("node: %d, nx: %d, ny: %d\n", page_data->click_data.node, nx, ny);
   ephysics_body_soft_body_triangle_move(page_data->body,
                                         page_data->click_data.node, nx, ny,
                                         -nz);
}

static Page_Data *
_page_add(Test_Data *test_data, EPhysics_Body *anchor, const char *img, Evas_Coord z)
{
   Evas_Object *evas_obj;
   EPhysics_Body *body;
   Evas_Coord x, y;
   Page_Data *page_data;

   page_data = calloc(1, sizeof(Page_Data));

   x = WIDTH / 2;
   y = FLOOR_Y - 280;

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", img);
   evas_object_move(evas_obj, x, y);
   evas_object_resize(evas_obj, 200, 282);
   evas_object_show(evas_obj);
   evas_object_layer_set(evas_obj, z);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   evas_object_geometry_get(evas_obj, &page_data->x, NULL, NULL, NULL);

   body = ephysics_body_cloth_add(test_data->world, 10, 20);
   ephysics_body_soft_body_position_iterations_set(body, 6);
   ephysics_body_soft_body_bending_constraints_add(body, 1);
   ephysics_body_restitution_set(body, 0);
   ephysics_body_evas_object_set(body, evas_obj, EINA_TRUE);

   ephysics_body_move(body, x, y, -z * 3);

   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);
   test_data->bodies = eina_list_append(test_data->bodies, body);

   ephysics_body_cloth_anchor_full_add(body, anchor,
                                       EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LEFT);


   page_data->body = body;
   page_data->evas_obj = evas_obj;

   return page_data;
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *evas_obj;
   EPhysics_Body *anchor;
   const char *img;
   Eina_Hash *pages;
   Page_Data *page_data;

   evas_obj = elm_image_add(test_data->win);
   elm_image_file_set(
      evas_obj, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "page-00");
   evas_object_move(evas_obj, (WIDTH / 2) - 200, FLOOR_Y - 280);
   evas_object_resize(evas_obj, 200, 282);
   evas_object_show(evas_obj);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, evas_obj);

   anchor = ephysics_body_back_boundary_add(test_data->world);
   ephysics_body_restitution_set(anchor, 0);
   test_data->bodies = eina_list_append(test_data->bodies, anchor);

   pages = eina_hash_pointer_new(NULL);
   test_data->data = pages;

   for (int i = 1; i <= PAGE_NUM; i++)
     {
        img = eina_stringshare_printf("page-0%d", (PAGE_NUM + 1) - i);
        page_data = _page_add(test_data, anchor, img, i);
        page_data->order = i;
        eina_hash_add(pages, &page_data->evas_obj, page_data);

        ephysics_body_collision_group_add(page_data->body, img);

        evas_object_event_callback_add(page_data->evas_obj,
                                       EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb,
                                       test_data);
        evas_object_event_callback_add(page_data->evas_obj,
                                       EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb,
                                       test_data);
        evas_object_event_callback_add(page_data->evas_obj,
                                       EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb,
                                       test_data);
     }
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;

   eina_hash_free(test_data->data);

   DBG("Restart pressed");
   test_clean(test_data);
   _world_populate(test_data);
}

void
test_flip_page(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_World *world;
   Test_Data *test_data;
   Evas_Object *tg;

   if (!ephysics_init())
     return;

   test_data = test_data_new();

   test_win_add(test_data, "Flip Page", EINA_TRUE);
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_simulation_set(world, 1/100.f, 5);
   ephysics_world_gravity_set(world, 0, 0, 9.8);
   ephysics_world_render_geometry_set(world, 0, 0, 20, WIDTH, HEIGHT, 2);

   ephysics_world_point_light_position_set(world, WIDTH / 2, HEIGHT / 2, -200);
   ephysics_camera_perspective_set(ephysics_world_camera_get(world), WIDTH/2,
                                   HEIGHT/2, 0, 1800);
   ephysics_camera_perspective_enabled_set(ephysics_world_camera_get(world),
                                           EINA_TRUE);
   test_data->world = world;
   _world_populate(test_data);

   tg = elm_check_add(test_data->win);
   elm_object_style_set(tg, "ephysics-test");
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, 0.5);
   elm_check_state_set(tg, EINA_FALSE);
   evas_object_show(tg);
   elm_layout_content_set(test_data->layout, "extra_input", tg);

   evas_object_smart_callback_add(tg, "changed", _changed_cb, world);
}
