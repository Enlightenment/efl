#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH


static void
_dismissed(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
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
_btn_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
_ctxpopup_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
   elm_object_item_text_get(event_info));
}

#define ITEM_NEW(_hov, _label, _icon)                                         \
   if (_icon)                                                                 \
     {                                                                        \
        ic = elm_icon_add(obj);                                               \
        elm_icon_standard_set(ic, _icon);                                     \
        elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);                   \
     }                                                                        \
   else                                                                       \
      ic = NULL;                                                              \
   it = elm_ctxpopup_item_append(_hov, _label, ic, _ctxpopup_item_cb, NULL);  \

static void
_list_item_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   ITEM_NEW(ctxpopup, "Go to home folder", "home");
   ITEM_NEW(ctxpopup, "Save file", "file");
   ITEM_NEW(ctxpopup, "Delete file", "delete");
   ITEM_NEW(ctxpopup, "Navigate to folder", "folder");
   elm_object_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, "Edit entry", "edit");
   ITEM_NEW(ctxpopup, "Set date and time", "clock");
   elm_object_item_disabled_set(it, EINA_TRUE);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb2(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   ITEM_NEW(ctxpopup, NULL, "home");
   ITEM_NEW(ctxpopup, NULL, "file");
   ITEM_NEW(ctxpopup, NULL, "delete");
   ITEM_NEW(ctxpopup, NULL, "folder");
   ITEM_NEW(ctxpopup, NULL, "edit");
   elm_object_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb3(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   ITEM_NEW(ctxpopup, "Eina", NULL);
   ITEM_NEW(ctxpopup, "Eet", NULL);
   ITEM_NEW(ctxpopup, "Evas", NULL);
   ITEM_NEW(ctxpopup, "Ecore", NULL);
   elm_object_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, "Embryo", NULL);
   ITEM_NEW(ctxpopup, "Edje", NULL);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);
}

static void
_list_item_cb4(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Object_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

   ITEM_NEW(ctxpopup, NULL, "home");
   ITEM_NEW(ctxpopup, NULL, "file");
   ITEM_NEW(ctxpopup, NULL, "delete");
   ITEM_NEW(ctxpopup, NULL, "folder");
   ITEM_NEW(ctxpopup, NULL, "edit");
   ITEM_NEW(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);

   (void)it;
}


static void
_list_item_cb5(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *btn, *sc, *bx;
   Evas_Coord x,y;

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
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   elm_object_content_set(ctxpopup, bx);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);

   evas_object_data_set(ctxpopup, "id", "list_item_5");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked, ctxpopup);
}

static void
_list_item_cb6(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *btn, *sc, *bx;
   Evas_Coord x,y;

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
   evas_object_smart_callback_add(ctxpopup,
                                  "dismissed",
                                  _dismissed,
                                  NULL);

   elm_object_content_set(ctxpopup, bx);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
   _print_current_dir(ctxpopup);

   evas_object_data_set(ctxpopup, "id", "list_item_6");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked, ctxpopup);
}

static void _list_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   elm_list_item_selected_set(event_info, EINA_FALSE);
}

void
test_ctxpopup(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *list;

   win = elm_win_util_standard_add("contextual-popup", "Contextual Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   list = elm_list_add(win);
   elm_win_resize_object_add(win, list);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_mode_set(list, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(list, "selected", _list_clicked, NULL);

#undef ITEM_NEW
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
   evas_object_show(list);
   elm_list_go(list);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
#endif
