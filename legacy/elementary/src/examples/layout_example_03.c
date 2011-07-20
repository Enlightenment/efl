//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` layout_example_03.c -o layout_example_03

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "."
#endif

#define TITLE "example/title"
#define SWALLOW "example/custom"

static Eina_Bool _btn_large = EINA_FALSE;

static void
_swallow_btn_cb(void *data, Evas_Object *btn, void *event_info __UNUSED__)
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
_size_changed_cb(void *data __UNUSED__, Evas_Object *layout, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *edje;
   Evas_Coord w, h;

   elm_layout_sizing_eval(layout);
   edje = elm_layout_edje_get(layout);
   edje_object_size_min_calc(edje, &w, &h);
   printf("Minimum size for this theme: %dx%d\n", w, h);
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *btn, *layout;

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255,255 ,255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   // Adding layout
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   elm_layout_file_set(
       layout, PACKAGE_DATA_DIR "/examples/layout_example.edj",
       "example/mylayout3");
   evas_object_show(layout);

   elm_object_signal_callback_add(layout, "size,changed", "", _size_changed_cb, layout);

   // Setting title
   const char *title = elm_layout_data_get(layout, "title");
   if (title)
     {
	elm_win_title_set(win, title);
	elm_object_text_part_set(layout, TITLE, title);
     }

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Enlarge me!");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(layout, SWALLOW, btn);
   evas_object_smart_callback_add(btn, "clicked", _swallow_btn_cb, layout);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 160, 160);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
