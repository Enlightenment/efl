#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

Elm_Gen_Item *
_elm_genlist_item_new(Widget_Data              *wd,
                      const Elm_Gen_Item_Class *itc,
                      const void               *data,
                      Elm_Gen_Item             *parent,
                      Evas_Smart_Cb             func,
                      const void               *func_data);

Evas_Object *
_elm_genlist_item_widget_get(const Elm_Gen_Item *it);

void
_elm_genlist_page_relative_set(Evas_Object *obj,
                               double       h_pagerel,
                               double       v_pagerel);

void
_elm_genlist_page_relative_get(const Evas_Object *obj,
                               double            *h_pagerel,
                               double            *v_pagerel);

void
_elm_genlist_page_size_set(Evas_Object *obj,
                           Evas_Coord   h_pagesize,
                           Evas_Coord   v_pagesize);

void
_elm_genlist_current_page_get(const Evas_Object *obj,
                              int               *h_pagenumber,
                              int               *v_pagenumber);

void
_elm_genlist_last_page_get(const Evas_Object *obj,
                           int               *h_pagenumber,
                           int               *v_pagenumber);

void
_elm_genlist_page_show(const Evas_Object *obj,
                       int                h_pagenumber,
                       int                v_pagenumber);

void
_elm_genlist_page_bring_in(const Evas_Object *obj,
                           int                h_pagenumber,
                           int                v_pagenumber);
