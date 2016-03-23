#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
aspect_fixed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_aspect_fixed_set(ic, elm_check_state_get(obj));
}

static void
fill_outside_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_fill_outside_set(ic, elm_check_state_get(obj));
}

static void
smooth_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_smooth_set(ic, elm_check_state_get(obj));
}

static void
bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("preload-prescale", "Preload & Prescale");
   elm_win_autodel_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ic);
   snprintf(buf, sizeof(buf), "%s/images/insanely_huge_test_image.jpg",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);

   elm_image_aspect_fixed_set(ic, EINA_FALSE);
   elm_image_preload_disabled_set(ic, EINA_TRUE);
   elm_image_prescale_set(ic, EINA_TRUE);
   evas_object_show(ic);

   evas_object_resize(win, 350, 350);
   evas_object_show(win);
}

void
test_icon(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *ic, *hbox, *tg, *bt;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("icon-test", "Icon Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   ic = elm_icon_add(box);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ic);
   evas_object_show(ic);

   hbox = elm_box_add(box);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   /* Test Aspect Fixed */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Aspect Fixed");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", aspect_fixed_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Fill Outside */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Fill Outside");
   evas_object_smart_callback_add(tg, "changed", fill_outside_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Smooth */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Smooth");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", smooth_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Preload, Prescale */
   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "Preload & Prescale");
   evas_object_smart_callback_add(bt, "clicked", bt_clicked, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/* Test: Icon Transparent */
static void
icon_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked!\n");
}

void
test_icon_transparent(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "icon-transparent", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Transparent");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_alpha_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_no_scale_set(ic, EINA_TRUE);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, 0.5, 0.5);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", icon_clicked, NULL);

   evas_object_show(win);
}

/* Test: Icon Standard */
static void
_standard_list_populate(Evas_Object *list, Elm_Icon_Lookup_Order order, int size)
{
   Evas_Object *ic;
   Eina_List *l;
   const char *group;
   char name[128], *p;

   elm_list_clear(list);
   l = elm_theme_group_base_list(NULL, "elm/icon/");
   EINA_LIST_FREE(l, group)
     {
        // group = "/elm/icon/standard-name/style/maybe_another_style??"
         snprintf(name, sizeof(name), "%s", group + 9);
        if ((p = strrchr(name, '/')))
          *p = '\0';
         // printf("Found group:%s  Name:%s\n", group, name);

         // quick hack to show only standard-compliant icons
         // apart from the "folder" one, all the others have "-" in the name
         if ((strrchr(name, '-') != NULL) || !strcmp(name, "folder"))
           {
               ic = elm_icon_add(list);
               elm_icon_order_lookup_set(ic, order);
               elm_icon_standard_set(ic, name);
               if (size)
                 evas_object_size_hint_min_set(ic, size, size);
               elm_list_item_append(list, name, ic, NULL, NULL, NULL);
           }

         eina_stringshare_del(group);
     }
   elm_list_go(list);
}

static void
_rdg_changed_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *li = data;
   Evas_Object *icon = evas_object_data_get(li, "resize_icon");
   Evas_Object *order_rdg = evas_object_data_get(li, "order_rdg");
   Evas_Object *size_rdg = evas_object_data_get(li, "size_rdg");

   _standard_list_populate(li, elm_radio_value_get(order_rdg),
                           elm_radio_value_get(size_rdg));

   elm_icon_order_lookup_set(icon, elm_radio_value_get(order_rdg));
}

static void
_slider_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *icon = data;
   int size = (int)elm_slider_value_get(obj);

   evas_object_size_hint_min_set(icon, size, size);
}

static void
_list_selected_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it = event_info;
   Evas_Object *icon = evas_object_data_get(obj, "resize_icon");

   elm_icon_standard_set(icon, elm_object_item_text_get(list_it));
}

static void
_std_btn_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win, *panes, *icon;

   win = elm_win_util_standard_add("icon-test-std-auto", "Icon Standard");
   elm_win_autodel_set(win, EINA_TRUE);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_panes_content_left_min_size_set(panes, 16);
   elm_panes_content_right_min_size_set(panes, 16);
   elm_win_resize_object_add(win, panes);
   evas_object_show(panes);

   icon = elm_icon_add(panes);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_standard_set(icon, "folder");
   elm_object_part_content_set(panes, "left", icon);
   evas_object_show(icon);

   icon = elm_icon_add(panes);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_standard_set(icon, "user-home");
   elm_object_part_content_set(panes, "right", icon);
   evas_object_show(icon);

   evas_object_resize(win, 300, 200);
   evas_object_show(win);
}

void
test_icon_standard(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *box, *hbox, *fr, *rd, *rdg, *icon, *sl, *bt;

   win = elm_win_util_standard_add("icon-test-std", "Icon Standard");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   li = elm_list_add(box);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(li, "selected", _list_selected_cb, NULL);
   _standard_list_populate(li, ELM_ICON_LOOKUP_FDO_THEME, 0);
   evas_object_show(li);

   // lookup order
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "standard icon order lookup");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);

   hbox = elm_box_add(fr);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   rdg = elm_radio_add(hbox);
   elm_radio_state_value_set(rdg, ELM_ICON_LOOKUP_FDO_THEME);
   elm_object_text_set(rdg, "fdo, theme");
   elm_box_pack_end(hbox, rdg);
   evas_object_show(rdg);
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, li);
   evas_object_data_set(li, "order_rdg", rdg);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, ELM_ICON_LOOKUP_THEME_FDO);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "theme, fdo");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, ELM_ICON_LOOKUP_FDO);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "fdo only");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, ELM_ICON_LOOKUP_THEME);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "theme only");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   // size
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "standard icon size");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);
   
   hbox = elm_box_add(fr);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   rdg = elm_radio_add(hbox);
   elm_radio_state_value_set(rdg, 0);
   elm_object_text_set(rdg, "Free");
   elm_box_pack_end(hbox, rdg);
   evas_object_show(rdg);
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, li);
   evas_object_data_set(li, "size_rdg", rdg);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 16);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "16");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 22);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "22");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 24);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "24");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 32);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "32");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 48);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "48");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 64);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "64");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 128);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "128");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   // pack the list
   elm_box_pack_end(box, li);

   // live resize
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "live resize");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);
   
   hbox = elm_box_add(fr);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   icon = elm_icon_add(hbox);
   elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_standard_set(icon, "folder");
   evas_object_size_hint_min_set(icon, 16, 16);
   elm_box_pack_end(hbox, icon);
   evas_object_show(icon);
   evas_object_data_set(li, "resize_icon", icon);

   sl = elm_slider_add(hbox);
   elm_object_text_set(sl, "min_size");
   elm_slider_min_max_set(sl, 16, 256);
   elm_slider_value_set(sl, 16);
   elm_slider_unit_format_set(sl, "%.0f px");
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(sl, "changed", _slider_changed_cb, icon);
   elm_box_pack_end(hbox, sl);
   evas_object_show(sl);

   bt = elm_button_add(box);
   elm_object_text_set(bt, "Another size test, without using min_size");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _std_btn_clicked_cb, NULL);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   // show the win
   evas_object_resize(win, 300, 400);
   evas_object_show(win);
}
