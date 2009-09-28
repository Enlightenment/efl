#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_box_vert(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-vert", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Vert");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

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
   evas_object_size_hint_align_set(ic, 0.0, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, EVAS_HINT_EXPAND, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

void
test_box_horiz(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-horiz", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Horiz");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

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

   evas_object_show(win);
}
#endif
