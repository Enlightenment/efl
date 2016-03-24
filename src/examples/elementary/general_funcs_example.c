/* Compile with:
 * gcc -g general_funcs_example.c -o general_funcs_example `pkg-config --cflags --libs elementary`
 */

#include <Elementary.h>

#define BIN_DIR  "/usr/bin"
#define LIB_DIR  "/usr/lib"
#define DATA_DIR "/usr/local/share/elementary"

#define WIDTH             300
#define HEIGHT            300

struct test_data
{
   Evas_Object *btn;
   Eina_Bool    btn_enabled;
   Eina_Bool    btn_gets_focus;
};

struct test_data d = {NULL, EINA_TRUE, EINA_TRUE};

static void
_btn_enabled_cb(void        *data,
                Evas_Object *obj,
                void        *event)
{
   elm_object_disabled_set(d.btn, !d.btn_enabled);
}

static void
/* focus callback */
_btn_focus_cb(void        *data,
              Evas_Object *obj,
              void        *event)
{
   elm_object_focus_set(d.btn, EINA_TRUE);
}

static void
/* unfocus callback */
_btn_unfocus_cb(void        *data,
                Evas_Object *obj,
                void        *event)
{
   elm_object_focus_set(d.btn, EINA_FALSE);
}

static void
/* focus allow callback */
_btn_focus_allow_cb(void        *data,
                    Evas_Object *obj,
                    void        *event)
{
   elm_object_focus_allow_set(d.btn, d.btn_gets_focus);
}

static void /* scaling callback */
_btn_scale_cb(void        *data,
              Evas_Object *obj,
              void        *event)
{
   elm_object_scale_set(d.btn, elm_slider_value_get(obj));
}

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   Evas_Object *win, *box, *frame, *check, *b, *slider;

   /* tell elm about our app so it can figure out where to get files */
   elm_app_compile_bin_dir_set(BIN_DIR);
   elm_app_compile_data_dir_set(DATA_DIR);
   elm_app_compile_lib_dir_set(LIB_DIR);
   elm_app_info_set(elm_main, "elementary", "images/logo.png");

   fprintf(stdout, "prefix was set to: %s\n", elm_app_prefix_dir_get());
   fprintf(stdout, "data directory is: %s\n", elm_app_data_dir_get());
   fprintf(stdout, "library directory is: %s\n", elm_app_lib_dir_get());
   fprintf(stdout, "locale directory is: %s\n", elm_app_locale_dir_get());

   win = elm_win_util_standard_add("top-level-funcs-example",
                                   "Elementary Top-level Functions Example");

   /* by using this policy value, we avoid having to
    * evas_object_smart_callback_add(win, "delete,request", _on_exit, NULL),
    * calling elm_exit() on that callback ourselves.
    */
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   /* outer box */
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   frame = elm_frame_add(win);
   elm_object_text_set(frame, "Button");
   evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(frame);
   elm_box_pack_end(box, frame);

   d.btn = elm_button_add(win);
   elm_object_text_set(d.btn, "Click me");
   elm_object_content_set(frame, d.btn);
   fprintf(stdout, "Elementary's finger size is set to %d pixels\n.",
           elm_config_finger_size_get());
   evas_object_show(d.btn);

   b = elm_box_add(win);
   elm_box_horizontal_set(b, EINA_FALSE);
   evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, b);
   evas_object_show(b);

   check = elm_check_add(win);
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(check, 0.0, 0.0);
   elm_object_text_set(check, "Button enabled");
   elm_check_state_pointer_set(check, &d.btn_enabled);
   evas_object_smart_callback_add(check, "changed", _btn_enabled_cb, NULL);
   elm_box_pack_end(box, check);
   evas_object_show(check);

   b = elm_button_add(win);
   evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(b, 0.0, 0.0);
   elm_object_text_set(b, "Focus top button");
   evas_object_smart_callback_add(b, "clicked", _btn_focus_cb, NULL);
   elm_box_pack_end(box, b);
   evas_object_show(b);

   b = elm_button_add(win);
   evas_object_size_hint_weight_set(b, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(b, 0.0, 0.0);
   elm_object_text_set(b, "Unfocus top button");
   evas_object_smart_callback_add(b, "clicked", _btn_unfocus_cb, NULL);
   elm_box_pack_end(box, b);
   evas_object_show(b);

   check = elm_check_add(win);
   evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(check, 0.0, 0.0);
   elm_object_text_set(check, "Button gets focus");
   elm_check_state_pointer_set(check, &d.btn_gets_focus);
   evas_object_smart_callback_add(check, "changed", _btn_focus_allow_cb, NULL);
   elm_box_pack_end(box, check);
   evas_object_show(check);

   slider = elm_slider_add(win);
   elm_slider_min_max_set(slider, 0, 4);
   elm_slider_unit_format_set(slider, "%1.1f");
   elm_slider_indicator_format_set(slider, "%1.1f");
   evas_object_size_hint_weight_set(
      slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(slider, "changed", _btn_scale_cb, NULL);
   elm_box_pack_end(box, slider);
   evas_object_show(slider);

   evas_object_resize(win, WIDTH, HEIGHT);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_show(win);

   elm_run(); /* and run the program now, starting to handle all
               * events, etc. */

   /* exit code */
   return 0;
}
ELM_MAIN()
