/*
 * gcc -o inwin_example inwin_example.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static Evas_Object *inwin = NULL;
static const char *styles[] = {
     "default",
     "minimal",
     "minimal_vertical"
};
static int current_style = 0;

static void
_inwin_hide(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   if (inwin)
     {
        evas_object_hide(inwin);
        return;
     }
   elm_object_text_set(obj, "No inwin!");
   elm_object_disabled_set(obj, EINA_TRUE);
}

static void
_inwin_destroy(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   if (inwin)
     {
        evas_object_del(inwin);
        inwin = NULL;
        return;
     }
   elm_object_text_set(obj, "No inwin!");
   elm_object_disabled_set(obj, EINA_TRUE);
}

static void
_btn_click_cb(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   Evas_Object *o, *parent;

   if (inwin)
     {
        elm_win_inwin_activate(inwin);
        return;
     }

   parent = elm_object_top_widget_get(obj);
   inwin = elm_win_inwin_add(parent);
   elm_object_style_set(inwin, styles[current_style]);
   evas_object_show(inwin);

   current_style = (current_style + 1) % 3;

   o = elm_box_add(parent);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_inwin_content_set(inwin, o);
   evas_object_show(o);

   o = elm_label_add(parent);
   elm_object_text_set(o, "Click on the first button to hide the Inwin.<ps>"
                       "Second to destroy it<ps>");
   evas_object_show(o);

   elm_box_pack_end(elm_win_inwin_content_get(inwin), o);

   o = elm_button_add(parent);
   elm_object_text_set(o, "Hide");
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _inwin_hide, NULL);

   elm_box_pack_end(elm_win_inwin_content_get(inwin), o);

   o = elm_button_add(parent);
   elm_object_text_set(o, "Destroy");
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _inwin_destroy, NULL);

   elm_box_pack_end(elm_win_inwin_content_get(inwin), o);
}

static void
_win_del_cb(void *data __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   if (inwin)
     {
        Evas_Object *hover, *o = elm_win_inwin_content_unset(inwin);
        evas_object_del(inwin);
        inwin = NULL;
        hover = elm_hover_add(obj);
        elm_hover_target_set(hover, obj);
        elm_hover_content_set(hover, "middle", o);
        evas_object_show(hover);
        return;
     }
   evas_object_del(obj);
}

static Eina_Bool
_screenshot_hack_cb(void *data)
{
   _btn_click_cb(NULL, data, NULL);
   return EINA_FALSE;
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *box, *o;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "inwin-example", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin Example");
   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, NULL);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   o = elm_button_add(win);
   elm_object_text_set(o, "Inwin!");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   evas_object_smart_callback_add(o, "clicked", _btn_click_cb, NULL);

   if (!strncmp(elm_engine_current_get(), "shot", 4))
     ecore_timer_add(0.1, _screenshot_hack_cb, o);

   elm_run();

   return 0;
}
ELM_MAIN();
