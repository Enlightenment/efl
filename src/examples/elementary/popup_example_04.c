//Compile with:
//gcc -o popup_example_04 popup_example_04.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _response_cb(void *data, Evas_Object *obj, void *event_info);
static void _block_clicked(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *popup, *content, *btn1, *btn2;

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   content = elm_label_add(win);
   elm_object_text_set(content, "<align=center>Content</align>");

   popup = elm_popup_add(win);

   //Setting popup content
   elm_object_content_set(popup, content);
   //Seting popup title-text
   elm_object_part_text_set(popup, "title,text", "Title");
   evas_object_show(popup);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked, NULL);

   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "access on/off");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_response_cb(void *data, Evas_Object *obj,
             void *event_info)
{
   Evas_Object *access;

   elm_config_access_set(!elm_config_access_get());

   access = elm_object_part_access_object_get(data, "access.title");
   printf("access object of popup title: %p (access mode: %d)\n", access, elm_config_access_get());
}

static void
_block_clicked(void *data, Evas_Object *obj,
               void *event_info)
{
   evas_object_hide(obj);
}
