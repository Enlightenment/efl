#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_menu_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   printf("menu dismissed callback is called!\n");
}

static void
_menu_show_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

static void
_populate_4(Evas_Object *menu, Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2;

   elm_menu_item_add(menu, menu_it, "go-bottom", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "go-first", "menu 3", NULL, NULL);
   elm_menu_item_separator_add(menu, menu_it);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-last", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-next", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
   menu_it2 = elm_menu_item_add(menu, menu_it, "go-up", "Disabled item", NULL,
                                NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
}

static void
_populate_3(Evas_Object *menu, Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2;

   elm_menu_item_add(menu, menu_it, "media-eject", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "media-playback-start", "menu 3", NULL,
                     NULL);
   elm_menu_item_separator_add(menu, menu_it);
   menu_it2 = elm_menu_item_add(menu, menu_it, "media-playback-stop",
                                "Disabled item", NULL, NULL);
   elm_object_item_disabled_set(menu_it2, EINA_TRUE);
}

static void
_populate_2(Evas_Object *menu, Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2, *menu_it3;

   elm_menu_item_add(menu, menu_it, "system-reboot", "menu 2", NULL, NULL);
   menu_it2 = elm_menu_item_add(menu, menu_it, "system-shutdown", "menu 3",
                                NULL, NULL);
   _populate_3(menu, menu_it2);

   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_separator_add(menu, menu_it);

   menu_it2 = elm_menu_item_add(menu, menu_it, "system-lock-screen", "menu 2",
                                NULL, NULL);
   elm_menu_item_separator_add(menu, menu_it);

   menu_it3 = elm_menu_item_add(menu, menu_it, "system-run", "Disabled item",
                                NULL, NULL);
   elm_object_item_disabled_set(menu_it3, EINA_TRUE);

   _populate_4(menu, menu_it2);
}

static void
_populate_1(Evas_Object *menu, Elm_Object_Item *menu_it)
{
   Elm_Object_Item *menu_it2, *menu_it3;
   Evas_Object *radio;

   radio = elm_radio_add(menu);
   elm_radio_state_value_set(radio, 0);
   elm_radio_value_set(radio, 0);
   elm_object_text_set(radio, "radio in menu");
   menu_it2 = elm_menu_item_add(menu, menu_it, "object-rotate-left", "menu 1",
                                NULL, NULL);
   menu_it3 = elm_menu_item_add(menu, menu_it, NULL, NULL, NULL, NULL);
   elm_object_item_content_set(menu_it3, radio);

   _populate_2(menu, menu_it2);
}

void
test_menu(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Evas_Object *win, *rect, *lbl, *menu;
   Elm_Object_Item *menu_it;

   win = elm_win_util_standard_add("menu", "Menu");
   elm_win_autodel_set(win, EINA_TRUE);

   lbl = elm_label_add(win);
   elm_object_text_set(lbl, "Click background to populate menu!");
   evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, lbl);
   evas_object_show(lbl);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, rect);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   evas_object_smart_callback_add(menu, "dismissed", _menu_dismissed_cb, NULL);

   elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);

   menu_it = elm_menu_item_add(menu, NULL, "mail-reply-all", "second item",
                               NULL, NULL);
   _populate_1(menu, menu_it);

   elm_menu_item_add(menu, menu_it, "window-new", "sub menu", NULL, NULL);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN,
                                  _menu_show_cb, menu);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

static void
_parent_set_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *mn = data;
   if (!mn) return;

   Evas_Object *parent = evas_object_data_get(mn, "parent_1");
   if (elm_menu_parent_get(mn) == parent)
     {
        parent = evas_object_data_get(mn, "parent_2");
     }

   elm_menu_parent_set(mn, parent);
}

static void
_icon_set_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Elm_Object_Item *menu_it = data;
   const char *icon_name = NULL;
   if (!menu_it) return;

   icon_name = elm_menu_item_icon_name_get(menu_it);
   if ((icon_name) && !strcmp(icon_name, "home"))
     {
        elm_menu_item_icon_name_set(menu_it, "file");
        return;
     }
   elm_menu_item_icon_name_set(menu_it, "home");
}

static void
_item_select_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Elm_Object_Item *menu_it = data;
   if (!menu_it) return;

   elm_menu_item_selected_set(menu_it, !elm_menu_item_selected_get(menu_it));
}

static void
_separators_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   const Eina_List *sis = NULL;
   const Eina_List *l = NULL;
   Elm_Object_Item *si;
   int separators = 0;

   Elm_Object_Item *menu_it = data;
   if (!menu_it) return;

   sis = elm_menu_item_subitems_get(menu_it);

   EINA_LIST_FOREACH(sis, l, si)
     if (elm_menu_item_is_separator(si)) separators++;

   printf("The number of separators: %d\n", separators);
}

static void
_open_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *mn = data;
   if (!mn) return;

   evas_object_show(mn);
}

static void
_close_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *mn = data;
   if (!mn) return;

   elm_menu_close(mn);
}

void
test_menu2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o_bg, *rect, *rect2, *mn, *bt, *vbx;
   Elm_Object_Item *menu_it, *menu_it2;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("menu2", "Menu 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o_bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/twofish.jpg", elm_app_data_dir_get());
   elm_bg_file_set(o_bg, buf, NULL);
   evas_object_size_hint_weight_set(o_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, o_bg);
   evas_object_show(o_bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_move(rect, 0, 0);
   evas_object_resize(rect, 124, 320);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   rect2 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_move(rect2, 124, 0);
   evas_object_resize(rect2, 124, 320);
   evas_object_color_set(rect2, 0, 0, 0, 0);
   evas_object_show(rect2);

   mn = elm_menu_add(win);
   elm_menu_item_add(mn, NULL, NULL, "first item", NULL, NULL);
   menu_it = elm_menu_item_add(mn, NULL, NULL, "second item", NULL, NULL);
   elm_menu_item_add(mn, menu_it, NULL, "item 1", NULL, NULL);
   elm_menu_item_separator_add(mn, menu_it);
   elm_menu_item_add(mn, menu_it, NULL, "item 2", NULL, NULL);
   menu_it2 = elm_menu_item_add(mn, NULL, NULL, "third item", NULL, NULL);
   evas_object_data_set(mn, "parent_1", rect);
   evas_object_data_set(mn, "parent_2", rect2);

   vbx = elm_box_add(win);
   evas_object_show(vbx);
   elm_box_pack_end(bx, vbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Menu Open");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _open_bt_clicked, mn);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon Set");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _icon_set_bt_clicked, menu_it);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Parent Set");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _parent_set_bt_clicked, mn);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Item Select");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _item_select_bt_clicked, menu_it2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Separators");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _separators_bt_clicked, menu_it);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Menu Close");
   elm_box_pack_end(vbx, bt);
   evas_object_smart_callback_add(bt, "clicked", _close_bt_clicked, mn);
   evas_object_show(bt);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
