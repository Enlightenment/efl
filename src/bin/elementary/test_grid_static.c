#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

struct _Api_Data
{
   Eo *grid;
   Eo *child;
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
         efl_pack_grid(dt->grid, dt->child, 5, 15, 60, 40);
         break;

      case GRID_UNPACK: /* 1 */
         efl_pack_unpack(dt->grid, dt->child);
         eo_del(dt->child);
         break;

      case GRID_SIZE: /* 2 */
         efl_pack_grid_size_get(dt->grid, &w, &h);
         printf("size w=<%d> h=<%d>\n", w, h);
         w = h = 100; /* grid size returns wrong values */
         w += 30;
         h += 10;

         efl_pack_grid_size_set(dt->grid, w, h);
         break;

      case GRID_CLEAR: /* 3 */
         efl_pack_clear(dt->grid);
         break;

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, const Eo_Event *ev)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(ev->object, str);
   elm_object_disabled_set(ev->object, a->state == API_STATE_LAST);
}

static void
_ch_grid(void *data, const Eo_Event *ev)
{
   Eo *gd = data;
   int x, y, w, h;

   // FIXME: old elm_grid API doesn't need grid object
   //elm_grid_pack_get(obj, &x, &y, &w, &h);
   //elm_grid_pack_set(obj, x - 1, y - 1, w + 2, h + 2);
   efl_pack_grid_position_get(gd, ev->object, &x, &y, &w, &h);
   efl_pack_grid(gd, ev->object, x - 1, y - 1, w + 2, h + 2);
}

static void
_win_del(void *data, const Eo_Event *ev EINA_UNUSED)
{
   free(data);
}

void
test_grid_static(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *gd, *bt, *rc, *en;
   api_data *api = calloc(1, sizeof(*api));

   win = eo_add(EFL_UI_WIN_STANDARD_CLASS, NULL,
                efl_ui_win_name_set(eo_self, "grid"),
                efl_text_set(eo_self, "Grid"),
                efl_ui_win_autodel_set(eo_self, EINA_TRUE),
                eo_event_callback_add(eo_self, EO_EVENT_DEL, _win_del, api));

   static int run_count = 0;
   if (((run_count++) % 2) == 0)
     {
        gd = eo_add(EFL_UI_GRID_STATIC_CLASS, win,
                    efl_gfx_size_hint_weight_set(eo_self, 1, 1));
     }
   else
     {
        gd = eo_add(EFL_UI_GRID_CLASS, win,
                    efl_gfx_size_hint_weight_set(eo_self, 1, 1),
                    efl_pack_layout_engine_set(eo_self, EFL_UI_GRID_STATIC_CLASS, NULL));
     }
   efl_pack(win, gd);
   api->data.grid = gd;
   efl_gfx_visible_set(gd, 1);

   /* FIXME: EO-ify entry & button & test_set! */

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "Entry text");
   elm_entry_single_line_set(en, EINA_TRUE);
   efl_pack_grid(gd, en, 50, 10, 40, 10);
   efl_gfx_visible_set(en, 1);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_text_set(en, "Entry text 2");
   elm_entry_single_line_set(en, EINA_TRUE);
   efl_pack_grid(gd, en, 60, 20, 30, 10);
   efl_gfx_visible_set(en, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   eo_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _api_bt_clicked, api);
   efl_pack_grid(gd, bt, 30, 0, 40, 10);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   efl_gfx_visible_set(bt, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   efl_pack_grid(gd, bt,  0,  0, 20, 20);
   efl_gfx_visible_set(bt, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   efl_pack_grid(gd, bt, 10, 10, 40, 20);
   api->data.child = bt;
   efl_gfx_visible_set(bt, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   efl_pack_grid(gd, bt, 10, 30, 20, 50);
   efl_gfx_visible_set(bt, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   efl_pack_grid(gd, bt, 80, 80, 20, 20);
   efl_gfx_visible_set(bt, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Change");
   efl_pack_grid(gd, bt, 40, 40, 20, 20);
   eo_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _ch_grid, gd);
   efl_gfx_visible_set(bt, 1);

   rc = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_gfx_color_set(rc, 128, 0, 0, 128);
   efl_pack_grid(gd, rc, 40, 70, 20, 10);
   efl_gfx_visible_set(rc, 1);

   rc = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_gfx_color_set(rc, 0, 128, 0, 128);
   efl_pack_grid(gd, rc, 60, 70, 10, 10);
   efl_gfx_visible_set(rc, 1);

   rc = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_gfx_color_set(rc, 0, 0, 128, 128);
   efl_pack_grid(gd, rc, 40, 80, 10, 10);
   efl_gfx_visible_set(rc, 1);

   rc = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_gfx_color_set(rc, 128, 0, 128, 128);
   efl_pack_grid(gd, rc, 50, 80, 10, 10);
   efl_gfx_visible_set(rc, 1);

   rc = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_gfx_color_set(rc, 128, 64, 0, 128);
   efl_pack_grid(gd, rc, 60, 80, 10, 10);
   efl_gfx_visible_set(rc, 1);

   efl_gfx_size_set(win, 480, 480);
   efl_gfx_visible_set(win, 1);
}
