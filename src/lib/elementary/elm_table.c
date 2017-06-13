#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include <elm_table.eo.h>
#include "elm_priv.h"
#include "elm_widget_table.h"

#define MY_CLASS ELM_TABLE_CLASS

#define MY_CLASS_NAME "Elm_Table"
#define MY_CLASS_NAME_LEGACY "elm_table"

static void
_focus_order_flush(Eo *obj)
{
   Elm_Widget_Smart_Data *wpd = efl_data_scope_get(obj, ELM_WIDGET_CLASS);
   Eina_List *order = evas_object_table_children_get(wpd->resize_obj);

   efl_ui_focus_manager_update_order(wpd->focus.manager, obj, order);
}

EOLIAN static Eina_Bool
_elm_table_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_table_elm_widget_focus_next(Eo *obj, void *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Eina_Bool int_ret;

   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

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
        items = evas_object_table_children_get
            (wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next, next_item);

   if (list_free) list_free((Eina_List *)items);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_table_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_table_elm_widget_focus_direction(Eo *obj, void *_pd EINA_UNUSED, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   Eina_Bool int_ret;

   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

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
        items = evas_object_table_children_get
            (wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_direction_get
       (obj, base, items, list_data_get, degree, direction, direction_item, weight);

   if (list_free)
     list_free((Eina_List *)items);

   return int_ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_mirrored_set(wd->resize_obj, rtl);
}

EOLIAN static Elm_Theme_Apply
_elm_table_elm_widget_theme_apply(Eo *obj, void *sd EINA_UNUSED)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   return int_ret;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_gfx_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get(wd->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
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
_elm_table_elm_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *child)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), child);
   if (!int_ret) return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_table_efl_canvas_group_group_add(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *table;

   elm_widget_sub_object_parent_add(obj);

   table = evas_object_table_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, table, EINA_TRUE);

   evas_object_event_callback_add
     (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   elm_obj_widget_theme_apply(obj);
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
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_table_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FILLER);

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
_elm_table_homogeneous_get(Eo *obj, void *_pd EINA_UNUSED)
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
_elm_table_padding_get(Eo *obj, void *_pd EINA_UNUSED, Evas_Coord *horizontal, Evas_Coord *vertical)
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
_elm_table_align_get(Eo *obj, void *_pd EINA_UNUSED, double *horizontal, double *vertical)
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
   _focus_order_flush(obj);
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
   _focus_order_flush(obj);
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
   _focus_order_flush(obj);
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

EOLIAN Eina_Bool
_elm_table_elm_widget_focus_register(Eo *obj, void *pd EINA_UNUSED, Efl_Ui_Focus_Manager *manager, Efl_Ui_Focus_Object *logical, Eina_Bool full)
{
   Eina_Bool result = elm_obj_widget_focus_register(efl_super(obj, MY_CLASS), manager, logical, full);

   //later registering children are automatically set into the order of the internal table
   _focus_order_flush(obj);

   return result;
}


/* Internal EO APIs and hidden overrides */

#define ELM_TABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_table)

#include "elm_table.eo.c"
