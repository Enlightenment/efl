//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` genlist_example_01.c -o genlist_example_01

#include <time.h>
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define N_ITEMS 300

static Elm_Genlist_Item_Class _itc;

static char *
_item_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   time_t t = (time_t)ecore_time_unix_get();
   char buf[256];
   int i = (int)(long)data;
   if (i % 2)
     {
	int n;
	snprintf(buf, sizeof(buf), "Very Long Item # %i - realized at %s", i, ctime(&t));
	n = strlen(buf);
	buf[n - 1] = '\0';
     }
   else
     snprintf(buf, sizeof(buf), "short item # %i", i);
   return strdup(buf);
}

static Evas_Object *
_item_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
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

static void
_show_status_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   Evas_Coord x, y, w, h, mx, my;
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(list);

   const Eina_List *selected, *l, *realized;
   printf("\nfirst selected item: %p\n", it);

   selected = elm_genlist_selected_items_get(list);
   printf("all selected items (%d): ", eina_list_count(selected));
   EINA_LIST_FOREACH(selected, l, it)
     printf("%p ", it);
   printf("\n");

   realized = elm_genlist_realized_items_get(list);
   printf("realized items (%d): ", eina_list_count(realized));
   EINA_LIST_FOREACH(realized, l, it)
      printf("%p  ", it);
   printf("\n");
   printf("genlist mode: %s\n", elm_genlist_mode_get(list));
   printf("mode item: %p\n", elm_genlist_mode_item_get(list));

   evas_object_geometry_get(list, &x, &y, &w, &h);
   mx = w / 2 + x;
   my = h / 2 + y;
   it = elm_genlist_at_xy_item_get(list, mx, my, NULL);
   printf("item in the middle of the screen: %p\n", it);
}

static void
_realize_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *list = data;
   elm_genlist_realized_items_update(list);
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *box, *hbox;
   Evas_Object *list, *btn;
   int i;

   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist - simple");
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

   list = elm_genlist_add(win);

   _itc.item_style = "default";
   _itc.func.label_get = _item_label_get;
   _itc.func.icon_get = _item_icon_get;
   _itc.func.state_get = NULL;
   _itc.func.del = NULL;

   Eina_Bool hbounce, vbounce;
   Elm_Scroller_Policy hp, vp;

   printf("default values:\n");
   printf("always select: %d\n", elm_genlist_always_select_mode_get(list));
   elm_genlist_bounce_get(list, &hbounce, &vbounce);
   printf("bounce - horizontal: %d, vertical: %d\n", hbounce, vbounce);
   printf("compress mode: %d\n", elm_genlist_compress_mode_get(list));
   printf("homogeneous: %d\n", elm_genlist_homogeneous_get(list));
   printf("horizontal mode: %d\n", elm_genlist_horizontal_mode_get(list));
   printf("longpress timeout: %0.3f\n",
	  elm_genlist_longpress_timeout_get(list));
   printf("multi selection: %d\n", elm_genlist_multi_select_get(list));
   printf("no selection mode: %d\n", elm_genlist_no_select_mode_get(list));
   printf("height for width enabled: %d\n",
	  elm_genlist_height_for_width_mode_get(list));
   elm_genlist_scroller_policy_get(list, &hp, &vp);
   printf("scroller policy - horizontal: %d, vertical: %d\n", hp, vp);
   printf("block count: %d\n", elm_genlist_block_count_get(list));
   printf("\n");

   elm_genlist_always_select_mode_set(list, EINA_FALSE);
   elm_genlist_bounce_set(list, EINA_FALSE, EINA_FALSE);
   elm_genlist_compress_mode_set(list, EINA_TRUE);
   elm_genlist_homogeneous_set(list, EINA_FALSE);
   elm_genlist_horizontal_mode_set(list, ELM_LIST_LIMIT);
   elm_genlist_multi_select_set(list, EINA_TRUE);
   elm_genlist_no_select_mode_set(list, EINA_FALSE);
   elm_genlist_height_for_width_mode_set(list, EINA_FALSE);
   elm_genlist_scroller_policy_set(list, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
   elm_genlist_longpress_timeout_set(list, 0.5);
   elm_genlist_block_count_set(list, 16);

   for (i = 0; i < N_ITEMS; i++)
     {
	elm_genlist_item_append(list, &_itc,
				(void *)(long)i, NULL,
				ELM_GENLIST_ITEM_NONE,
				_item_sel_cb, NULL);
     }

   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, list);
   evas_object_show(list);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Show status");
   evas_object_size_hint_weight_set(btn, 0, 0);
   evas_object_size_hint_align_set(btn, 0.5, 0.5);
   evas_object_smart_callback_add(btn, "clicked", _show_status_cb, list);
   elm_box_pack_end(hbox, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Realize");
   evas_object_size_hint_weight_set(btn, 0, 0);
   evas_object_size_hint_align_set(btn, 0.5, 0.5);
   evas_object_smart_callback_add(btn, "clicked", _realize_cb, list);
   elm_box_pack_end(hbox, btn);
   evas_object_show(btn);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 200, 640);
   evas_object_resize(win, 200, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
