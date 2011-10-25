//Compile with:
//gcc -g -DPACKAGE_DATA_DIR="\"<directory>\"" `pkg-config --cflags --libs elementary` transit_example_03.c -o transit_example_03
// where directory is the a path where images/plant_01.jpg can be found.

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

/* structure to hold context for many callbacks */
struct Context {
     Eina_Bool events_enabled;
     Eina_Bool auto_reverse;
     Eina_Bool final_state_keep;
     int repeat_times;
     Elm_Transit_Tween_Mode tween_mode;
     Evas_Object *obj;
};

static void
_transit_translation(Elm_Transit *trans)
{
   /* considering the original position (x0, y0), moves the object from
    * (x0 - 20, y0 - 50) to (x0 + 70, y0 + 150) */
   elm_transit_effect_translation_add(trans, -20, -50, 70, 150);
}

static void
_transit_color(Elm_Transit *trans)
{
   /* changes the object color from 100, 255, 100, 255 to
    * 200, 50, 200, 50 */
   elm_transit_effect_color_add(trans, 100, 255, 100, 255, 200, 50, 200, 50);
}

static void
_transit_rotation(Elm_Transit *trans)
{
   /* rotates the object from its original angle to 135 degrees to the right */
   elm_transit_effect_rotation_add(trans, 0.0, 135.0);
}

static void
_transit_wipe(Elm_Transit *trans)
{
   /* hide the object clipping it from the left to the right */
   elm_transit_effect_wipe_add(trans,
			       ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE,
			       ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT);
}

static void
_transit_zoom(Elm_Transit *trans)
{
   /* zoom the object from its original size to 2x */
   elm_transit_effect_zoom_add(trans, 1.0, 2.0);
}

static void
_transit_resizing(Elm_Transit *trans)
{
   /* resize the object from 250x100 to 400x160 */
   elm_transit_effect_resizing_add(trans, 250, 100, 400, 160);
}


/* helper structure that will hold the transit checkboxes string, callbacks
 * and checked statuses */
static struct {
     const char *label;
     void (*transition_add_cb)(Elm_Transit *);
     Eina_Bool checked;
} _transitions[] = {
       { "Translation", _transit_translation, EINA_FALSE },
       { "Color", _transit_color, EINA_FALSE },
       { "Rotation", _transit_rotation, EINA_FALSE },
       { "Wipe", _transit_wipe, EINA_FALSE },
       { "Zoom", _transit_zoom, EINA_FALSE },
       { "Resizing", _transit_resizing, EINA_FALSE },
       { NULL, NULL, EINA_FALSE }
};

static void
on_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* quit the mainloop (elm_run) */
   elm_exit();
}

/* add a checkbox to the box with the given label, and uses the checked
 * pointer as state_pointer to this checkbox */
static void
_checkbox_transition_add(Evas_Object *box, const char *label, Eina_Bool *checked)
{
   Evas_Object *check = elm_check_add(elm_object_parent_widget_get(box));
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(check, 0.0, 0.0);
   elm_object_text_set(check, label);
   elm_check_state_pointer_set(check, checked);
   elm_box_pack_end(box, check);
   evas_object_show(check);
}

static void
_transit_start(void *data, Evas_Object *o, void *event_info __UNUSED__)
{
   Elm_Transit *trans = NULL;
   int i;
   struct Context *ctxt = data;
   Evas_Object *obj = ctxt->obj; // the object on which the transition will be
				 // applied

   // FIXME: Should check if there's another transit going before starting a new
   // one

   /* initialization: create the transition and add the object to it */
   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);

   /* from our helper structure and array, check if the specified transition is
    * checked and use its callback to add this transition to trans */
   for (i = 0; _transitions[i].label; i++)
     {
	if (_transitions[i].checked)
	  _transitions[i].transition_add_cb(trans);
     }

   /* get the various options for this transition from the context structure */
   elm_transit_event_enabled_set(trans, ctxt->events_enabled);
   elm_transit_auto_reverse_set(trans, ctxt->auto_reverse);
   elm_transit_objects_final_state_keep_set(trans, ctxt->final_state_keep);
   elm_transit_tween_mode_set(trans, ctxt->tween_mode);
   elm_transit_repeat_times_set(trans, ctxt->repeat_times);

   /* set the transition time to 2 seconds and start it */
   elm_transit_duration_set(trans, 2.0);
   elm_transit_go(trans);
}

/* callback useful just to know whether we can receive events from the
 * object or not */
static void
_object_clicked(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   printf("object clicked!\n");
}

/* update our context with the given value for repeat count */
static void
_cb_repeat_changed(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   int *repeat_cnt = data;

   *repeat_cnt = elm_spinner_value_get(obj);
}

/* update our context with the given tween mode for the transition */
static void
_cb_tween_changed(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   Elm_Transit_Tween_Mode *mode = data;
   double val = 0.0;

   val = elm_spinner_value_get(obj);
   if (val == 1.0)
     *mode = ELM_TRANSIT_TWEEN_MODE_LINEAR;
   else if (val == 2.0)
     *mode = ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL;
   else if (val == 3.0)
     *mode = ELM_TRANSIT_TWEEN_MODE_DECELERATE;
   else if (val == 4.0)
     *mode = ELM_TRANSIT_TWEEN_MODE_ACCELERATE;
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *obj, *icon, *box, *vbox, *vbox2, *hbox, *btn, *fr;
   Evas_Object *cbox, *dummy, *spinner;
   Elm_Transit *trans;
   char buf[PATH_MAX];
   int i;
   struct Context context;

   /* initialize our context */
   context.events_enabled = EINA_FALSE;
   context.auto_reverse = EINA_FALSE;
   context.final_state_keep = EINA_FALSE;
   context.repeat_times = 0;
   context.tween_mode = ELM_TRANSIT_TWEEN_MODE_LINEAR;

   /* add a window */
   win = elm_win_add(NULL, "transit", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit Example");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, EINA_TRUE);

   /* add a scalable white background to this window */
   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255, 255, 255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(bg, 640, 640);
   evas_object_size_hint_max_set(bg, 640, 640);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* add a vertical box that will hold everything */
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   /* a dummy background to create some space for the animation */
   dummy = elm_bg_add(win);
   evas_object_size_hint_weight_set(dummy, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, dummy);
   evas_object_show(dummy);

   /* add an object that we are going to play with */
   /* this object isn't packed inside the box because we don't want it to have
    * its size, position, aspect or anything else controled by the container */
   obj = elm_button_add(win);
   elm_object_text_set(obj, "Transformed object!");
   icon = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_07.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon, buf, NULL);
   elm_object_content_set(obj, icon);
   evas_object_move(obj, 160, 60);
   evas_object_resize(obj, 250, 100);
   evas_object_show(obj);
   context.obj = obj;

   /* a callback to know if clicks are being received */
   evas_object_smart_callback_add(obj, "clicked", _object_clicked, NULL);

   /* button to start our transition */
   btn = elm_button_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(btn, "Transit!");
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _transit_start, &context);

   /* horizontal box to help visual organization */
   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   /* horizontal box that will hold the many transition checkboxes */
   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, 0.0);

   /* create the respective checkboxes based on our helper structure and
    * array */
   for (i = 0; _transitions[i].label; i++)
     _checkbox_transition_add(vbox, _transitions[i].label,
			      &_transitions[i].checked);

   elm_box_pack_end(hbox, vbox);
   evas_object_show(vbox);

   /* vertical box that will hold the many transition option checkboxes */
   vbox2 = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox2, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(vbox2, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(hbox, vbox2);
   evas_object_show(vbox2);

   /* the rest of this code adds widgets to control some of the behavior of
    * the transitions */
   cbox = elm_check_add(win);
   evas_object_size_hint_weight_set(cbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cbox, 0.0, 0.0);
   elm_object_text_set(cbox, "Events enabled");
   elm_check_state_pointer_set(cbox, &context.events_enabled);
   elm_box_pack_end(vbox2, cbox);
   evas_object_show(cbox);

   cbox = elm_check_add(win);
   evas_object_size_hint_weight_set(cbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cbox, 0.0, 0.0);
   elm_object_text_set(cbox, "Auto reverse");
   elm_check_state_pointer_set(cbox, &context.auto_reverse);
   elm_box_pack_end(vbox2, cbox);
   evas_object_show(cbox);

   cbox = elm_check_add(win);
   evas_object_size_hint_weight_set(cbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cbox, 0.0, 0.0);
   elm_object_text_set(cbox, "Keep final state");
   elm_check_state_pointer_set(cbox, &context.final_state_keep);
   elm_box_pack_end(vbox2, cbox);
   evas_object_show(cbox);

   spinner = elm_spinner_add(win);
   elm_object_style_set(spinner, "vertical");
   elm_spinner_min_max_set(spinner, 0, 4);
   elm_spinner_label_format_set(spinner, "%.0f");
   elm_spinner_editable_set(spinner, EINA_FALSE);
   evas_object_size_hint_weight_set(spinner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(spinner, 0.0, EVAS_HINT_FILL);
   evas_object_smart_callback_add(spinner, "changed", _cb_repeat_changed, &context.repeat_times);
   elm_box_pack_end(vbox2, spinner);
   evas_object_show(spinner);

   spinner = elm_spinner_add(win);
   elm_object_style_set(spinner, "vertical");
   elm_spinner_min_max_set(spinner, 1, 4);
   elm_spinner_label_format_set(spinner, "%.0f");
   elm_spinner_editable_set(spinner, EINA_FALSE);
   elm_spinner_special_value_add(spinner, 1, "linear");
   elm_spinner_special_value_add(spinner, 2, "sinusoidal");
   elm_spinner_special_value_add(spinner, 3, "decelerate");
   elm_spinner_special_value_add(spinner, 4, "accelerate");
   evas_object_size_hint_weight_set(spinner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(spinner, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(spinner, 200, 30);
   evas_object_smart_callback_add(spinner, "changed", _cb_tween_changed, &context.tween_mode);
   elm_box_pack_end(vbox2, spinner);
   evas_object_show(spinner);

   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
