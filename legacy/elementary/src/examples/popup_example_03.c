//Compile with:
//gcc -o popup_example_03 popup_example_03.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _response_cb(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *popup, *btn1, *btn2, *icon1;
   Elm_Object_Item *popup_it1;
   char buf[256];

   elm_app_info_set(elm_main, "elementary", "images/logo_small.png");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   popup = elm_popup_add(win);

   icon1 = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(icon1, buf, NULL);

   //Seting popup title-text
   elm_object_part_text_set(popup, "title,text", "Title");

   //Appending popup content-items
   elm_popup_item_append(popup, "Message", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Email", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Contacts", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Video", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Music", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Memo", NULL, _item_selected_cb, NULL);
   popup_it1 = elm_popup_item_append(popup, "Radio", NULL, _item_selected_cb,
                                     NULL);

   //Changing the label of the item
   elm_object_item_text_set(popup_it1, "FM");
   elm_popup_item_append(popup, "Messenger", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Settings", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "App Installer", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Browser", NULL, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "Weather", icon1, _item_selected_cb, NULL);
   elm_popup_item_append(popup, "News Feeds", NULL, _item_selected_cb, NULL);

   //Seting popup title-text
   elm_object_part_text_set(popup, "title,text", "Title");

   // Creating the first action button
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");

   //Appending the fist action button
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

   //Creating the second action button
   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);

   //Appending the second action button
   elm_object_part_content_set(popup, "button2", btn2);

   //Display the popup object
   evas_object_show(popup);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_item_selected_cb(void *data, Evas_Object *obj,
                  void *event_info)
{
   printf("popup item selected: %s\n", elm_object_item_text_get(event_info));
}

static void
_response_cb(void *data, Evas_Object *obj,
             void *event_info)
{
   evas_object_hide(data);
}
