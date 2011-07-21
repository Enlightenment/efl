#include "widget_preview_tmpl_head.c"

Evas_Object *o = evas_object_rectangle_add(evas_object_evas_get(win));
evas_object_resize(o, 200, 130);
evas_object_show(o);

Evas_Object *ctxpopup = elm_ctxpopup_add(o);

Evas_Object *ic = elm_icon_add(ctxpopup);
elm_icon_standard_set(ic, "home");
elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
elm_ctxpopup_item_append(ctxpopup, "Go to home folder", ic, NULL, NULL);

ic = elm_icon_add(ctxpopup);
elm_icon_standard_set(ic, "delete");
elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
elm_ctxpopup_item_append(ctxpopup, "Delete file", ic, NULL, NULL);

ic = elm_icon_add(ctxpopup);
elm_icon_standard_set(ic, "folder");
elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
Elm_Ctxpopup_Item *it = elm_ctxpopup_item_append(ctxpopup, "Navigate to folder", ic, NULL, NULL);
elm_ctxpopup_item_disabled_set(it, EINA_TRUE);

evas_object_size_hint_weight_set(ctxpopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_move(ctxpopup, 0, 0);
evas_object_show(ctxpopup);

#include "widget_preview_tmpl_foot.c"
