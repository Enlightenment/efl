#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *box;           /* Use this to get box content     */
};
typedef struct _api_data api_data;

enum _api_state
{
   HOVERSEL_HORIZ,
   HOVERSEL_END,
   HOVERSEL_LABAL_SET,
   HOVERSEL_ICON_UNSET,
   HOVERSEL_CLEAR_OPEN,
   HOVERSEL_CLEAR,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Evas_Object *icon;

   const Eina_List *items = elm_box_children_get(api->box);
   if (!eina_list_count(items))
     return;

   /* use elm_box_children_get() to get list of children */
   switch(api->state)
     { /* Put all api-changes under switch */
      case HOVERSEL_HORIZ:  /* Make first hover horiz (0) */
         elm_hoversel_horizontal_set(eina_list_nth(items, 0), EINA_TRUE);
         elm_hoversel_hover_begin(eina_list_nth(items, 0));
         break;

      case HOVERSEL_END:  /* Make first hover horiz (1) */
         elm_hoversel_hover_begin(eina_list_nth(items, 1));
         elm_hoversel_hover_end(eina_list_nth(items, 1));
         break;

      case HOVERSEL_LABAL_SET: /* set second hover label (2) */
         elm_object_text_set(eina_list_nth(items, 1), "Label from API");
         break;

      case HOVERSEL_ICON_UNSET: /* 3 */
         elm_object_text_set(eina_list_nth(items, 5), "Label only");
         icon = elm_object_part_content_unset(eina_list_nth(items, 5), "icon");
         evas_object_del(icon);
         break;

      case HOVERSEL_CLEAR_OPEN: /* 4 */
         elm_hoversel_hover_begin(eina_list_nth(items, 1));
         elm_hoversel_clear(eina_list_nth(items, 1));
         break;

      case HOVERSEL_CLEAR: /* 5 */
         elm_hoversel_clear(eina_list_nth(items, 0));
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

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

static void
_hoversel_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   printf("Hover button is clicked and 'clicked' callback is called.\n");
}

static void
_hoversel_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);

   printf("'selected' callback is called. (selected item : %s)\n", txt);
   elm_object_text_set(obj, txt);
}

static void
_hoversel_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("'dismissed' callback is called.\n");
}

static void
_hoversel_expanded_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = data;

   printf("'expanded' callback is called.\n");
   printf("old style: %s\n", elm_object_item_style_get(it));
   //item type is button. set the style of button
   elm_object_item_style_set(it, "anchor");
   printf("new style: %s\n", elm_object_item_style_get(it));
}

void
test_hoversel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *ic, *bxx, *hoversel;
   Elm_Object_Item *it;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("hoversel", "HoverSel");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   api->box = bx;
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, bx);

   hoversel = elm_hoversel_add(win);
// FIXME: need to add horizontal hoversel theme to default some day
//   elm_hoversel_horizontal_set(bt, EINA_TRUE);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Labels");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 4 - Long Label Here", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_smart_callback_add(hoversel, "clicked",
                                  _hoversel_clicked_cb, NULL);
   evas_object_smart_callback_add(hoversel, "selected",
                                  _hoversel_selected_cb, NULL);
   evas_object_smart_callback_add(hoversel, "dismissed",
                                  _hoversel_dismissed_cb, NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Some Icons");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "All Icons");
   elm_hoversel_item_add(hoversel, "Item 1", "apps", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 2", "arrow_down", ELM_ICON_STANDARD,
                         NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "All Icons");
   elm_hoversel_item_add(hoversel, "Item 1", "apps", ELM_ICON_STANDARD, NULL,
                         NULL);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_hoversel_item_add(hoversel, "Item 2", buf, ELM_ICON_FILE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Disabled Hoversel");
   elm_hoversel_item_add(hoversel, "Item 1", "apps", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 2", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_object_disabled_set(hoversel, EINA_TRUE);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Icon + Label");

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_object_part_content_set(hoversel, "icon", ic);
   evas_object_show(ic);

   elm_hoversel_item_add(hoversel, "Item 1", "apps", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 2", "arrow_down", ELM_ICON_STANDARD,
                         NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Custom Item Style");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 4", NULL, ELM_ICON_NONE, NULL, NULL);
   it = elm_hoversel_item_add(hoversel, "Manage items", NULL, ELM_ICON_NONE, NULL, NULL);
   evas_object_smart_callback_add(hoversel, "clicked",
                                  _hoversel_clicked_cb, NULL);
   evas_object_smart_callback_add(hoversel, "selected",
                                  _hoversel_selected_cb, NULL);
   evas_object_smart_callback_add(hoversel, "dismissed",
                                  _hoversel_dismissed_cb, NULL);
   //pass the last item as data and use elm_object_item_style_set() to change the item style.
   evas_object_smart_callback_add(hoversel, "expanded",
                                  _hoversel_expanded_cb, it);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);

   evas_object_resize(win, 320, 500);

   evas_object_show(win);
}

static void
_item_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,focused: %p\n", it);
}

static void
_item_unfocused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,unfocused: %p\n", it);
}

void
test_hoversel_focus(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *hoversel;

   win = elm_win_util_standard_add("hoversel focus", "Hoversel Focus");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Vertical");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 4 - Long Label Here", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_smart_callback_add(hoversel, "clicked",
                                  _hoversel_clicked_cb, NULL);
   evas_object_smart_callback_add(hoversel, "selected",
                                  _hoversel_selected_cb, NULL);
   evas_object_smart_callback_add(hoversel, "dismissed",
                                  _hoversel_dismissed_cb, NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);
   elm_object_focus_set(hoversel, EINA_TRUE);
   evas_object_smart_callback_add(hoversel, "item,focused", _item_focused_cb, NULL);
   evas_object_smart_callback_add(hoversel, "item,unfocused", _item_unfocused_cb, NULL);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_horizontal_set(hoversel, EINA_TRUE);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Horizontal");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);
   evas_object_smart_callback_add(hoversel, "item,focused", _item_focused_cb, NULL);
   evas_object_smart_callback_add(hoversel, "item,unfocused", _item_unfocused_cb, NULL);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Icons");
   elm_hoversel_item_add(hoversel, "Item 1", "apps", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 2", "arrow_down", ELM_ICON_STANDARD,
                         NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_hoversel_item_add(hoversel, "Item 4", "close", ELM_ICON_STANDARD, NULL,
                         NULL);
   elm_box_pack_end(bx, hoversel);
   evas_object_show(hoversel);
   evas_object_smart_callback_add(hoversel, "item,focused", _item_focused_cb, NULL);
   evas_object_smart_callback_add(hoversel, "item,unfocused", _item_unfocused_cb, NULL);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);
}
