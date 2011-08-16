//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` ctxpopup_example_01.c -o ctxpopup_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void
_ctxpopup_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("ctxpopup item selected: %s\n", elm_ctxpopup_item_label_get(event_info));
}

Elm_Object_Item *item_new(Evas_Object *ctxpopup, const char * label, const char *icon)
{
   Evas_Object *ic = elm_icon_add(ctxpopup);
   elm_icon_standard_set(ic, icon);
   elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
   return elm_ctxpopup_item_append(ctxpopup, label, ic, _ctxpopup_item_cb, NULL);
}

static void
_list_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);

   item_new(ctxpopup, "Go to home folder", "home");
   item_new(ctxpopup, "Save file", "file");
   item_new(ctxpopup, "Delete file", "delete");
   it = item_new(ctxpopup, "Navigate to folder", "folder");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   item_new(ctxpopup, "Edit entry", "edit");
   it = item_new(ctxpopup, "Set date and time", "clock");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);

   elm_list_item_selected_set(event_info, EINA_FALSE);
}

static void
_list_item_cb2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
   elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

   item_new(ctxpopup, NULL, "home");
   item_new(ctxpopup, NULL, "file");
   item_new(ctxpopup, NULL, "delete");
   item_new(ctxpopup, NULL, "folder");
   it = item_new(ctxpopup, NULL, "edit");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   item_new(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 380, 40);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);

   elm_list_item_selected_set(event_info, EINA_FALSE);
}

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *list;

   win = elm_win_add(NULL, "Contextual Popup", ELM_WIN_BASIC);
   elm_win_title_set(win, "Contextual Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   list = elm_list_add(win);
   elm_win_resize_object_add(win, list);
   elm_list_mode_set(list, ELM_LIST_COMPRESS);

   elm_list_item_append(list, "Ctxpopup with icons and labels", NULL, NULL,
                        _list_item_cb, NULL);
   elm_list_item_append(list, "Ctxpopup with icons only", NULL, NULL,
                        _list_item_cb2, NULL);
   evas_object_show(list);
   elm_list_go(list);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();
}
ELM_MAIN()
