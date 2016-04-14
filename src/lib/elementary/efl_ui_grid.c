#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_grid.eo.h"

#define MY_CLASS EFL_UI_GRID_CLASS
#define MY_CLASS_NAME "Efl.Ui.Grid"

typedef struct _Efl_Ui_Grid_Data Efl_Ui_Grid_Data;
typedef struct _Grid_Item_Iterator Grid_Item_Iterator;
typedef struct _Grid_Item Grid_Item;

struct _Grid_Item
{
   EINA_INLIST;

   Efl_Pack_Item *object;
   int colspan, rowspan;
   int col, row;

   Eina_Bool linear : 1;
};

struct _Efl_Ui_Grid_Data
{
   Grid_Item *items;

   int cols, rows; // requested
   int max_span;
   int lastcol, lastrow;
   Efl_Orient dir1, dir2;
   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;
   Eina_Bool linear_recalc : 1;
};

struct _Grid_Item_Iterator
{
   Eina_List     *list;
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Efl_Ui_Grid    *object;
};

static inline Eina_Bool
_horiz(Efl_Orient dir)
{
   return dir % 180 == EFL_ORIENT_RIGHT;
}

EOLIAN static Eina_Bool
_efl_ui_grid_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_elm_widget_focus_next(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
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
_efl_ui_grid_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_elm_widget_focus_direction(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
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

EOLIAN static Eina_Bool
_efl_ui_grid_elm_widget_theme_apply(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_theme_apply(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return EINA_TRUE;
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_size_hint_min_get
     (wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get
     (wd->resize_obj, &maxw, &maxh);
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
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Efl_Ui_Grid_Data *pd = eo_data_scope_get(data, MY_CLASS);

   _sizing_eval(data, pd);
}

EOLIAN static void
_efl_ui_grid_evas_object_smart_add(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Evas_Object *table;

   elm_widget_sub_object_parent_add(obj);

   table = evas_object_table_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, table, EINA_TRUE);

   evas_object_event_callback_add
     (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   evas_obj_smart_add(eo_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static void
_efl_ui_grid_evas_object_smart_del(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
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

   evas_obj_smart_del(eo_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_grid_eo_base_constructor(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   evas_obj_type_set(obj, MY_CLASS_NAME);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FILLER);

   pd->dir1 = EFL_ORIENT_RIGHT;
   pd->dir2 = EFL_ORIENT_DOWN;
   pd->lastcol = -1;
   pd->lastrow = -1;

   return obj;
}

EOLIAN static void
_efl_ui_grid_efl_pack_padding_set(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, double h, double v, Eina_Bool scalable)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (h < 0) h = 0;
   if (v < 0) v = 0;
   pd->pad.h = h;
   pd->pad.v = v;
   pd->pad.scalable = !!scalable;
   if (pd->pad.scalable)
     {
        double scale = elm_object_scale_get(obj);
        evas_object_table_padding_set(wd->resize_obj, h * scale, v * scale);
     }
   else
     evas_object_table_padding_set(wd->resize_obj, h, v);
}

EOLIAN static void
_efl_ui_grid_efl_pack_padding_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, double *h, double *v, Eina_Bool *scalable)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (scalable) *scalable = pd->pad.scalable;
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_pack_grid(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Pack_Item *subobj, int col, int row, int colspan, int rowspan)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (col < 0) col = 0;
   if (row < 0) row = 0;
   if (colspan < 1) colspan = 1;
   if (rowspan < 1) rowspan = 1;

   if ((0xffff - col) < colspan)
     {
        ERR("col + colspan > 0xffff. adjusted");
        colspan = 0xffff - col;
     }
   if ((0xffff - row) < rowspan)
     {
        ERR("row + rowspan > 0xffff, adjusted");
        rowspan = 0xffff - row;
     }

   if ((col + colspan) >= 0x7ffff)
     WRN("col + colspan getting rather large (>32767)");
   if ((row + rowspan) >= 0x7ffff)
     WRN("row + rowspan getting rather large (>32767)");

   if ((pd->lastcol < (col + colspan - 1)) ||
       (pd->lastrow < (row + rowspan - 1)))
     {
        pd->lastcol = col + colspan - 1;
        pd->lastrow = row + rowspan - 1;
     }

   elm_widget_sub_object_add(obj, subobj);
   evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_child_position_set(Eo *obj, Efl_Ui_Grid_Data *pd, Evas_Object *subobj, int col, int row, int colspan, int rowspan)
{
   _efl_ui_grid_efl_pack_grid_pack_grid(obj, pd, subobj, col, row, colspan, rowspan);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_child_position_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan)
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

EOLIAN static Efl_Pack_Item *
_efl_ui_grid_efl_pack_grid_grid_child_at(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, int col, int row)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return evas_object_table_child_get(wd->resize_obj, col, row);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_unpack(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Efl_Pack_Item *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (evas_object_table_unpack(wd->resize_obj, subobj))
     {
        if (elm_widget_sub_object_del(obj, subobj))
          return EINA_TRUE;
        return EINA_FALSE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_grid_efl_pack_clear(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_clear(wd->resize_obj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_grid_efl_pack_unpack_all(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_clear(wd->resize_obj, EINA_FALSE);
}

EOLIAN void
_efl_ui_grid_evas_object_smart_calculate(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_smart_calculate(wd->resize_obj);
}

EOLIAN void
_efl_ui_grid_efl_pack_layout_update(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   _sizing_eval(obj, pd);
}

EOLIAN void
_efl_ui_grid_efl_pack_layout_request(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   evas_object_smart_need_recalculate_set(obj, EINA_TRUE);
}

static Eina_Bool
_grid_item_iterator_next(Grid_Item_Iterator *it, void **data)
{
   Efl_Pack_Item *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Elm_Layout *
_grid_item_iterator_get_container(Grid_Item_Iterator *it)
{
   return it->object;
}

static void
_grid_item_iterator_free(Grid_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_ui_grid_efl_pack_contents_iterate(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Grid_Item_Iterator *it;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = evas_object_table_children_get(wd->resize_obj);
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_grid_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_grid_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_grid_item_iterator_free);
   it->object = obj;

   return &it->iterator;
}

EOLIAN static int
_efl_ui_grid_efl_pack_contents_count(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Eina_List *li;
   int count;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   /* FIXME */
   li = evas_object_table_children_get(wd->resize_obj);
   count = eina_list_count(li);
   eina_list_free(li);

   return count;
}

EOLIAN static Eina_List *
_efl_ui_grid_efl_pack_grid_grid_children_at(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, int col, int row)
{
   Eina_List *l = NULL;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   /* FIXME: only one item returned */
   return eina_list_append(l, evas_object_table_child_get(wd->resize_obj, col, row));
}

EOLIAN static void
_efl_ui_grid_efl_pack_linear_direction_set(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Orient orient)
{
   EINA_SAFETY_ON_FALSE_RETURN((orient % 90) == 0);

   pd->dir1 = orient;

   /* if both orientations are either horizontal or vertical, need to adjust
    * secondary orientation (dir2) */
   if (_horiz(pd->dir1) == _horiz(pd->dir2))
     {
        if (!_horiz(pd->dir1))
          pd->dir2 = EFL_ORIENT_RIGHT;
        else
          pd->dir2 = EFL_ORIENT_DOWN;
     }

   efl_pack_layout_request(obj);
}

EOLIAN static Efl_Orient
_efl_ui_grid_efl_pack_linear_direction_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->dir1;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_directions_set(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Orient primary, Efl_Orient secondary)
{
   EINA_SAFETY_ON_FALSE_RETURN((primary % 90) == 0);
   EINA_SAFETY_ON_FALSE_RETURN((secondary % 90) == 0);

   pd->dir1 = primary;
   pd->dir2 = secondary;

   if (_horiz(pd->dir1) == _horiz(pd->dir2))
     {
        ERR("specified two orientations in the same axis, secondary orientation "
            " is reset to a valid default");
        switch (pd->dir1)
          {
           case EFL_ORIENT_DOWN:
           case EFL_ORIENT_UP:
             pd->dir2 = EFL_ORIENT_RIGHT;
             break;
           default:
             pd->dir2 = EFL_ORIENT_DOWN;
             break;
          }
     }

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_directions_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Efl_Orient *primary, Efl_Orient *secondary)
{
   if (primary) *primary = pd->dir1;
   if (secondary) *secondary = pd->dir2;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_size_set(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, int cols, int rows)
{
   /* FIXME: what's the behaviour if items were packed OUTSIDE this box? */

   if (cols < 0) cols = 0;
   if (rows < 0) rows = 0;

   efl_pack_columns_set(obj, cols);
   efl_pack_rows_set(obj, rows);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd EINA_UNUSED, int *cols, int *rows)
{
   if (cols) *cols = efl_pack_columns_get(obj);
   if (rows) *rows = efl_pack_rows_get(obj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_max_span_set(Eo *obj, Efl_Ui_Grid_Data *pd, int maxx)
{
   /* FIXME: what's the behaviour if items were packed OUTSIDE this range? */

   if (maxx < 0) maxx = 0;
   pd->max_span = maxx;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_grid_efl_pack_grid_max_span_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->max_span;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_columns_set(Eo *obj, Efl_Ui_Grid_Data *pd, int columns)
{
   /* FIXME: what's the behaviour if items were packed OUTSIDE this range? */
   pd->cols = columns;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_grid_efl_pack_grid_columns_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->cols;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_rows_set(Eo *obj, Efl_Ui_Grid_Data *pd, int rows)
{
   /* FIXME: what's the behaviour if items were packed OUTSIDE this range? */
   pd->rows = rows;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_grid_efl_pack_grid_rows_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->rows;
}

EOLIAN static void
_efl_ui_grid_efl_pack_pack(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Efl_Pack_Item *subobj)
{
   /* this is just an alias */
   efl_pack_end(obj, subobj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Pack_Item *subobj)
{
   EINA_SAFETY_ON_NULL_RETURN(subobj);

   int col = pd->lastcol;
   int row = pd->lastrow;

   if (_horiz(pd->dir1))
     {
        col++;
        if (pd->max_span && (col >= pd->max_span))
          {
             col = 0;
             row++;
          }
        if (row < 0) row = 0;
     }
   else
     {
        row++;
        if (pd->max_span && (row >= pd->max_span))
          {
             row = 0;
             col++;
          }
        if (col < 0) col = 0;
     }

   DBG("packing new obj at %d,%d", col, row);
   pd->lastcol = col;
   pd->lastrow = row;
   efl_pack_grid(obj, subobj, col, row, 1, 1);
}

#include "efl_ui_grid.eo.c"
