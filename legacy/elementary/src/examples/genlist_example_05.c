//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` genlist_example_04.c -o genlist_example_04

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define N_ITEMS 6

typedef struct _Node_Data {
     Eina_List *children;
     int value;
     int level;
     Eina_Bool favorite;
} Node_Data;

static Elm_Genlist_Item_Class _itc;
static Elm_Genlist_Item_Class _itp;
static Elm_Genlist_Item_Class _itfav;
static int nitems = 0;

static char *
_item_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   char buf[256];
   Node_Data *d = data;

   if (!strcmp(part, "elm.text"))
     snprintf(buf, sizeof(buf), "Item # %i (level %i)", d->value, d->level);

   return strdup(buf);
}

static Evas_Object *
_item_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "file");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static void
_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n",
	  data, obj, event_info);
}

static char *
_parent_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   Node_Data *d = data;

   snprintf(buf, sizeof(buf), "Group %d (%d items)", d->value / 7,
	    eina_list_count(d->children));

   return strdup(buf);
}

static Evas_Object *
_parent_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part __UNUSED__)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "folder");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static char *
_favorite_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   char buf[256];
   Node_Data *d = data;

   if (!strcmp(part, "elm.text"))
     snprintf(buf, sizeof(buf), "Favorite # %i", d->value);

   return strdup(buf);
}

static Evas_Object *
_favorite_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "apps");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static void
_append_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it, *parent = NULL;
   Node_Data *pdata, *d = malloc(sizeof(*d));

   d->children = NULL;
   d->value = nitems++;
   d->favorite = EINA_FALSE;

   it = elm_genlist_selected_item_get(list);
   if (it)
     parent = elm_genlist_item_parent_get(it);

   if (parent)
     {
	d->level = elm_genlist_item_expanded_depth_get(parent) + 1;
	pdata = elm_genlist_item_data_get(parent);
	pdata->children = eina_list_append(pdata->children, d);
     }
   else
     d->level = 0;

   elm_genlist_item_append(list, &_itc,
			   d, parent,
			   ELM_GENLIST_ITEM_NONE,
			   _item_sel_cb, NULL);

}

static void
_favorite_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);

   if (!it)
     return;

   Node_Data *d = elm_genlist_item_data_get(it);
   d->favorite = !d->favorite;
   if (d->favorite)
     elm_genlist_item_item_class_update(it, &_itfav);
   else
     {
	if (d->children)
	  elm_genlist_item_item_class_update(it, &_itp);
	else
	  elm_genlist_item_item_class_update(it, &_itc);
     }

   elm_genlist_item_update(it);
}

static void
_add_child_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);
   Elm_Genlist_Item *prev, *parent;

   if (!it)
     return;

   Node_Data *d = elm_genlist_item_data_get(it);
   prev = elm_genlist_item_prev_get(it);
   parent = elm_genlist_item_parent_get(it);

   Eina_Bool change_item = !d->children;

   // creating new item data
   Node_Data *ndata = malloc(sizeof(*ndata));
   ndata->value = nitems++;
   ndata->children = NULL;
   ndata->favorite = EINA_FALSE;
   ndata->level = elm_genlist_item_expanded_depth_get(it) + 1;
   d->children = eina_list_append(d->children, ndata);

   // Changing leaf item to parent item
   if (change_item)
     {
        elm_genlist_item_del(it);

	if (prev != parent)
	  it = elm_genlist_item_insert_after(list, &_itp, d, parent, prev,
					     ELM_GENLIST_ITEM_SUBITEMS,
					     _item_sel_cb, NULL);
	else
	  it = elm_genlist_item_prepend(list, &_itp, d, parent,
					ELM_GENLIST_ITEM_SUBITEMS,
					_item_sel_cb, NULL);
	elm_genlist_item_expanded_set(it, EINA_FALSE);
	elm_genlist_item_selected_set(it, EINA_TRUE);
     }
   else if (elm_genlist_item_expanded_get(it))
     {
	elm_genlist_item_append(list, &_itc, ndata, it,
				ELM_GENLIST_ITEM_NONE, _item_sel_cb, NULL);
     }

   elm_genlist_item_update(it);

}

static void
_clear_list(Node_Data *d)
{
   Node_Data *tmp;

   EINA_LIST_FREE(d->children, tmp)
      _clear_list(tmp);
   free(d);
}

static void
_del_item_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);
   Elm_Genlist_Item *parent = NULL;

   if (!it)
     return;

   Node_Data *pdata, *d = elm_genlist_item_data_get(it);
   parent = elm_genlist_item_parent_get(it);
   elm_genlist_item_subitems_clear(it);
   elm_genlist_item_del(it);

   _clear_list(d);

   if (!parent)
     return;

   pdata = elm_genlist_item_data_get(parent);
   pdata->children = eina_list_remove(pdata->children, d);
   elm_genlist_item_update(parent);
}

static void
_expand_request_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   printf("expand request on item: %p\n", event_info);
   elm_genlist_item_expanded_set(it, EINA_TRUE);
}

static void
_contract_request_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   printf("contract request on item: %p\n", event_info);
   elm_genlist_item_expanded_set(it, EINA_FALSE);
}

static void
_expanded_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Eina_List *l;
   Elm_Genlist_Item *it = event_info;
   Node_Data *it_data, *d = elm_genlist_item_data_get(it);
   Evas_Object *list = elm_genlist_item_genlist_get(it);

   Elm_Genlist_Item_Class *ic;

   EINA_LIST_FOREACH(d->children, l, it_data)
     {
	Elm_Genlist_Item *nitem;
	Elm_Genlist_Item_Flags flags = ELM_GENLIST_ITEM_NONE;
	printf("expanding item: #%d from parent #%d\n", it_data->value, d->value);
	if (it_data->favorite)
	  ic = &_itfav;
	else if (it_data->children)
	  {
	     ic = &_itp;
	     flags = ELM_GENLIST_ITEM_SUBITEMS;
	  }
	else
	  ic = &_itc;

	nitem = elm_genlist_item_append(list, ic, it_data, it,
					flags, _item_sel_cb, NULL);
	elm_genlist_item_expanded_set(nitem, EINA_FALSE);
     }
}

static void
_contracted_cb(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static Evas_Object *
_button_add(Evas_Object *list, Evas_Object *box, const char *label, Evas_Smart_Cb cb)
{
   Evas_Object *bt;

   bt = elm_button_add(elm_object_parent_widget_get(list));
   elm_object_text_set(bt, label);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   if (cb)
     evas_object_smart_callback_add(bt, "clicked", cb, list);

   return bt;
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *box, *fbox;
   Evas_Object *list;
   int i;

   win = elm_win_add(NULL, "icon", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255,255 ,255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   _itc.item_style = "default";
   _itc.func.label_get = _item_label_get;
   _itc.func.content_get = _item_content_get;
   _itc.func.state_get = NULL;
   _itc.func.del = NULL;

   _itp.item_style = "default";
   _itp.func.label_get = _parent_label_get;
   _itp.func.content_get = _parent_content_get;
   _itp.func.state_get = NULL;
   _itp.func.del = NULL;

   _itfav.item_style = "default";
   _itfav.func.label_get = _favorite_label_get;
   _itfav.func.content_get = _favorite_content_get;
   _itfav.func.state_get = NULL;
   _itfav.func.del = NULL;

   list = elm_genlist_add(win);

   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, list);
   evas_object_show(list);

   fbox = elm_box_add(win);
   elm_box_layout_set(fbox, evas_object_box_layout_flow_horizontal,
		      NULL, NULL);
   evas_object_size_hint_weight_set(fbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(fbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, fbox);
   evas_object_show(fbox);

   _button_add(list, fbox, "append item", _append_cb);
   _button_add(list, fbox, "favorite", _favorite_cb);
   _button_add(list, fbox, "add child", _add_child_cb);
   _button_add(list, fbox, "del item", _del_item_cb);

   for (i = 0; i < N_ITEMS; i++)
     {
	Elm_Genlist_Item *gli, *glg;
	Node_Data *data = malloc(sizeof(*data)); // data for this item
	data->children = NULL;
	data->value = i;
	data->favorite = EINA_FALSE;
	nitems++;

	Node_Data *pdata; // data for the parent of the group

	printf("creating item: #%d\n", data->value);
	if (i % 3 == 0)
	  {
	     glg = gli = elm_genlist_item_append(list, &_itp, data, NULL,
						 ELM_GENLIST_ITEM_SUBITEMS,
						 _item_sel_cb, NULL);
	     elm_genlist_item_expanded_set(glg, EINA_TRUE);
	     pdata = data;
	     data->level = 0;
	  }
	else
	  {
	     gli = elm_genlist_item_append(list, &_itc, data, glg,
					   ELM_GENLIST_ITEM_NONE,
					   _item_sel_cb, NULL);
	     pdata->children = eina_list_append(pdata->children, data);
	     data->level = 1;
	  }
     }

   evas_object_smart_callback_add(list, "expand,request", _expand_request_cb, list);
   evas_object_smart_callback_add(list, "contract,request", _contract_request_cb, list);
   evas_object_smart_callback_add(list, "expanded", _expanded_cb, list);
   evas_object_smart_callback_add(list, "contracted", _contracted_cb, list);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 800);
   evas_object_resize(win, 420, 600);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
