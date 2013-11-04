#include <Elementary_Cursor.h>
#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>


#define WEIGHT evas_object_size_hint_weight_set
#define ALIGN_ evas_object_size_hint_align_set
#define EXPAND(X) WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define FILL(X) ALIGN_((X), EVAS_HINT_FILL, EVAS_HINT_FILL)

static Elm_Gengrid_Item_Class *gic, *ggic;

Evas_Object *grid_content_get(void *data, Evas_Object *obj, const char *part);
char *grid_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                    const char *part EINA_UNUSED);
Eina_Bool grid_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                         const char *part EINA_UNUSED);
void grid_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED);

typedef struct _Item_Data
{
   Elm_Object_Item *item;
   const char *path;
   int mode;
   int onoff;
} Item_Data;

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   Evas_Object *box;           /* Use this to get box content     */
   Evas_Object *grid;
};
typedef struct _api_data api_data;

enum _api_state
{
   GRID_ALIGN_SET,
   GRID_BRING_IN,
   GRID_NO_SELECT_MODE,
   GRID_NO_BOUNCE,
   GRID_PAGE_RELATIVE,
   GRID_PAGE_SIZE,
   GRID_TOOLTIP_SET_TEXT,
   GRID_TOOLTIP_UNSET,
   GRID_ITEM_CLASS_SET,
   GRID_ITEM_UPDATE_SET,
   GRID_PAGE_BRING_IN,
   GRID_PAGE_SHOW,
   GRID_TOOLTIP_CONTENT_CB,
   GRID_TOOLTIP_STYLE_SET,
   GRID_TOOLTIP_WINDOW_MODE_SET,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Evas_Object *grid = api->grid;

   /* use elm_box_children_get() to get list of children */
   switch(api->state)
     { /* Put all api-changes under switch */
      case GRID_ALIGN_SET: /* 0 */
         elm_gengrid_align_set(grid, 0.2, 0.8);
         break;

      case GRID_BRING_IN: /* 1 */
         elm_gengrid_item_bring_in(elm_gengrid_first_item_get(grid), ELM_GENGRID_ITEM_SCROLLTO_IN);
         break;

      case GRID_NO_SELECT_MODE: /* 2 */
         elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_NONE);
         break;

      case GRID_NO_BOUNCE: /* 3 */
         elm_scroller_bounce_set(grid, EINA_TRUE, EINA_FALSE);
         break;

      case GRID_PAGE_RELATIVE: /* 4 */
         elm_scroller_bounce_set(grid, EINA_TRUE, EINA_TRUE);
         elm_scroller_page_relative_set(grid, 0.5, 0.5);
         break;

      case GRID_PAGE_SIZE: /* 5 */
         elm_scroller_page_size_set(grid, 50, 25);
         break;

      case GRID_TOOLTIP_SET_TEXT: /* 6 */
           {
              Elm_Object_Item *item = elm_gengrid_first_item_get(grid);
              elm_gengrid_item_tooltip_text_set(item, "This is the first item");
           }
         break;

      case GRID_TOOLTIP_UNSET: /* 7 */
           {
              Elm_Object_Item *item = elm_gengrid_first_item_get(grid);
              elm_gengrid_item_tooltip_unset(item);
           }
         break;

      case API_STATE_LAST:
         break;

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static const char *img[9] =
{
   "panel_01.jpg",
   "plant_01.jpg",
   "rock_01.jpg",
   "rock_02.jpg",
   "sky_01.jpg",
   "sky_02.jpg",
   "sky_03.jpg",
   "sky_04.jpg",
   "wood_01.jpg",
};

static const char *cur[4] =
{
   ELM_CURSOR_CIRCLE,
   ELM_CURSOR_CLOCK,
   ELM_CURSOR_COFFEE_MUG,
   ELM_CURSOR_CROSS,
};

static int n_current_pic = 0;
static void
_horizontal_grid(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   elm_gengrid_horizontal_set(grid, elm_check_state_get(obj));
}

static void
grid_drag_up(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag up: %p\n", event_info);
}

static void
grid_drag_right(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag right: %p\n", event_info);
}

static void
grid_drag_down(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag down: %p\n", event_info);
}

static void
grid_drag_left(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag left: %p\n", event_info);
}

static void
grid_drag_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag stop: %p\n", event_info);
}

static void
grid_selected(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Selected: %p\n", event_info);
}

static void
grid_double_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Double clicked: %p\n", event_info);
}

static void
grid_longpress(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("longpress %p\n", event_info);
}

static void
grid_moved(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("moved %p\n", event_info);
}

static void
grid_item_check_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Item_Data *id = data;
   id->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", id, id->onoff);
}

char *
grid_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   const Item_Data *id = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", id->path);
   return strdup(buf);
}

Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   const Item_Data *id = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *image = elm_image_add(obj);
        elm_image_file_set(image, id->path, NULL);
        elm_image_aspect_fixed_set(image, EINA_FALSE);
        evas_object_show(image);
        return image;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        elm_check_state_set(ck, id->onoff);
        evas_object_smart_callback_add(ck, "changed", grid_item_check_changed, data);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}

Eina_Bool
grid_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

void
grid_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
   free(data);
}

static void
grid_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on grid obj [%p], pointer [%p]\n", data, obj, event_info);
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

static void
always_select_mode_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   if (elm_check_state_get(obj))
     elm_gengrid_select_mode_set(api->grid, ELM_OBJECT_SELECT_MODE_ALWAYS);
   else
     elm_gengrid_select_mode_set(api->grid, ELM_OBJECT_SELECT_MODE_DEFAULT);
}

static void
multi_select_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   elm_gengrid_multi_select_set(api->grid, elm_check_state_get(obj));
}

static void
clear_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   elm_gengrid_clear(api->grid);
}

static Evas_Object *
create_gengrid(Evas_Object *obj, int items)
{
   Evas_Object *grid = NULL;
   static Item_Data id[144];
   int i, n;
   char buf[PATH_MAX];

   grid = elm_gengrid_add(obj);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 200,
                             elm_config_scale_get() * 150);
   elm_gengrid_reorder_mode_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpress, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < items; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (!(i % 5))
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(gic);

   return grid;
}

static void
restore_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   elm_box_clear(api->box);
   api->grid = create_gengrid(obj, (12 * 12));
   elm_box_pack_end(api->box, api->grid);
   evas_object_show(api->grid);
}

static void
filled_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *box = (Evas_Object *)data;
   Evas_Object *grid;

   elm_box_clear(box);
   grid = create_gengrid(box, 1);
   elm_gengrid_filled_set(grid, elm_check_state_get(obj));
   elm_box_pack_end(box, grid);
   evas_object_show(grid);
}

static void
filled_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *content_box, *grid, *tg;

   win = elm_win_util_standard_add("test filled", "Test Filled");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 450, 200);
   evas_object_show(win);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   content_box = elm_box_add(win);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, content_box);
   elm_box_pack_end(box, content_box);
   evas_object_show(content_box);

   grid = create_gengrid(win, 1);
   elm_box_pack_end(content_box, grid);
   evas_object_show(grid);

   tg = elm_check_add(win);
   elm_object_text_set(tg, "Filled");
   evas_object_smart_callback_add(tg, "changed", filled_cb, content_box);
   elm_box_pack_end(box, tg);
   evas_object_show(tg);
}

static void
cursor_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = (Evas_Object *)data;
   Elm_Object_Item *item = NULL;
   int i = 0;

   for ((item = elm_gengrid_first_item_get(grid)); item;
        (item = elm_gengrid_item_next_get(item)))
     {
        if (!elm_check_state_get(obj))
          {
             elm_gengrid_item_cursor_unset(item);
             continue;
          }

        elm_gengrid_item_cursor_set(item, cur[i]);
        elm_gengrid_item_cursor_engine_only_set(item, EINA_FALSE);
        i++;
     }
}

static void
cursor_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *content_box, *hbox, *grid, *tg;

   win = elm_win_util_standard_add("test cursor", "Test Cursor");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 450, 450);
   evas_object_show(win);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   content_box = elm_box_add(win);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, content_box);
   elm_box_pack_end(box, content_box);
   evas_object_show(content_box);

   grid = create_gengrid(win, 4);
   elm_box_pack_end(content_box, grid);
   evas_object_show(grid);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);

   tg = elm_check_add(win);
   elm_object_text_set(tg, "Cursor");
   evas_object_smart_callback_add(tg, "changed", cursor_cb, grid);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);
}

static void
_btn_bring_in_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   Elm_Object_Item *it = elm_gengrid_selected_item_get(data);
   if (!it) return;
   elm_gengrid_item_bring_in(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_btn_show_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   Elm_Object_Item *it = elm_gengrid_selected_item_get(data);
   if (!it) return;
   elm_gengrid_item_show(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

void
test_gengrid(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bxx, *bx, *tg;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("gengrid", "GenGrid");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   api->box = bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *)api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   /* Create GenGrid */
   api->grid = create_gengrid(win, (12 * 12));
   elm_box_pack_end(bxx, api->grid);
   evas_object_show(api->grid);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(bxx, bx);
   evas_object_show(bx);

   /* Gengrid Always Select Mode Test */
   tg = elm_check_add(win);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Always Select Mode");
   evas_object_smart_callback_add(tg, "changed", always_select_mode_cb,
                                  (void *)api);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   tg = elm_check_add(win);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(tg, "Multi Select Mode");
   elm_check_state_set(tg, EINA_TRUE);
   elm_gengrid_multi_select_set(api->grid, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", multi_select_cb,
                                  (void *)api);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   /* Gengrid Clear Test */
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", clear_bt_clicked,
                                  (void *)api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Restore");
   evas_object_smart_callback_add(bt, "clicked", restore_bt_clicked,
                                  (void *) api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   /* Gengrid Filled Test */
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Check Filled");
   evas_object_smart_callback_add(bt, "clicked", filled_bt_clicked, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   /* Item Cursor Test */
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Check Cursor");
   evas_object_smart_callback_add(bt, "clicked", cursor_bt_clicked,
                                  (void *) api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring in");
   evas_object_smart_callback_add(bt, "clicked", _btn_bring_in_clicked_cb, api->grid);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show");
   evas_object_smart_callback_add(bt, "clicked", _btn_show_clicked_cb, api->grid);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
}

static void
_before_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   Elm_Object_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_insert_before(grid, gic, id, sel, grid_sel,
                                             NULL);
}

static int
compare_cb(const void *data1, const void *data2)
{
   Item_Data *ti1 = elm_object_item_data_get(data1);
   Item_Data *ti2 = elm_object_item_data_get(data2);
   return strlen(ti1->path) - strlen(ti2->path);
}

static void
_sorted_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_sorted_insert(grid, gic, id, compare_cb, grid_sel,
                                             NULL);
}

static void
_after_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   Elm_Object_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_insert_after(grid, gic, id, sel, grid_sel,
                                            NULL);
}

static void
_delete_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   Eina_List *l, *l2, *l3;
   Elm_Object_Item *gg_it;

   l = (Eina_List*)elm_gengrid_selected_items_get(grid);
   if (!l) return;
   EINA_LIST_FOREACH_SAFE(l, l2, l3, gg_it)
     elm_object_item_del(gg_it);
}

static void
_prepend_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_prepend(grid, gic, id, grid_sel, NULL);
}

static void
_append_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_append(grid, gic, id, grid_sel, NULL);
}

static void
_size_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   int size = elm_spinner_value_get(obj);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * size,
                             elm_config_scale_get() * size);
}

void
test_gengrid2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid, *bx, *hbx, *bt, *ck;

   win = elm_win_util_standard_add("gengrid2", "GenGrid 2");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_multi_select_set(grid, EINA_FALSE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(grid, 600, 500);
   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   hbx = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append");
   evas_object_smart_callback_add(bt, "clicked", _append_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prepend");
   evas_object_smart_callback_add(bt, "clicked", _prepend_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert before");
   evas_object_smart_callback_add(bt, "clicked", _before_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert after");
   evas_object_smart_callback_add(bt, "clicked", _after_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sorted insert");
   evas_object_smart_callback_add(bt, "clicked", _sorted_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete");
   evas_object_smart_callback_add(bt, "clicked", _delete_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_spinner_add(win);
   elm_spinner_min_max_set(bt, 10, 1024);
   elm_spinner_value_set(bt, 150);
   elm_spinner_label_format_set(bt, "Item size: %.0f");
   evas_object_smart_callback_add(bt, "changed", _size_changed, grid);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Horizontal Mode");
   evas_object_smart_callback_add(ck, "changed", _horizontal_grid, grid);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = grid_del;

   /* item_class_ref is needed for gic. some items can be added in callbacks */
   elm_gengrid_item_class_ref(gic);
   elm_gengrid_item_class_free(gic);
}

void
test_gengrid3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[144];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("gengrid_group", "GenGrid Group");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_group_item_size_set(grid,
                                   elm_config_scale_get() * 31,
                                   elm_config_scale_get() * 31);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   elm_gengrid_reorder_mode_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpress, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   ggic = elm_gengrid_item_class_new();
   ggic->item_style = "group_index";
   ggic->func.text_get = grid_text_get;
   ggic->func.content_get = NULL;
   ggic->func.state_get = NULL;
   ggic->func.del = NULL;

   n = 0;
   for (i = 0; i < 12 * 12; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        if (i == 0 || i == 18 || i == 53 || i == 100)
          //if (i == 0 || i == 18)
          id[i].item = elm_gengrid_item_append(grid, ggic, &(id[i]), grid_sel, NULL);
        else
          id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (!(i % 5))
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(gic);
   elm_gengrid_item_class_free(ggic);

   evas_object_show(grid);
}

/* test gengrid item styles */

static Evas_Object *
_gengrid_create(Evas_Object *obj, int items, const char *style)
{
   static Evas_Object *grid = NULL;
   Elm_Gengrid_Item_Class *ic;
   Item_Data *id;
   int i, n;
   char buf[PATH_MAX];

   if (grid)
     elm_gengrid_clear(grid);
   else
     {
        if (!obj) return NULL;
        grid = elm_gengrid_add(obj);
        elm_gengrid_item_size_set(grid,
                                  elm_config_scale_get() * 150,
                                  elm_config_scale_get() * 150);
        EXPAND(grid); FILL(grid);
     }

   ic = elm_gengrid_item_class_new();
   if (style) ic->item_style = style;
   else ic->item_style = "default";
   ic->func.text_get = grid_text_get;
   ic->func.content_get = grid_content_get;
   ic->func.state_get = NULL;
   ic->func.del = grid_del;

   n = 0;
   for (i = 0; i < items; i++)
     {
        id = calloc(1, sizeof(Item_Data));
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id->mode = i;
        id->path = eina_stringshare_add(buf);
        id->item = elm_gengrid_item_append(grid, ic, id, NULL, NULL);
     }
   elm_gengrid_item_class_free(ic);

   return grid;
}

static void
_item_style_sel_cb(void *data, Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   _gengrid_create(NULL, (12*12), data);
}

static Evas_Object *
_item_styles_list_create(Evas_Object *parent)
{
   Evas_Object *list;

   list = elm_list_add(parent);
   EXPAND(list); FILL(list);
   elm_list_item_append(list, "default", NULL, NULL,
                        _item_style_sel_cb, "default");
   elm_list_item_append(list, "default_style", NULL, NULL,
                        _item_style_sel_cb, "default_style");
   elm_list_item_append(list, "up", NULL, NULL,
                        _item_style_sel_cb, "up");
   elm_list_item_append(list, "album-preview", NULL, NULL,
                        _item_style_sel_cb, "album-preview");
   elm_list_item_append(list, "thumb", NULL, NULL,
                        _item_style_sel_cb, "thumb");
   elm_list_go(list);

   return list;
}

/* Set elementary widget's min size.
 * We should not set min size hint to elementary widgets because elementary has
 * its own size policy/logic. This is an official trick from raster.
 * @param obj the actual object that you want to set the min size
 * @param parent parent object for elm_table_add
 * @param w min width
 * @param h min height
 */
Evas_Object *
_elm_min_set(Evas_Object *obj, Evas_Object *parent, Evas_Coord w, Evas_Coord h)
{
   Evas_Object *table, *rect;

   table = elm_table_add(parent);

   rect = evas_object_rectangle_add(evas_object_evas_get(table));
   evas_object_size_hint_min_set(rect, w, h);
   EXPAND(rect); FILL(rect);
   elm_table_pack(table, rect, 0, 0, 1, 1);

   EXPAND(obj); FILL(obj);
   elm_table_pack(table, obj, 0, 0, 1, 1);

   return table;
}

void
test_gengrid_item_styles(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *gengrid, *list, *table;

   win = elm_win_util_standard_add("gengrid-styles", "Gengrid Item Styles");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   EXPAND(box);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   list = _item_styles_list_create(box);
   evas_object_show(list);

   table = _elm_min_set(list, box, 100, 0);
   WEIGHT(table, 0, EVAS_HINT_EXPAND);
   FILL(table);
   elm_box_pack_end(box, table);
   evas_object_show(table);

   gengrid = _gengrid_create(win, (12 * 12), NULL);
   elm_box_pack_end(box, gengrid);
   evas_object_show(gengrid);
}

static void
_bring_in2_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[5000];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("horz bring_in", "Horz Bring_in");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 200);
   evas_object_show(win);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_group_item_size_set(grid,
                                   elm_config_scale_get() * 31,
                                   elm_config_scale_get() * 31);
   elm_gengrid_horizontal_set(grid, EINA_TRUE);
   elm_gengrid_align_set(grid, 0.4, 0.8);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 5000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (i == 2400)
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   evas_object_show(grid);
   elm_gengrid_item_bring_in(id[2400].item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_bring_in1_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[5000];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("vertical bring_in", "Vertical Bring_in");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 400);
   evas_object_show(win);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_group_item_size_set(grid,
                                   elm_config_scale_get() * 31,
                                   elm_config_scale_get() * 31);
   elm_gengrid_align_set(grid, 0.1, 1.0);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 5000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (i == 1430)
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   elm_gengrid_item_bring_in(id[1430].item, ELM_GENGRID_ITEM_SCROLLTO_IN);
   evas_object_show(grid);
}


static void
_show2_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[10000];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("horz show", "Horz Show");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 200);
   evas_object_show(win);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_group_item_size_set(grid,
                                   elm_config_scale_get() * 31,
                                   elm_config_scale_get() * 31);
   elm_gengrid_horizontal_set(grid, EINA_TRUE);
   elm_gengrid_align_set(grid, 0.3, 0.7);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 10000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (i == 3333)
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   evas_object_show(grid);
   elm_gengrid_item_show(id[3333].item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_show1_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[10000];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("vertical show", "Vertical Show");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid,
                             elm_config_scale_get() * 150,
                             elm_config_scale_get() * 150);
   elm_gengrid_group_item_size_set(grid,
                                   elm_config_scale_get() * 31,
                                   elm_config_scale_get() * 31);
   elm_gengrid_align_set(grid, 0.1, 0.3);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 10000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (i == 2579)
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   evas_object_show(grid);
   elm_gengrid_item_show(id[2579].item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

void
test_gengrid4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bx;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("gengrid show/scroll_in", "GenGrid Show/Bring_in");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);
   evas_object_resize(win, 300, 300);
   evas_object_show(win);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Vertical Show");
   evas_object_smart_callback_add(bt, "clicked", _show1_clicked_cb, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Vertical Bring_in");
   evas_object_smart_callback_add(bt, "clicked", _bring_in1_clicked_cb, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Horz Show");
   evas_object_smart_callback_add(bt, "clicked", _show2_clicked_cb, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Horz Bring_in");
   evas_object_smart_callback_add(bt, "clicked", _bring_in2_clicked_cb, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
}
