//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` genlist_example_01.c -o genlist_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define N_ITEMS 30

static Elm_Genlist_Item_Class _itc;

static char *
_item_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(long)data);
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

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg;
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

   _itc.item_style = "default";
   _itc.func.label_get = _item_label_get;
   _itc.func.content_get = _item_icon_get;
   _itc.func.state_get = NULL;
   _itc.func.del = NULL;

   list = elm_genlist_add(win);

   for (i = 0; i < N_ITEMS; i++)
     {
	elm_genlist_item_append(list, &_itc,
				(void *)(long)i, NULL,
				ELM_GENLIST_ITEM_NONE,
				_item_sel_cb, NULL);
     }

   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, list);
   evas_object_show(list);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
