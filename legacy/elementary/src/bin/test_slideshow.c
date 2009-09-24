/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *slideshow, *bt_start, *bt_stop;

static void
_notify_show(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_show(data);
   elm_notify_timer_init(data);
}

static void
_next(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_next(data);
}

static void
_previous(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_previous(data);
}

static void
_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_notify_timeout_set(data, 0);
}


static void
_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_notify_timeout_set(data, 3);
}

static void
_hv_select(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_transition_set(slideshow, data);
   elm_hoversel_label_set(obj, data);
}

static void
_start(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_slideshow_timeout_set(slideshow, (int)elm_spinner_value_get(data));

   elm_widget_disabled_set(bt_start, 1);
   elm_widget_disabled_set(bt_stop, 0);
}

static void
_stop(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_slideshow_timeout_set(slideshow, 0);
   elm_widget_disabled_set(bt_start, 0);
   elm_widget_disabled_set(bt_stop, 1);
}

static void
_spin(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   if(elm_slideshow_timeout_get(slideshow) > 0)
     elm_slideshow_timeout_set(slideshow, (int)elm_spinner_value_get(data));
}

   void
test_slideshow(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *notify, *bx, *bt, *hv, *spin;
   char buf[PATH_MAX];
   const Eina_List *l;
   const char *transition;

   win = elm_win_add(NULL, "Slideshow", ELM_WIN_BASIC);
   elm_win_title_set(win, "Slideshow");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   slideshow = elm_slideshow_add(win);
   elm_slideshow_loop_set(slideshow, 1);
   elm_win_resize_object_add(win, slideshow);
   evas_object_size_hint_weight_set(slideshow, 1.0, 1.0);
   evas_object_show(slideshow);

   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/logo.png", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/plant_01.jpg", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/rock_01.jpg", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/rock_02.jpg", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/sky_01.jpg", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/sky_04.jpg", NULL);
   elm_slideshow_image_add(slideshow, PACKAGE_DATA_DIR"/images/wood_01.jpg", NULL);

   notify = elm_notify_add(win);
   elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM);
   elm_win_resize_object_add(win, notify);
   elm_notify_timeout_set(notify, 3);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);
   elm_notify_content_set(notify, bx);
   evas_object_show(bt);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_IN, _mouse_in, notify);
   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_OUT, _mouse_out, notify);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Previous");
   evas_object_smart_callback_add(bt, "clicked", _previous, slideshow);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Next");
   evas_object_smart_callback_add(bt, "clicked", _next, slideshow);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   hv = elm_hoversel_add(win);
   elm_box_pack_end(bx, hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(elm_slideshow_transitions_get(slideshow), l, transition)
      elm_hoversel_item_add(hv, transition, NULL, 0, _hv_select, transition);
   elm_hoversel_label_set(hv, eina_list_data_get(elm_slideshow_transitions_get(slideshow)));
   evas_object_show(hv);

   spin = elm_spinner_add(win);
   elm_spinner_label_format_set(spin, "%2.0f secs.");
   evas_object_smart_callback_add(spin, "changed", _spin, spin);
   elm_spinner_step_set(spin, 1);
   elm_spinner_min_max_set(spin, 1, 30);
   elm_spinner_value_set(spin, 3);
   elm_box_pack_end(bx, spin);
   evas_object_show(spin);

   bt = elm_button_add(win);
   bt_start = bt;
   elm_button_label_set(bt, "Start");
   evas_object_smart_callback_add(bt, "clicked", _start, spin);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   bt_stop = bt;
   elm_button_label_set(bt, "Stop");
   evas_object_smart_callback_add(bt, "clicked", _stop, spin);
   elm_box_pack_end(bx, bt);
   elm_widget_disabled_set(bt, 1);
   evas_object_show(bt);


   evas_object_smart_callback_add(slideshow, "clicked", _notify_show, notify);
   evas_object_smart_callback_add(slideshow, "move", _notify_show, notify);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

#endif
