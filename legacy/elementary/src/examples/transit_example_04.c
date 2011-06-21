//Compile with:
//gcc -g -DPACKAGE_DATA_DIR="\"<directory>\"" `pkg-config --cflags --libs elementary` transit_example_04.c -o transit_example_04
// where directory is the a path where images/plant_01.jpg can be found.


#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_transit_flip(Elm_Transit *trans)
{
   elm_transit_effect_flip_add(trans, ELM_TRANSIT_EFFECT_FLIP_AXIS_X, EINA_TRUE);
}

static void
_transit_blend(Elm_Transit *trans)
{
   elm_transit_effect_blend_add(trans);
}

static void
_transit_fade(Elm_Transit *trans)
{
   elm_transit_effect_fade_add(trans);
}

static void
_transit_resizable_flip(Elm_Transit *trans)
{
   elm_transit_effect_resizable_flip_add(
      trans, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
}

static struct {
     const char *label;
     void (*transition_add_cb)(Elm_Transit *);
     Eina_Bool checked;
} _transitions[] = {
       { "Flip", _transit_flip, EINA_FALSE },
       { "Blend", _transit_blend, EINA_FALSE },
       { "Fade", _transit_fade, EINA_FALSE },
       { "Resizable Flip", _transit_resizable_flip, EINA_FALSE },
       { NULL, NULL, EINA_FALSE }
};

static void
on_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* quit the mainloop (elm_run) */
   elm_exit();
}

static void
_checkbox_transition_add(Evas_Object *box, const char *label, Eina_Bool *checked)
{
   Evas_Object *check = elm_check_add(elm_object_parent_widget_get(box));
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(check, 0.0, 0.0);
   elm_check_label_set(check, label);
   elm_check_state_pointer_set(check, checked);
   elm_box_pack_end(box, check);
   evas_object_show(check);
}

static void
_transit_start(void *data, Evas_Object *o, void *event_info)
{
   Elm_Transit *trans = NULL;
   Eina_List *objs = data, *l;
   Evas_Object *obj;
   int i;

   trans = elm_transit_add();
   EINA_LIST_FOREACH(objs, l, obj)
      elm_transit_object_add(trans, obj);

   // FIXME: Should check if there's another transit going before starting a new
   // one

   for (i = 0; _transitions[i].label; i++)
     {
	if (_transitions[i].checked)
	  _transitions[i].transition_add_cb(trans);
     }

   elm_transit_duration_set(trans, 2.0);
   elm_transit_go(trans);
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *obj, *icon, *box, *vbox, *btn, *fr, *dummy;
   Elm_Transit *trans;
   Eina_List *objs = NULL;
   char buf[PATH_MAX];
   int i;

   /* add a window */
   win = elm_win_add(NULL, "transit", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit Example");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, 1);

   /* add a scalable white background to this window */
   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255, 255, 255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(bg, 640, 640);
   evas_object_size_hint_max_set(bg, 640, 640);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   dummy = elm_bg_add(win);
   evas_object_size_hint_weight_set(dummy, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, dummy);
   evas_object_show(dummy);

   /* add an object that we are going to play with */
   obj = elm_button_add(win);
   elm_button_label_set(obj, "Transformed object!");
   icon = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_07.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon, buf, NULL);
   elm_button_icon_set(obj, icon);
   evas_object_move(obj, 160, 60);
   evas_object_resize(obj, 250, 100);
   evas_object_show(obj);

   objs = eina_list_append(objs, obj);

   /* add another object that we are going to play with */
   obj = elm_button_add(win);
   elm_button_label_set(obj, "Another object!");
   icon = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_08.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon, buf, NULL);
   elm_button_icon_set(obj, icon);
   evas_object_move(obj, 160, 60);
   evas_object_resize(obj, 250, 100);

   objs = eina_list_append(objs, obj);

   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_button_label_set(btn, "Transit!");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", _transit_start, objs);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, 0.0);

   for (i = 0; _transitions[i].label; i++)
     _checkbox_transition_add(vbox, _transitions[i].label, &_transitions[i].checked);

   elm_box_pack_end(box, vbox);
   evas_object_show(vbox);

   evas_object_show(win);

   elm_run();

   eina_list_free(objs);

   return 0;
}

ELM_MAIN()
