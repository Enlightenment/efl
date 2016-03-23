/*
 * gcc -o win_example win_example.c `pkg-config --cflags --libs elementary ecore-x`
 */
#ifdef HAVE_ELEMENTARY_X
# include <Ecore_X.h>
#endif

#include <Elementary.h>

static Evas_Object *win2;

static void
_btn_activate_cb(void *data, Evas_Object *obj, void *event)
{
   elm_win_activate(data);
}

static void
_btn_lower_cb(void *data, Evas_Object *obj, void *event)
{
   elm_win_lower(data);
}

static void
_btn_raise_cb(void *data, Evas_Object *obj, void *event)
{
   elm_win_raise(data);
}

static void
_btn_borderless_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_borderless_get(data);
   elm_win_borderless_set(data, !flag);
}

static void
_btn_shaped_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_shaped_get(data);
   elm_win_shaped_set(data, !flag);
}

static void
_btn_alpha_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_alpha_get(data);
   elm_win_alpha_set(data, !flag);
}

static void
_btn_fullscreen_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_fullscreen_get(data);
   elm_win_fullscreen_set(data, !flag);
}

static void
_btn_maximized_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_maximized_get(data);
   elm_win_maximized_set(data, !flag);
}

static void
_btn_iconified_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_iconified_get(data);
   elm_win_iconified_set(data, !flag);
}

static void
_btn_rotation_cb(void *data, Evas_Object *obj, void *event)
{
   int angle = elm_win_rotation_get(data);
   angle = (angle + 90) % 360;
   elm_win_rotation_set(data, angle);
}

static void
_btn_rotation_resize_cb(void *data, Evas_Object *obj, void *event)
{
   int angle = elm_win_rotation_get(data);
   angle = (angle + 90) % 360;
   elm_win_rotation_with_resize_set(data, angle);
}

static void
_btn_sticky_cb(void *data, Evas_Object *obj, void *event)
{
   Eina_Bool flag = elm_win_sticky_get(data);
   elm_win_sticky_set(data, !flag);
}

static void
_yes_quit_cb(void *data, Evas_Object *obj, void *event)
{
   elm_exit();
}

static void
_no_quit_cb(void *data, Evas_Object *obj, void *event)
{
   evas_object_del(data);
}

static void
_main_win_del_cb(void *data, Evas_Object *obj, void *event)
{
   Evas_Object *msg, *box, *box2, *btn, *lbl, *sep;

   msg = elm_notify_add(obj);
   elm_notify_align_set(msg, 0.5, 0.5);
   elm_notify_allow_events_set(msg, EINA_FALSE);
   evas_object_show(msg);

   box = elm_box_add(obj);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(msg, box);
   evas_object_show(box);

   lbl = elm_label_add(obj);
   elm_object_text_set(lbl, "Really want quit?");
   evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, lbl);
   evas_object_show(lbl);

   sep = elm_separator_add(obj);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(box, sep);
   evas_object_show(sep);

   box2 = elm_box_add(obj);
   elm_box_horizontal_set(box2, EINA_TRUE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   btn = elm_button_add(obj);
   elm_object_text_set(btn, "Yes");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", _yes_quit_cb, NULL);

   btn = elm_button_add(obj);
   elm_object_text_set(btn, "No");
   elm_box_pack_end(box2, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", _no_quit_cb, msg);
}

static void
_force_focus_cb(void *data, Evas_Object *obj, void *event)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = elm_win_xwindow_get(data);
   ecore_x_window_focus(xwin);
#endif
}

static void
_win_focused_cb(void *data, Evas_Object *obj, void *event)
{
   const char *name = data;
   printf("Window focused: %s\n", name);
}

static Eina_Bool
key_down()
{
   elm_win_fullscreen_set(win2, 0);

   return ECORE_CALLBACK_PASS_ON;
}

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Evas_Object *win, *bigbox, *box, *btn, *o;
   char buf[256];

   elm_app_info_set(elm_main, "elementary", "images/logo.png");

   win = elm_win_util_standard_add("win-example", "Elm_Win Example");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   evas_object_smart_callback_add(win, "focus,in", _win_focused_cb, "mainwin");

   evas_object_smart_callback_add(win, "delete,request", _main_win_del_cb,
                                  NULL);

   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, key_down, NULL);

   bigbox = elm_box_add(win);
   elm_box_horizontal_set(bigbox, EINA_TRUE);
   evas_object_size_hint_weight_set(bigbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bigbox);
   evas_object_show(bigbox);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bigbox, box);
   evas_object_show(box);

   win2 = elm_win_add(NULL, "sub-win-example", ELM_WIN_DIALOG_BASIC);
   elm_win_autodel_set(win2, EINA_TRUE);
   elm_win_title_set(win2, "Managed window");
   evas_object_show(win2);

   evas_object_smart_callback_add(win2, "focus,in", _win_focused_cb, "managed");

   o = elm_icon_add(win2);
   sprintf(buf, "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(o, buf, NULL);
   elm_image_resizable_set(o, EINA_FALSE, EINA_FALSE);
   elm_image_no_scale_set(o, EINA_TRUE);
   elm_win_resize_object_add(win2, o);
   evas_object_show(o);

   o = elm_label_add(win);
   elm_object_text_set(o, "<b>Managed Window</b>");
   elm_box_pack_end(box, o);
   evas_object_show(o);

#define WIN_ACTION(name) \
   do { \
        btn = elm_button_add(win); \
        elm_object_text_set(btn, #name); \
        elm_box_pack_end(box, btn); \
        evas_object_show(btn); \
        evas_object_smart_callback_add(btn, "clicked", _btn_##name##_cb, win2); \
   } while (0)

   WIN_ACTION(activate);
   WIN_ACTION(lower);
   WIN_ACTION(raise);
   WIN_ACTION(borderless);
   WIN_ACTION(shaped);
   WIN_ACTION(alpha);
   WIN_ACTION(fullscreen);
   WIN_ACTION(maximized);
   WIN_ACTION(iconified);
   WIN_ACTION(rotation);
   WIN_ACTION(rotation_resize);
   WIN_ACTION(sticky);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bigbox, box);
   evas_object_show(box);

   win2 = elm_win_util_standard_add("sub-win-example2", "Non-managed window");
   elm_win_autodel_set(win2, EINA_TRUE);
   elm_win_override_set(win2, EINA_TRUE);
   evas_object_show(win2);

   evas_object_smart_callback_add(win2, "focus,in", _win_focused_cb,
                                  "override");

   bigbox = elm_box_add(win2);
   evas_object_size_hint_weight_set(bigbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win2, bigbox);
   evas_object_show(bigbox);

   o = elm_label_add(win2);
   elm_object_text_set(o, "This window should have no borders or titlebar.<ps>"
                       "It was set in override mode, so the Window Manager<ps>"
                       "should ignore everything about it.<ps>"
                       "It's up to the program to handle it properly, and some"
                       "of the actions performed on it may not have any effect."
                      );
   elm_box_pack_end(bigbox, o);
   evas_object_show(o);

   o = elm_entry_add(win2);
   elm_object_text_set(o, "See if you can focus me");
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bigbox, o);
   evas_object_show(o);

   o = elm_separator_add(win2);
   elm_separator_horizontal_set(o, EINA_TRUE);
   elm_box_pack_end(bigbox, o);
   evas_object_show(o);

   o = elm_button_add(win2);
   elm_object_text_set(o, "Focus me");
   elm_box_pack_end(bigbox, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _force_focus_cb, win2);

   o = elm_label_add(win);
   elm_object_text_set(o, "<b>Override Window</b>");
   elm_box_pack_end(box, o);
   evas_object_show(o);

   WIN_ACTION(activate);
   WIN_ACTION(lower);
   WIN_ACTION(raise);
   WIN_ACTION(borderless);
   WIN_ACTION(shaped);
   WIN_ACTION(alpha);
   WIN_ACTION(fullscreen);
   WIN_ACTION(maximized);
   WIN_ACTION(iconified);
   WIN_ACTION(rotation);
   WIN_ACTION(rotation_resize);
   WIN_ACTION(sticky);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
