#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
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
         elm_object_part_content_unset(eina_list_nth(items, 5), "icon");
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
_cleanup_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

void
test_hoversel(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *bt, *ic, *bxx;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("hoversel", "HoverSel");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   elm_win_resize_object_add(win, bxx);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   api->box = bx;
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, bx);

   bt = elm_hoversel_add(win);
// FIXME: need to add horizontal hoversel theme to default some day
//   elm_hoversel_horizontal_set(bt, 1);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "Labels");
   elm_hoversel_item_add(bt, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4 - Long Label Here", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "Some Icons");
   elm_hoversel_item_add(bt, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "All Icons");
   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", "arrow_down", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "All Icons");
   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_hoversel_item_add(bt, "Item 2", buf, ELM_ICON_FILE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "Disabled Hoversel");
   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", "close", ELM_ICON_STANDARD, NULL, NULL);
   elm_object_disabled_set(bt, 1);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "Icon + Label");

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_object_part_content_set(bt, "icon", ic);
   evas_object_show(ic);

   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", "arrow_down", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 300);

   evas_object_show(win);
}
#endif
