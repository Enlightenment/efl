#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static Elm_Genlist_Item_Class itci;
char *gli_label_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[256];
   int j = (int)data;
   snprintf(buf, sizeof(buf), "%c%c", 
            'A' + ((j >> 4) & 0xf),
            'a' + ((j     ) & 0xf)
            );
   return strdup(buf);
}

void
index_changed2(void *data, Evas_Object *obj, void *event_info)
{
   // called on a change but delayed in case multiple changes happen in a
   // short timespan
   elm_genlist_item_top_bring_in(event_info);
}

void
index_changed(void *data, Evas_Object *obj, void *event_info)
{
   // this is calld on every change, no matter how often
   // elm_genlist_item_bring_in(event_info);
}

void
index_selected(void *data, Evas_Object *obj, void *event_info)
{
   // called on final select
   elm_genlist_item_top_bring_in(event_info);
}

void
test_index(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *id;
   Elm_Genlist_Item *it;
   int i, j;

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "Index");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gl);
   evas_object_show(gl);
   
   id = elm_index_add(win);
   evas_object_size_hint_weight_set(id, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, id);
   
   evas_object_show(id);

   itci.item_style     = "default";
   itci.func.label_get = gli_label_get;
   itci.func.icon_get  = NULL;
   itci.func.state_get = NULL;
   itci.func.del       = NULL;

   j = 0;
   for (i = 0; i < 100; i++)
     {
        it = elm_genlist_item_append(gl, &itci,
                                     (void *)j/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, NULL/* func */,
                                     NULL/* func data */);
        if ((j & 0xf) == 0)
          {
             char buf[32];
             
             snprintf(buf, sizeof(buf), "%c", 'A' + ((j >> 4) & 0xf));
             elm_index_item_append(id, buf, it);
          }
        j += 2;
     }
   evas_object_smart_callback_add(id, "delay,changed", index_changed2, NULL);
   evas_object_smart_callback_add(id, "changed", index_changed, NULL);
   evas_object_smart_callback_add(id, "selected", index_selected, NULL);
   elm_index_item_go(id, 0);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
