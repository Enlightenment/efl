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

static void
_on_done(void        *data,
         Evas_Object *obj,
         void        *event_info)
{
   elm_exit();
}

static Evas_Object *slideshow, *bt_start, *bt_stop;
static Elm_Slideshow_Item_Class itc;

static char img1[256], img2[256], img3[256], img4[256], img5[256], img6[256], img7[256], img8[256], img9[256];

static void
_notify_show(void        *data,
             Evas        *e,
             Evas_Object *obj,
             void        *event_info)
{
   evas_object_show(data);
}

/* jump to next item, cyclically */
static void
_next(void        *data,
      Evas_Object *obj,
      void        *event_info)
{
   elm_slideshow_next(data);
}

static void
_previous(void        *data,
          Evas_Object *obj,
          void        *event_info)
{
   elm_slideshow_previous(data);
}

static void
_first(void        *data,
       Evas_Object *obj,
       void        *event_info)
{
   elm_slideshow_item_show(data);
}

static void
_last(void        *data,
      Evas_Object *obj,
      void        *event_info)
{
   elm_slideshow_item_show(data);
}

static void
_mouse_in(void        *data,
          Evas        *e,
          Evas_Object *obj,
          void        *event_info)
{
   elm_notify_timeout_set(data, 0.0);
   evas_object_show(data);
}

static void
_mouse_out(void        *data,
           Evas        *e,
           Evas_Object *obj,
           void        *event_info)
{
   elm_notify_timeout_set(data, 3.0);
}

/* transition changed */
static void
_transition_select(void        *data,
           Evas_Object *obj,
           void        *event_info)
{
   elm_slideshow_transition_set(slideshow, data);
   elm_object_text_set(obj, data);
}

static void
_layout_select(void        *data,
               Evas_Object *obj,
               void        *event_info)
{
   elm_slideshow_layout_set(slideshow, data);
   elm_object_text_set(obj, data);
}

/* start the show! */
static void
_start(void        *data,
       Evas_Object *obj,
       void        *event_info)
{
   elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));

   elm_object_disabled_set(bt_start, EINA_TRUE);
   elm_object_disabled_set(bt_stop, EINA_FALSE);
}

static void
_stop(void        *data,
      Evas_Object *obj,
      void        *event_info)
{
   elm_slideshow_timeout_set(slideshow, 0.0);
   elm_object_disabled_set(bt_start, EINA_FALSE);
   elm_object_disabled_set(bt_stop, EINA_TRUE);
}

/* slideshow transition time has changed */
static void
_spin(void        *data,
      Evas_Object *obj,
      void        *event_info)
{
   if (elm_slideshow_timeout_get(slideshow) > 0)
     elm_slideshow_timeout_set(slideshow, elm_spinner_value_get(data));
}

/* get our images to make slideshow items */
static Evas_Object *
_get(void        *data,
     Evas_Object *obj)
{
   Evas_Object *photo = elm_photo_add(obj);
   elm_photo_file_set(photo, data);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");

   return photo;
}

/* ordering alphabetically */
static int
_cmp_func(const void *data1,
          const void *data2)
{
   const char *img_path1, *img_path2;

   const Elm_Object_Item *slide_it1 = data1;
   const Elm_Object_Item *slide_it2 = data2;

   img_path1 = elm_object_item_data_get(slide_it1);
   img_path2 = elm_object_item_data_get(slide_it2);

   return strcasecmp(img_path1, img_path2);
}

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   Evas_Object *win, *bg, *notify, *bx, *bt, *hv, *spin;
   Elm_Object_Item *slide_first, *slide_last, *slide_it;
   const char *transition, *layout;
   const Eina_List *l, *list;
   const char *data_dir;

   elm_app_info_set(elm_main, "elementary", "images");
   data_dir = elm_app_data_dir_get();
   snprintf(img1, sizeof(img1), "%s/images/logo.png", data_dir);
   snprintf(img2, sizeof(img2), "%s/images/plant_01.jpg", data_dir);
   snprintf(img3, sizeof(img3), "%s/images/rock_01.jpg", data_dir);
   snprintf(img4, sizeof(img4), "%s/images/rock_02.jpg", data_dir);
   snprintf(img5, sizeof(img5), "%s/images/sky_01.jpg", data_dir);
   snprintf(img6, sizeof(img6), "%s/images/sky_04.jpg", data_dir);
   snprintf(img7, sizeof(img7), "%s/images/wood_01.jpg", data_dir);
   snprintf(img8, sizeof(img8), "%s/images/mystrale.jpg", data_dir);
   snprintf(img9, sizeof(img9), "%s/images/mystrale_2.jpg", data_dir);

   win = elm_win_add(NULL, "slideshow", ELM_WIN_BASIC);
   elm_win_title_set(win, "Slideshow example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   evas_object_resize(win, 600, 400);
   evas_object_show(win);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   slideshow = elm_slideshow_add(win);
   elm_slideshow_loop_set(slideshow, EINA_TRUE);
   evas_object_size_hint_weight_set(slideshow,
                                    EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, slideshow);
   evas_object_show(slideshow);

   itc.func.get = _get;
   itc.func.del = NULL;

   slide_first = elm_slideshow_item_sorted_insert(slideshow, &itc, img1, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img2, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img3, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img4, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img5, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img6, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img7, _cmp_func);
   elm_slideshow_item_sorted_insert(slideshow, &itc, img8, _cmp_func);
   slide_last = elm_slideshow_item_add(slideshow, &itc, img9);

   list = elm_slideshow_items_get(slideshow);
   fprintf(stdout, "List of items in the slideshow:\n");
   EINA_LIST_FOREACH(list, l, slide_it)
     fprintf(stdout, "\t%s\n",
             (const char *)elm_object_item_data_get(slide_it));

   notify = elm_notify_add(win);
   elm_notify_orient_set(notify, ELM_NOTIFY_ORIENT_BOTTOM);
   evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "First");
   evas_object_smart_callback_add(bt, "clicked", _first, slide_first);
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

   _notify_show(notify, NULL, NULL, NULL);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
