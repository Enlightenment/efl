//Compile with:
//gcc -o popup_example_01 popup_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _block_clicked(void *data, Evas_Object *obj, void *event_info);
static void _timeout(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *popup, *content;

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

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

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_block_clicked(void *data, Evas_Object *obj,
               void *event_info)
{
   evas_object_hide(obj);
}

static void
_timeout(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_hide(obj);
}
