#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_hoversel(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "hoversel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hoversel");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   bt = elm_hoversel_add(win);
// FIXME: need to add horizontal hoversel theme to default some day   
//   elm_hoversel_horizontal_set(bt, 1);
   elm_hoversel_hover_parent_set(bt, win);
   elm_object_text_set(bt, "Labels");
   elm_hoversel_item_add(bt, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4 - Long Label Here", NULL, ELM_ICON_NONE, NULL, NULL);
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
   elm_icon_file_set(ic, buf, NULL);
   elm_hoversel_icon_set(bt, ic);
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
