//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` genlist_example_03.c -o genlist_example_03

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define N_ITEMS 30

static Elm_Genlist_Item_Class _itc;

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
	snprintf(buf, sizeof(buf), "created at %s", ctime(&t));
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

static Evas_Object *
_genlist_add(Evas_Object *box)
{
   Evas_Object *list = elm_genlist_add(elm_object_parent_widget_get(box));
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(list, 200, 160);
   elm_box_pack_end(box, list);
   evas_object_show(list);

   return list;
}

static void
_genlist_fill(Evas_Object *list)
{
   int i;

   for (i = 0; i < N_ITEMS; i++)
     {
	elm_genlist_item_append(list, &_itc,
				(void *)(long)i, NULL,
				ELM_GENLIST_ITEM_NONE,
				_item_sel_cb, NULL);
     }
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *box;
   Evas_Object *list;

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
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   _itc.item_style = "double_label";
   _itc.func.label_get = _item_label_get;
   _itc.func.content_get = _item_content_get;
   _itc.func.state_get = NULL;
   _itc.func.del = NULL;

   list = _genlist_add(box);
   _genlist_fill(list);

   list = _genlist_add(box);
   elm_genlist_horizontal_mode_set(list, ELM_LIST_LIMIT);
   _genlist_fill(list);

   list = _genlist_add(box);
   elm_genlist_compress_mode_set(list, EINA_TRUE);
   _genlist_fill(list);

   list = _genlist_add(box);
   elm_genlist_height_for_width_mode_set(list, EINA_TRUE);
   _genlist_fill(list);

   evas_object_size_hint_min_set(bg, 800, 160);
   evas_object_size_hint_max_set(bg, 800, 640);
   evas_object_resize(win, 800, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
