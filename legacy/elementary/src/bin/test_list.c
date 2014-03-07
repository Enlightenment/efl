#include "test.h"
#include <Elementary_Cursor.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *list;
};
typedef struct _api_data api_data;

enum _api_state
{
   ITEM_PREPEND,        /* 0 */
   ITEM_INSERT_BEFORE,  /* 1 */
   ITEM_INSERT_AFTER,   /* 2 */
   ITEM_SEPARATOR_SET,  /* 3 */
   LIST_ITEM_DEL,       /* 4 */
   SCROLLER_POLICY_SET_ON,
   SCROLLER_POLICY_SET_OFF, /* Back to AUTO next */
   TOOLTIP_TEXT_SET,    /* 7 */
   TOOLTIP_UNSET,       /* 8 */
   ITEM_CURSOR_SET,     /* 9 */
   ITEM_CURSOR_STYLE_SET,
   DISABLED_SET,        /* 11 */
   MODE_SET_COMPRESS,   /* 12 */
   MODE_SET_LIMIT,      /* 13 */
   MODE_SET_EXPAND,     /* 14 */
   HORIZONTAL_SET,      /* 15 */
   BOUNCE_SET,          /* 16 */
   LIST_CLEAR,          /* 17 */
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
/** HOW TO TEST ************************
0 ITEM PREPEND
Scroll to end
1 INSERT BEFORE
Scroll to end
2 INSERT AFTER
3 INSERT SEPERATOR
Scroll to end
4 ITEM DEL
5 POLICY ON, BOUNCE_SET(TRUE, TRUE)
6 POLICY OFF
Scroll to end
7 TOOLTIP last-item
8 Cancel tootip
9 Curosr set on last item
10 Cursor style set last item
11 DISABLE last item
12 MODE COMPRESS
13 MODE LIMIT
14 MODE EXPAND
15 HORIZ SET
16 VERT MODE, BOUNCE(TRUE, FALSE) try to bounce on Y-axis
17 List clear
*** HOW TO TEST ***********************/
   Evas_Object *li = api->list;

   switch(api->state)
     { /* Put all api-changes under switch */
      case ITEM_PREPEND: /* 0 */
           {
              const Eina_List *items = elm_list_items_get(li);
              elm_list_item_prepend(li, "PREPEND", NULL, NULL, NULL, NULL);
              elm_list_go(li);
              elm_list_item_bring_in(eina_list_nth(items, 0));
           }
         break;

      case ITEM_INSERT_BEFORE: /* 1 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_list_item_insert_before(li,
                        eina_list_nth(items, eina_list_count(items)-1),
                        "1-before-last", NULL, NULL, NULL, NULL);
                  elm_list_go(li);
                  elm_list_item_bring_in(eina_list_data_get(eina_list_last(items)));
                }
           }
         break;

      case ITEM_INSERT_AFTER: /* 2 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_list_item_insert_after(li,
                        eina_list_nth(items, eina_list_count(items)-2),
                        "insert-after", NULL, NULL, NULL, NULL);
                  elm_list_go(li);
                  elm_list_item_bring_in(eina_list_data_get(eina_list_last(items)));
                }
           }
         break;

      case ITEM_SEPARATOR_SET: /* 3 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_list_item_separator_set(eina_list_nth(items, eina_list_count(items)-3), EINA_TRUE);
                  elm_list_item_bring_in(eina_list_nth(items, eina_list_count(items)-3));
                  elm_list_go(li);
                }
           }
         break;

      case LIST_ITEM_DEL: /* 4 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_del(eina_list_data_get(eina_list_last(items)));
                }
           }
         break;

      case SCROLLER_POLICY_SET_ON: /* 5 */
         elm_scroller_bounce_set(li, EINA_TRUE, EINA_TRUE);
         elm_scroller_policy_set(li, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
         break;

      case SCROLLER_POLICY_SET_OFF: /* Back to AUTO next (6) */
         elm_scroller_policy_set(li, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
         break;

      case TOOLTIP_TEXT_SET: /* 7 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_tooltip_text_set(eina_list_data_get(eina_list_last(items)), "Tooltip set from API");
                }
              elm_scroller_policy_set(li, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
           }
         break;

      case TOOLTIP_UNSET: /* 8 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_tooltip_unset(eina_list_data_get(eina_list_last(items)));
                }
           }
         break;

      case ITEM_CURSOR_SET: /* 9 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_cursor_set(eina_list_data_get(eina_list_last(items)), ELM_CURSOR_HAND2);
                }
           }
         break;

      case ITEM_CURSOR_STYLE_SET: /* 10 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_cursor_style_set(eina_list_data_get(eina_list_last(items)), "transparent");
                }
           }
         break;

      case DISABLED_SET: /* 11 */
           {
              const Eina_List *items = elm_list_items_get(li);
              if (eina_list_count(items))
                {
                  elm_object_item_disabled_set(eina_list_data_get(eina_list_last(items)), EINA_TRUE);
                }
           }
         break;

      case MODE_SET_COMPRESS: /* 12 */
         elm_list_mode_set(li, ELM_LIST_COMPRESS);
         break;

      case MODE_SET_LIMIT: /* 13 */
         elm_list_mode_set(li, ELM_LIST_LIMIT);
         break;

      case MODE_SET_EXPAND: /* 14 */
         elm_list_mode_set(li, ELM_LIST_EXPAND);
         break;

      case HORIZONTAL_SET: /* 15 */
         elm_list_mode_set(li, ELM_LIST_SCROLL); /* return to default mode */
         elm_list_horizontal_set(li, EINA_TRUE);
         break;

      case BOUNCE_SET: /* 16 */
         elm_list_horizontal_set(li, EINA_FALSE);
         elm_scroller_bounce_set(li, EINA_TRUE, EINA_FALSE);
         break;

      case LIST_CLEAR: /* 17 */
         elm_list_clear(li);
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
my_show_it(void        *data,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   elm_list_item_show(data);
}

static void
scroll_top(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   printf("Top edge!\n");
}

static void
scroll_bottom(void        *data EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void        *event_info EINA_UNUSED)
{
   printf("Bottom edge!\n");
}

static void
scroll_left(void        *data EINA_UNUSED,
            Evas_Object *obj EINA_UNUSED,
            void        *event_info EINA_UNUSED)
{
   printf("Left edge!\n");
}

static void
scroll_right(void        *data EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void        *event_info EINA_UNUSED)
{
   printf("Right edge!\n");
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   free(data);
}

void
test_list(void        *data EINA_UNUSED,
          Evas_Object *obj EINA_UNUSED,
          void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *ic, *ic2, *bx, *tb2, *bt, *bxx;
   char buf[PATH_MAX];
   Elm_Object_Item *list_it1, *list_it2, *list_it3, *list_it4, *list_it5;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("list", "List");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   li = elm_list_add(win);
   elm_list_mode_set(li, ELM_LIST_LIMIT);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   api->list = li;

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, li);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   list_it1 = elm_list_item_append(li, "Hello", ic, NULL, NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL, NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, ".", ic, NULL, NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   list_it2 = elm_list_item_append(li, "How", ic, ic2, NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx, NULL, NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);
   list_it3 = elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);
   list_it4 = elm_list_item_append(li, "Here", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL, NULL, NULL);
   list_it5 = elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL, NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Hello");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it1);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "How");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it2);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "doing");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it3);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Here");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it4);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 3, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Maybe this...");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it5);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 4, 1, 1);
   evas_object_show(bt);

   evas_object_show(tb2);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);

   evas_object_smart_callback_add(li, "edge,top", scroll_top, NULL);
   evas_object_smart_callback_add(li, "edge,bottom", scroll_bottom, NULL);
}

void
test_list_horizontal(void        *data EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *ic, *ic2, *bx, *tb2, *bt;
   char buf[PATH_MAX];
   Elm_Object_Item *list_it1, *list_it2, *list_it3, *list_it4;

   win = elm_win_util_standard_add("list-horizontal", "List Horizontal");
   elm_win_autodel_set(win, EINA_TRUE);

   li = elm_list_add(win);
   elm_list_horizontal_set(li, EINA_TRUE);
   elm_list_mode_set(li, ELM_LIST_LIMIT);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, li);

   evas_object_smart_callback_add(li, "edge,left", scroll_left, NULL);
   evas_object_smart_callback_add(li, "edge,right", scroll_right, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   list_it1 = elm_list_item_append(li, "Hello", ic, NULL, NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL, NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, ".", ic, NULL, NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   list_it2 = elm_list_item_append(li, "How", ic, ic2, NULL, NULL);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   elm_list_item_append(li, "are", bx, NULL, NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);
   list_it3 = elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);

   list_it4 = elm_list_item_append(li, "And", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "here", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "we", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "done", NULL, NULL, NULL, NULL);
   elm_object_item_disabled_set(elm_list_item_append(li, "with", NULL, NULL, NULL, NULL), EINA_TRUE);
   elm_object_item_disabled_set(elm_list_item_append(li, "items.", NULL, NULL, NULL, NULL), EINA_TRUE);

   elm_list_go(li);

   evas_object_show(li);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Hello");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it1);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "How");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it2);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "doing");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it3);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "And");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, list_it4);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 3, 0, 1, 1);
   evas_object_show(bt);

   evas_object_show(tb2);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/***********/

static void
my_li2_clear(void        *data,
             Evas_Object *obj EINA_UNUSED,
             void        *event_info EINA_UNUSED)
{
   elm_list_clear(data);
}

static void
my_li2_sel(void        *data EINA_UNUSED,
           Evas_Object *obj,
           void        *event_info EINA_UNUSED)
{
   Elm_Object_Item *list_it = elm_list_selected_item_get(obj);
   elm_list_item_selected_set(list_it, EINA_FALSE);
   printf("item selected\n");
}

void
test_list2(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx, *bx2, *bt;
   char buf[PATH_MAX];
   Elm_Object_Item *list_it;

   win = elm_win_add(NULL, "list2", ELM_WIN_BASIC);
   elm_win_title_set(win, "List 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   li = elm_list_add(bx);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_mode_set(li, ELM_LIST_LIMIT);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Unselect on selected", ic, NULL, my_li2_sel, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   list_it = elm_list_item_append(li, "world", ic, NULL, NULL, NULL);
   elm_list_item_selected_set(list_it, EINA_TRUE);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, ".", ic, NULL, NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, "How", ic, ic2, NULL, NULL);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx2, NULL, NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Longer label.", NULL, NULL, NULL, NULL);

   elm_list_go(li);

   elm_box_pack_end(bx, li);
   evas_object_show(li);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_li2_clear, li);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);
}

/***********/

static void
_bt_clicked(void        *data EINA_UNUSED,
            Evas_Object *obj EINA_UNUSED,
            void        *event_info EINA_UNUSED)
{
   printf("button was clicked\n");
}

static void
_it_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   printf("item was clicked\n");
   if (!data) return;
   Evas_Object *li = data;
   Evas_Object *lb;
   char str[128];

   Elm_Object_Item *lit = elm_list_selected_item_get(li);
   if (!lit) return;
   sprintf(str, "%s is selected", elm_object_item_text_get(lit));

   lb = evas_object_data_get(li, "label");
   elm_object_text_set(lb, str);
}

void
test_list3(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *ic, *ic2, *bx;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("list3", "List 3");
   elm_win_autodel_set(win, EINA_TRUE);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, li);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   ic2 = elm_button_add(win);
   elm_object_text_set(ic2, "Click me");
   evas_object_smart_callback_add(ic2, "clicked", _bt_clicked, NULL);
   evas_object_propagate_events_set(ic2, EINA_FALSE);
   elm_list_item_append(li, "Hello", ic, ic2, _it_clicked, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   ic2 = elm_button_add(win);
   elm_object_text_set(ic2, "Click me");
   evas_object_smart_callback_add(ic2, "clicked", _bt_clicked, NULL);
   elm_list_item_append(li, "world", ic, ic2, _it_clicked, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, ".", ic, NULL, NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, "How", ic, ic2, NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   elm_list_item_append(li, "are", bx, NULL, NULL, NULL);
   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL, NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

///////////////////////////////////////////////////////////////////////////////////////

struct Pginfo
{
   Evas_Object *naviframe, *win;
};

static void
test_list4_back_cb(void        *data,
                   Evas_Object *obj EINA_UNUSED,
                   void        *event_info EINA_UNUSED)
{
   struct Pginfo *info = data;
   if (!info) return;

   elm_naviframe_item_pop(info->naviframe);
}

static void
test_list4_swipe(void        *data,
                 Evas_Object *obj EINA_UNUSED,
                 void        *event_info)
{
   Evas_Object *box, *entry, *button;
   struct Pginfo *info = data;
   char *item_data;
   if ((!event_info) || (!data)) return;

   item_data = elm_object_item_data_get(event_info);

   box = elm_box_add(info->win);
   elm_box_horizontal_set(box, EINA_FALSE);
   elm_box_homogeneous_set(box, EINA_FALSE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);

   entry = elm_entry_add(info->win);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_editable_set(entry, EINA_FALSE);
   elm_object_text_set(entry, item_data);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(entry);

   button = elm_button_add(info->win);
   elm_object_text_set(button, "back");
   evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
   evas_object_smart_callback_add(button, "clicked", test_list4_back_cb, info);
   evas_object_show(button);

   elm_box_pack_start(box, entry);
   elm_box_pack_end(box, button);

   elm_naviframe_item_simple_push(info->naviframe, box);
}

void
test_list4(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *ic, *ic2, *naviframe;
   static struct Pginfo info = {NULL, NULL};
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("list4", "List 4");
   elm_win_autodel_set(win, EINA_TRUE);
   info.win = win;

   naviframe = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, naviframe);
   evas_object_show(naviframe);
   info.naviframe = naviframe;

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(li, "swipe", test_list4_swipe, &info);
   elm_naviframe_item_simple_push(naviframe, li);

   static char pf_data[] = "Pink Floyd were formed in 1965, and originally consisted of university"                       \
                           "students Roger Waters, Nick Mason, Richard Wright, and Syd Barrett. The group were a popular" \
                           "fixture on London's underground music scene, and under Barrett's leadership released two "    \
                           "charting singles, \"Arnold Layne\" and \"See Emily Play\", and a successful debut album, "    \
                           "ThePiper at the Gates of Dawn. In 1968, guitarist and singer David Gilmour joined the "       \
                           "line-up. Barrett was soon removed, due to his increasingly erratic behaviour. Following "     \
                           "Barrett's departure, bass player and singer Roger Waters became the band's lyricist and "     \
                           "conceptual leader, with Gilmour assuming lead guitar and much of the vocals. With this "      \
                           "line-up, Floyd went on to achieve worldwide critical and commercial success with the "        \
                           "conceptalbums The Dark Side of the Moon, Wish You Were Here, Animals, and The Wall.";
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/mystrale.jpg", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Pink Floyd", ic, NULL, NULL, &pf_data);

   static char ds_data[] = "Dire Straits were a British rock band, formed by Mark Knopfler "                          \
                           "(lead vocals and lead guitar), his younger brother David Knopfler (rhythm guitar and "    \
                           "backing vocals), John Illsley (bass guitar and backing vocals), and Pick Withers (drums " \
                           "and percussion), and managed by Ed Bicknell, active between 1977 and 1995. Although the " \
                           "band was formed in an era when punk rock was at the forefront, Dire Straits played a more "
                           "bluesy style, albeit with a stripped-down sound that appealed to audiences weary of the "   \
                           "overproduced stadium rock of the 1970s.[citation needed] In their early days, Mark and "    \
                           "David requested that pub owners turn down their sound so that patrons could converse "      \
                           "while the band played, an indication of their unassuming demeanor. Despite this oddly "     \
                           "self-effacing approach to rock and roll, Dire Straits soon became hugely successful, with " \
                           "their first album going multi-platinum globally.";
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/mystrale_2.jpg", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Dire Straits", ic, NULL, NULL, &ds_data);

   static char uh_data[] = "Uriah Heep are an English hard rock band. The band released several "                       \
                           "commercially successful albums in the 1970s such as Uriah Heep Live (1973), but their "     \
                           "audience declined by the 1980s, to the point where they became essentially a cult band in " \
                           "the United States and United Kingdom. Uriah Heep maintain a significant following in "      \
                           "Germany, the Netherlands, Scandinavia, the Balkans, Japan and Russia, where they still "    \
                           "perform at stadium-sized venues.";
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_17.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Uriah Heep", ic, NULL, NULL, &uh_data);

   static char r_data[] = "Rush is a Canadian rock band formed in August 1968, in the Willowdale "                       \
                          "neighbourhood of Toronto, Ontario. The band is composed of bassist, keyboardist, and lead "   \
                          "vocalist Geddy Lee, guitarist Alex Lifeson, and drummer and lyricist Neil Peart. The band "   \
                          "and its membership went through a number of re-configurations between 1968 and 1974, "        \
                          "achieving their current form when Peart replaced original drummer John Rutsey in July 1974, " \
                          "two weeks before the group's first United States tour.";
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_21.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, "Rush", ic, ic2, NULL, &r_data);

   elm_list_go(li);

   evas_object_show(li);
   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/////////////////////////////////////////////////////////////////////////////////////////
struct list5_data_cb
{
   Evas_Object *win, *list;
};

static void
test_list5_item_del(void        *data,
                    Evas_Object *obj EINA_UNUSED,
                    void        *event_info EINA_UNUSED)
{
   elm_object_item_del(data);
}

static void
test_list5_swipe(void        *data EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void        *event_info)
{
   Evas_Object *button;
   struct list5_data_cb *info = elm_object_item_data_get(event_info);

   if (elm_object_item_part_content_get(event_info, "end")) return;

   button = elm_button_add(info->win);
   elm_object_text_set(button, "delete");
   evas_object_propagate_events_set(button, EINA_FALSE);
   evas_object_smart_callback_add(button, "clicked", test_list5_item_del,
                                  event_info);
   elm_object_item_part_content_set(event_info, "end", button);
   elm_object_item_part_text_set(event_info, "default", "Enlightenment");
}

void
test_list5(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li;
   static struct list5_data_cb info;

   win = elm_win_util_standard_add("list5", "List 5");
   elm_win_autodel_set(win, EINA_TRUE);
   info.win = win;

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(li, "swipe", test_list5_swipe, NULL);
   elm_win_resize_object_add(win, li);
   evas_object_show(li);
   info.list = li;

   elm_list_item_append(li, "Network", NULL, NULL, NULL, &info);
   elm_list_item_append(li, "Audio", NULL, NULL, NULL, &info);

   elm_list_go(li);
   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

static void
_first_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   char str[128];
   Evas_Object *li = data, *lb;
   Elm_Object_Item *lit = elm_list_first_item_get(li);
   if (!lit) return;

   sprintf(str, "%s is selected", elm_object_item_text_get(lit));
   elm_list_item_bring_in(lit);
   elm_list_item_selected_set(lit, EINA_TRUE);

   lb = evas_object_data_get(li, "label");
   elm_object_text_set(lb, str);
}

static void
_prev_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   char str[128];
   Evas_Object *li = data, *lb;

   Elm_Object_Item *lit = elm_list_selected_item_get(li);
   if (!lit) return;
   lit = elm_list_item_prev(lit);
   if (!lit) return;

   sprintf(str, "%s is selected", elm_object_item_text_get(lit));
   elm_list_item_bring_in(lit);
   elm_list_item_selected_set(lit, EINA_TRUE);

   lb = evas_object_data_get(li, "label");
   elm_object_text_set(lb, str);
}

static void
_next_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   char str[128];
   Evas_Object *li = data, *lb;

   Elm_Object_Item *lit = elm_list_selected_item_get(li);
   if (!lit) return;
   lit = elm_list_item_next(lit);
   if (!lit) return;

   sprintf(str, "%s is selected", elm_object_item_text_get(lit));
   elm_list_item_bring_in(lit);
   elm_list_item_selected_set(lit, EINA_TRUE);

   lb = evas_object_data_get(li, "label");
   elm_object_text_set(lb, str);
}

static void
_last_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   char str[128];
   Evas_Object *li = data, *lb;
   Elm_Object_Item *lit = elm_list_last_item_get(li);
   if (!lit) return;

   sprintf(str, "%s is selected", elm_object_item_text_get(lit));
   elm_list_item_bring_in(lit);
   elm_list_item_selected_set(lit, EINA_TRUE);

   lb = evas_object_data_get(li, "label");
   elm_object_text_set(lb, str);
}

void
test_list6(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *gd, *bt, *li, *lb;

   win = elm_win_util_standard_add("list6", "List 6");
   elm_win_autodel_set(win, EINA_TRUE);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gd);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   elm_grid_pack(gd, li, 4, 4, 92, 72);
   evas_object_show(li);

   elm_list_item_append(li, "Eina", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Eet", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Evas", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Ecore", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Embryo", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Edje", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Efreet", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "E_dbus", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Eeze", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Expedite", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Emotion", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Ethumb", NULL, NULL, _it_clicked, li);
   elm_list_item_append(li, "Elementary", NULL, NULL, _it_clicked, li);
   elm_list_go(li);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "First");
   evas_object_smart_callback_add(bt, "clicked", _first_bt_clicked, li);
   elm_grid_pack(gd, bt, 4, 80, 20, 10);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prev");
   evas_object_smart_callback_add(bt, "clicked", _prev_bt_clicked, li);
   elm_grid_pack(gd, bt, 28, 80, 20, 10);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next");
   evas_object_smart_callback_add(bt, "clicked", _next_bt_clicked, li);
   elm_grid_pack(gd, bt, 52, 80, 20, 10);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Last");
   evas_object_smart_callback_add(bt, "clicked", _last_bt_clicked, li);
   elm_grid_pack(gd, bt, 76, 80, 20, 10);
   evas_object_show(bt);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Nothing is selected");
   elm_grid_pack(gd, lb, 4, 90, 92, 10);
   evas_object_show(lb);

   evas_object_data_set(li, "label", lb);

   evas_object_show(gd);
   evas_object_resize(win, 480, 480);
   evas_object_show(win);
}

static void
_it_clicked_cb(void *data EINA_UNUSED, Evas_Object *li,
                 void *event_info EINA_UNUSED)
{
   Elm_Object_Item *lit = elm_list_selected_item_get(li);
   printf("Item clicked. %s is selected\n", elm_object_item_text_get(lit));
}

void
test_list7(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *li;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "list7", ELM_WIN_BASIC);
   elm_win_title_set(win, "List Always Select Mode");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, li);
   elm_list_select_mode_set(li, ELM_OBJECT_SELECT_MODE_ALWAYS);

   elm_list_item_append(li, "Items", NULL, NULL, _it_clicked_cb, NULL);
   elm_list_item_append(li, "callbacks", NULL, NULL, _it_clicked_cb, NULL);
   elm_list_item_append(li, "should be called", NULL, NULL, _it_clicked_cb,
                        NULL);
   elm_list_item_append(li, "only once, each time", NULL, NULL, _it_clicked_cb,
                        NULL);
   elm_list_item_append(li, "an item is clicked.", NULL, NULL, _it_clicked_cb,
                        NULL);
   elm_list_item_append(li, "Including already", NULL, NULL, _it_clicked_cb,
                        NULL);
   elm_list_item_append(li, "selected ones.", NULL, NULL, _it_clicked_cb, NULL);

   elm_list_go(li);
   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

static const unsigned _list_focus_objects = 5;
static const char *_list_focus_names[] = {"None", "Square", "Button", "Check", "Box"};
static const int _list_focus_combo[] = { 1, 0, 2, 33, 43, 44, 10, 30, 22, 11, 10, -1 };

static Evas_Object *
test_list8_content_get(Evas_Object *obj, unsigned type, Eina_Bool horiz)
{
   Evas_Object *cnt = NULL;

   switch(type)
     {
      case 1:
         cnt = elm_bg_add(obj);
         evas_object_color_set(cnt, 128, 18, 128, 255);
         evas_object_size_hint_min_set(cnt, 50, 50);
         break;
      case 2:
         cnt = elm_button_add(obj);
         break;
      case 3:
         cnt = elm_check_add(obj);
         break;
      case 4:
         cnt = elm_box_add(obj);
         elm_box_horizontal_set(cnt, !horiz);
         evas_object_size_hint_align_set(cnt, EVAS_HINT_FILL, EVAS_HINT_FILL);
         elm_box_pack_end(cnt, test_list8_content_get(obj, 2, horiz));
         elm_box_pack_end(cnt, test_list8_content_get(obj, 3, horiz));
         elm_box_pack_end(cnt, test_list8_content_get(obj, 2, horiz));
         break;
      default:
         break;

     }

   if (cnt)
   {
       evas_object_size_hint_weight_set(cnt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
       evas_object_show(cnt);
   }

   return cnt;
}

static void
test_list8_focus_on_selection_set(Evas_Object *gl, Evas_Object *chk, Eina_Bool focus)
{
    elm_list_focus_on_selection_set(gl, focus);
    elm_check_state_set(chk, focus);
    printf("list_focus_on_selection = %s\n", (focus) ? "true" : "false");
}

static void
test_list8_focus_check_changed(void *data, Evas_Object *obj, void *event_info  EINA_UNUSED)
{
   Eina_Bool nextstate = !elm_list_focus_on_selection_get(data);
   test_list8_focus_on_selection_set(data, obj, nextstate);
}

static void
test_list8_focus_animate_check_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_animate_set((Evas_Object *)data,
                                       elm_check_state_get(obj));
}

static void
_item_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("%s: %p\n", (char *)data, event_info);
}

static void
_list_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   printf("\n=== Key Down : %s ===\n", ev->keyname);
}

void test_list_focus(const char *name, const char *title, Eina_Bool horiz)
{
   Evas_Object *win, *li, *bx, *bxx, *chk;
   unsigned int lhand, rhand, idx;
   char buf[256];

   win = elm_win_util_standard_add(name, title);
   elm_win_autodel_set(win, EINA_TRUE);

   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_list_horizontal_set(li, horiz);
   elm_list_select_mode_set(li, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(bxx, li);
   evas_object_show(li);
   evas_object_smart_callback_add(li, "item,focused", _item_cb, "item,focused");
   evas_object_smart_callback_add(li, "item,unfocused", _item_cb, "item,unfocused");
   evas_object_smart_callback_add(li, "selected", _item_cb, "selected");
   evas_object_smart_callback_add(li, "unselected", _item_cb, "unselected");
   evas_object_smart_callback_add(li, "activated", _item_cb, "activated");
   evas_object_smart_callback_add(li, "highlighted", _item_cb, "highlighted");
   evas_object_smart_callback_add(li, "unhighlighted", _item_cb, "unhighlighted");
   evas_object_event_callback_add(li, EVAS_CALLBACK_KEY_DOWN, _list_key_down_cb, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   chk = elm_check_add(win);
   elm_object_text_set(chk, "Focus Animation");
   elm_check_state_set(chk, EINA_TRUE);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(chk, "changed",
                                  test_list8_focus_animate_check_changed, win);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   chk = elm_check_add(win);
   elm_object_text_set(chk, "Focus on selection");
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(chk, "changed", test_list8_focus_check_changed, li);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   test_list8_focus_on_selection_set(li, chk, EINA_FALSE);

   elm_box_pack_end(bxx, bx);

   for (idx = 0; _list_focus_combo[idx] >= 0; idx++)
     {
        lhand = _list_focus_combo[idx] / 10;
        rhand = _list_focus_combo[idx] % 10;

        if (lhand > 4) lhand = 4;
        if (rhand > 4) rhand = 4;
        snprintf(buf, sizeof(buf), " %s / %s ",
            _list_focus_names[lhand],
            _list_focus_names[rhand]);

        elm_list_item_append(li, buf,
                test_list8_content_get(li, lhand, horiz),
                test_list8_content_get(li, rhand, horiz),
                NULL, NULL);
     }

   elm_list_go(li);
   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_list8(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
    test_list_focus("list-focus", "List Focus", EINA_FALSE);
}

void
test_list9(void        *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void        *event_info EINA_UNUSED)
{
    test_list_focus("list-focus-horizontal", "List Focus Horizontal", EINA_TRUE);
}

void
test_list_separator(void        *data EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void        *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *ic, *ic2, *bx, *bxx;
   char buf[PATH_MAX];
   Elm_Object_Item *list_it_sep;

   win = elm_win_util_standard_add("list", "List Separator");
   elm_win_autodel_set(win, EINA_TRUE);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxx, li);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Hello", ic, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, ".", ic, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_image_resizable_set(ic2, EINA_FALSE, EINA_FALSE);
   elm_list_item_append(li, "How", ic, ic2, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);

   list_it_sep = elm_list_item_append(li, NULL, NULL, NULL, NULL, NULL);
   elm_list_item_separator_set(list_it_sep, EINA_TRUE);

   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/***********/

typedef struct _List_Multi_Data List_Multi_Data;
struct _List_Multi_Data
{
   Evas_Object *list;
   Evas_Object *rd1;
   Evas_Object *rd2;
};

static void
_multi_select_changed_cb(void *data, Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   Eina_Bool multi = elm_check_state_get(obj);
   List_Multi_Data *ld = data;
   if (!ld) return;

   elm_list_multi_select_set(ld->list, multi);
   elm_object_disabled_set(ld->rd1, !multi);
   elm_object_disabled_set(ld->rd2, !multi);
}

static void
_multi_select_mode_changed_cb(void *data, Evas_Object *obj,
                              void *event_info EINA_UNUSED)
{
   elm_list_multi_select_mode_set(data, elm_radio_value_get(obj));
}

static void
_multi_select_frame_create(Evas_Object *bx, List_Multi_Data *ld)
{
   Evas_Object *fr, *bx2, *bx3, *tg, *rd, *rdg;
   if (!ld) return;

   fr = elm_frame_add(bx);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(fr, "Multi Select Option");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(fr);
   elm_object_content_set(fr, bx2);
   evas_object_show(bx2);

   tg = elm_check_add(bx2);
   elm_object_style_set(tg, "toggle");
   elm_object_text_set(tg, "Multi Select Mode");
   elm_box_pack_end(bx2, tg);
   evas_object_show(tg);

   bx3 = elm_box_add(bx2);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   elm_box_pack_end(bx2, bx3);
   evas_object_show(bx3);

   ld->rd1 = rdg = rd = elm_radio_add(bx3);
   elm_radio_state_value_set(rd, ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT);
   elm_object_text_set(rd, "Default Mode");
   elm_box_pack_end(bx3, rd);
   evas_object_show(rd);
   elm_object_disabled_set(rd, EINA_TRUE);
   evas_object_smart_callback_add(rd, "changed",
                                  _multi_select_mode_changed_cb, ld->list);

   ld->rd2 = rd = elm_radio_add(bx3);
   elm_radio_state_value_set(rd, ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "With Control Mode");
   elm_box_pack_end(bx3, rd);
   evas_object_show(rd);
   elm_object_disabled_set(rd, EINA_TRUE);
   evas_object_smart_callback_add(rd, "changed",
                                  _multi_select_mode_changed_cb, ld->list);

   evas_object_smart_callback_add(tg, "changed",
                                  _multi_select_changed_cb, ld);
}

void
test_list_multi_select(void *data EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *bx;
   List_Multi_Data *ld = calloc(1, sizeof(List_Multi_Data));

   win = elm_win_util_standard_add("list-multi-select", "List Multi Select");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, ld);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ld->list = li = elm_list_add(bx);
   _multi_select_frame_create(bx, ld);

   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, li);
   evas_object_show(li);

   elm_list_item_append(li, "you", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Longer label.", NULL, NULL, NULL, NULL);

   elm_list_go(li);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);
}
