#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_mapbuf_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

Evas_Object *tb = elm_table_add(win);
evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
evas_object_show(tb);

unsigned int i, j;
for (i = 0; i < 8; i++)
  {
     for (j = 0; j < 8; j++)
       {
          Evas_Object *ic = elm_icon_add(win);
          elm_icon_standard_set(ic, "home");
          elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
          evas_object_show(ic);
          elm_table_pack(tb, ic, i, j, 1, 1);
       }
  }

elm_object_content_set(o, tb);
elm_mapbuf_enabled_set(o, EINA_TRUE);

#include "widget_preview_tmpl_foot.c"
