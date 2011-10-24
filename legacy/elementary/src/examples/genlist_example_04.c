//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` genlist_example_04.c -o genlist_example_04

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define N_ITEMS 300

static Elm_Genlist_Item_Class _itc;
static Elm_Genlist_Item_Class _itc_group;
static int nitems = 0;

static char *
_item_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   time_t t = (time_t)ecore_time_unix_get();
   char buf[256];
   int i = (int)(long)data;

   if (!strcmp(part, "elm.text"))
     snprintf(buf, sizeof(buf), "Item # %i", i);
   else
     {
	int n;
	snprintf(buf, sizeof(buf), "realized at %s", ctime(&t));
	n = strlen(buf);
	buf[n - 1] = '\0';
     }

   return strdup(buf);
}

static Evas_Object *
_item_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "clock");

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
_group_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   int i = (int)(long)data;

   snprintf(buf, sizeof(buf), "Group %d (item #%d)", i / 7, i);

   return strdup(buf);
}

static Evas_Object *
_group_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part __UNUSED__)
{
   Evas_Object *ic = elm_icon_add(obj);

   if (!strcmp(part, "elm.swallow.icon"))
     elm_icon_standard_set(ic, "home");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static void
_append_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;

   elm_genlist_item_append(list, &_itc,
			   (void *)(long)nitems++, NULL,
			   ELM_GENLIST_ITEM_NONE,
			   _item_sel_cb, NULL);

}

static void
_prepend_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;

   elm_genlist_item_prepend(list, &_itc,
			   (void *)(long)nitems++, NULL,
			   ELM_GENLIST_ITEM_NONE,
			   _item_sel_cb, NULL);

}

static void
_insert_before_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);

   if (!it)
     return;

   elm_genlist_item_insert_before(list, &_itc,
				  (void *)(long)nitems++, NULL,
				  it, ELM_GENLIST_ITEM_NONE,
				  _item_sel_cb, NULL);

}

static void
_insert_after_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);

   if (!it)
     return;

   elm_genlist_item_insert_after(list, &_itc,
				 (void *)(long)nitems++, NULL,
				 it, ELM_GENLIST_ITEM_NONE,
				 _item_sel_cb, NULL);

}

static void
_next_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it;

   it = elm_genlist_selected_item_get(list);
   if (it)
     it = elm_genlist_item_next_get(it);

   if (!it)
     it = elm_genlist_first_item_get(list);

   elm_genlist_item_selected_set(it, EINA_TRUE);
   elm_genlist_item_show(it);
}

static void
_prev_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it;

   it = elm_genlist_selected_item_get(list);
   if (it)
     it = elm_genlist_item_prev_get(it);

   if (!it)
     it = elm_genlist_last_item_get(list);

   elm_genlist_item_selected_set(it, EINA_TRUE);
   elm_genlist_item_show(it);
}

static void
_bring_in_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_bring_in(it);
}

static void
_show_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_show(it);
}

static void
_middle_in_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_middle_bring_in(it);
}

static void
_middle_show_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_middle_show(it);
}

static void
_top_in_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_top_bring_in(it);
}

static void
_top_show_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Genlist_Item *it = data;

   if (!it)
     return;

   elm_genlist_item_top_show(it);
}

static void
_realize_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);

   if (!it)
     return;

   elm_genlist_item_update(it);
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
   Evas_Object *bt_bring_in, *bt_show;
   Evas_Object *bt_middle_in, *bt_middle_show;
   Evas_Object *bt_top_in, *bt_top_show;
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

   _itc.item_style = "double_label";
   _itc.func.label_get = _item_label_get;
   _itc.func.content_get = _item_icon_get;
   _itc.func.state_get = NULL;
   _itc.func.del = NULL;

   _itc_group.item_style = "group_index";
   _itc_group.func.label_get = _group_label_get;
   _itc_group.func.content_get = _group_icon_get;
   _itc_group.func.state_get = NULL;
   _itc_group.func.del = NULL;

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

   _button_add(list, fbox, "prepend", _prepend_cb);
   _button_add(list, fbox, "append", _append_cb);
   _button_add(list, fbox, "insert before", _insert_before_cb);
   _button_add(list, fbox, "insert after", _insert_after_cb);
   _button_add(list, fbox, "prev", _prev_cb);
   _button_add(list, fbox, "next", _next_cb);
   _button_add(list, fbox, "realize", _realize_cb);
   bt_bring_in = _button_add(list, fbox, "bring #50", NULL);
   bt_show = _button_add(list, fbox, "show #50", NULL);
   bt_middle_in = _button_add(list, fbox, "bring to middle #200", NULL);
   bt_middle_show = _button_add(list, fbox, "show in middle #200", NULL);
   bt_top_in = _button_add(list, fbox, "bring to top #250", NULL);
   bt_top_show = _button_add(list, fbox, "show in top #250", NULL);

   for (i = 0; i < N_ITEMS; i++)
     {
	Elm_Genlist_Item *gli, *glg;

	if (i % 7 == 0)
	  {
	     glg = gli = elm_genlist_item_append(list, &_itc_group,
						 (void *)(long)nitems++, NULL,
						 ELM_GENLIST_ITEM_GROUP,
						 _item_sel_cb, NULL);
	     elm_genlist_item_display_only_set(glg, EINA_TRUE);
	  }
	else
	  {
	     gli = elm_genlist_item_append(list, &_itc,
					   (void *)(long)nitems++, glg,
					   ELM_GENLIST_ITEM_NONE,
					   _item_sel_cb, NULL);
	  }

	switch (i)
	  {
	   case 3:
	      elm_genlist_item_disabled_set(gli, EINA_TRUE);
	      break;
	   case 50:
	      evas_object_smart_callback_add(
		 bt_bring_in, "clicked", _bring_in_cb, gli);
	      evas_object_smart_callback_add(
		 bt_show, "clicked", _show_cb, gli);
	      break;
	   case 200:
	      evas_object_smart_callback_add(
		 bt_middle_in, "clicked", _middle_in_cb, gli);
	      evas_object_smart_callback_add(
		 bt_middle_show, "clicked", _middle_show_cb, gli);
	      break;
	   case 250:
	      evas_object_smart_callback_add(
		 bt_top_in, "clicked", _top_in_cb, gli);
	      evas_object_smart_callback_add(
		 bt_top_show, "clicked", _top_show_cb, gli);
	  }
     }

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 420, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
