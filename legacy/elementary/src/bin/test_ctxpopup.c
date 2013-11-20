#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static int list_mouse_down = 0;

static void
_dismissed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup_data = evas_object_data_get(obj, "im");
   if (ctxpopup_data) evas_object_del(ctxpopup_data);
   evas_object_del(obj);
}

static void
_print_current_dir(Evas_Object *obj)
{
   Elm_Ctxpopup_Direction dir;
   dir = elm_ctxpopup_direction_get(obj);

   switch(dir)
     {
      case ELM_CTXPOPUP_DIRECTION_LEFT:
         printf("ctxpopup direction: left!\n");
         break;
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
         printf("ctxpopup direction: right!\n");
         break;
      case ELM_CTXPOPUP_DIRECTION_UP:
         printf("ctxpopup direction: up!\n");
         break;
      case ELM_CTXPOPUP_DIRECTION_DOWN:
         printf("ctxpopup direction: down!\n");
         break;
      case ELM_CTXPOPUP_DIRECTION_UNKNOWN:
         printf("ctxpopup direction: unknow!\n");
         break;
     }
}

static void
_btn_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("Button Clicked\n");

   Evas_Object *im;
   char buf[PATH_MAX];
   void *ctxpopup_data;

   ctxpopup_data = evas_object_data_get(data, "id");
   if (!ctxpopup_data) return;

   if (!strcmp("list_item_6", (char *) ctxpopup_data))
     {
        ctxpopup_data = evas_object_data_get(data, "im");
        if (ctxpopup_data) return;

        im = evas_object_image_filled_add(evas_object_evas_get(obj));
        snprintf(buf, sizeof(buf), "%s/images/%s",
                 elm_app_data_dir_get(), "twofish.jpg");
        evas_object_image_file_set(im, buf, NULL);
        evas_object_move(im, 40, 40);
        evas_object_resize(im, 320, 320);
        evas_object_show(im);
        evas_object_data_set((Evas_Object *)data, "im", im);

        evas_object_raise((Evas_Object *)data);
     }
}

static void
_ctxpopup_item_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
   elm_object_item_text_get(event_info));
}

static Elm_Object_Item *
_ctxpopup_item_new(Evas_Object *obj, const char *label, const char *icon)
{
   Evas_Object *ic = NULL;
   Elm_Object_Item *it = NULL;

   if (!obj) return NULL;

   if (icon)
     {
        ic = elm_icon_add(obj);
        elm_icon_standard_set(ic, icon);
        elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
     }

   it = elm_ctxpopup_item_append(obj, label, ic, _ctxpopup_item_cb, NULL);
   return it;
}

static void
_list_item_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it = NULL;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   _ctxpopup_item_new(ctxpopup, "Go to home folder", "home");
   _ctxpopup_item_new(ctxpopup, "Save file", "file");
   _ctxpopup_item_new(ctxpopup, "Delete file", "delete");
   it = _ctxpopup_item_new(ctxpopup, "Navigate to folder", "folder");
   elm_object_item_disabled_set(it, EINA_TRUE);
   _ctxpopup_item_new(ctxpopup, "Edit entry", "edit");
   it = _ctxpopup_item_new(ctxpopup, "Set date and time", "clock");
   elm_object_item_disabled_set(it, EINA_TRUE);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb2(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it = NULL;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   _ctxpopup_item_new(ctxpopup, NULL, "home");
   _ctxpopup_item_new(ctxpopup, NULL, "file");
   _ctxpopup_item_new(ctxpopup, NULL, "delete");
   _ctxpopup_item_new(ctxpopup, NULL, "folder");
   it = _ctxpopup_item_new(ctxpopup, NULL, "edit");
   elm_object_item_disabled_set(it, EINA_TRUE);
   _ctxpopup_item_new(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb3(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup;
   Elm_Object_Item *it = NULL;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   _ctxpopup_item_new(ctxpopup, "Eina", NULL);
   _ctxpopup_item_new(ctxpopup, "Eet", NULL);
   _ctxpopup_item_new(ctxpopup, "Evas", NULL);
   it = _ctxpopup_item_new(ctxpopup, "Ecore", NULL);
   elm_object_item_disabled_set(it, EINA_TRUE);
   _ctxpopup_item_new(ctxpopup, "Embryo", NULL);
   _ctxpopup_item_new(ctxpopup, "Edje", NULL);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb4(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

   _ctxpopup_item_new(ctxpopup, NULL, "home");
   _ctxpopup_item_new(ctxpopup, NULL, "file");
   _ctxpopup_item_new(ctxpopup, NULL, "delete");
   _ctxpopup_item_new(ctxpopup, NULL, "folder");
   _ctxpopup_item_new(ctxpopup, NULL, "edit");
   _ctxpopup_item_new(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}


static void
_list_item_cb5(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup, *btn, *sc, *bx;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   bx = elm_box_add(obj);
   evas_object_size_hint_min_set(bx, 150, 150);

   sc = elm_scroller_add(bx);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(sc);

   btn = elm_button_add(sc);
   elm_object_text_set(btn, "Enlightenment");
   evas_object_size_hint_min_set(btn, 140, 140);

   elm_object_content_set(sc, btn);

   elm_box_pack_end(bx, sc);

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   elm_object_content_set(ctxpopup, bx);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);

   evas_object_data_set(ctxpopup, "id", "list_item_5");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked, ctxpopup);
}

static void
_list_item_cb6(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup, *btn, *sc, *bx;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;

   bx = elm_box_add(obj);
   evas_object_size_hint_min_set(bx, 200, 150);

   sc = elm_scroller_add(bx);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(sc);

   btn = elm_button_add(sc);
   elm_object_text_set(btn, "Ctxpop will be on the top of layer");
   evas_object_size_hint_min_set(btn, 190, 140);

   elm_object_content_set(sc, btn);

   elm_box_pack_end(bx, sc);

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   elm_object_content_set(ctxpopup, bx);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);

   evas_object_data_set(ctxpopup, "id", "list_item_6");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked, ctxpopup);
}

static void
_ctxpopup_item_disable_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
          elm_object_item_text_get(event_info));

   elm_object_item_disabled_set(event_info, EINA_TRUE);
}

static void
_ctxpopup_item_delete_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
          elm_object_item_text_get(event_info));

   evas_object_del(obj);
}

static void
_list_item_cb7(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ctxpopup;
   Evas_Coord x,y;

   if (list_mouse_down > 0) return;
   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

   elm_ctxpopup_item_append(ctxpopup, "Disable this item", NULL, _ctxpopup_item_disable_cb, ctxpopup);
   elm_ctxpopup_item_append(ctxpopup, "Delete this ctxpopup", NULL, _ctxpopup_item_delete_cb, ctxpopup);
   elm_ctxpopup_item_append(ctxpopup, "Another item", NULL, _ctxpopup_item_cb, NULL);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
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

void
test_ctxpopup(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *list;

   win = elm_win_util_standard_add("contextual-popup", "Contextual Popup");
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
   evas_object_smart_callback_add(list, "selected", _list_clicked, NULL);

   elm_list_item_append(list, "Ctxpopup with icons and labels", NULL, NULL,
                        _list_item_cb, NULL);
   elm_list_item_append(list, "Ctxpopup with icons only", NULL, NULL,
                        _list_item_cb2, NULL);
   elm_list_item_append(list, "Ctxpopup with labels only", NULL, NULL,
                        _list_item_cb3, NULL);
   elm_list_item_append(list, "Ctxpopup at horizontal mode", NULL, NULL,
                        _list_item_cb4, NULL);
   elm_list_item_append(list, "Ctxpopup with user content", NULL, NULL,
                        _list_item_cb5, NULL);
   elm_list_item_append(list, "Ctxpopup with restacking", NULL, NULL,
                        _list_item_cb6, NULL);
   elm_list_item_append(list, "Ctxpopup with callback function", NULL, NULL,
                        _list_item_cb7, NULL);
   evas_object_show(list);
   elm_list_go(list);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
