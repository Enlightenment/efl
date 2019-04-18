#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>
#include <elm_table_eo.h>
#include "elm_priv.h"
#include "elm_widget_table.h"

#define MY_CLASS ELM_TABLE_CLASS

#define MY_CLASS_NAME "Elm_Table"
#define MY_CLASS_NAME_LEGACY "elm_table"

static void
_elm_table_efl_ui_focus_composition_prepare(Eo *obj, void *pd EINA_UNUSED)
{
   Eina_List *l, *ll;
   Efl_Ui_Widget *elem;

   Elm_Widget_Smart_Data *wpd = efl_data_scope_get(obj, EFL_UI_WIDGET_CLASS);
   Eina_List *order = evas_object_table_children_get(wpd->resize_obj);

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

   evas_object_table_mirrored_set(wd->resize_obj, rtl);
}

EOLIAN static Eina_Error
_elm_table_efl_ui_widget_theme_apply(Eo *obj, void *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   return int_ret;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (!efl_alive_get(obj)) return;

   evas_object_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_on_size_hints_changed(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

EOLIAN static Eina_Bool
_elm_table_efl_ui_widget_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *child)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), child);
   if (!int_ret) return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_table_efl_canvas_group_group_add(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *table;

   table = evas_object_table_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, table);

   evas_object_event_callback_add
     (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static void
_elm_table_efl_canvas_group_group_del(Eo *obj, void *_pd EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_event_callback_del_full
     (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   /* let's make our table object the *last* to be processed, since it
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
elm_table_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_table_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   return obj;
}

EOLIAN static void
_elm_table_homogeneous_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool homogeneous)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_homogeneous_set
     (wd->resize_obj, homogeneous);
}

EOLIAN static Eina_Bool
_elm_table_homogeneous_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   return evas_object_table_homogeneous_get(wd->resize_obj);
}

EOLIAN static void
_elm_table_padding_set(Eo *obj, void *_pd EINA_UNUSED, Evas_Coord horizontal, Evas_Coord vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_padding_set
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_table_padding_get(const Eo *obj, void *_pd EINA_UNUSED, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_padding_get
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_table_align_set(Eo *obj, void *_pd EINA_UNUSED, double horizontal, double vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_align_set
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_table_align_get(const Eo *obj, void *_pd EINA_UNUSED, double *horizontal, double *vertical)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_align_get
     (wd->resize_obj, horizontal, vertical);
}

EOLIAN static void
_elm_table_pack(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj, int col, int row, int colspan, int rowspan)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (col < 0)
     {
        ERR("col < 0");
        return;
     }
   if (colspan < 1)
     {
        ERR("colspan < 1");
        return;
     }
   if ((0xffff - col) < colspan)
     {
        ERR("col + colspan > 0xffff");
        return;
     }
   if ((col + colspan) >= 0x7ffff)
     {
        WRN("col + colspan getting rather large (>32767)");
     }
   if (row < 0)
     {
        ERR("row < 0");
        return;
     }
   if (rowspan < 1)
     {
        ERR("rowspan < 1");
        return;
     }
   if ((0xffff - row) < rowspan)
     {
        ERR("row + rowspan > 0xffff");
        return;
     }
   if ((row + rowspan) >= 0x7ffff)
     {
        WRN("row + rowspan getting rather large (>32767)");
     }

   elm_widget_sub_object_add(obj, subobj);
   evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static void
_elm_table_unpack(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _elm_widget_sub_object_redirect_to_top(obj, subobj);
   evas_object_table_unpack(wd->resize_obj, subobj);
}

EAPI void
elm_table_pack_set(Evas_Object *subobj,
                   int col,
                   int row,
                   int colspan,
                   int rowspan)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_TABLE_CHECK(obj);
   elm_obj_table_pack_set(obj, subobj, col, row, colspan, rowspan);
}

EOLIAN static void
_elm_table_pack_set(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj, int col, int row, int colspan, int rowspan)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
   efl_ui_focus_composition_dirty(obj);
}

EAPI void
elm_table_pack_get(Evas_Object *subobj,
                   int *col,
                   int *row,
                   int *colspan,
                   int *rowspan)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);
   ELM_TABLE_CHECK(obj);
   elm_obj_table_pack_get(obj, subobj, col, row, colspan, rowspan);
}

EOLIAN static void
_elm_table_pack_get(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short icol, irow, icolspan, irowspan;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_pack_get
     (wd->resize_obj, subobj, &icol, &irow, &icolspan, &irowspan);
   if (col) *col = icol;
   if (row) *row = irow;
   if (colspan) *colspan = icolspan;
   if (rowspan) *rowspan = irowspan;
}

EOLIAN static void
_elm_table_clear(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool clear)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_clear(wd->resize_obj, clear);
   efl_ui_focus_composition_dirty(obj);
}

EOLIAN static Evas_Object*
_elm_table_child_get(const Eo *obj, void *_pd EINA_UNUSED, int col, int row)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return evas_object_table_child_get(wd->resize_obj, col, row);
}

EOLIAN static void
_elm_table_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN void
_elm_table_efl_canvas_group_group_calculate(Eo *obj, void *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_smart_calculate(wd->resize_obj);
}

/* Internal EO APIs and hidden overrides */

#define ELM_TABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_table)

#include "elm_table_eo.c"
