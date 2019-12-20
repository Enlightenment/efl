#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

static void
_all_select(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_multi_selectable_all_select(data);
}

static void
_all_unselect(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_multi_selectable_all_unselect(data);
}

static void
_selection_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   if (efl_ui_selectable_selected_get(ev->object))
     efl_ui_layout_orientation_set(data, EFL_UI_LAYOUT_ORIENTATION_VERTICAL);
   else
     efl_ui_layout_orientation_set(data, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
}

static void
_focus_item_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Widget *element_0 = efl_pack_content_get(data, 0);

   EINA_SAFETY_ON_NULL_RETURN(element_0);

   efl_ui_focus_manager_focus_set(data, element_0);
}

static void
_scroll_to_animated_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Widget *element_1154 = efl_pack_content_get(data, 1154);

   EINA_SAFETY_ON_NULL_RETURN(element_1154);

   efl_ui_collection_item_scroll(data, element_1154, EINA_TRUE);
}

static void
_scroll_to_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Widget *element_10 = efl_pack_content_get(data, 10);

   EINA_SAFETY_ON_NULL_RETURN(element_10);

   efl_ui_collection_item_scroll(data, element_10, EINA_FALSE);
}

static void
_change_min_size_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   static Eina_Bool b = EINA_FALSE;
   Efl_Ui_Widget *element_0 =  efl_pack_content_get(data, 0);

   EINA_SAFETY_ON_NULL_RETURN(element_0);

   if (!b)
     {
        efl_gfx_hint_size_min_set(element_0, EINA_SIZE2D(40, 200));
     }
   else
     {
        efl_gfx_hint_size_min_set(element_0, EINA_SIZE2D(40, 40));
     }
   b = !b;
}

typedef struct {
  Efl_Ui_Check *v, *h;
  Efl_Ui_Collection *c;
} Match_Content_Ctx;

static void
_selection_changed_match_content_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Match_Content_Ctx *c = data;
   Eina_Bool v,h;

   v = efl_ui_selectable_selected_get(c->v);
   h = efl_ui_selectable_selected_get(c->h);

   efl_ui_scrollable_match_content_set(c->c, v, h);
}

static void
_widget_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_add_item(Efl_Ui_Collection *c, Eo *cont)
{
   Efl_Class *itc = efl_key_data_get(c, "__item_class");
   char buf[PATH_MAX];
   int r = 0, g = 0, b = 0;
   int i = efl_content_count(c);
   Eo *rect;
   Eo *il;


   il = efl_add(itc, c);

   snprintf(buf, sizeof(buf), "%d - Test %d", i, i%13);
   efl_text_set(il, buf);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, c);
   switch (i % 5)
     {
      case 0:
         r = 255;
         break;
      case 1:
         g = 255;
         break;
      case 2:
         b = 255;
         break;
      case 3:
         r = g = b = 255;
         break;
      case 4:
         r = g = b = 0;
         break;
   }
   efl_gfx_color_set(rect, r, g, b, 255);
   efl_content_set(il, rect);
   if (itc == EFL_UI_GRID_DEFAULT_ITEM_CLASS)
     efl_gfx_hint_size_min_set(il, EINA_SIZE2D(100, 180));
   else
     efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
   efl_pack_end(cont, il);
}

static void
_remove_all_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_pack_clear(data);
}

static void
_add_one_item(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _add_item(data, data);
}

static void
_add_thousend_items(void *data, const Efl_Event *ev EINA_UNUSED)
{
   for (int i = 0; i < 1000; ++i)
     {
        _add_item(data, data);
     }
}

static void
_select_value_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Collection *c = data;

   efl_ui_multi_selectable_select_mode_set(c, efl_ui_radio_group_selected_value_get(ev->object));
}

void create_item_container_ui(const Efl_Class *collection_class, const Efl_Class *item, const char *name)
{
   Efl_Ui_Win *win, *o, *tbl, *item_container, *bx, *git;
   Match_Content_Ctx *ctx = calloc(1, sizeof(*ctx));

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, name),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   tbl = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, tbl);

   item_container = o = efl_add(collection_class, win);
   efl_key_data_set(o, "__item_class", item);
   efl_event_callback_add(o, EFL_EVENT_DEL, _widget_del_cb, ctx);
   for (int i = 0; i < 200; ++i)
     {
        _add_item(o, o);
     }
   for (int j = 0; j < 5; ++j)
     {
        Eina_Strbuf *buf = eina_strbuf_new();

        eina_strbuf_append_printf(buf, "Group #%d", j);
        git = efl_add(EFL_UI_GROUP_ITEM_CLASS, o);
        efl_text_set(git, eina_strbuf_release(buf));
        efl_pack_end(o, git);
        efl_gfx_hint_size_min_set(git, EINA_SIZE2D(40, 40+40));
        for (int i = 0; i < 200; ++i)
          {
            _add_item(o, git);
          }
     }

   efl_pack_table(tbl, o, 1, 0, 1, 14);
   ctx->c = o;

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Focus item 0");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _focus_item_cb, item_container);
   efl_pack_table(tbl, o, 0, 1, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Scroll to 1154 ANIMATED");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _scroll_to_animated_cb, item_container);
   efl_pack_table(tbl, o, 0, 2, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Scroll to 10");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _scroll_to_cb, item_container);
   efl_pack_table(tbl, o, 0, 3, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Change min size of 0");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _change_min_size_cb, item_container);
   efl_pack_table(tbl, o, 0, 4, 1, 1);

   o = efl_add(EFL_UI_CHECK_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Vertical");
   efl_event_callback_add(o, EFL_UI_EVENT_SELECTED_CHANGED, _selection_changed_cb, item_container);
   efl_ui_selectable_selected_set(o, EINA_TRUE);
   efl_pack_table(tbl, o, 0, 5, 1, 1);

   o = efl_add(EFL_UI_CHECK_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Match Vertical");
   efl_event_callback_add(o, EFL_UI_EVENT_SELECTED_CHANGED, _selection_changed_match_content_cb, ctx);
   efl_pack_table(tbl, o, 0, 6, 1, 1);
   ctx->v = o;

   o = efl_add(EFL_UI_CHECK_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Match Horizontal");
   efl_event_callback_add(o, EFL_UI_EVENT_SELECTED_CHANGED, _selection_changed_match_content_cb, ctx);
   efl_pack_table(tbl, o, 0, 7, 1, 1);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(260, 200));
   ctx->h = o;

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Remove all items");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _remove_all_cb, item_container);
   efl_pack_table(tbl, o, 0, 8, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Add 1 item");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _add_one_item, item_container);
   efl_pack_table(tbl, o, 0, 9, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Add 1000 item");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _add_thousend_items, item_container);
   efl_pack_table(tbl, o, 0, 10, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Select All");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _all_select, item_container);
   efl_pack_table(tbl, o, 0, 11, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Unselect All");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _all_unselect, item_container);
   efl_pack_table(tbl, o, 0, 12, 1, 1);


   bx = efl_add(EFL_UI_RADIO_BOX_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_event_callback_add(bx, EFL_UI_RADIO_GROUP_EVENT_VALUE_CHANGED, _select_value_cb, item_container);
   efl_pack_table(tbl, bx, 0, 13, 1, 1);
   o = efl_add(EFL_UI_RADIO_CLASS, bx,
           efl_ui_radio_state_value_set(efl_added, EFL_UI_SELECT_MODE_SINGLE));
   efl_text_set(o, "Singleselect");
   efl_pack_end(bx, o);
   o = efl_add(EFL_UI_RADIO_CLASS, bx,
           efl_ui_radio_state_value_set(efl_added, EFL_UI_SELECT_MODE_MULTI));
   efl_text_set(o, "Multiselect");
   efl_pack_end(bx, o);
}

void test_efl_ui_collection_grid(void *data EINA_UNUSED,
                                   Efl_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   create_item_container_ui(EFL_UI_GRID_CLASS, EFL_UI_GRID_DEFAULT_ITEM_CLASS, "Efl.Ui.Grid");
}

void test_efl_ui_collection_list(void *data EINA_UNUSED,
                                   Efl_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   create_item_container_ui(EFL_UI_LIST_CLASS, EFL_UI_LIST_DEFAULT_ITEM_CLASS, "Efl.Ui.List");
}
