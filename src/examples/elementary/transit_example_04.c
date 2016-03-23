//Compile with:
//gcc -o transit_example_04 transit_example_04.c `pkg-config --cflags --libs elementary` -DDATA_DIR="\"<directory>\""
//where directory is the a path where images/icon_07.png can be found.

#include <Elementary.h>

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
_checkbox_transition_add(Evas_Object *box, const char *label, Eina_Bool *checked)
{
   Evas_Object *check = elm_check_add(box);
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(check, 0.0, 0.0);
   elm_object_text_set(check, label);
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

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *icon, *box, *hbox, *btn, *rect;
   Eina_List *objs = NULL;
   char buf[PATH_MAX];
   int i;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images/icon_07.png");

   /* add a window */
   win = elm_win_util_standard_add("transit", "Transit Example");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   /* add an object that we are going to play with */
   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(btn, "Transformed object!");
   icon = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_07.png", elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_part_content_set(btn, "icon", icon);
   evas_object_move(btn, 50, 50);
   evas_object_resize(btn, 200, 50);
   evas_object_show(btn);

   objs = eina_list_append(objs, btn);

   /* add another object that we are going to play with */
   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(btn, "Another object!");
   icon = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_08.png", elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_part_content_set(btn, "icon", icon);
   evas_object_move(btn, 50, 50);
   evas_object_resize(btn, 200, 50);

   objs = eina_list_append(objs, btn);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   for (i = 0; _transitions[i].label; i++)
     _checkbox_transition_add(hbox, _transitions[i].label, &_transitions[i].checked);

   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(btn, "Transit!");
   elm_box_pack_end(hbox, btn);
   evas_object_show(btn);

   evas_object_smart_callback_add(btn, "clicked", _transit_start, objs);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, rect);
   evas_object_show(rect);

   evas_object_resize(win, 300, 100);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
