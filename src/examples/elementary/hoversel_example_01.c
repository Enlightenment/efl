//Compile with:
//gcc -o hoversel_example_01 hoversel_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _print_items(void *data, Evas_Object *obj, void *event_info);
static void _add_item(void *data, Evas_Object *obj, void *event_info);

static void
_clear_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   elm_hoversel_clear((Evas_Object *)data);
}

static void
_reverse_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   elm_object_mirrored_set(data, !elm_object_mirrored_get (data));
   printf("\nMirrored mode is now %s", elm_object_mirrored_get(data) ? "ON" : "OFF" );
   fflush(stdout);
}

Eina_Bool is_eng = EINA_TRUE;
static void
_arabic_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   is_eng = !is_eng;
   elm_object_text_set(data, is_eng ?
                       "Enable Arabic for new items" :
                       "Enable English for new items");
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *rect, *hoversel, *btn = NULL;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("hoversel", "Hoversel");
   elm_win_autodel_set(win, EINA_TRUE);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_show(rect);

   hoversel = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_hoversel_horizontal_set(hoversel, EINA_FALSE);
   elm_object_text_set(hoversel, "Add an item to Hoversel");
   elm_object_part_content_set(hoversel, "icon", rect);
   elm_hoversel_item_add(hoversel, "Print items", NULL, ELM_ICON_NONE,
                         _print_items, NULL);
   elm_hoversel_item_add(hoversel, "Option 2", "home", ELM_ICON_STANDARD, NULL,
                         NULL);
   evas_object_smart_callback_add(hoversel, "clicked", _add_item, NULL);

   evas_object_resize(hoversel, 180, 30);
   evas_object_move(hoversel, 10, 10);
   evas_object_show(hoversel);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Clear all Items");
   evas_object_smart_callback_add(btn, "clicked", _clear_btn_clicked_cb, hoversel);
   evas_object_resize(btn, 180, 30);
   evas_object_move(btn, 10, 50);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Change Mirror Mode");
   evas_object_smart_callback_add(btn, "clicked", _reverse_btn_clicked_cb, hoversel);
   evas_object_resize(btn, 180, 30);
   evas_object_move(btn, 10, 90);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Enable Arabic for new items");
   evas_object_smart_callback_add(btn, "clicked", _arabic_btn_clicked_cb, btn);
   evas_object_resize(btn, 180, 30);
   evas_object_move(btn, 10, 130);
   evas_object_show(btn);

   evas_object_resize(win, 200, 300);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_print_items(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   const Eina_List *items = elm_hoversel_items_get(obj);
   const Eina_List *l;
   Elm_Object_Item *hoversel_it;

   EINA_LIST_FOREACH(items, l, hoversel_it)
     printf("%s\n", elm_object_item_text_get(hoversel_it));
}

static void
_add_item(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   static int num = 0;
   Eina_Slstr *str;

   if(is_eng)
     str = eina_slstr_printf("item %d", ++num);
   else
     str = eina_slstr_printf("بند %d", ++num);

   elm_hoversel_item_add(obj, str, NULL, ELM_ICON_NONE, NULL, str);
}
