#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define POPUP_POINT_MAX 6

typedef struct
{
   double x;
   double y;
} Evas_Rel_Coord_Point;

static Evas_Object *g_popup = NULL;
static int times = 0;
static Evas_Rel_Coord_Point _popup_point[POPUP_POINT_MAX] =
{
   { 0.01, 0.01 },
   { 0.2, 0.2 },
   { 0.5, 0.5 },
   { 0.99, 0.01 },
   { 0.01, 0.99 },
   { 0.99, 0.99 }
};

static void
_response_cb(void *data EINA_UNUSED, Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

static void
_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_popup_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   static int k = 0;
   double h = -1, v = -1;

   elm_popup_align_set(data, _popup_point[k].x, _popup_point[k].y);
   elm_popup_align_get(data, &h, &v);

   printf("elm_popup_align_get :: Aligned: %lf %lf\n", h, v);

   k++;
   if (k >= POPUP_POINT_MAX)
     k = 0;
}

static void
_g_popup_response_cb(void *data, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   evas_object_hide(data);
}

static void
_restack_block_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
   // remove restacked image if there is any
   Evas_Object *popup_data = evas_object_data_get(obj, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_del(obj);
}

static void
_restack_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Evas_Object *popup_data = evas_object_data_get(data, "im");
   if (popup_data) evas_object_del(popup_data);
   evas_object_del(data);
}

static void
_block_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (data)
     evas_object_del(data);
   evas_object_del(obj);
}

static void
_item_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   printf("popup item selected: %s\n", elm_object_item_text_get(event_info));
}

static void
_list_click(void *data EINA_UNUSED, Evas_Object *obj,
            void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = elm_list_selected_item_get(obj);
   if (!it) return;
   elm_list_item_selected_set(it, EINA_FALSE);
}

static void
_popup_center_text_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   elm_object_text_set(popup, "This Popup has content area and "
                       "timeout value is 3 seconds");
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "timeout", _response_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_text_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_object_text_set(popup, "This Popup has content area and "
                       "action area set, action area has one button Close");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);

   // popup text
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set, action area has one button Close");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_block_clicked_event_cb(void *data,
                                                Evas_Object *obj EINA_UNUSED,
                                                void *event_info EINA_UNUSED)
{
   Evas_Object *popup;

   popup = elm_popup_add(data);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
                                  NULL);

   // popup text
   elm_object_text_set(popup, "This Popup has title area and content area. "
                       "When clicked on blocked event region, popup gets "
                       "deleted");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_text_block_clicked_event_with_parent_cb(void *data,
                                                            Evas_Object *obj EINA_UNUSED,
                                                            void *event_info EINA_UNUSED)
{
   Evas_Object *bg, *popup;

   bg = elm_bg_add(data);
   elm_bg_color_set(bg, 255, 0, 0);
   evas_object_resize(bg, 350, 200);
   evas_object_move(bg, 100, 100);
   evas_object_show(bg);

   popup = elm_popup_add(bg);
   evas_object_smart_callback_add(popup, "block,clicked",
                                  _block_clicked_cb, bg);

   // popup text
   elm_object_text_set(popup, "This Popup has title area and content area. "
                       "Its blocked event region is a small rectangle. "
                       "When clicked on blocked event region, popup gets "
                       "deleted");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_bottom_title_text_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);

   // popup text
   elm_object_text_set(popup, "This Popup has title area, content area and "
                       "action area set with content being character wrapped. "
                       "action area has three buttons OK, Cancel and Close");
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup title icon
   icon = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_part_content_set(popup, "title,icon", icon);

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_title_content_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                       void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *icon, *btn, *btn1, *btn2, *btn3;
   char buf[256];

   popup = elm_popup_add(data);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup content
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Content");
   icon = elm_icon_add(btn);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(icon, buf, NULL);
   elm_object_content_set(btn, icon);
   elm_object_content_set(popup, btn);

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_item_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,focused:%p\n", it);
}

static void
_item_unfocused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;

   printf("item,unfocused:%p\n", it);
}

static void
_popup_center_title_item_3button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   char buf[256];
   unsigned int i;
   Evas_Object *popup, *icon1, *btn1, *btn2, *btn3;

   popup = elm_popup_add(data);

   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup items
   icon1 = elm_icon_add(popup);
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

   // popup buttons
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _popup_close_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   elm_object_part_content_set(popup, "button3", btn3);
   evas_object_smart_callback_add(btn3, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
   evas_object_smart_callback_add(popup, "item,focused", _item_focused_cb, NULL);
   evas_object_smart_callback_add(popup, "item,unfocused", _item_unfocused_cb, NULL);
}

static void
_restack_btn_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
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
_popup_center_title_text_2button_restack_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btn2;

   popup = elm_popup_add(data);
   evas_object_smart_callback_add(popup, "block,clicked",
                                  _restack_block_clicked_cb, NULL);

   // popup text
   elm_object_text_set(popup, "When you click the 'Restack' button, "
                       "an image will be located under this popup");
   // popup title
   elm_object_part_text_set(popup, "title,text", "Title");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Restack");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _restack_btn_clicked, popup);

   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Close");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked",
                                  _restack_popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_center_text_1button_hide_show_cb(void *data, Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
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
   elm_object_text_set(g_popup, "Hide this popup by using the button."
                       "When you click list item again, you can see this popup.");

   // popup buttons
   btn = elm_button_add(g_popup);
   elm_object_text_set(btn, "Hide");
   elm_object_part_content_set(g_popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _g_popup_response_cb, g_popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(g_popup);
}

static void
_toggle_button_cb(void *data,
                  Evas_Object *obj,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *btn = data;
   char buf[] = "button0";
   int i;

   i = (int)(uintptr_t)evas_object_data_get(btn, "index");

   buf[6] = '0' + i + 1;
   if (evas_object_visible_get(btn))
     {
        elm_object_part_content_unset(obj, buf);
        evas_object_hide(btn);
     }
   else
     elm_object_part_content_set(obj, buf, btn);
}

static void
_popup_center_text_3button_add_remove_button_cb(void *data,
                                                Evas_Object *obj EINA_UNUSED,
                                                void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btns[3];

   char buf[256];
   int i;

   popup = elm_popup_add(data);

   // popup title
   elm_object_part_text_set(popup, "title,text",
                            "Click the item to toggle button");

   // popup buttons
   for (i = 0; i < 3; ++i)
     {
        snprintf(buf, sizeof(buf), "Btn #%d", i + 1);
        btns[i] = elm_button_add(popup);
        evas_object_data_set(btns[i], "index", (void*)(uintptr_t)i);
        elm_object_text_set(btns[i], buf);

        elm_popup_item_append(popup, buf, NULL, _toggle_button_cb, btns[i]);

        snprintf(buf, sizeof(buf), "button%d", i + 1);
        elm_object_part_content_set(popup, buf, btns[i]);
        evas_object_smart_callback_add(btns[i], "clicked",
                                       _popup_close_cb, popup);
     }

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_transparent_cb(void *data, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_object_style_set(popup, "transparent");
   elm_object_text_set(popup, "This Popup has transparent background");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Close");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_popup_transparent_align_cb(void *data, Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btn1;

   popup = elm_popup_add(data);
   elm_object_style_set(popup, "transparent");
   elm_object_text_set(popup, "This Popup has transparent background");

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Move");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_align_cb, popup);

   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "Close");
   elm_object_part_content_set(popup, "button2", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_list_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_popup_center_title_list_content_1button_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup, *list;
   Evas_Object *btn;

   popup = elm_popup_add(data);
   elm_object_part_text_set(popup, "title,text", "Title");

   // list as a popup content
   list = elm_list_add(popup);
   elm_list_mode_set(list, ELM_LIST_EXPAND);
   elm_list_item_append(list, "List Item #1", NULL, NULL, _list_cb, popup);
   elm_list_item_append(list, "List Item #2", NULL, NULL, _list_cb, popup);
   elm_list_item_append(list, "List Item #3", NULL, NULL, _list_cb, popup);
   elm_object_content_set(popup, list);

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

static void
_subpopup_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                            void *event_info EINA_UNUSED)
{
   Evas_Object *popup;
   Evas_Object *btn, *btnclose;

   popup = elm_popup_add(data);
   elm_object_style_set(popup, "subpopup");
   elm_object_part_text_set(popup, "title,text", "Title");

   // button as a popup content
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "content");
   elm_object_part_content_set(popup, "elm.swallow.content", btn);

   // popup buttons
   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   //popup-base close button
   btnclose = elm_button_add(popup);
   //TODO: write a X style button theme
   elm_object_text_set(btnclose, "x");
   elm_object_part_content_set(popup, "elm.swallow.closebtn", btnclose);
   evas_object_smart_callback_add(btnclose, "clicked", _popup_close_cb, popup);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
}

void
test_popup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *list;

   win = elm_win_util_standard_add("popup", "Popup");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   list = elm_list_add(win);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, list);
   elm_list_mode_set(list, ELM_LIST_LIMIT);
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
   elm_list_item_append(list,
                        "popup-center-title + text (block,clicked handling with parent)",
                        NULL, NULL,
                        _popup_center_title_text_block_clicked_event_with_parent_cb, win);
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
   elm_list_item_append(list, "popup-center-text + 3 button (check add, remove buttons)", NULL, NULL,
                        _popup_center_text_3button_add_remove_button_cb, win);
   elm_list_item_append(list, "popup-transparent", NULL, NULL,
                        _popup_transparent_cb, win);
   elm_list_item_append(list, "popup-transparent-align", NULL, NULL,
                        _popup_transparent_align_cb, win);
   elm_list_item_append(list, "popup-center-title + list content + 1 button",
                        NULL, NULL, _popup_center_title_list_content_1button_cb,
                        win);
   elm_list_item_append(list, "subpopup + X button",
                        NULL, NULL, _subpopup_cb,
                        win);
   elm_list_go(list);
   evas_object_show(list);

   evas_object_resize(win, 480, 400);
   evas_object_show(win);
}
