//Compile with:
//gcc -o ctxpopup_example_01 ctxpopup_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static int list_mouse_down = 0;

static void
_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(obj);
}

static void
_ctxpopup_item_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("ctxpopup item selected: %s\n", elm_object_item_text_get(event_info));
}

Elm_Object_Item *item_new(Evas_Object *ctxpopup, const char * label, const char *icon)
{
   Evas_Object *ic = elm_icon_add(ctxpopup);
   elm_icon_standard_set(ic, icon);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   return elm_ctxpopup_item_append(ctxpopup, label, ic, _ctxpopup_item_cb, NULL);
}

static void
_list_item_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

   item_new(ctxpopup, "Go to home folder", "home");
   item_new(ctxpopup, "Save file", "file");
   item_new(ctxpopup, "Delete file", "delete");
   it = item_new(ctxpopup, "Navigate to folder", "folder");
   elm_object_item_disabled_set(it, EINA_TRUE);
   item_new(ctxpopup, "Edit entry", "edit");
   it = item_new(ctxpopup, "Set date and time", "clock");
   elm_object_item_disabled_set(it, EINA_TRUE);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);

   elm_list_item_selected_set(event_info, EINA_FALSE);
}

static void
_list_item_cb2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);
   elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

   item_new(ctxpopup, NULL, "home");
   item_new(ctxpopup, NULL, "file");
   item_new(ctxpopup, NULL, "delete");
   item_new(ctxpopup, NULL, "folder");
   it = item_new(ctxpopup, NULL, "edit");
   elm_object_item_disabled_set(it, EINA_TRUE);
   item_new(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);

   elm_list_item_selected_set(event_info, EINA_FALSE);
}

static void
_list_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   list_mouse_down++;
}

static void
_list_mouse_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   list_mouse_down--;
}

static void
_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   list_mouse_down = 0;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *list;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("Contextual Popup", "Contextual Popup");
   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);
   elm_win_autodel_set(win, EINA_TRUE);

   list = elm_list_add(win);
   evas_object_event_callback_add(list, EVAS_CALLBACK_MOUSE_DOWN,
                                  _list_mouse_down, NULL);
   evas_object_event_callback_add(list, EVAS_CALLBACK_MOUSE_UP,
                                  _list_mouse_up, NULL);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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

   return 0;
}
ELM_MAIN()
