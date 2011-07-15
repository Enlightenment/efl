//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` hoversel_example_01.c -o hoversel_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void _print_items(void *data, Evas_Object *obj, void *event_info);
static void _rm_items(void *data, Evas_Object *obj, void *event_info);
static void _sel(void *data, Evas_Object *obj, void *event_info);
static void _free(void *data, Evas_Object *obj, void *event_info);
static void _add_item(void *data, Evas_Object *obj, void *event_info);

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *rect, *hoversel;
   Elm_Hoversel_Item *it;

   win = elm_win_add(NULL, "hoversel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hoversel");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_show(rect);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_hoversel_horizontal_set(hoversel, EINA_FALSE);
   elm_object_text_set(hoversel, "Hoversel");
   elm_hoversel_icon_set(hoversel, rect);
   elm_hoversel_item_add(hoversel, "Print items", NULL, ELM_ICON_NONE,
                         _print_items, NULL);
   elm_hoversel_item_add(hoversel, "Option 2", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   it = elm_hoversel_item_add(hoversel, "Clear all items", "close",
                         ELM_ICON_STANDARD, _rm_items, NULL);
   evas_object_smart_callback_add(hoversel, "selected", _sel, it);
   evas_object_smart_callback_add(hoversel, "clicked", _add_item, NULL);

   evas_object_resize(hoversel, 180, 30);
   evas_object_move(hoversel, 10, 10);
   evas_object_show(hoversel);

   evas_object_resize(win, 200, 300);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_print_items(void *data, Evas_Object *obj, void *event_info)
{
   const Eina_List *items = elm_hoversel_items_get(obj);
   const Eina_List *l;
   Elm_Hoversel_Item *it;

   EINA_LIST_FOREACH(items, l, it)
     printf("%s\n", elm_hoversel_item_label_get(it));
}

static void
_rm_items(void *data, Evas_Object *obj, void *event_info)
{
   if(!elm_hoversel_expanded_get(obj))
      elm_hoversel_clear(obj);
}

static void
_sel(void *data, Evas_Object *obj, void *event_info)
{
   if(!elm_hoversel_expanded_get(obj) && event_info != data)
      elm_hoversel_item_del(event_info);
}

static void
_add_item(void *data, Evas_Object *obj, void *event_info)
{
   static int num = 0;
   char *str = malloc(sizeof(char) * 10);
   Elm_Hoversel_Item *it;

   snprintf(str, 10, "item %d", ++num);

   it = elm_hoversel_item_add(obj, str, NULL, ELM_ICON_NONE, NULL, str);
   elm_hoversel_item_del_cb_set(it, _free);
}

static void
_free(void *data, Evas_Object *obj, void *event_info)
{
   free(data);
}
