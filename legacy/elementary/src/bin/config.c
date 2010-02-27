#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static int quiet = 0;
static int interactive = 1;

static const char *theme_set = NULL;
static const char *finger_size_set = NULL;
static const char *scale_set = NULL;

static void
my_win_del(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
sc_round(void *data, Evas_Object *obj, void *event_info)
{
   double val = elm_slider_value_get(obj);
   double v;
   
   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
sc_change(void *data, Evas_Object *obj, void *event_info)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);
   
   if (scale == val) return;
   elm_scale_all_set(val);
}

static void
fs_round(void *data, Evas_Object *obj, void *event_info)
{
   double val = elm_slider_value_get(obj);
   double v;
   
   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
fs_change(void *data, Evas_Object *obj, void *event_info)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);
   
   if (scale == val) return;
   elm_finger_size_all_set(val);
}

static void
status_win(void)
{
   Evas_Object *win, *bg, *bx0;

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(win, "Elementary Config");

   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);
   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, 1.0, 1.0);
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   if (!interactive)
     {
        Evas_Object *lb, *fr;
        
        fr = elm_frame_add(win);
        evas_object_size_hint_weight_set(fr, 1.0, 1.0);
        elm_frame_label_set(fr, "Information");
        elm_box_pack_end(bx0, fr);
        evas_object_show(fr);
        
        lb = elm_label_add(win);
        elm_label_label_set(lb,
                            "Applying configuration change"
                            );
        elm_frame_content_set(fr, lb);
        evas_object_show(lb);
     }
   else
     {
        Evas_Object *lb, *pd, *bx2, *fr, *sl, *sp;
        
        fr = elm_frame_add(win);
        evas_object_size_hint_weight_set(fr, 1.0, 1.0);
        evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_frame_label_set(fr, "Sizing");
        elm_box_pack_end(bx0, fr);
        evas_object_show(fr);
        
        bx2 = elm_box_add(win);
        evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
        evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);
        
        pd = elm_frame_add(win);
        evas_object_size_hint_weight_set(pd, 1.0, 0.0);
        evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
        elm_object_style_set(pd, "pad_medium");
        elm_box_pack_end(bx2, pd);
        evas_object_show(pd);
        
        lb = elm_label_add(win);
        evas_object_size_hint_weight_set(lb, 1.0, 0.0);
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
        elm_label_label_set(lb,"<hilight>Scale</>");
        elm_frame_content_set(pd, lb);
        evas_object_show(lb);
        
        sl = elm_slider_add(win);
        evas_object_size_hint_weight_set(sl, 1.0, 0.0);
        evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
        elm_slider_span_size_set(sl, 120);
        elm_slider_unit_format_set(sl, "%1.2f");
        elm_slider_indicator_format_set(sl, "%1.2f");
        elm_slider_min_max_set(sl, 0.25, 5.0);
        elm_slider_value_set(sl, elm_scale_get());
        elm_box_pack_end(bx2, sl);
        evas_object_show(sl);

        evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
        evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);
        
        sp = elm_separator_add(win);
        elm_separator_horizontal_set(sp, 1);
        evas_object_size_hint_weight_set(sp, 1.0, 0.0);
        evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx2, sp);
        evas_object_show(sp);
        
        pd = elm_frame_add(win);
        evas_object_size_hint_weight_set(pd, 1.0, 0.0);
        evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
        elm_object_style_set(pd, "pad_medium");
        elm_box_pack_end(bx2, pd);
        evas_object_show(pd);
        
        lb = elm_label_add(win);
        evas_object_size_hint_weight_set(lb, 1.0, 0.0);
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
        elm_label_label_set(lb, "<hilight>Finger Size</><br>");
        elm_frame_content_set(pd, lb);
        evas_object_show(lb);
        
        sl = elm_slider_add(win);
        evas_object_size_hint_weight_set(sl, 1.0, 0.0);
        evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
        elm_slider_span_size_set(sl, 120);
        elm_slider_unit_format_set(sl, "%1.0f");
        elm_slider_indicator_format_set(sl, "%1.0f");
        elm_slider_min_max_set(sl, 5, 200);
        elm_slider_value_set(sl, elm_finger_size_get());
        elm_box_pack_end(bx2, sl);
        evas_object_show(sl);

        evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
        evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);
        
        // FIXME: add theme selector (basic mode and advanced for fallbacks)
        // FIXME: save config
        // FIXME: profile selector / creator etc.

        elm_frame_content_set(fr, bx2);
        evas_object_show(bx2);
     }

   evas_object_show(win);
}

static int
_exit_timer(void *data)
{
   elm_exit();
}

/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI int
elm_main(int argc, char **argv)
{
   int i;
   
   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             printf("Usage:\n"
                    "  -h                This help\n"
                    "  -q                Quiet mode (dont show window)\n"
                    "  -t THEME          Set theme to THEME (ELM_THEME spec)\n"
                    "  -f SIZE           Set finger size to SIZE pixels\n"
                    "  -s SCALE          Set scale factor to SCALE\n"
                    );
          }
        else if (!strcmp(argv[i], "-q"))
          {
             quiet = 1;
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-t")) && (i < argc - 1))
          {
             i++;
             theme_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-f")) && (i < argc - 1))
          {
             i++;
             finger_size_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-s")) && (i < argc - 1))
          {
             i++;
             scale_set = argv[i];
             interactive = 0;
          }
     }
   /* put ere any init specific to this app like parsing args etc. */
   if (!quiet)
     {
        status_win(); /* create main window */
        if (!interactive)
          ecore_timer_add(2.0, _exit_timer, NULL);
     }
   if (!interactive)
     {
        if (theme_set)
          {
             elm_theme_all_set(theme_set);
          }
        if (finger_size_set)
          {
             elm_finger_size_all_set(atoi(finger_size_set));
          }
        if (scale_set)
          {
             elm_scale_all_set(atof(scale_set));
          }
        if (quiet)
          {
             elm_exit();
          }
     }
   elm_run(); /* and run the program now  and handle all events etc. */
   /* if the mainloop that elm_run() runs exist - we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* all emeentary apps should use this. but it right after elm_main() */
ELM_MAIN()
