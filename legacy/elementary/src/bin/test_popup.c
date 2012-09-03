#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *g_popup = NULL;
static int times = 0;

static void
_response_cb(void *data, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   Evas_Object *popup_data = evas_object_data_get(data, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_hide(data);
   evas_object_del(data);
}

static void
_g_popup_response_cb(void *data, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

static void
_block_clicked_cb(void *data __UNUSED__, Evas_Object *obj,
                  void *event_info __UNUSED__)
{
   printf("\nblock,clicked callback\n");
   Evas_Object *popup_data = evas_object_data_get(obj, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_del(obj);
}

static void
_item_selected_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   printf("popup item selected: %s\n", elm_object_item_text_get(event_info));
}

static void
_list_click(void *data __UNUSED__, Evas_Object *obj,
            void *event_info __UNUSED__)
{
   Elm_Object_Item *it = elm_list_selected_item_get(obj);
   if (!it) return;
   elm_list_item_selected_set(it, EINA_FALSE);
}

static void
_popup_center_text_cb(void *data, Evas_Object *obj __UNUSED__,
                      void *event_info __UNUSED__)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has content area and "
                       "timeout value is 3 seconds");
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "timeout", _response_cb, popup);
   evas_object_show(popup);
}

static void
_popup_center_text_1button_cb(void *data, Evas_Object *obj __UNUSED__,
                              void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has content area and "
                       "action area set, action area has one button Close");
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void
_popup_center_title_text_1button_cb(void *data, Evas_Object *obj __UNUSED__,
                                    void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set, action area has one button Close");
   elm_object_part_text_set(popup, "title,text", "Title");
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void
_popup_center_title_text_block_clicked_event_cb(void *data,
                                                Evas_Object *obj __UNUSED__,
                                                void *event_info __UNUSED__)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has title area and content area. "
                       "When clicked on blocked event region, popup gets "
                       "deleted");
   elm_object_part_text_set(popup, "title,text", "Title");
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
                                  NULL);
   evas_object_show(popup);
}

static void
_popup_bottom_title_text_3button_cb(void *data, Evas_Object *obj __UNUSED__,
                                    void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set with content being character wrapped. "
                       "action area has three buttons OK, Cancel and Close");
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);
   elm_object_part_text_set(popup, "title,text", "Title");
   icon = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_part_content_set(popup, "title,icon", icon);
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void
_popup_center_title_content_3button_cb(void *data, Evas_Object *obj __UNUSED__,
                                       void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Content");
   icon = elm_icon_add(btn);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_content_set(btn, icon);
   elm_object_content_set(popup, btn);
   elm_object_part_text_set(popup, "title,text", "Title");
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void
_popup_center_title_item_3button_cb(void *data, Evas_Object *obj __UNUSED__,
                                    void *event_info __UNUSED__)
{
   char buf[256];
   unsigned int i;
   Evas_Object *popup, *icon1, *btn1, *btn2, *btn3;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   icon1 = elm_icon_add(popup);
   elm_object_part_text_set(popup, "title,text", "Title");
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon1, buf, NULL);
   for (i = 0; i < 10; i++)
     {
        snprintf(buf, sizeof(buf), "Item%u", i+1);
        if (i == 3)
          elm_popup_item_append(popup, buf, icon1, _item_selected_cb, NULL);
        else
          elm_popup_item_append(popup, buf, NULL, _item_selected_cb, NULL);
     }
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
   evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void
_restack_btn_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *im;
   char buf[PATH_MAX];
   void *popup_data;

   popup_data = evas_object_data_get(data, "im");
   if (popup_data) return;

   im = evas_object_image_filled_add(evas_object_evas_get(obj));
   snprintf(buf, sizeof(buf), "%s/images/%s",
            elm_app_data_dir_get(), "twofish.jpg");
   evas_object_image_file_set(im, buf, NULL);
   evas_object_move(im, 40, 40);
   evas_object_resize(im, 320, 320);
   evas_object_show(im);
   evas_object_data_set((Evas_Object *)data, "im", im);

   evas_object_raise((Evas_Object *)data);
}

static void
_popup_center_title_text_2button_restack_cb(void *data, Evas_Object *obj __UNUSED__,
                                    void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *btn, *btn2;

   popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "When you click the 'Restack' button, "
                       "an image will be located under this popup");
   elm_object_part_text_set(popup, "title,text", "Title");
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Restack");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _restack_btn_clicked, popup);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
                                  NULL);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Close");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);

   evas_object_show(popup);
}

static void
_popup_center_text_1button_hide_show_cb(void *data, Evas_Object *obj __UNUSED__,
                              void *event_info __UNUSED__)
{
   Evas_Object *btn;
   char str[128];

   times++;
   if (g_popup)
     {
        sprintf(str, "You have checked this popup %d times.", times);
        elm_object_text_set(g_popup, str);
        evas_object_show(g_popup);
        return;
     }

   g_popup = elm_popup_add(data);
   evas_object_size_hint_weight_set(g_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(g_popup, "Hide this popup by using the button."
                       "When you click list item again, you can see this popup.");
   btn = elm_button_add(g_popup);
   elm_object_text_set(btn, "Hide");
   elm_object_part_content_set(g_popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _g_popup_response_cb, g_popup);

   evas_object_show(g_popup);
}

static void
_popup_transparent_cb(void *data, Evas_Object *obj __UNUSED__,
                      void *event_info __UNUSED__)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_object_style_set(popup, "transparent");
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup, "This Popup has transparent background");
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

void
test_popup(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   Evas_Object *win, *list;

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   list = elm_list_add(win);
   elm_win_resize_object_add(win, list);
   elm_list_mode_set(list, ELM_LIST_LIMIT);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(list, "selected", _list_click, NULL);

   elm_list_item_append(list, "popup-center-text", NULL, NULL,
                        _popup_center_text_cb, win);
   elm_list_item_append(list, "popup-center-text + 1 button", NULL, NULL,
                        _popup_center_text_1button_cb, win);
   elm_list_item_append(list, "popup-center-title + text + 1 button", NULL,
                        NULL, _popup_center_title_text_1button_cb, win);

   elm_list_item_append(list,
                        "popup-center-title + text (block,clicked handling)",
                        NULL, NULL,
                        _popup_center_title_text_block_clicked_event_cb, win);
   elm_list_item_append(list, "popup-bottom-title + text + 3 buttons", NULL,
                        NULL, _popup_bottom_title_text_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + content + 3 buttons", NULL,
                        NULL, _popup_center_title_content_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + items + 3 buttons", NULL,
                        NULL, _popup_center_title_item_3button_cb, win);
   elm_list_item_append(list, "popup-center-title + text + 2 buttons (check restacking)", NULL, NULL,
                        _popup_center_title_text_2button_restack_cb, win);
   elm_list_item_append(list, "popup-center-text + 1 button (check hide, show)", NULL, NULL,
                        _popup_center_text_1button_hide_show_cb, win);
   elm_list_item_append(list, "popup-transparent", NULL, NULL,
                        _popup_transparent_cb, win);
   elm_list_go(list);
   evas_object_show(list);
   evas_object_show(win);
   evas_object_resize(win, 480, 400);
}

#endif

