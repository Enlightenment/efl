#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
my_bt_go_300_300(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 300, 318, 318);
}

void
my_bt_go_900_300(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 300, 318, 318);
}

void
my_bt_go_300_900(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 900, 318, 318);
}

void
my_bt_go_900_900(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 900, 318, 318);
}

void
my_bt_freeze_toggle(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   if (elm_check_state_get(obj))
     elm_object_scroll_freeze_push((Evas_Object *)data);
   else
     elm_object_scroll_freeze_pop((Evas_Object *)data);
}

void
my_bt_hold_toggle(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   if (elm_check_state_get(obj))
     elm_object_scroll_hold_push((Evas_Object *)data);
   else
     elm_object_scroll_hold_pop((Evas_Object *)data);
}

void
_sc_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord x = 0, y = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(data, x, y);
}

void
_sc_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord w = 0, h = 0;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(data, w, h);
}

void
test_scroller(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg2, *tb, *tb2, *sc, *bt, *ck1, *ck2, *bx, *bx2, *fr;
   int i, j, n;
   char buf[PATH_MAX];
   Evas_Coord x = 0, y = 0, w = 0, h = 0;
   const char *img[9] =
     {
        "panel_01.jpg",
        "plant_01.jpg",
        "rock_01.jpg",
        "rock_02.jpg",
        "sky_01.jpg",
        "sky_02.jpg",
        "sky_03.jpg",
        "sky_04.jpg",
        "wood_01.jpg"
     };

   win = elm_win_util_standard_add("scroller", "Scroller");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(fr, "Options");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(win);
   elm_object_content_set(fr, bx2);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_show(bx2);

   ck1 = elm_check_add(win);
   elm_object_text_set(ck1, "Freeze");
   elm_box_pack_end(bx2, ck1);
   evas_object_show(ck1);

   ck2 = elm_check_add(win);
   elm_object_text_set(ck2, "Hold");
   elm_box_pack_end(bx2, ck2);
   evas_object_show(ck2);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, sc);

   tb = elm_table_add(win);

   n = 0;
   for (j = 0; j < 12; j++)
     {
        for (i = 0; i < 12; i++)
          {
             bg2 = elm_bg_add(win);
             snprintf(buf, sizeof(buf), "%s/images/%s",
                      elm_app_data_dir_get(), img[n]);
             n++;
             if (n >= 9) n = 0;
             elm_bg_file_set(bg2, buf, NULL);
             evas_object_size_hint_weight_set(bg2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(bg2, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_size_hint_min_set(bg2, 318, 318);
             elm_table_pack(tb, bg2, i, j, 1, 1);
             evas_object_show(bg2);
          }
     }

   elm_object_content_set(sc, tb);
   evas_object_show(tb);

   elm_scroller_page_relative_set(sc, 1.0, 1.0);
   evas_object_show(sc);

   evas_object_smart_callback_add(ck1, "changed", my_bt_freeze_toggle, tb);
   evas_object_smart_callback_add(ck2, "changed", my_bt_hold_toggle, tb);

   tb2 = elm_table_add(win);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 300 300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_300_300, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 900 300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_900_300, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 300 900");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_300_900, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.9);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 900 900");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_900_900, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_table_pack(tb2, bt, 2, 2, 1, 1);
   evas_object_show(bt);

   evas_object_event_callback_add(sc, EVAS_CALLBACK_MOVE, _sc_move_cb, tb2);
   evas_object_event_callback_add(sc, EVAS_CALLBACK_RESIZE, _sc_resize_cb, tb2);
   evas_object_geometry_get(sc, &x, &y, &w, &h);
   evas_object_move(tb2, x, y);
   evas_object_resize(tb2, w, h);
   evas_object_show(tb2);

   evas_object_resize(win, 320, 420);
   evas_object_show(win);
}

void
click_through(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("click went through on %p\n", obj);
}

void
test_scroller2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bt, *bx, *bx2, *sc, *tb, *tb2, *rc;
   int i, j;

   win = elm_win_util_standard_add("scroller2", "Scroller 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);

   /* { */
   for (i = 0; i < 3; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }
   /* } */

   /* { */
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, 0.5);
   elm_scroller_bounce_set(sc, EINA_TRUE, EINA_FALSE);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_box_pack_end(bx, sc);
   evas_object_show(sc);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);

   for (i = 0; i < 10; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "... Horizontal scrolling ...");
        evas_object_smart_callback_add(bt, "clicked", click_through, NULL);
        elm_box_pack_end(bx2, bt);
        evas_object_show(bt);
     }

   elm_object_content_set(sc, bx2);
   evas_object_show(bx2);
   /* } */

   /* { */
   for (i = 0; i < 3; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }
   /* } */

   /* { */
   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 200, 120);
   elm_table_pack(tb, rc, 0, 0, 1, 1);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 0, 0, 1, 1);
   evas_object_show(sc);

   tb2 = elm_table_add(win);

   for (j = 0; j < 16; j++)
     {
        for (i = 0; i < 16; i++)
          {
             bt = elm_button_add(win);
             elm_object_text_set(bt, "Both");
             evas_object_smart_callback_add(bt, "clicked", click_through, NULL);
             elm_table_pack(tb2, bt, i, j, 1, 1);
             evas_object_show(bt);
          }
     }

   elm_object_content_set(sc, tb2);
   evas_object_show(tb2);
   /* } */

   for (i = 0; i < 24; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   elm_win_resize_object_add(win, sc);
   elm_object_content_set(sc, bx);
   evas_object_show(bx);
   evas_object_show(sc);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
