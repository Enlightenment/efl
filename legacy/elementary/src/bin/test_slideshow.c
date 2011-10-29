#include <Elementary.h>
#include "elementary_config.h"
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *slideshow, *bt_start, *bt_stop;
static Elm_Slideshow_Item_Class itc;
static const char *img1 = PACKAGE_DATA_DIR"/images/logo.png";
static const char *img2 = PACKAGE_DATA_DIR"/images/plant_01.jpg";
static const char *img3 = PACKAGE_DATA_DIR"/images/rock_01.jpg";
static const char *img4 = PACKAGE_DATA_DIR"/images/rock_02.jpg";
static const char *img5 = PACKAGE_DATA_DIR"/images/sky_01.jpg";
static const char *img6 = PACKAGE_DATA_DIR"/images/sky_04.jpg";
static const char *img7 = PACKAGE_DATA_DIR"/images/wood_01.jpg";
static const char *img8 = PACKAGE_DATA_DIR"/images/mystrale.jpg";
static const char *img9 = PACKAGE_DATA_DIR"/images/mystrale_2.jpg";

static void
_notify_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_show(data);
}

static void
_next(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_slideshow_next(data);
}

static void
_previous(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_slideshow_previous(data);
}

static void
_mouse_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_notify_timeout_set(data, 0.0);
   evas_object_show(data);
}


static void
_mouse_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_notify_timeout_set(data, 3.0);
}

static void
_hv_select(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_slideshow_transition_set(slideshow, data);
   elm_object_text_set(obj, data);
}

static void
_layout_select(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_slideshow_layout_set(slideshow, data);
   elm_object_text_set(obj, data);
}

static void
_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));

   elm_object_disabled_set(bt_start, EINA_TRUE);
   elm_object_disabled_set(bt_stop, EINA_FALSE);
}

static void
_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_slideshow_timeout_set(slideshow, 0.0);
   elm_object_disabled_set(bt_start, EINA_FALSE);
   elm_object_disabled_set(bt_stop, EINA_TRUE);
}

static void
_spin(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (elm_slideshow_timeout_get(slideshow) > 0)
     elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));
}

static Evas_Object *
_get(void *data, Evas_Object *obj)
{
   //Evas_Object *photo = elm_photocam_add(obj);
   //elm_photocam_file_set(photo, data);
   //elm_photocam_zoom_mode_set(photo, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);

   Evas_Object *photo = elm_photo_add(obj);
   elm_photo_file_set(photo, data);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");

   return photo;
}



void
test_slideshow(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *notify, *bx, *bt, *hv, *spin;
   const Eina_List *l;
   const char *transition, *layout;

   win = elm_win_add(NULL, "slideshow", ELM_WIN_BASIC);
   elm_win_title_set(win, "Slideshow");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   slideshow = elm_slideshow_add(win);
   elm_slideshow_loop_set(slideshow, EINA_TRUE);
   elm_win_resize_object_add(win, slideshow);
   evas_object_size_hint_weight_set(slideshow, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(slideshow);

   itc.func.get = _get;
   itc.func.del = NULL;

   elm_slideshow_item_add(slideshow, &itc, img1);
   elm_slideshow_item_add(slideshow, &itc, img2);
   elm_slideshow_item_add(slideshow, &itc, img3);
   elm_slideshow_item_add(slideshow, &itc, img4);
   elm_slideshow_item_add(slideshow, &itc, img9);
   elm_slideshow_item_add(slideshow, &itc, img5);
   elm_slideshow_item_add(slideshow, &itc, img6);
   elm_slideshow_item_add(slideshow, &itc, img7);
   elm_slideshow_item_add(slideshow, &itc, img8);

   notify = elm_notify_add(win);
   elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM);
   elm_win_resize_object_add(win, notify);
   elm_notify_timeout_set(notify, 3.0);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_object_content_set(notify, bx);
   evas_object_show(bx);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_IN, _mouse_in,
                                  notify);
   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_OUT, _mouse_out,
                                  notify);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Previous");
   evas_object_smart_callback_add(bt, "clicked", _previous, slideshow);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next");
   evas_object_smart_callback_add(bt, "clicked", _next, slideshow);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   hv = elm_hoversel_add(win);
   elm_box_pack_end(bx, hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(elm_slideshow_transitions_get(slideshow), l, transition)
      elm_hoversel_item_add(hv, transition, NULL, 0, _hv_select, transition);
   elm_object_text_set(hv, eina_list_data_get(elm_slideshow_transitions_get(slideshow)));
   evas_object_show(hv);

   hv = elm_hoversel_add(win);
   elm_box_pack_end(bx, hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(elm_slideshow_layouts_get(slideshow), l, layout)
       elm_hoversel_item_add(hv, layout,  NULL, 0, _layout_select, layout);
   elm_object_text_set(hv, elm_slideshow_layout_get(slideshow));
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
   elm_object_text_set(bt, "Start");
   evas_object_smart_callback_add(bt, "clicked", _start, spin);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   bt_stop = bt;
   elm_object_text_set(bt, "Stop");
   evas_object_smart_callback_add(bt, "clicked", _stop, spin);
   elm_box_pack_end(bx, bt);
   elm_object_disabled_set(bt, EINA_TRUE);
   evas_object_show(bt);

   evas_object_event_callback_add(slideshow, EVAS_CALLBACK_MOUSE_UP,
                                  _notify_show, notify);
   evas_object_event_callback_add(slideshow, EVAS_CALLBACK_MOUSE_MOVE,
                                  _notify_show, notify);

   evas_object_resize(win, 500, 400);
   evas_object_show(win);
}

#endif
