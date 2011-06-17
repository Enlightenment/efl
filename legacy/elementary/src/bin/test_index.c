#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static Elm_Genlist_Item_Class itci;
char *gli_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   int j = (long)data;
   snprintf(buf, sizeof(buf), "%c%c",
            'A' + ((j >> 4) & 0xf),
            'a' + ((j     ) & 0xf)
            );
   return strdup(buf);
}

void
index_changed2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   // called on a change but delayed in case multiple changes happen in a
   // short timespan
   elm_genlist_item_top_bring_in(event_info);
}

void
index_changed(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   // this is calld on every change, no matter how often
   // elm_genlist_item_bring_in(event_info);
}

void
index_selected(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   // called on final select
   elm_genlist_item_top_bring_in(event_info);
}

void
test_index(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
                                     (void *)(long)j/* item data */,
                                     NULL/* parent */, ELM_GENLIST_ITEM_NONE,
                                     NULL/* func */, NULL/* func data */);
        if (!(j & 0xf))
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

/***********/

typedef struct _Test_Index2_Elements
{
   Evas_Object *entry, *lst, *id;
} Test_Index2_Elements;

void
test_index2_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* FIXME it won't be called if elm_test main window is closed */
   free(data);
}

int
test_index2_cmp(const void *data1, const void *data2)
{
   const char *label1, *label2;
   const Elm_List_Item *it1 = data1;
   const Elm_List_Item *it2 = data2;

   label1 = elm_list_item_label_get(it1);
   label2 = elm_list_item_label_get(it2);

   return strcasecmp(label1, label2);
}

int
test_index2_icmp(const void *data1, const void *data2)
{
   const char *label1, *label2;
   const Elm_Index_Item *it1 = data1;
   const Elm_Index_Item *it2 = data2;

   label1 = elm_index_item_letter_get(it1);
   label2 = elm_index_item_letter_get(it2);

   return strcasecmp(label1, label2);
}

void
test_index2_it_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Test_Index2_Elements *gui = data;
   Elm_List_Item *it;
   const char *label;
   char letter[2];

   label = elm_entry_entry_get(gui->entry);
   snprintf(letter, sizeof(letter), "%c", label[0]);
   it = elm_list_item_sorted_insert(gui->lst, label, NULL, NULL, NULL, NULL,
	 test_index2_cmp);
   elm_index_item_sorted_insert(gui->id, letter, it, test_index2_icmp,
	 test_index2_cmp);
   elm_list_go(gui->lst);
   /* FIXME it's not showing the recently added item */
   elm_list_item_show(it);
}

void
test_index2_it_del(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Test_Index2_Elements *gui = data;
   const char *label, *label_next;
   Elm_List_Item *it, *it_next;

   it = elm_list_selected_item_get(obj);
   it_next = elm_list_item_next(it);

   if (!it_next)
     {
	elm_index_item_del(gui->id, it);
	elm_list_item_del(it);
	return;
     }

   label = elm_list_item_label_get(it);
   label_next = elm_list_item_label_get(it_next);

   if (label[0] == label_next[0])
     {
	Elm_Index_Item *iit;
	iit = elm_index_item_find(gui->id, it);
	elm_index_item_data_set(iit, it_next);
     }
   else
     elm_index_item_del(gui->id, it);

   elm_list_item_del(it);
}

void
test_index2_id_changed(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   elm_list_item_show(event_info);
}

void
test_index2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *box, *bt;
   Test_Index2_Elements *gui;

   gui = malloc(sizeof(*gui));

   win = elm_win_add(NULL, "sorted-index-list", ELM_WIN_BASIC);
   elm_win_title_set(win, "Sorted Index and List");
   evas_object_smart_callback_add(win, "delete-request", test_index2_del, gui);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   gui->id = elm_index_add(win);
   evas_object_size_hint_weight_set(gui->id, EVAS_HINT_EXPAND,
	 EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gui->id);
   evas_object_smart_callback_add(gui->id, "delay,changed",
	 test_index2_id_changed, NULL);
   evas_object_show(gui->id);

   gui->entry = elm_entry_add(win);
   elm_entry_scrollable_set(gui->entry, EINA_TRUE);
   elm_entry_entry_set(gui->entry, "Label");
   elm_entry_single_line_set(gui->entry, EINA_TRUE);
   evas_object_size_hint_weight_set(gui->entry, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(gui->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, gui->entry);
   evas_object_show(gui->entry);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Add");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", test_index2_it_add, gui);
   evas_object_show(bt);

   gui->lst = elm_list_add(win);
   elm_box_pack_end(box, gui->lst);
   evas_object_size_hint_weight_set(gui->lst, EVAS_HINT_EXPAND,
	 EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(gui->lst, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(gui->lst, "selected", test_index2_it_del,
	 gui);
   elm_list_go(gui->lst);
   evas_object_show(gui->lst);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
