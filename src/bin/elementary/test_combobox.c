#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef _WIN32
# include <evil_private.h> /* strcasestr */
#endif

#include <Elementary.h>

static void
_combobox_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   printf("Hover button is clicked and 'clicked' callback is called.\n");
}

static void
_combobox_item_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'item,selected' callback is called. (selected item : %s)\n", txt);
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
   elm_entry_cursor_end_set(obj);
}

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

static Evas_Object *gl_content_get(void *data EINA_UNUSED, Evas_Object *obj,
                                    const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.end"))
     snprintf(buf, sizeof(buf), "%s/images/bubble.png", elm_app_data_dir_get());
   else
     snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
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
   char buf[256];
   // if the key is empty/NULL, return true for item
   if (!strlen((char *)key)) return EINA_TRUE;
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   if (strcasestr(buf, (char *)key))
     return EINA_TRUE;
   // Default case should return false (item fails filter hence will be hidden)
   return EINA_FALSE;
}

static void
_gl_filter_restart_cb(void *data EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   elm_genlist_filter_set(obj, (void *)elm_object_text_get(obj));
}

static void
_gl_filter_finished_cb(void *data EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("Filter finished\n");
}

void
test_combobox(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *combobox, *fr;
   Elm_Genlist_Item_Class *itc;
   win = elm_win_util_standard_add("combobox", "Combobox");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = gl_text_get;
   itc->func.content_get = gl_content_get;
   itc->func.state_get = gl_state_get;
   itc->func.filter_get = gl_filter_get;
   itc->func.del = NULL;

   fr = elm_frame_add(win);
   elm_object_style_set(fr, "pad_huge");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(combobox, "guide", "Short List");
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   for (int i = 0; i < 5; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   elm_object_content_set(fr, combobox);
   evas_object_show(combobox);

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(combobox, "guide", "A long list");
   for (int i = 0; i < 1000; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,selected",
                                  _combobox_item_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);
   evas_object_smart_callback_add(combobox, "filter,done",
                                  _gl_filter_finished_cb, NULL);
   evas_object_smart_callback_add(combobox, "changed",
                                  _gl_filter_restart_cb, NULL);
   elm_box_pack_end(bx, combobox);
   evas_object_show(combobox);

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_text_set(combobox, "Disabled Combobox");
   for (int i = 0; i < 1000; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,selected",
                                  _combobox_item_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "filter,done",
                                  _gl_filter_finished_cb, NULL);
   elm_object_disabled_set(combobox, EINA_TRUE);
   elm_box_pack_end(bx, combobox);
   evas_object_show(combobox);

   fr = elm_frame_add(win);
   elm_object_style_set(fr, "pad_huge");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(combobox, "guide", "Short List");
   for (int i = 0; i < 5; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   elm_object_content_set(fr, combobox);
   evas_object_show(combobox);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           500 * elm_config_scale_get());
   evas_object_show(win);

   elm_genlist_item_class_free(itc);
}
