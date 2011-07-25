#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_disk_sel(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
   Elm_Diskselector_Item *it = event_info;
   printf("Equinox: %s\n", elm_diskselector_item_label_get(it));
}

static void
_disk_next(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
   Elm_Diskselector_Item *next, *prev, *it = event_info;
   prev = elm_diskselector_item_prev_get(it);
   next = elm_diskselector_item_next_get(it);
   printf("Prev: %s, Next: %s\n", elm_diskselector_item_label_get(prev),
          elm_diskselector_item_label_get(next));
}

static void
_print_disk_info_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Diskselector_Item *it = event_info;
   printf("Selected label: %s\n", elm_diskselector_item_label_get(it));
}

static Evas_Object *
_disk_create(Evas_Object *win, Eina_Bool round)
{
   Elm_Diskselector_Item *it;
   Evas_Object *di;

   di = elm_diskselector_add(win);

   elm_diskselector_item_append(di, "January", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "February", NULL, _disk_next, NULL);
   elm_diskselector_item_append(di, "March", NULL, _disk_sel, NULL);
   elm_diskselector_item_append(di, "April", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "May", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "June", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "July", NULL, NULL, NULL);
   it = elm_diskselector_item_append(di, "August", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "September", NULL, _disk_sel, NULL);
   elm_diskselector_item_append(di, "October", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "November", NULL, NULL, NULL);
   elm_diskselector_item_append(di, "December", NULL, NULL, NULL);

   elm_diskselector_item_selected_set(it, EINA_TRUE);
   elm_diskselector_round_set(di, round);

   return di;
}

void
test_diskselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *disk, *ic;
   Elm_Diskselector_Item *it;
   char buf[PATH_MAX];
   int idx = 0;

   char *month_list[] = {
      "Jan", "Feb", "Mar",
      "Apr", "May", "Jun",
      "Jul", "Aug", "Sep",
      "Oct", "Nov", "Dec"
   };
   char date[3];

   win = elm_win_add(NULL, "diskselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Disk Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   disk = _disk_create(win, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   it = elm_diskselector_selected_item_get(disk);
   elm_diskselector_item_selected_set(it, EINA_FALSE);

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   it = elm_diskselector_first_item_get(disk);
   it = elm_diskselector_item_next_get(it);
   elm_diskselector_item_selected_set(it, EINA_TRUE);

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   elm_diskselector_side_label_length_set(disk, 4);

   disk = elm_diskselector_add(win);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 1, 1);
   elm_diskselector_item_append(disk, "Sunday", ic, NULL, NULL);
   elm_diskselector_item_append(disk, "Monday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Tuesday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Wednesday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Thursday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Friday", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Saturday", NULL, NULL, NULL);
   elm_diskselector_round_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   disk = elm_diskselector_add(win);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 1, 1);
   elm_diskselector_item_append(disk, "머리스타일", ic, NULL, NULL);
   elm_diskselector_item_append(disk, "プロが伝授する", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "生上访要求政府", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "English", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "والشريعة", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "עִבְרִית", NULL, NULL, NULL);
   elm_diskselector_item_append(disk, "Grüßen", NULL, NULL, NULL);
   elm_diskselector_round_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   // displayed item number setting example
   disk = elm_diskselector_add(win);
   elm_diskselector_display_item_num_set(disk, 5);

   for (idx = 0; idx < (int)(sizeof(month_list) / sizeof(month_list[0])); idx++)
     {
        it = elm_diskselector_item_append(disk, month_list[idx], NULL, NULL, NULL);
     }

   elm_diskselector_item_selected_set(it, EINA_TRUE);
   elm_diskselector_round_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   // displayed item number setting example
   disk = elm_diskselector_add(win);
   elm_diskselector_display_item_num_set(disk, 7);

   for (idx = 1; idx < 31; idx++)
     {
        snprintf(date, sizeof(date), "%d", idx);
        it = elm_diskselector_item_append(disk, date, NULL, NULL, NULL);
     }

   elm_diskselector_item_selected_set(it, EINA_TRUE);
   elm_diskselector_round_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);


   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
