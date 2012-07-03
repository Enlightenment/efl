/**
 * Simple Elementary's <b>progress bar widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g progressbar_example.c -o progressbar_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

#include <time.h>

typedef struct Progressbar_Example
{
   Evas_Object *pb1;
   Evas_Object *pb2; /* pulsing */
   Evas_Object *pb3;
   Evas_Object *pb4;
   Evas_Object *pb5;
   Evas_Object *pb6; /* pulsing */
   Evas_Object *pb7;
   Evas_Object *pb8; /* pulsing */

   Eina_Bool    run;
   Ecore_Timer *timer;
} Progressbar_Example;

static Progressbar_Example example_data;

static Eina_Bool
_progressbar_example_value_set(void *data)
{
   double progress;

   progress = elm_progressbar_value_get(example_data.pb1);
   if (progress < 1.0) progress += 0.0123;
   else progress = 0.0;

   /* just the non-pulsing ones need an update */
   elm_progressbar_value_set(example_data.pb1, progress);
   elm_progressbar_value_set(example_data.pb3, progress);
   elm_progressbar_value_set(example_data.pb4, progress);
   elm_progressbar_value_set(example_data.pb5, progress);
   elm_progressbar_value_set(example_data.pb7, progress);

   if (progress < 1.0) return ECORE_CALLBACK_RENEW;

   example_data.run = 0;
   return ECORE_CALLBACK_CANCEL;
}

static void
_progressbar_example_start(void        *data,
                           Evas_Object *obj,
                           void        *event_info)
{
   elm_progressbar_pulse(example_data.pb2, EINA_TRUE);
   elm_progressbar_pulse(example_data.pb6, EINA_TRUE);
   elm_progressbar_pulse(example_data.pb8, EINA_TRUE);

   if (!example_data.run)
     {
        example_data.timer = ecore_timer_add(
            0.1, _progressbar_example_value_set, NULL);
        example_data.run = EINA_TRUE;
     }
}

/* end of show */
static void
_progressbar_example_stop(void        *data,
                          Evas_Object *obj,
                          void        *event_info)
{
   elm_progressbar_pulse(example_data.pb2, EINA_FALSE);
   elm_progressbar_pulse(example_data.pb6, EINA_FALSE);
   elm_progressbar_pulse(example_data.pb8, EINA_FALSE);

   if (example_data.run)
     {
        ecore_timer_del(example_data.timer);
        example_data.run = EINA_FALSE;
     }
}

/* Format callback */
static char *
_progress_format_cb(double val)
{
   static char buf[30];
   int files = (1-val)*14000;
   if (snprintf(buf, 30, "%i files left", files) > 0)
     return strdup(buf);
   return NULL;
}

static void
_progress_format_free(char *str)
{
   free(str);
}

/* Callback for "changed" signal */
static void
_on_changed(void        *data,
            Evas_Object *obj,
            void        *event_info)
{
   static char buf[30];
   static time_t tstart = 0;
   static double eta = 0;
   time_t tdiff;
   double val;
   Evas_Object *label =  (Evas_Object *)data;

   val = elm_progressbar_value_get(obj);
   if (val == 0)
     {
        tstart = 0;
        elm_object_text_set(label, "ETA: N/A");
        return;
     }

   /* First invocation */
   if (tstart == 0)
     {
        tstart = time(NULL);
     }

   /* Calculate ETA and update */
   tdiff = time(NULL) - tstart;
   eta = 0.3*eta + 0.7*(tdiff/val)*(1-val);
   snprintf(buf, 30, "ETA: %.0fs", eta);
   elm_object_text_set(label, buf);
}

static void
_on_done(void        *data,
         Evas_Object *obj,
         void        *event_info)
{
   _progressbar_example_stop(NULL, NULL, NULL);
   elm_exit();
}

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   Evas_Object *win, *bg, *pb, *bx, *hbx, *bt, *bt_bx, *ic1, *ic2, *label;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "images/logo_small.png");
   win = elm_win_add(NULL, "progressbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Progress bar example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   /* pb with no label, default unit label and no icon */
   pb = elm_progressbar_add(win);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);
   example_data.pb1 = pb;

   /* pb with label, and set to pulse */
   pb = elm_progressbar_add(win);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(pb, "Infinite bounce");
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);
   example_data.pb2 = pb;

   ic1 = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic1, buf, NULL);
   evas_object_size_hint_aspect_set(ic1, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   /* pb with label, icon, custom unit label function and span size set */
   pb = elm_progressbar_add(win);
   elm_object_text_set(pb, "Label");
   elm_object_part_content_set(pb, "icon", ic1);
   elm_progressbar_inverted_set(pb, EINA_TRUE);
   elm_progressbar_unit_format_function_set(pb, _progress_format_cb,
                                            _progress_format_free);
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, pb);
   evas_object_show(ic1);
   evas_object_show(pb);
   example_data.pb3 = pb;

   /* pb with label and changed trigger  */
   pb = elm_progressbar_add(win);
   elm_object_text_set(pb, "Label");
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);

   label = elm_label_add(win);
   elm_object_text_set(label, "ETA: N/A");
   evas_object_size_hint_align_set(label, 0.5, 0.5);
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, label);
   evas_object_show(label);

   evas_object_smart_callback_add(pb, "changed", _on_changed, label);
   example_data.pb4 = pb;

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   /* vertical pb */
   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx, pb);
   elm_object_text_set(pb, "percent");
   evas_object_show(pb);
   example_data.pb5 = pb;

   /* vertical pb, with pulse and custom (small) span size */
   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_progressbar_span_size_set(pb, 80);
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_progressbar_unit_format_set(pb, NULL);
   elm_object_text_set(pb, "Infinite bounce");
   elm_box_pack_end(hbx, pb);
   evas_object_show(pb);
   example_data.pb6 = pb;

   ic2 = elm_icon_add(win);
   elm_image_file_set(ic2, buf, NULL);
   evas_object_size_hint_aspect_set(ic2, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   /* vertical pb, inverted, with custom unit format and icon*/
   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   elm_object_text_set(pb, "Label");
   elm_object_part_content_set(pb, "icon", ic2);
   elm_progressbar_inverted_set(pb, EINA_TRUE);
   elm_progressbar_unit_format_set(pb, "%1.2f%%");
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx, pb);
   evas_object_show(ic2);
   evas_object_show(pb);
   example_data.pb7 = pb;

   /* "wheel" style progress bar */
   pb = elm_progressbar_add(win);
   elm_object_style_set(pb, "wheel");
   elm_object_text_set(pb, "Style: wheel");
   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);
   example_data.pb8 = pb;

   bt_bx = elm_box_add(win);
   elm_box_horizontal_set(bt_bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bt_bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, bt_bx);
   evas_object_show(bt_bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Start");
   evas_object_smart_callback_add(bt, "clicked", _progressbar_example_start,
                                  NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Stop");
   evas_object_smart_callback_add(bt, "clicked", _progressbar_example_stop,
                                  NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
