#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static int quiet = 0;
static int interactive = 1;

static const char *theme_set = NULL;
static const char *finger_size_set = NULL;
static const char *scale_set = NULL;

static void
my_win_del(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
sc_round(void *data, Evas_Object *obj, void *event_info)
{
   double val = elm_slider_value_get(obj);
   double v;
   
   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
sc_change(void *data, Evas_Object *obj, void *event_info)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);
   
   if (scale == val) return;
   elm_scale_all_set(val);
}

static void
fs_round(void *data, Evas_Object *obj, void *event_info)
{
   double val = elm_slider_value_get(obj);
   double v;
   
   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
fs_change(void *data, Evas_Object *obj, void *event_info)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);
   
   if (scale == val) return;
   elm_finger_size_all_set(val);
}

static void
_status_basic(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *lb, *fr;
   
   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   elm_frame_label_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);
   
   lb = elm_label_add(win);
   elm_label_label_set(lb,
                       "Applying configuration change"
                       );
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);
}

static void
_status_config(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *lb, *pd, *bx2, *fr, *sl, *sp;
   
   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_label_set(fr, "Sizing");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);
   
   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);
   
   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);
   
   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb,"<hilight>Scale</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);
   
   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_scale_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);
   
   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);
   
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx2, sp);
   evas_object_show(sp);
   
   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);
   
   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb, "<hilight>Finger Size</><br>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);
   
   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_finger_size_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);
   
   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);
   
   // FIXME: add theme selector (basic mode and advanced for fallbacks)
   // FIXME: save config
   // FIXME: profile selector / creator etc.
   // 
   elm_frame_content_set(fr, bx2);
   evas_object_show(bx2);
}

static void
_flip_to(Evas_Object *win, const char *name)
{
   Evas_Object *wid, *fl, *holder, *front, *back;
   int front_vis;
   wid        = evas_object_data_get(win, name);
   fl         = evas_object_data_get(win, "flip");
   holder     = evas_object_data_get(win, "holder");
   front_vis  = elm_flip_front_get(fl);
   if (front_vis)
     {
        front = elm_flip_content_front_get(fl);
        back  = elm_flip_content_back_get(fl);
        elm_table_pack(holder, back, 0, 0, 1, 1);
        evas_object_hide(back);
//        elm_table_unpack(holder, wid); // this should NOT be needed - but in evas_table/elm_table
        elm_flip_content_back_set(fl, wid);
     }
   else
     {
        front = elm_flip_content_front_get(fl);
        back  = elm_flip_content_back_get(fl);
        elm_table_pack(holder, front, 0, 0, 1, 1);
        evas_object_hide(front);
//        elm_table_unpack(holder, wid); // this should NOT be needed - but in evas_table/elm_table
        elm_flip_content_front_set(fl, wid);
     }
   
   evas_object_show(wid);
   elm_flip_go(fl, ELM_FLIP_ROTATE_Y_CENTER_AXIS);
}

static void
_cf_sizing(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "sizing");
}

static void
_cf_themes(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "themes");
}

static void
_cf_fonts(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "fonts");
}

static void
_cf_profiles(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "profiles");
}

static void
_cf_scrolling(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "scrolling");
}

static void
_cf_rendering(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "rendering");
}

static void
_cf_caches(void *data, Evas_Object *obj, void *event_info)
{
   _flip_to(data, "caches");
}

static void
_theme_use(void *data, Evas_Object *obj, void *event_info)
{
   printf("not implemented\n");
}

static void
_theme_browse(void *data, Evas_Object *obj, void *event_info)
{
   printf("not implemented\n");
}

static void
_status_config_sizing(Evas_Object *win, Evas_Object *holder)
{
   Evas_Object *lb, *pd, *bx2, *sl, *sp;
   
   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);
   
   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);
   
   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb,"<hilight>Scale</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);
   
   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_scale_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);
   
   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);
   
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx2, sp);
   evas_object_show(sp);
   
   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);
   
   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb, "<hilight>Finger Size</><br>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);
   
   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_finger_size_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);
   
   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);

   evas_object_data_set(win, "sizing", bx2);
   
   elm_table_pack(holder, bx2, 0, 0, 1, 1);
}

static Evas_Object *
_sample_theme_new(Evas_Object *win)
{
   Evas_Object *base, *bg, *bt, *ck, *rd, *rdg, *sl, *fr, *li, *rc, *sp;
   
   base = elm_table_add(win);
   evas_object_size_hint_weight_set(base, 1.0, 1.0);
   evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);
   
   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, bg, 0, 0, 2, 4);
   evas_object_show(bg);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   elm_table_pack(base, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_check_label_set(ck, "Check");
   elm_table_pack(base, ck, 0, 1, 1, 1);
   evas_object_show(ck);
   
   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 0);
   elm_radio_label_set(rd, "Radio 1");
   elm_table_pack(base, rd, 1, 0, 1, 1);
   evas_object_show(rd);
   rdg = rd;
   
   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 1);
   elm_radio_label_set(rd, "Radio 2");
   elm_radio_group_add(rd, rdg);
   elm_table_pack(base, rd, 1, 1, 1, 1);
   evas_object_show(rd);
   
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(base, sl, 0, 2, 2, 1);
   evas_object_show(sp);
   
   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Slider");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 1, 10);
   elm_slider_value_set(sl, 4);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, sl, 0, 3, 2, 1);
   evas_object_show(sl);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Frame");
   elm_table_pack(base, fr, 0, 4, 2, 1);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);
   
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(fr, li);
   evas_object_show(li);
   
   elm_list_item_append(li, "List Item 1", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Second Item", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Third Item", NULL, NULL,  NULL, NULL);
   elm_list_go(li);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 120, 120);
   elm_table_pack(base, rc, 0, 4, 2, 1);
   
   return base;
}

static void
_status_config_themes(Evas_Object *win, Evas_Object *holder)
{
   Evas_Object *tb, *rc, *sc, *sp, *li, *pd, *fr, *bt, *sample;
   
   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   
   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 240, 120);
   elm_table_pack(tb, rc, 0, 0, 1, 1);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 240, 240);
   elm_table_pack(tb, rc, 0, 1, 1, 1);
   
   /////////////////////////////////////////////
   
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 1.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);
   
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(pd, li);
   evas_object_show(li);
   
   // FIXME: list all themes:
   // ~/.elementary/themes/*.edj
   // $PREFIX/datadir/themes/*.edj
   
   elm_list_item_append(li, "theme 1", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 2", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 3", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 4", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 5", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 6", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "theme 7", NULL, NULL,  NULL, NULL);

   elm_list_go(li);
   
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 1.0);
   evas_object_size_hint_align_set(pd, 0.9, 0.9);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);
   
   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _theme_browse, win);
   elm_button_label_set(bt, "Browse...");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_frame_content_set(pd, bt);
   evas_object_show(bt);
   
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 1, 1, 1);
   evas_object_show(pd);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Preview");
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(pd, fr);
   evas_object_show(fr);
   
   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, 0, 0);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(fr, sc);
   evas_object_show(sc);
   
   sample = _sample_theme_new(win);
   elm_scroller_content_set(sc, sample);
   evas_object_show(sample);
   evas_object_data_set(win, "sample", sample);
   
   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(tb, sp, 0, 2, 1, 1);
   evas_object_show(sp);
   
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_table_pack(tb, pd, 0, 3, 1, 1);
   evas_object_show(pd);
   
   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _theme_use, win);
   elm_button_label_set(bt, "Use Theme");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_frame_content_set(pd, bt);
   evas_object_show(bt);
   
   evas_object_data_set(win, "themes", tb);
   
   elm_table_pack(holder, tb, 0, 0, 1, 1);
}

static void
_unimplemented(Evas_Object *win, Evas_Object *holder, const char *name)
{
   Evas_Object *lb, *pd, *bx2;
   
   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);
   
   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);
   
   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, 0.5, 0.5);
   elm_label_label_set(lb,"<hilight>Not implemented yet</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);
   
   evas_object_data_set(win, name, bx2);
   
   elm_table_pack(holder, bx2, 0, 0, 1, 1);
}

static void
_status_config_fonts(Evas_Object *win, Evas_Object *holder)
{
   _unimplemented(win, holder, "fonts");
}

static void
_status_config_profiles(Evas_Object *win, Evas_Object *holder)
{
   _unimplemented(win, holder, "profiles");
}

static void
_status_config_scrolling(Evas_Object *win, Evas_Object *holder)
{
   _unimplemented(win, holder, "scrolling");
}

static void
_status_config_rendering(Evas_Object *win, Evas_Object *holder)
{
   _unimplemented(win, holder, "rendering");
}

static void
_status_config_caches(Evas_Object *win, Evas_Object *holder)
{
   _unimplemented(win, holder, "caches");
}

static void
_status_config_full(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *tb, *holder, *fl;
   Elm_Toolbar_Item *it;
   
   tb = elm_toolbar_add(win);
   elm_toolbar_homogenous_set(tb, 0);
   evas_object_size_hint_weight_set(tb, 1.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   it = elm_toolbar_item_add(tb, NULL, "Sizing", _cf_sizing, win);
   elm_toolbar_item_add(tb, NULL, "Theme", _cf_themes, win);
   
   elm_toolbar_item_add(tb, NULL, "Fonts", _cf_fonts, win);
   elm_toolbar_item_add(tb, NULL, "Profiles", _cf_profiles, win);
   elm_toolbar_item_add(tb, NULL, "Scrolling", _cf_scrolling, win);
   elm_toolbar_item_add(tb, NULL, "Rendering", _cf_rendering, win);
   elm_toolbar_item_add(tb, NULL, "Caches", _cf_caches, win);
   elm_box_pack_end(bx0, tb);
   evas_object_show(tb);
   
   holder = elm_table_add(win);
   evas_object_size_hint_align_set(holder, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(holder, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx0, holder);
   evas_object_show(holder);
   evas_object_data_set(win, "holder", holder);
   
   _status_config_sizing(win, holder);
   _status_config_themes(win, holder);
   _status_config_fonts(win, holder);
   _status_config_profiles(win, holder);
   _status_config_rendering(win, holder);
   _status_config_scrolling(win, holder);
   _status_config_caches(win, holder);
   
   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_table_pack(holder, fl, 0, 0, 1, 1);
   evas_object_show(fl);
   evas_object_data_set(win, "flip", fl);
   
   elm_toolbar_item_select(it);
}

static void
status_win(void)
{
   Evas_Object *win, *bg, *bx0;

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(win, "Elementary Config");

   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);
   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, 1.0, 1.0);
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   if (!interactive) _status_basic(win, bx0);
//   else _status_config(win, bx0);
   else _status_config_full(win, bx0);
   
   evas_object_show(win);
}

static int
_exit_timer(void *data)
{
   elm_exit();
}

/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI int
elm_main(int argc, char **argv)
{
   int i;
   
   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             printf("Usage:\n"
                    "  -h                This help\n"
                    "  -q                Quiet mode (dont show window)\n"
                    "  -t THEME          Set theme to THEME (ELM_THEME spec)\n"
                    "  -f SIZE           Set finger size to SIZE pixels\n"
                    "  -s SCALE          Set scale factor to SCALE\n"
                    );
          }
        else if (!strcmp(argv[i], "-q"))
          {
             quiet = 1;
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-t")) && (i < argc - 1))
          {
             i++;
             theme_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-f")) && (i < argc - 1))
          {
             i++;
             finger_size_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-s")) && (i < argc - 1))
          {
             i++;
             scale_set = argv[i];
             interactive = 0;
          }
     }
   /* put ere any init specific to this app like parsing args etc. */
   if (!quiet)
     {
        status_win(); /* create main window */
        if (!interactive)
          ecore_timer_add(2.0, _exit_timer, NULL);
     }
   if (!interactive)
     {
        if (theme_set)
          {
             elm_theme_all_set(theme_set);
          }
        if (finger_size_set)
          {
             elm_finger_size_all_set(atoi(finger_size_set));
          }
        if (scale_set)
          {
             elm_scale_all_set(atof(scale_set));
          }
        if (quiet)
          {
             elm_exit();
          }
     }
   elm_run(); /* and run the program now  and handle all events etc. */
   /* if the mainloop that elm_run() runs exist - we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* all emeentary apps should use this. but it right after elm_main() */
ELM_MAIN()
