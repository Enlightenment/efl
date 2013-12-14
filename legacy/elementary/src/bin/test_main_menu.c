#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
static void
_click_me(void *data, Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = data;
   Eina_Bool disabled = elm_object_item_disabled_get(it);
   printf("The first item is now %s\n", disabled ? "enabled" : "disabled");
   elm_object_item_disabled_set(it, !disabled);
}

void
test_main_menu(void *data EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *win, *menu, *label, *bx;
   Elm_Object_Item *menu_it, *menu_it1;
   char *s;
   Eina_Bool enabled = EINA_TRUE;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("menu", "Menu");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   label = elm_label_add(win);
   elm_object_text_set(label, "Note: the D-Bus menu example requires support from the "
                              "desktop environment to display the application menu");
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, label);
   evas_object_show(label);

   s = getenv("ELM_DISABLE_EXTERNAL_MENU");
   if (s)
     enabled = !atoi(s);

   if (!enabled)
     {
        label = elm_label_add(win);
        elm_object_text_set(label, "(ELM_DISABLE_EXTERNAL_MENU environment "
                            "variable is set. Using local menu instead)");
        evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        elm_box_pack_end(bx, label);
        evas_object_show(label);
     }

   menu = elm_win_main_menu_get(win);

   menu_it = elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "elementary", "first item", NULL, NULL);
   menu_it1 = elm_menu_item_add(menu, menu_it, NULL, "submenu", NULL, NULL);
   elm_menu_item_add(menu, menu_it1, NULL, "first item", NULL, NULL);
   elm_menu_item_add(menu, menu_it1, "gimp", "second item", NULL, NULL);

   menu_it = elm_menu_item_add(menu, NULL, NULL, "second item", NULL, NULL);
   menu_it1 = elm_menu_item_add(menu, menu_it, NULL, "disabled item", NULL, NULL);
   elm_object_item_disabled_set(menu_it1, EINA_TRUE);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_add(menu, menu_it, NULL, "click me :-)", _click_me, menu_it1);
   elm_menu_item_add(menu, menu_it, "applications-email-panel", "third item", NULL, NULL);
   menu_it1 = elm_menu_item_add(menu, menu_it, NULL, "sub menu", NULL, NULL);
   elm_menu_item_add(menu, menu_it1, NULL, "first item", NULL, NULL);

   evas_object_resize(win, 250, 350);
   evas_object_show(win);
}
