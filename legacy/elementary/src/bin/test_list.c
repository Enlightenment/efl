#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void
my_show_it(void *data, Evas_Object *obj, void *event_info)
{
   elm_list_item_show(data);
}

void
test_list(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx, *tb2, *bt;
   char buf[PATH_MAX];
   Elm_List_Item *it1, *it2, *it3, *it4, *it5;

   win = elm_win_add(NULL, "list", ELM_WIN_BASIC);
   elm_win_title_set(win, "List");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   li = elm_list_add(win);
   elm_win_resize_object_add(win, li);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 1, 1);
   it1 = elm_list_item_append(li, "Hello", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   it2 = elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx, NULL,  NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   it3 = elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   it4 = elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL,  NULL, NULL);
   it5 = elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);
   
   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Hello");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, it1);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "How");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, it2);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);   
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "doing");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, it3);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Here");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, it4);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 3, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Maybe this...");
   evas_object_smart_callback_add(bt, "clicked", my_show_it, it5);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 0, 4, 1, 1);
   evas_object_show(bt);

   evas_object_show(tb2);
   
   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/***********/

static void
my_li2_clear(void *data, Evas_Object *obj, void *event_info)
{
   elm_list_clear(data);
}

static void
my_li2_sel(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it = elm_list_selected_item_get(obj);
   elm_list_item_selected_set(it, 0);
//   elm_list_item_selected_set(event_info, 0);
}

void
test_list2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx, *bx2, *bt;
   char buf[PATH_MAX];
   Elm_List_Item *it;

   win = elm_win_add(NULL, "list-2", ELM_WIN_BASIC);
   elm_win_title_set(win, "List 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   li = elm_list_add(win);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_horizontal_mode_set(li, ELM_LIST_LIMIT);
//   elm_list_multi_select_set(li, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   it = elm_list_item_append(li, "Hello", ic, NULL,  my_li2_sel, NULL);
   elm_list_item_selected_set(it, 1);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx2, NULL, NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Longer label.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   elm_box_pack_end(bx, li);
   evas_object_show(li);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_li2_clear, li);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/***********/

void
test_list3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "list-3", ELM_WIN_BASIC);
   elm_win_title_set(win, "List 3");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   li = elm_list_add(win);
   elm_win_resize_object_add(win, li);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_horizontal_mode_set(li, ELM_LIST_COMPRESS);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Hello", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   elm_list_item_append(li, "are", bx, NULL,  NULL, NULL);
   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}
#endif
