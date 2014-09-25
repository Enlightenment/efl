#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_grid.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define MY_CLASS ELM_GRID_CLASS
#define MY_CLASS_NAME "Elm_Grid"
#define MY_CLASS_NAME_LEGACY "elm_grid"

EOLIAN static Eina_Bool
_elm_grid_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_grid_elm_widget_focus_next(Eo *obj, void *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Eina_Bool int_ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_grid_children_get(wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

   if (list_free) list_free((Eina_List *)items);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_grid_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_grid_elm_widget_focus_direction(Eo *obj, void *_pd EINA_UNUSED, const Evas_Object *base, double degree, Evas_Object **direction, double *weight)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Eina_Bool int_ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_grid_children_get(wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_direction_get(obj, base, items, list_data_get,
                                             degree, direction, weight);

   if (list_free) list_free((Eina_List *)items);

   return int_ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_mirrored_set(wd->resize_obj, rtl);
}

EOLIAN static Eina_Bool
_elm_grid_elm_widget_theme_apply(Eo *obj, void *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return EINA_TRUE;
}

EOLIAN static void
_elm_grid_evas_object_smart_add(Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Object *grid;

   elm_widget_sub_object_parent_add(obj);

   grid = evas_object_grid_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, grid, EINA_TRUE);
   evas_object_grid_size_set(wd->resize_obj, 100, 100);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_FALSE);

   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static void
_elm_grid_evas_object_smart_del(Eo *obj, void *_pd EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* let's make our grid object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(wd->subobjs, l, child)
     {
        if (child == wd->resize_obj)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_grid_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static void
_elm_grid_eo_base_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_FILLER));
}

EOLIAN static void
_elm_grid_size_set(Eo *obj, void *_pd EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_size_set(wd->resize_obj, w, h);
}

EOLIAN static void
_elm_grid_size_get(Eo *obj, void *_pd EINA_UNUSED, Evas_Coord *w, Evas_Coord *h)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_size_get(wd->resize_obj, w, h);
}

EOLIAN static void
_elm_grid_pack(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_grid_pack(wd->resize_obj, subobj, x, y, w, h);
}

EOLIAN static void
_elm_grid_unpack(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_del(obj, subobj);
   evas_object_grid_unpack(wd->resize_obj, subobj);
}

EOLIAN static void
_elm_grid_clear(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool clear)
{
   Eina_List *chld;
   Evas_Object *o;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!clear)
     {
        chld = evas_object_grid_children_get(wd->resize_obj);
        EINA_LIST_FREE(chld, o)
          elm_widget_sub_object_del(obj, o);
     }

   evas_object_grid_clear(wd->resize_obj, clear);
}

EAPI void
elm_grid_pack_set(Evas_Object *subobj,
                  Evas_Coord x,
                  Evas_Coord y,
                  Evas_Coord w,
                  Evas_Coord h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_GRID_CHECK(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_pack(wd->resize_obj, subobj, x, y, w, h);
}

EAPI void
elm_grid_pack_get(Evas_Object *subobj,
                  int *x,
                  int *y,
                  int *w,
                  int *h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_GRID_CHECK(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_pack_get
     (wd->resize_obj, subobj, x, y, w, h);
}

EOLIAN static Eina_List*
_elm_grid_children_get(Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return evas_object_grid_children_get(wd->resize_obj);
}

static void
_elm_grid_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_grid.eo.c"
