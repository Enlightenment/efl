#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_list_item_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_show(data);
}

static void
_ctxpopup_item_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("ctxpopup item selected: %s\n",
          elm_ctxpopup_item_label_get(event_info));
}

static void _list_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   elm_list_item_selected_set(event_info, EINA_FALSE);
}

void
test_ctxpopup(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *list, *hov1, *hov2, *hov3, *hov4, *ic;
   Elm_Ctxpopup_Item *it;

   win = elm_win_add(NULL, "Contexual Popup 2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Contextual Popup 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   list = elm_list_add(win);
   elm_win_resize_object_add(win, list);
   elm_list_mode_set(list, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(list, "selected", _list_clicked, NULL);

#define ITEM_NEW(_hov, _icon, _label)                                   \
   ic = elm_icon_add(win);                                              \
   elm_icon_standard_set(ic, _icon);                                    \
   elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);                      \
   it = elm_ctxpopup_item_append(_hov, _label, ic, _ctxpopup_item_cb, NULL);

   hov1 = elm_ctxpopup_add(win);

   ITEM_NEW(hov1, "home", "Go to home folder");
   ITEM_NEW(hov1, "file", "Save file");
   ITEM_NEW(hov1, "delete", "Delete file");
   ITEM_NEW(hov1, "folder", "Navigate to folder");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(hov1, "edit", "Edit entry");
   ITEM_NEW(hov1, "clock", "Set date and time");
   ITEM_NEW(hov1, "arrow_up", "Go up");
   ITEM_NEW(hov1, "arrow_down", "Go down");
   ITEM_NEW(hov1, "arrow_left", "Go left");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(hov1, "arrow_right", "Go right");

   hov2 = elm_ctxpopup_add(win);

   ITEM_NEW(hov2, "home", NULL);
   ITEM_NEW(hov2, "file", NULL);
   ITEM_NEW(hov2, "delete", NULL);
   ITEM_NEW(hov2, "folder", NULL);
   ITEM_NEW(hov2, "edit", NULL);
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(hov2, "clock", NULL);
   ITEM_NEW(hov2, "arrow_up", NULL);
   ITEM_NEW(hov2, "arrow_down", NULL);
   ITEM_NEW(hov2, "arrow_left", NULL);
   ITEM_NEW(hov2, "arrow_right", NULL);

   hov3 = elm_ctxpopup_add(win);
   elm_ctxpopup_horizontal_set(hov3, EINA_TRUE);

   ITEM_NEW(hov3, "home", "Zero");
   ITEM_NEW(hov3, "file", "Um");
   ITEM_NEW(hov3, "delete", "Dos");
   ITEM_NEW(hov3, "folder", "Tri");
   ITEM_NEW(hov3, "edit", "Vier");
   ITEM_NEW(hov3, "clock", "五");

   hov4 = elm_ctxpopup_add(win);

   ITEM_NEW(hov4, NULL, "Eina");
   ITEM_NEW(hov4, NULL, "Eet");
   ITEM_NEW(hov4, NULL, "Evas");
   ITEM_NEW(hov4, NULL, "Ecore");
   elm_ctxpopup_item_disabled_set(it, EINA_TRUE);
   ITEM_NEW(hov4, NULL, "Embryo");
   ITEM_NEW(hov4, NULL, "Edje");

#undef ITEM_NEW

   elm_list_item_append(list, "Ctxpopup with icons and labels", NULL, NULL,
                        _list_item_cb, hov1);
   elm_list_item_append(list, "Ctxpopup with icons only", NULL, NULL,
                        _list_item_cb, hov2);
   elm_list_item_append(list, "Ctxpopup at horizontal mode", NULL, NULL,
                        _list_item_cb, hov3);
   elm_list_item_append(list, "Ctxpopup with no icons", NULL, NULL,
                        _list_item_cb, hov4);
   evas_object_show(list);
   elm_list_go(list);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}
#endif
