#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
struct _Api_Data
{
   Evas_Object *grid;
   Evas_Object *child;
};
typedef struct _Api_Data Api_Data;

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   Api_Data data;
};
typedef struct _api_data api_data;

enum _api_state
{
   GRID_PACK_SET,
   GRID_UNPACK,
   GRID_SIZE,
   GRID_CLEAR,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Api_Data *dt = &api->data;
   int w, h;

   switch(api->state)
     { /* Put all api-changes under switch */
      case GRID_PACK_SET: /* 0 */
         elm_grid_pack_set(dt->child, 5, 15, 60, 40);
         break;

      case GRID_UNPACK: /* 1 */
         elm_grid_unpack(dt->grid, dt->child);
         evas_object_del(dt->child);
         break;

      case GRID_SIZE: /* 2 */
         elm_grid_size_get(dt->grid, &w, &h);
         printf("size w=<%d> h=<%d>\n", w, h);
         w = h = 100; /* grid size returns wrong values */
         w += 30;
         h += 10;

         elm_grid_size_set(dt->grid, w, h);
         break;

      case GRID_CLEAR: /* 3 */
         elm_grid_clear(dt->grid, EINA_TRUE);
         break;

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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

static void
_ch_grid(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int x, y, w, h;

   elm_grid_pack_get(obj, &x, &y, &w, &h);
   elm_grid_pack_set(obj, x - 1, y - 1, w + 2, h + 2);
}

static void
_cleanup_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

void
test_grid(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *gd, *bt, *rc, *en;

   win = elm_win_util_standard_add("grid", "Grid");
   elm_win_autodel_set(win, EINA_TRUE);
   api_data *api = calloc(1, sizeof(api_data));

   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   elm_win_resize_object_add(win, gd);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   api->data.grid = gd;
   evas_object_show(gd);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "Entry text");
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_grid_pack(gd, en, 50, 10, 40, 10);
   evas_object_show(en);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "Entry text 2");
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_grid_pack(gd, en, 60, 20, 30, 10);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_grid_pack(gd, bt, 30, 0, 40, 10);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt,  0,  0, 20, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 10, 10, 40, 20);
   api->data.child = bt;
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 10, 30, 20, 50);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 80, 80, 20, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Change");
   elm_grid_pack(gd, bt, 40, 40, 20, 20);
   evas_object_smart_callback_add(bt, "clicked", _ch_grid, gd);
   evas_object_show(bt);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 0, 0, 128);
   elm_grid_pack(gd, rc, 40, 70, 20, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 0, 128, 0, 128);
   elm_grid_pack(gd, rc, 60, 70, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 0, 0, 128, 128);
   elm_grid_pack(gd, rc, 40, 80, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 0, 128, 128);
   elm_grid_pack(gd, rc, 50, 80, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 64, 0, 128);
   elm_grid_pack(gd, rc, 60, 80, 10, 10);
   evas_object_show(rc);

   evas_object_resize(win, 480, 480);
   evas_object_show(win);
}
#endif
