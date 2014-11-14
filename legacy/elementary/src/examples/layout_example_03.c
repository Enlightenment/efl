//Compile with:
//gcc -g layout_example_03.c -o layout_example_03 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

#define TITLE "example/title"
#define SWALLOW "example/custom"

static Eina_Bool _btn_large = EINA_FALSE;

static void
_swallow_btn_cb(void *data, Evas_Object *btn, void *event_info)
{
   Evas_Object *layout = data;

   if (_btn_large == EINA_FALSE)
     {
        _btn_large = EINA_TRUE;
        elm_object_signal_emit(layout, "button,enlarge", "");
        elm_object_text_set(btn, "Reduce me!");
     }
   else
     {
        _btn_large = EINA_FALSE;
        elm_object_signal_emit(layout, "button,reduce", "");
        elm_object_text_set(btn, "Enlarge me!");
     }
}

static void
_size_changed_cb(void *data, Evas_Object *layout, const char *emission, const char *source)
{
   Evas_Object *edje;
   Evas_Coord w, h;

   elm_layout_sizing_eval(layout);
   edje = elm_layout_edje_get(layout);
   edje_object_size_min_calc(edje, &w, &h);
   printf("Minimum size for this theme: %dx%d\n", w, h);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *btn, *layout;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "examples/layout_example.edj");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("layout", "Layout Example");
   elm_win_autodel_set(win, EINA_TRUE);

   // Adding layout
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   snprintf(buf, sizeof(buf), "%s/examples/layout_example.edj", elm_app_data_dir_get());
   elm_layout_file_set(layout, buf, "example/mylayout3");
   evas_object_show(layout);

   elm_object_signal_callback_add(layout, "size,changed", "", _size_changed_cb, layout);

   // Setting title
   const char *title = elm_layout_data_get(layout, "title");
   if (title)
     {
        elm_win_title_set(win, title);
        elm_object_part_text_set(layout, TITLE, title);
     }

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Enlarge me!");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(layout, SWALLOW, btn);
   evas_object_smart_callback_add(btn, "clicked", _swallow_btn_cb, layout);

   evas_object_resize(win, 160, 160);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
