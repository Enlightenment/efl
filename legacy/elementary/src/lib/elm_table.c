#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_table.h"

EAPI const char ELM_TABLE_SMART_NAME[] = "elm_table";

EVAS_SMART_SUBCLASS_NEW
  (ELM_TABLE_SMART_NAME, _elm_table, Elm_Table_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, NULL);

static Eina_Bool
_elm_table_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Eina_Bool ret;
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   ELM_TABLE_DATA_GET(obj, sd);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_table_children_get
            (ELM_WIDGET_DATA(sd)->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

   if (list_free) list_free((Eina_List *)items);

   return ret;
}

static Eina_Bool
_elm_table_smart_focus_direction(const Evas_Object *obj,
                                 const Evas_Object *base,
                                 double degree,
                                 Evas_Object **direction,
                                 double *weight)
{
   Eina_Bool ret;
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   ELM_TABLE_DATA_GET(obj, sd);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_table_children_get
            (ELM_WIDGET_DATA(sd)->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   ret = elm_widget_focus_list_direction_get
       (obj, base, items, list_data_get, degree, direction, weight);

   if (list_free)
     list_free((Eina_List *)items);

   return ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_mirrored_set(ELM_WIDGET_DATA(sd)->resize_obj, rtl);
}

static Eina_Bool
_elm_table_smart_theme(Evas_Object *obj)
{
   if (!_elm_table_parent_sc->theme(obj)) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return EINA_TRUE;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_size_hint_min_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_on_size_hints_changed(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static Eina_Bool
_elm_table_smart_sub_object_del(Evas_Object *obj,
                                Evas_Object *child)
{
   if (!_elm_table_parent_sc->sub_object_del(obj, child)) return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_table_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Widget_Smart_Data);

   priv->resize_obj = evas_object_table_add(evas_object_evas_get(obj));

   evas_object_event_callback_add
     (priv->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   _elm_table_parent_sc->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   _elm_table_smart_theme(obj);
}

static void
_elm_table_smart_del(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (ELM_WIDGET_DATA(sd)->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   /* let's make our table object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(ELM_WIDGET_DATA(sd)->subobjs, l, child)
     {
        if (child == ELM_WIDGET_DATA(sd)->resize_obj)
          {
             ELM_WIDGET_DATA(sd)->subobjs =
               eina_list_demote_list(ELM_WIDGET_DATA(sd)->subobjs, l);
             break;
          }
     }

   _elm_table_parent_sc->base.del(obj);
}

static void
_elm_table_smart_set_user(Elm_Table_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_table_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_table_smart_del;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_table_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->theme = _elm_table_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_table_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_table_smart_focus_direction;
}

EAPI const Elm_Table_Smart_Class *
elm_table_smart_class_get(void)
{
   static Elm_Table_Smart_Class _sc =
     ELM_TABLE_SMART_CLASS_INIT_NAME_VERSION(ELM_TABLE_SMART_NAME);
   static const Elm_Table_Smart_Class *class = NULL;

   if (class) return class;

   _elm_table_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_table_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_table_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_table_homogeneous_set(Evas_Object *obj,
                          Eina_Bool homogeneous)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_homogeneous_set
     (ELM_WIDGET_DATA(sd)->resize_obj, homogeneous);
}

EAPI Eina_Bool
elm_table_homogeneous_get(const Evas_Object *obj)
{
   ELM_TABLE_CHECK(obj) EINA_FALSE;
   ELM_TABLE_DATA_GET(obj, sd);

   return evas_object_table_homogeneous_get(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI void
elm_table_padding_set(Evas_Object *obj,
                      Evas_Coord horizontal,
                      Evas_Coord vertical)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_padding_set
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_table_padding_get(const Evas_Object *obj,
                      Evas_Coord *horizontal,
                      Evas_Coord *vertical)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_padding_get
     (ELM_WIDGET_DATA(sd)->resize_obj, horizontal, vertical);
}

EAPI void
elm_table_pack(Evas_Object *obj,
               Evas_Object *subobj,
               int x,
               int y,
               int w,
               int h)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_table_pack(ELM_WIDGET_DATA(sd)->resize_obj, subobj, x, y, w, h);
}

EAPI void
elm_table_unpack(Evas_Object *obj,
                 Evas_Object *subobj)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   elm_widget_sub_object_del(obj, subobj);
   evas_object_table_unpack(ELM_WIDGET_DATA(sd)->resize_obj, subobj);
}

EAPI void
elm_table_pack_set(Evas_Object *subobj,
                   int x,
                   int y,
                   int w,
                   int h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_pack(ELM_WIDGET_DATA(sd)->resize_obj, subobj, x, y, w, h);
}

EAPI void
elm_table_pack_get(Evas_Object *subobj,
                   int *x,
                   int *y,
                   int *w,
                   int *h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);
   unsigned short ix, iy, iw, ih;

   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_pack_get
     (ELM_WIDGET_DATA(sd)->resize_obj, subobj, &ix, &iy, &iw, &ih);
   if (x) *x = ix;
   if (y) *y = iy;
   if (w) *w = iw;
   if (h) *h = ih;
}

EAPI void
elm_table_clear(Evas_Object *obj,
                Eina_Bool clear)
{
   ELM_TABLE_CHECK(obj);
   ELM_TABLE_DATA_GET(obj, sd);

   evas_object_table_clear(ELM_WIDGET_DATA(sd)->resize_obj, clear);
}
