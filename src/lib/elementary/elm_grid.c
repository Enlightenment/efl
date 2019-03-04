#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>
#include <elm_grid.eo.h>
#include "elm_priv.h"
#include "elm_widget_grid.h"

#define MY_CLASS ELM_GRID_CLASS
#define MY_CLASS_NAME "Elm_Grid"
#define MY_CLASS_NAME_LEGACY "elm_grid"

static void
_elm_grid_efl_ui_focus_composition_prepare(Eo *obj, void *pd EINA_UNUSED)
{
   Eina_List *l, *ll;
   Efl_Ui_Widget *elem;

   Elm_Widget_Smart_Data *wpd = efl_data_scope_get(obj, EFL_UI_WIDGET_CLASS);
   Eina_List *order = evas_object_grid_children_get(wpd->resize_obj);

   EINA_LIST_FOREACH_SAFE(order, l, ll, elem)
     {
        if (!efl_isa(elem, EFL_UI_WIDGET_CLASS))
          order = eina_list_remove(order, elem);
     }

   efl_ui_focus_composition_elements_set(obj, order);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_mirrored_set(wd->resize_obj, rtl);
}

EOLIAN static Eina_Error
_elm_grid_efl_ui_widget_theme_apply(Eo *obj, void *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   return int_ret;
}

EOLIAN static void
_elm_grid_efl_canvas_group_group_add(Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Object *grid;

   elm_widget_sub_object_parent_add(obj);

   grid = evas_object_grid_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, grid);
   evas_object_grid_size_set(wd->resize_obj, 100, 100);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static void
_elm_grid_efl_canvas_group_group_del(Eo *obj, void *_pd EINA_UNUSED)
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

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_grid_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_grid_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   return obj;
}

EOLIAN static void
_elm_grid_grid_size_set(Eo *obj, void *_pd EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_size_set(wd->resize_obj, w, h);
}

EOLIAN static void
_elm_grid_grid_size_get(const Eo *obj, void *_pd EINA_UNUSED, Evas_Coord *w, Evas_Coord *h)
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
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_grid_unpack(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _elm_widget_sub_object_redirect_to_top(obj, subobj);
   evas_object_grid_unpack(wd->resize_obj, subobj);
   efl_ui_focus_composition_dirty(obj);
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
          _elm_widget_sub_object_redirect_to_top(obj, o);
     }

   evas_object_grid_clear(wd->resize_obj, clear);
   efl_ui_focus_composition_dirty(obj);
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
   efl_ui_focus_composition_dirty(obj);
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
_elm_grid_children_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return evas_object_grid_children_get(wd->resize_obj);
}

static void
_elm_grid_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define ELM_GRID_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_grid)

#include "elm_grid.eo.c"
