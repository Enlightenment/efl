//Compile with:
//gcc -o hoversel_example_01 hoversel_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _print_items(void *data, Evas_Object *obj, void *event_info);
static void _rm_items(void *data, Evas_Object *obj, void *event_info);
static void _sel(void *data, Evas_Object *obj, void *event_info);
static void _free(void *data, Evas_Object *obj, void *event_info);
static void _add_item(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *rect, *hoversel;
   Elm_Object_Item *hoversel_it;

   win = elm_win_add(NULL, "hoversel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hoversel");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   evas_object_resize(win, 200, 300);
   evas_object_show(win);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_show(rect);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_hoversel_horizontal_set(hoversel, EINA_FALSE);
   elm_object_text_set(hoversel, "Hoversel");
   elm_object_part_content_set(hoversel, "icon", rect);
   elm_hoversel_item_add(hoversel, "Print items", NULL, ELM_ICON_NONE,
                         _print_items, NULL);
   elm_hoversel_item_add(hoversel, "Option 2", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   hoversel_it = elm_hoversel_item_add(hoversel, "Clear all items", "close",
                                       ELM_ICON_STANDARD, _rm_items, NULL);
   evas_object_smart_callback_add(hoversel, "selected", _sel, hoversel_it);
   evas_object_smart_callback_add(hoversel, "clicked", _add_item, NULL);

   evas_object_resize(hoversel, 180, 30);
   evas_object_move(hoversel, 10, 10);
   evas_object_show(hoversel);


   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()

static void
_print_items(void *data, Evas_Object *obj, void *event_info)
{
   const Eina_List *items = elm_hoversel_items_get(obj);
   const Eina_List *l;
   Elm_Object_Item *hoversel_it;

   EINA_LIST_FOREACH(items, l, hoversel_it)
     printf("%s\n", elm_object_item_text_get(hoversel_it));
}

static void
_rm_items(void *data, Evas_Object *obj, void *event_info)
{
   if (!elm_hoversel_expanded_get(obj))
     elm_hoversel_clear(obj);
}

static void
_sel(void *data, Evas_Object *obj, void *event_info)
{
   if (!elm_hoversel_expanded_get(obj) && event_info != data)
     elm_object_item_del(event_info);
}

static void
_add_item(void *data, Evas_Object *obj, void *event_info)
{
   static int num = 0;
   char *str = malloc(sizeof(char) * 10);
   Elm_Object_Item *hoversel_it;

   snprintf(str, 10, "item %d", ++num);

   hoversel_it = elm_hoversel_item_add(obj, str, NULL, ELM_ICON_NONE, NULL,
                                       str);
   elm_object_item_del_cb_set(hoversel_it, _free);
}

static void
_free(void *data, Evas_Object *obj, void *event_info)
{
   free(data);
}
