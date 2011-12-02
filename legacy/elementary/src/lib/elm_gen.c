#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "elm_gen.h"

EAPI void
elm_gen_clear(Evas_Object *obj)
{
   elm_genlist_clear(obj);
}

EAPI void
elm_gen_item_selected_set(Elm_Gen_Item *it,
                          Eina_Bool     selected)
{
   elm_genlist_item_selected_set(it, selected);
}

EAPI Eina_Bool
elm_gen_item_selected_get(const Elm_Gen_Item *it)
{
   return elm_genlist_item_selected_get(it);
}

EAPI void
elm_gen_always_select_mode_set(Evas_Object *obj,
                               Eina_Bool    always_select)
{
   elm_genlist_always_select_mode_set(obj, always_select);
}

EAPI Eina_Bool
elm_gen_always_select_mode_get(const Evas_Object *obj)
{
   return elm_genlist_always_select_mode_get(obj);
}

EAPI void
elm_gen_no_select_mode_set(Evas_Object *obj,
                           Eina_Bool    no_select)
{
   elm_genlist_no_select_mode_set(obj, no_select);
}

EAPI Eina_Bool
elm_gen_no_select_mode_get(const Evas_Object *obj)
{
   return elm_genlist_no_select_mode_get(obj);
}

EAPI void
elm_gen_bounce_set(Evas_Object *obj,
                   Eina_Bool    h_bounce,
                   Eina_Bool    v_bounce)
{
   elm_genlist_bounce_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_gen_bounce_get(const Evas_Object *obj,
                   Eina_Bool         *h_bounce,
                   Eina_Bool         *v_bounce)
{
   elm_genlist_bounce_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_gen_page_relative_set(Evas_Object *obj,
                              double   h_pagerel,
                              double   v_pagerel)
{
   _elm_genlist_page_relative_set(obj, h_pagerel, v_pagerel);
}

EAPI void
elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel)
{
   _elm_genlist_page_relative_get(obj, h_pagerel, v_pagerel);
}

EAPI void
elm_gen_page_size_set(Evas_Object *obj,
                      Evas_Coord   h_pagesize,
                      Evas_Coord   v_pagesize)
{
   _elm_genlist_page_size_set(obj, h_pagesize, v_pagesize);
}

EAPI void
elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   _elm_genlist_current_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   _elm_genlist_last_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   _elm_genlist_page_show(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   _elm_genlist_page_bring_in(obj, h_pagenumber, v_pagenumber);
}

EAPI Elm_Gen_Item *
elm_gen_first_item_get(const Evas_Object *obj)
{
   return (Elm_Gen_Item *)elm_genlist_first_item_get(obj);
}

EAPI Elm_Gen_Item *
elm_gen_last_item_get(const Evas_Object *obj)
{
   return (Elm_Gen_Item *)elm_genlist_last_item_get(obj);
}

EAPI Elm_Gen_Item *
elm_gen_item_next_get(const Elm_Gen_Item *it)
{
   return (Elm_Gen_Item *)elm_genlist_item_next_get(it);
}

EAPI Elm_Gen_Item *
elm_gen_item_prev_get(const Elm_Gen_Item *it)
{
   return (Elm_Gen_Item *)elm_genlist_item_prev_get(it);
}

EAPI Evas_Object *
elm_gen_item_widget_get(const Elm_Gen_Item *it)
{
   return _elm_genlist_item_widget_get(it);
}
