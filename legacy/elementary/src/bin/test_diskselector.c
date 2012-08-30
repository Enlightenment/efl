#include "test.h"
#include <Elementary_Cursor.h>
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
   SCROLLER_POLICY_SET_ON,
   SCROLLER_POLICY_SET_OFF,
   BOUNCE_SET,
   ITEM_LABEL_SET,
   TOOLTIP_TEXT_SET,
   ITEM_TOOLTIP_UNSET,
   ITEM_CURSOR_SET,
   ITEM_CURSOR_UNSET,
   ITEM_ICON_SET,
   SELECTED_SET,
   ITEM_PREV_GET,
   ITEM_DEL_NOT_SELECTED,
   ITEM_DEL, /* delete when selected */
   CLEAR,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   const Eina_List *disks = elm_box_children_get(api->box);
   if (!eina_list_count(disks))
     return;

   switch(api->state)
     { /* Put all api-changes under switch */
      case SCROLLER_POLICY_SET_ON: /* 0 */
           {  /* Get first disk */
              Evas_Object *disk = eina_list_nth(disks, 0);
              elm_scroller_policy_set(disk, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
           }
         break;

      case SCROLLER_POLICY_SET_OFF: /* 1 */
           {  /* Get first disk */
              Evas_Object *disk = eina_list_nth(disks, 0);
              elm_scroller_policy_set(disk, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
           }
         break;

      case BOUNCE_SET: /* 2 */
           {  /* Get second disk, cancel bounce */
              Evas_Object *disk = eina_list_nth(disks, 1);
              elm_scroller_bounce_set(disk, EINA_FALSE, EINA_FALSE);
           }
         break;

      case ITEM_LABEL_SET: /* 3 */
         elm_object_item_text_set(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)), "Label from API");
         break;

      case TOOLTIP_TEXT_SET: /* 4 */
         elm_object_item_tooltip_text_set(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)), "Tooltip from API");
         break;

      case ITEM_TOOLTIP_UNSET: /* 5 */
         elm_object_item_tooltip_unset(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)));
         break;

      case ITEM_CURSOR_SET: /* 6 */
         elm_object_item_cursor_set(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)), ELM_CURSOR_HAND2);
         break;

      case ITEM_CURSOR_UNSET: /* 7 */
         elm_object_item_cursor_unset(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)));
         break;

      case ITEM_ICON_SET: /* 8 */
           {  /* Set icon of selected item in first disk */
              char buf[PATH_MAX];
              Evas_Object *ic = elm_icon_add(elm_object_parent_widget_get(eina_list_nth(disks, 0)));
              snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
              elm_image_file_set(ic, buf, NULL);
              elm_object_item_part_content_set(elm_diskselector_selected_item_get(eina_list_nth(disks, 0)), NULL, ic);
              evas_object_show(ic);
           }
         break;

      case SELECTED_SET: /* 9 */
           {  /* Select NEXT item of third disk */
              elm_diskselector_item_selected_set(elm_diskselector_last_item_get(eina_list_nth(disks, 1)), EINA_TRUE);
              elm_diskselector_item_selected_set(elm_diskselector_item_next_get(elm_diskselector_selected_item_get(eina_list_nth(disks, 2))), EINA_TRUE);
           }
         break;

      case ITEM_PREV_GET: /* 10 */
           {  /* Select PREV item of third disk */
              elm_diskselector_item_selected_set(elm_diskselector_item_prev_get(elm_diskselector_selected_item_get(eina_list_nth(disks, 2))), EINA_TRUE);
           }
         break;

      case ITEM_DEL_NOT_SELECTED: /* 11 */
           {  /* Remove selected item of third disk */
              elm_object_item_del(elm_diskselector_item_prev_get(elm_diskselector_selected_item_get(eina_list_nth(disks, 2))));
           }
         break;

      case ITEM_DEL: /* 12 */
           {  /* Remove selected item of first disk */
              elm_object_item_del(elm_diskselector_selected_item_get(eina_list_nth(disks, 1)));
           }
         break;

      case CLEAR: /* 13 */
         elm_diskselector_clear(eina_list_nth(disks, 0));
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
_disk_sel(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
   Elm_Object_Item *ds_it = event_info;
   printf("Equinox: %s\n", elm_object_item_text_get(ds_it));
}

static void
_disk_next(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
   Elm_Object_Item *next_ds_it, *prev_ds_it, *ds_it = event_info;
   prev_ds_it = elm_diskselector_item_prev_get(ds_it);
   next_ds_it = elm_diskselector_item_next_get(ds_it);
   printf("Prev: %s, Next: %s\n", elm_object_item_text_get(prev_ds_it),
          elm_object_item_text_get(next_ds_it));
}

static void
_print_disk_info_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Object_Item *ds_it = event_info;
   printf("Selected label: %s\n", elm_object_item_text_get(ds_it));
}

static void
_item_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Object_Item *ds_it = event_info;
   printf("Clicked label: %s\n", elm_object_item_text_get(ds_it));
}

static Evas_Object *
_disk_create(Evas_Object *win, Eina_Bool rnd)
{
   Elm_Object_Item *ds_it;
   Evas_Object *di;

   di = elm_diskselector_add(win);

   elm_diskselector_item_append(di, "January", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "February", NULL, _disk_next, NULL);
   elm_diskselector_item_append(di, "March", NULL, _disk_sel, NULL);
   elm_diskselector_item_append(di, "April", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "May", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "June", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "July", NULL, NULL, NULL);
   ds_it = elm_diskselector_item_append(di, "August", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "September", NULL, _disk_sel, NULL);
   elm_diskselector_item_append(di, "October", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "November", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "December", NULL, NULL, NULL);

   elm_diskselector_item_selected_set(ds_it, EINA_TRUE);
   elm_diskselector_round_enabled_set(di, rnd);

   return di;
}

static void
_cleanup_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

void
test_diskselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *disk, *ic, *bxx, *bt;
   Elm_Object_Item *ds_it;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));
   int idx = 0;

   char *month_list[] = {
      "Jan", "Feb", "Mar",
      "Apr", "May", "Jun",
      "Jul", "Aug", "Sep",
      "Oct", "Nov", "Dec"
   };
   char date[3];

   win = elm_win_util_standard_add("diskselector", "Disk Selector");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   elm_win_resize_object_add(win, bxx);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
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

   disk = _disk_create(win, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   ds_it = elm_diskselector_selected_item_get(disk);
   elm_diskselector_item_selected_set(ds_it, EINA_FALSE);

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   ds_it = elm_diskselector_first_item_get(disk);
   ds_it = elm_diskselector_item_next_get(ds_it);
   elm_diskselector_item_selected_set(ds_it, EINA_TRUE);

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   elm_diskselector_side_text_max_length_set(disk, 4);

   disk = elm_diskselector_add(win);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 1, 1);
   elm_diskselector_item_append(disk, "Sunday", ic, NULL, NULL);
   elm_diskselector_item_append(disk, "Monday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Tuesday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Wednesday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Thursday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Friday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Saturday", NULL, NULL, NULL);
   elm_diskselector_round_enabled_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   disk = elm_diskselector_add(win);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 1, 1);
   elm_diskselector_item_append(disk, "머리스타일", ic, NULL, NULL);
   elm_diskselector_item_append(disk, "プロが伝授する", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "生上访要求政府", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "English", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "والشريعة", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "עִבְרִית", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Grüßen", NULL, NULL, NULL);
   elm_diskselector_round_enabled_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   // displayed item number setting example
   disk = elm_diskselector_add(win);
   elm_diskselector_display_item_num_set(disk, 5);
   printf("Number of Items in DiskSelector : %d\n",
          elm_diskselector_display_item_num_get(disk));

   for (idx = 0; idx < (int)(sizeof(month_list) / sizeof(month_list[0])); idx++)
     {
        ds_it = elm_diskselector_item_append(disk, month_list[idx], NULL, NULL,
                                             NULL);
     }

   elm_diskselector_item_selected_set(ds_it, EINA_TRUE);
   elm_diskselector_round_enabled_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   evas_object_smart_callback_add(disk, "clicked", _item_clicked_cb, NULL);

   // displayed item number setting example
   disk = elm_diskselector_add(win);
   elm_diskselector_display_item_num_set(disk, 7);
   printf("Number of Items in DiskSelector : %d\n",
          elm_diskselector_display_item_num_get(disk));

   for (idx = 1; idx < 31; idx++)
     {
        snprintf(date, sizeof(date), "%d", idx);
        ds_it = elm_diskselector_item_append(disk, date, NULL, NULL, NULL);
     }

   elm_diskselector_item_selected_set(ds_it, EINA_TRUE);
   elm_diskselector_round_enabled_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
