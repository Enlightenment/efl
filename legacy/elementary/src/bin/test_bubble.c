#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_print_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("bubble clicked\n");
}

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   Evas_Object *win;    /* Parent Window of widgets        */
   void *box;           /* Use this to get box content     */
};
typedef struct _api_data api_data;

enum _api_state
{
   BUBBLE_SET_CORNER_1,
   BUBBLE_SET_CORNER_2,
   BUBBLE_SET_ICON_CONTENT,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   const Eina_List *items = elm_box_children_get(api->box);
   if (!eina_list_count(items))
     return;

   switch(api->state)
     { /* Put all api-changes under switch */
      case BUBBLE_SET_CORNER_1:
         elm_bubble_pos_set(eina_list_nth(items, 0), ELM_BUBBLE_POS_BOTTOM_LEFT);
         elm_object_text_set(elm_object_content_get(eina_list_nth(items, 0)),
                  "Corner: base (bottom-left) - with icon");
         elm_bubble_pos_set(eina_list_nth(items, 1), ELM_BUBBLE_POS_TOP_RIGHT);
         elm_object_text_set(elm_object_content_get(eina_list_nth(items, 1)),
                  "Corner: base (top-right) - no icon");
         break;

      case BUBBLE_SET_CORNER_2:
         elm_bubble_pos_set(eina_list_nth(items, 0), ELM_BUBBLE_POS_TOP_RIGHT);
         elm_object_text_set(elm_object_content_get(eina_list_nth(items, 0)),
                  "Corner: base (top-right) - with icon");
         elm_bubble_pos_set(eina_list_nth(items, 1), ELM_BUBBLE_POS_BOTTOM_LEFT);
         elm_object_text_set(elm_object_content_get(eina_list_nth(items, 1)),
                  "Corner: base (bottom-left) - no icon");
         break;

      case BUBBLE_SET_ICON_CONTENT:
           {
              char buf[PATH_MAX];
              Evas_Object *ct, *ic = elm_icon_add(api->win);

              snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
              elm_image_file_set(ic, buf, NULL);
              elm_image_resizable_set(ic, 0, 0);
              elm_object_content_set(eina_list_nth(items, 0), ic);
              ct = elm_label_add(api->win);
              elm_object_text_set(ct, "Using icon as top-bubble content");
              elm_object_content_set(eina_list_nth(items, 1), ct);
              evas_object_size_hint_align_set(ic, 0.5, 0.5);
              evas_object_show(ic);
           }
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
test_bubble(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *ic, *bb, *ct, *bxx, *bt;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("bubble", "Bubble");
   api->win = win;
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   elm_win_resize_object_add(win, bxx);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bxx);

   bx = elm_box_add(win);
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

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_part_text_set(bb, "info", "Corner: bottom_right");
   elm_object_part_content_set(bb, "icon", ic);
   elm_bubble_pos_set(bb, ELM_BUBBLE_POS_BOTTOM_RIGHT);
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ct = elm_label_add(win);
   elm_object_text_set(ct,
                       "\"The future of the art: R or G or B?\",  by Rusty");
   elm_object_content_set(bb, ct);

   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 2");
   elm_object_part_text_set(bb, "info", "10:32 4/11/2008");
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ct = elm_label_add(win);
   elm_object_text_set(ct, "Corner: base (top-left) - no icon");
   elm_object_content_set(bb, ct);

   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   evas_object_show(win);
}
#endif
