#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_item_added_cb(void *data, const Efl_Event *ev)
{
   Eina_Array *array = data;
   const char *str = ev->info;

   printf("item added %s\n", str);
   eina_array_push(array, str);
}

static void
_item_deleted_cb(void *data, const Efl_Event *ev)
{
   Eina_Array_Iterator iterator;
   const char *item;
   unsigned int i;
   Eina_Array *array = data;
   const char *str = ev->info;

   printf("item deleted %s\n", str);
   eina_array_clean(array);

   const Eina_Array *tags_array = efl_ui_tags_items_get(ev->object);
   EINA_ARRAY_ITER_NEXT(tags_array, i, item, iterator)
     {
        eina_array_push(array, item);
        printf("item #%u: %s\n", i, item);
     }
}

static void
_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tags = data;

   if (efl_ui_tags_expanded_get(tags))
     efl_ui_tags_expanded_set(tags, EINA_FALSE);
   else
     efl_ui_tags_expanded_set(tags, EINA_TRUE);
}


void
test_ui_tags(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *layout, *tags;

   const char* strs[] = {
        "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
   };

   Eina_Array *array;
   unsigned int i;
   char buf[PATH_MAX];

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Tags"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   snprintf(buf, sizeof(buf), "%s/objects/multibuttonentry.edj", elm_app_data_dir_get());
   layout = efl_add(EFL_UI_LAYOUT_OBJECT_CLASS, win,
                    elm_layout_file_set(efl_added, buf, "multibuttonentry_test"),
                    elm_win_resize_object_add(win, efl_added));

   array = eina_array_new(10);
   for (i = 0; i < 9; i++)
       eina_array_push(array, strs[i]);

   tags = efl_add(EFL_UI_TAGS_CLASS, layout,
                  efl_ui_tags_items_set(efl_added, array),
                  efl_event_callback_add(efl_added, EFL_UI_TAGS_EVENT_ITEM_ADDED, _item_added_cb, array),
                  efl_event_callback_add(efl_added, EFL_UI_TAGS_EVENT_ITEM_DELETED, _item_deleted_cb, array),
                  efl_text_set(efl_added, "To :"),
                  efl_ui_format_string_set(efl_added, "+ %d items"),
                  elm_object_part_content_set(layout, "multibuttonentry", efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Change mode"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _clicked, tags),
           elm_object_part_content_set(layout, "box", efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}
