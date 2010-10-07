#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_disk_sel(void *data, Evas_Object * obj, void *event_info)
{
   Elm_Diskpicker_Item *it = event_info;
   printf("Equinox: %s\n", elm_diskpicker_item_label_get(it));
}

static void
_disk_next(void *data, Evas_Object * obj, void *event_info)
{
   Elm_Diskpicker_Item *next, *prev, *it = event_info;
   prev = elm_diskpicker_item_prev(it);
   next = elm_diskpicker_item_next(it);
   printf("Prev: %s, Next: %s\n", elm_diskpicker_item_label_get(prev),
          elm_diskpicker_item_label_get(next));
}

static void
_print_disk_info_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Diskpicker_Item *it = event_info;
   printf("Selected label: %s\n", elm_diskpicker_item_label_get(it));
}

static Evas_Object *
_disk_create(Evas_Object *win, Eina_Bool round)
{
   Elm_Diskpicker_Item *it;
   Evas_Object *di;

   di = elm_diskpicker_add(win);

   elm_diskpicker_item_append(di, "January", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "February", NULL, _disk_next, NULL);
   elm_diskpicker_item_append(di, "March", NULL, _disk_sel, NULL);
   elm_diskpicker_item_append(di, "April", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "May", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "June", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "July", NULL, NULL, NULL);
   it = elm_diskpicker_item_append(di, "August", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "September", NULL, _disk_sel, NULL);
   elm_diskpicker_item_append(di, "October", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "November", NULL, NULL, NULL);
   elm_diskpicker_item_append(di, "December", NULL, NULL, NULL);

   elm_diskpicker_item_selected_set(it);
   elm_diskpicker_round_set(di, round);

   return di;
}

void
test_diskpicker(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *disk, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "diskpicker", ELM_WIN_BASIC);
   elm_win_title_set(win, "Diskpicker");
   elm_win_autodel_set(win, 1);

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

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   disk = _disk_create(win, EINA_FALSE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);
   elm_diskpicker_side_label_lenght_set(disk, 4);

   disk = elm_diskpicker_add(win);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 1, 1);
   elm_diskpicker_item_append(disk, "Sunday", ic, NULL, NULL);
   elm_diskpicker_item_append(disk, "Monday", NULL, NULL, NULL);
   elm_diskpicker_item_append(disk, "Tuesday", NULL, NULL, NULL);
   elm_diskpicker_item_append(disk, "Wednesday", NULL, NULL, NULL);
   elm_diskpicker_item_append(disk, "Thursday", NULL, NULL, NULL);
   elm_diskpicker_item_append(disk, "Friday", NULL, NULL, NULL);
   elm_diskpicker_item_append(disk, "Saturday", NULL, NULL, NULL);
   elm_diskpicker_round_set(disk, EINA_TRUE);
   evas_object_size_hint_weight_set(disk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(disk, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, disk);
   evas_object_show(disk);
   evas_object_smart_callback_add(disk, "selected", _print_disk_info_cb, NULL);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
