#include <Elementary.h>

static Elm_Genlist_Item_Class itci;
char *gli_label_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)data);
   return strdup(buf);
}
void gli_del(const void *data, Evas_Object *obj)
{
}

void
test_index(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *id;
   Elm_Genlist_Item *it;
   int i;

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "Index");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   elm_win_resize_object_add(win, gl);
   evas_object_show(gl);
   
   id = elm_index_add(win);
   evas_object_size_hint_weight_set(id, 1.0, 1.0);
   elm_win_resize_object_add(win, id);
   evas_object_show(id);

   itci.item_style     = "default";
   itci.func.label_get = gli_label_get;
   itci.func.icon_get  = NULL;
   itci.func.state_get = NULL;
   itci.func.del       = gli_del;

   for (i = 0; i < 100; i++)
     {
        // FIXME: add to index too
        it = elm_genlist_item_append(gl, &itci,
                                     i/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, NULL/* func */,
                                     NULL/* func data */);
        if ((i % 10) == 0)
          {
             char buf[32];
             
             snprintf(buf, sizeof(buf), "%i", i / 10);
             elm_index_item_append(id, buf, it);
          }
     }

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
