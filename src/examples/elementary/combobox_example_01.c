//Compile with:
//gcc -o combobox_example_01 combobox_example_01.c -g `pkg-config --cflags --libs elementary`
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <string.h>

#include <Elementary.h>

static void
_combobox_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   printf("Hover button is clicked and 'clicked' callback is called.\n");
}

static void
_combobox_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'selected' callback is called. (selected item : %s)\n", txt);
}

static void
_combobox_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("'dismissed' callback is called.\n");
}

static void
_combobox_expanded_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("'expanded' callback is called.\n");
}

static void
_combobox_item_pressed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'item,pressed' callback is called. (selected item : %s)\n", txt);
   elm_object_text_set(obj, txt);
   elm_combobox_hover_end(obj);
}

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

static Eina_Bool gl_state_get(void *data EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED,
                               const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
gl_filter_get(void *data, Evas_Object *obj EINA_UNUSED, void *key)
{
   // if the key is empty/NULL, return true for item
   if (!strlen((char *)key)) return EINA_TRUE;
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   if (strcasestr(buf, (char *)key))
     return EINA_TRUE;
   // Default case should return false (item fails filter hence will be hidden)
   return EINA_FALSE;
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *bg;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("combobox", "Combobox");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   Evas_Object *bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   Evas_Object *combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(combobox, "guide", "A Simple list");
   elm_box_pack_end(bx, combobox);
   evas_object_show(combobox);

   Elm_Genlist_Item_Class *itc;
   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = gl_text_get;
   itc->func.content_get = NULL;
   itc->func.state_get = gl_state_get;
   itc->func.filter_get = gl_filter_get;
   itc->func.del = NULL;

   for (int i = 0; i < 1000; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                             NULL, ELM_GENLIST_ITEM_NONE, NULL,
                             (void*)(uintptr_t)(i * 10));
   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "selected",
                                  _combobox_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);

   evas_object_resize(win, 300, 500);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
