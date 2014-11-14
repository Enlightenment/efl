/**
 * Simple Elementary's <b>slide show widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -o slideshow_example slideshow_example.c -g `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

#define IMG_NUM 8

static Evas_Object *slideshow, *bt_start, *bt_stop;
static Elm_Slideshow_Item_Class itc;

static void
_notify_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_show(data);
}

/* jump to next item, cyclically */
static void
_next(void  *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_next(data);
}

static void
_previous(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_previous(data);
}

static void
_first(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_item_show(data);
}

static void
_last(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_item_show(data);
}

static void
_mouse_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_notify_timeout_set(data, 0.0);
   evas_object_show(data);
}

static void
_mouse_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   elm_notify_timeout_set(data, 3.0);
}

/* transition changed */
static void
_transition_select(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_transition_set(slideshow, data);
   elm_object_text_set(obj, data);
}

static void
_layout_select(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_layout_set(slideshow, data);
   elm_object_text_set(obj, data);
}

/* start the show! */
static void
_start(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));

   elm_object_disabled_set(bt_start, EINA_TRUE);
   elm_object_disabled_set(bt_stop, EINA_FALSE);
}

static void
_stop(void *data, Evas_Object *obj, void *event_info)
{
   elm_slideshow_timeout_set(slideshow, 0.0);
   elm_object_disabled_set(bt_start, EINA_FALSE);
   elm_object_disabled_set(bt_stop, EINA_TRUE);
}

/* slideshow transition time has changed */
static void
_spin(void *data, Evas_Object *obj, void *event_info)
{
   if (elm_slideshow_timeout_get(slideshow) > 0)
     elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));
}

/* get our images to make slideshow items */
static Evas_Object *
_get(void *data, Evas_Object *obj)
{
   Evas_Object *photo = elm_photo_add(obj);
   elm_photo_file_set(photo, data);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");

   return photo;
}

/* ordering alphabetically */
static int
_cmp_func(const void *data1, const void *data2)
{
   const char *img_path1, *img_path2;

   const Elm_Object_Item *slide_it1 = data1;
   const Elm_Object_Item *slide_it2 = data2;

   img_path1 = elm_object_item_data_get(slide_it1);
   img_path2 = elm_object_item_data_get(slide_it2);

   return strcasecmp(img_path1, img_path2);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *notify, *bx, *bt, *hv, *spin;
   Elm_Object_Item *slide_first = NULL, *slide_last = NULL, *slide_it = NULL;
   const char *transition, *layout;
   const Eina_List *l, *list;
   const char *data_dir;
   char img[IMG_NUM][PATH_MAX];
   char *img_files[] =
     {
        "logo.png", "plant_01.jpg", "rock_01.jpg", "rock_02.jpg", "sky_01.jpg",
        "wood_01.jpg", "mystrale.jpg", "mystrale_2.jpg"
     };
   int i = 0;

   elm_app_info_set(elm_main, "elementary", "images");

   data_dir = elm_app_data_dir_get();
   for (i = 0; i < IMG_NUM; i++)
     snprintf(img[i], PATH_MAX, "%s/images/%s", data_dir, img_files[i]);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("slideshow", "Slideshow example");
   elm_win_autodel_set(win, EINA_TRUE);

   slideshow = elm_slideshow_add(win);
   elm_slideshow_loop_set(slideshow, EINA_TRUE);
   evas_object_size_hint_weight_set(slideshow,
                                    EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, slideshow);
   evas_object_show(slideshow);

   itc.func.get = _get;
   itc.func.del = NULL;

   for (i = 0; i < IMG_NUM; i++)
     {
        slide_it = elm_slideshow_item_sorted_insert(slideshow, &itc, img[i],
                                                    _cmp_func);
        if (!slide_first) slide_first = slide_it;
     }
   slide_last = slide_it;

   list = elm_slideshow_items_get(slideshow);
   fprintf(stdout, "List of items in the slideshow:\n");
   EINA_LIST_FOREACH(list, l, slide_it)
     fprintf(stdout, "\t%s\n",
             (const char *)elm_object_item_data_get(slide_it));

   notify = elm_notify_add(win);
   elm_notify_align_set(notify, 0.5, 1.0);
   evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_notify_timeout_set(notify, 3.0);
   evas_object_show(notify);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_object_content_set(notify, bx);
   evas_object_show(bx);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_IN,
                                  _mouse_in_cb, notify);
   evas_object_event_callback_add(bx, EVAS_CALLBACK_MOUSE_OUT,
                                  _mouse_out_cb, notify);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "First");
   evas_object_smart_callback_add(bt, "clicked", _first, slide_first);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Last");
   evas_object_smart_callback_add(bt, "clicked", _last, slide_last);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   hv = elm_hoversel_add(win);
   elm_box_pack_end(bx, hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(elm_slideshow_transitions_get(slideshow), l, transition)
     elm_hoversel_item_add(hv, transition, NULL, 0, _transition_select,
                           transition);
   elm_object_text_set(hv, eina_list_data_get(
         elm_slideshow_transitions_get(slideshow)));
   evas_object_show(hv);

   hv = elm_hoversel_add(win);
   elm_box_pack_end(bx, hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(elm_slideshow_layouts_get(slideshow), l, layout)
     elm_hoversel_item_add(hv, layout, NULL, 0, _layout_select, layout);
   elm_object_text_set(hv, elm_slideshow_layout_get(slideshow));
   evas_object_show(hv);

   spin = elm_spinner_add(win);
   elm_spinner_label_format_set(spin, "%2.0f s");
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

   evas_object_resize(win, 600, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
