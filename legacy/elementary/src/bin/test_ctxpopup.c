#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_ctxpopup_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
          elm_ctxpopup_item_label_get(event_info));
}

#define ITEM_NEW(_hov, _label, _icon)                                           \
   if(_icon)                                                                    \
     {                                                                          \
        ic = elm_icon_add(obj);                                                 \
        elm_icon_standard_set(ic, _icon);                                       \
        elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);                         \
     }                                                                          \
   else                                                                         \
      ic = NULL;                                                                \
   it = elm_ctxpopup_item_append(_hov, _label, ic, _ctxpopup_item_cb, NULL);    \

static void
_list_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Ctxpopup_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);

   ITEM_NEW(ctxpopup, "Go to home folder", "home");
   ITEM_NEW(ctxpopup, "Save file", "file");
   ITEM_NEW(ctxpopup, "Delete file", "delete");
   ITEM_NEW(ctxpopup, "Navigate to folder", "folder");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, "Edit entry", "edit");
   ITEM_NEW(ctxpopup, "Set date and time", "clock");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
}

static void
_list_item_cb2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Ctxpopup_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);

   ITEM_NEW(ctxpopup, NULL, "home");
   ITEM_NEW(ctxpopup, NULL, "file");
   ITEM_NEW(ctxpopup, NULL, "delete");
   ITEM_NEW(ctxpopup, NULL, "folder");
   ITEM_NEW(ctxpopup, NULL, "edit");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, NULL, "clock");

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_size_hint_max_set(ctxpopup, 240, 240);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
}

static void
_list_item_cb3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Ctxpopup_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);

   ITEM_NEW(ctxpopup, "Eina", NULL);
   ITEM_NEW(ctxpopup, "Eet", NULL);
   ITEM_NEW(ctxpopup, "Evas", NULL);
   ITEM_NEW(ctxpopup, "Ecore", NULL);
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(ctxpopup, "Embryo", NULL);
   ITEM_NEW(ctxpopup, "Edje", NULL);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
}

static void
_list_item_cb4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ctxpopup, *ic;
   Elm_Ctxpopup_Item *it;
   Evas_Coord x,y;

   ctxpopup = elm_ctxpopup_add(obj);
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
}


static void
_list_item_cb5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
   elm_button_label_set(btn, "Enlightenment");
   evas_object_size_hint_min_set(btn, 140, 140);

   elm_scroller_content_set(sc, btn);

   elm_box_pack_end(bx, sc);

   ctxpopup = elm_ctxpopup_add(obj);
   elm_ctxpopup_content_set(ctxpopup, bx);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   evas_object_move(ctxpopup, x, y);
   evas_object_show(ctxpopup);
}


static void _list_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   elm_list_item_selected_set(event_info, EINA_FALSE);
}

void
test_ctxpopup(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *list;

   win = elm_win_add(NULL, "Contextual Popup", ELM_WIN_BASIC);
   elm_win_title_set(win, "Contextual Popup");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   list = elm_list_add(win);
   elm_win_resize_object_add(win, list);
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
   evas_object_show(list);
   elm_list_go(list);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
#endif
