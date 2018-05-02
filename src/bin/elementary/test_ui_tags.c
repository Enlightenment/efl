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

void
test_ui_tags(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx;

   const char* strs[] = {
        "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
   };

   Eina_Array *array;
   unsigned int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Tags"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   array = eina_array_new(10);
   for (i = 0; i < 9; i++)
       eina_array_push(array, strs[i]);

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));

   efl_add(EFL_UI_TAGS_CLASS, bx,
           efl_ui_tags_items_set(efl_added, array),
           efl_event_callback_add(efl_added, EFL_UI_TAGS_EVENT_ITEM_ADDED, _item_added_cb, array),
           efl_event_callback_add(efl_added, EFL_UI_TAGS_EVENT_ITEM_DELETED, _item_deleted_cb, array),
           efl_text_set(efl_added, "To :"),
           efl_pack(bx, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}
