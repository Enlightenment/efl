//Compile with:
//gcc -o popup_example_01 popup_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__ __attribute__((unused))
# define PACKAGE_DATA_DIR "../../data"
#endif

static void _block_clicked(void *data, Evas_Object *obj, void *event_info);
static void _timeout(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *popup, *content;

   win = elm_win_add(NULL, "popup", ELM_WIN_BASIC);
   elm_win_title_set(win, "Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 128, 128, 128);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   content = elm_label_add(win);
   elm_object_text_set(content, "<align=center>Content</align>");

   popup = elm_popup_add(win);
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "timeout", _timeout, NULL);

   //Setting popup content
   elm_object_content_set(popup, content);
   //Seting popup title-text
   elm_object_part_text_set(popup, "title,text", "Title");
   evas_object_show(popup);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked, NULL);

   evas_object_show(win);
   evas_object_resize(win, 480, 800);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

static void
_block_clicked(void *data __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   evas_object_hide(obj);
}

static void
_timeout(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   evas_object_hide(obj);
}
