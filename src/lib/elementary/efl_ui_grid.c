#include "efl_ui_grid_private.h"

#include "../evas/canvas/evas_table.eo.h"

#define MY_CLASS EFL_UI_GRID_CLASS
#define MY_CLASS_NAME "Efl.Ui.Grid"
#define MY_CLASS_NAME_LEGACY "elm_grid"

typedef struct _Custom_Table_Data Custom_Table_Data;

static void _subobj_del_cb(void *data, const Efl_Event *event);
static void _item_remove(Efl_Ui_Grid *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx *subobj);

struct _Custom_Table_Data
{
   Efl_Ui_Grid      *parent;
   Efl_Ui_Grid_Data *gd;
};

EFL_CALLBACKS_ARRAY_DEFINE(subobj_callbacks,
                          { EFL_EVENT_DEL, _subobj_del_cb });

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

EOLIAN static Elm_Theme_Apply
_efl_ui_grid_elm_widget_theme_apply(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   return int_ret;
}

static void
_layout_updated_emit(Efl_Ui_Grid *obj)
{
   efl_event_callback_legacy_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_gfx_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get(wd->resize_obj, &maxw, &maxh);
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
_table_size_hints_changed(void *data, Evas *e EINA_UNUSED,
                          Evas_Object *table EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Efl_Ui_Grid_Data *pd = efl_data_scope_get(data, MY_CLASS);

   _sizing_eval(data, pd);
}

/* Custom table class: overrides smart_calculate. */
static void _custom_table_calc(Eo *obj, Custom_Table_Data *pd);

static Eina_Bool
_custom_table_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(class_ops,
         EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _custom_table_calc)
   );

   return efl_class_functions_set(klass, NULL, &class_ops);
};

static const Efl_Class_Description custom_table_class_desc = {
   EO_VERSION, "Efl.Ui.Grid.Internal", EFL_CLASS_TYPE_REGULAR,
   sizeof(Custom_Table_Data), _custom_table_initializer, NULL, NULL
};

EFL_DEFINE_CLASS(_efl_ui_grid_custom_table_class_get, &custom_table_class_desc,
                EVAS_TABLE_CLASS, NULL)

#define CUSTOM_TABLE_CLASS _efl_ui_grid_custom_table_class_get()

static void
_custom_table_calc(Eo *obj, Custom_Table_Data *pd)
{
   int cols, rows;

   evas_object_table_col_row_size_get(obj, &cols, &rows);
   if ((cols < 1) || (rows < 1)) return;

   efl_pack_layout_update(pd->parent);
   _layout_updated_emit(pd->parent);
}
/* End of custom table class */

EOLIAN static void
_efl_ui_grid_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _sizing_eval(obj, pd);
   efl_canvas_group_calculate(efl_super(wd->resize_obj, CUSTOM_TABLE_CLASS));
}

EOLIAN void
_efl_ui_grid_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_grid_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   Custom_Table_Data *custom;
   Evas_Object *table;

   elm_widget_sub_object_parent_add(obj);

   table = efl_add(CUSTOM_TABLE_CLASS, obj);
   custom = efl_data_scope_get(table, CUSTOM_TABLE_CLASS);
   custom->gd = pd;
   custom->parent = obj;

   evas_object_table_homogeneous_set(table, EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE);
   elm_widget_resize_object_set(obj, table, EINA_TRUE);

   evas_object_event_callback_add
         (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _table_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static void
_efl_ui_grid_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_event_callback_del_full
         (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _table_size_hints_changed, obj);

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

EOLIAN static Eo *
_efl_ui_grid_efl_object_constructor(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FILLER);

   pd->dir1 = EFL_ORIENT_RIGHT;
   pd->dir2 = EFL_ORIENT_DOWN;
   pd->last_col = -1;
   pd->last_row = -1;
   pd->req_cols = 0;
   pd->req_rows = 0;

   return obj;
}



EOLIAN static void
_efl_ui_grid_efl_pack_pack_padding_set(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, double h, double v, Eina_Bool scalable)
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
_efl_ui_grid_efl_pack_pack_padding_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, double *h, double *v, Eina_Bool *scalable)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (scalable) *scalable = pd->pad.scalable;
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

static void
_subobj_del_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Grid *obj = data;
   Efl_Ui_Grid_Data *pd = efl_data_scope_get(obj, EFL_UI_GRID_CLASS);

   efl_event_callback_array_del(event->object, subobj_callbacks(), data);
   _item_remove(obj, pd, event->object);

   if (!elm_widget_sub_object_del(obj, event->object))
     WRN("failed to remove child from its parent");
}

static Eina_Bool
_pack_at(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx *subobj,
         int col, int row, int colspan, int rowspan, Eina_Bool linear)
{
   Grid_Item *gi = NULL;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (col < 0) col = 0;
   if (row < 0) row = 0;

   // note: we could have colspan = -1 mean "full width" if req_cols is set?
   if (colspan < 1) colspan = 1;
   if (rowspan < 1) rowspan = 1;

   if (((int64_t) col + (int64_t) colspan) > (int64_t) INT_MAX)
     colspan = INT_MAX - col;

   if (((int64_t) row + (int64_t) rowspan) > (int64_t) INT_MAX)
     rowspan = INT_MAX - row;

   if ((pd->req_cols && ((col + colspan) > pd->req_cols)) ||
       (pd->req_rows && ((row + rowspan) > pd->req_rows)))
     {
        ERR("grid requested size exceeded! packing in extra cell at "
            "%d,%d %dx%d (grid: %dx%d)",
            col, row, colspan, rowspan, pd->req_cols, pd->req_rows);
     }

   if (obj == elm_widget_parent_widget_get(subobj))
     {
        gi = efl_key_data_get(subobj, GRID_ITEM_KEY);
        if (gi)
          {
             gi->col = col;
             gi->row = row;
             gi->col_span = colspan;
             gi->row_span = rowspan;
             gi->linear = EINA_FALSE;
          }
        else ERR("object is a child but internal data was not found!");
     }

   if (!gi)
     {
        gi = calloc(1, sizeof(*gi));
        gi->col = col;
        gi->row = row;
        gi->col_span = colspan;
        gi->row_span = rowspan;
        gi->linear = !!linear;
        gi->object = subobj; // xref(, obj);
        pd->count++;
        pd->items = (Grid_Item *)
              eina_inlist_append(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));

        efl_key_data_set(subobj, GRID_ITEM_KEY, gi);
        elm_widget_sub_object_add(obj, subobj);
        efl_event_callback_legacy_call(obj, EFL_CONTAINER_EVENT_CONTENT_ADDED, subobj);
        efl_event_callback_array_add(subobj, subobj_callbacks(), obj);
     }

   return evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_grid_pack_grid(Eo *obj, Efl_Ui_Grid_Data *pd,
                                     Efl_Gfx *subobj,
                                     int col, int row, int colspan, int rowspan)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   return _pack_at(obj, pd, subobj, col, row, colspan, rowspan, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_grid_grid_position_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan)
{
   int c = -1, r = -1, cs = 0, rs = 0;
   Grid_Item *gi;
   Eina_Bool ret = EINA_FALSE;

   if (obj != elm_widget_parent_widget_get(subobj))
     {
        ERR("%p is not a child of %p", subobj, obj);
        goto end;
     }

   gi = efl_key_data_get(subobj, GRID_ITEM_KEY);
   if (gi)
     {
        c = gi->col;
        r = gi->row;
        cs = gi->col_span;
        rs = gi->row_span;
     }

   ret = EINA_TRUE;

end:
   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;
   return ret;
}

EOLIAN static Efl_Gfx *
_efl_ui_grid_efl_pack_grid_grid_content_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, int col, int row)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return evas_object_table_child_get(wd->resize_obj, col, row);
}

static void
_item_remove(Efl_Ui_Grid *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx *subobj)
{
   Grid_Item *gi = efl_key_data_get(subobj, GRID_ITEM_KEY);
   Grid_Item *gi2, *last = NULL;

   if (!gi)
     {
        WRN("item %p has no grid internal data", subobj);
        EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), gi)
          if (gi->object == subobj)
            break;
        if (!gi)
          {
             ERR("item %p was not found in this grid", subobj);
             return;
          }
     }

   if (!gi->linear)
     goto end;

   EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(pd->items), gi2)
     {
        if (gi2 == gi) continue;
        if (!gi2->linear) continue;
        last = gi2;
        break;
     }
   if (last)
     {
        if (_horiz(pd->dir1))
          {
             pd->last_col = last->col + last->col_span - 1;
             pd->last_row = last->row;
          }
        else
          {
             pd->last_row = last->row + last->row_span - 1;
             pd->last_col = last->col;
          }
     }
   else
     {
        pd->last_col = -1;
        pd->last_row = -1;
     }

end:
   efl_event_callback_legacy_call(obj, EFL_CONTAINER_EVENT_CONTENT_REMOVED, subobj);
   pd->items = (Grid_Item *)
         eina_inlist_remove(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));
   pd->count--;
   efl_key_data_set(subobj, GRID_ITEM_KEY, NULL);
   free(gi);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_unpack(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   _item_remove(obj, pd, subobj);
   if (evas_object_table_unpack(wd->resize_obj, subobj))
     {
        if (elm_widget_sub_object_del(obj, subobj))
          return EINA_TRUE;
        return EINA_FALSE; // oops - unlikely
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_pack_clear(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_table_clear(wd->resize_obj, EINA_TRUE);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_unpack_all(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_table_clear(wd->resize_obj, EINA_FALSE);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_grid_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   evas_object_smart_need_recalculate_set(obj, EINA_TRUE);
}

static Eina_Bool
_grid_item_iterator_next(Grid_Item_Iterator *it, void **data)
{
   Efl_Gfx *sub;

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

static inline Eina_Iterator *
_grid_item_iterator_create(Eo *obj, Eina_List *list)
{
   Grid_Item_Iterator *it;
   if (!list) return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = list;
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_grid_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_grid_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_grid_item_iterator_free);
   it->object = obj;

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_ui_grid_efl_container_content_iterate(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   Eina_List *list;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   list = evas_object_table_children_get(wd->resize_obj);
   return _grid_item_iterator_create(obj, list);
}

EOLIAN static int
_efl_ui_grid_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->count;
}

EOLIAN static Eina_Iterator *
_efl_ui_grid_efl_pack_grid_grid_contents_get(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED,
                                                int col, int row, Eina_Bool below)
{
   Eina_List *list, *atlist = NULL;
   Evas_Object *sobj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   list = evas_object_table_children_get(wd->resize_obj);
   EINA_LIST_FREE(list, sobj)
     {
        Grid_Item *gi = efl_key_data_get(sobj, GRID_ITEM_KEY);
        if (!gi) continue;

        if ((gi->col == col) && (gi->row == row))
          atlist = eina_list_append(atlist, sobj);
        else if (below)
          {
             if ((gi->col <= col) && (gi->col + gi->col_span >= col) &&
                 (gi->row <= row) && (gi->row + gi->row_span >= row))
               atlist = eina_list_append(atlist, sobj);
          }
     }

   return _grid_item_iterator_create(obj, atlist);
}

EOLIAN static void
_efl_ui_grid_efl_orientation_orientation_set(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Orient orient)
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
_efl_ui_grid_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->dir1;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_orientation_set(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Orient primary, Efl_Orient secondary)
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
_efl_ui_grid_efl_pack_grid_grid_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Efl_Orient *primary, Efl_Orient *secondary)
{
   if (primary) *primary = pd->dir1;
   if (secondary) *secondary = pd->dir2;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_size_set(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, int cols, int rows)
{
   if (cols < 0) cols = 0;
   if (rows < 0) rows = 0;

   pd->req_cols = cols;
   pd->req_rows = rows;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd EINA_UNUSED, int *cols, int *rows)
{
   if (cols) *cols = efl_pack_grid_columns_get(obj);
   if (rows) *rows = efl_pack_grid_rows_get(obj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_columns_set(Eo *obj, Efl_Ui_Grid_Data *pd, int columns)
{
   pd->req_cols = columns;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_grid_efl_pack_grid_grid_columns_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   if (!pd->req_cols)
     {
        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);
        int cols;
        evas_object_table_col_row_size_get(wd->resize_obj, &cols, NULL);
        return cols;
     }
   return pd->req_cols;
}

EOLIAN static void
_efl_ui_grid_efl_pack_grid_grid_rows_set(Eo *obj, Efl_Ui_Grid_Data *pd, int rows)
{
   pd->req_rows = rows;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_grid_efl_pack_grid_grid_rows_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   if (!pd->req_rows)
     {
        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);
        int rows;
        evas_object_table_col_row_size_get(wd->resize_obj, &rows, NULL);
        return rows;
     }
   return pd->req_rows;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_pack(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Efl_Gfx *subobj)
{
   /* this is just an alias */
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx *subobj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   int col = pd->last_col;
   int row = pd->last_row;

   if (_horiz(pd->dir1))
     {
        col++;
        if (pd->req_cols && (col >= pd->req_cols))
          {
             col = 0;
             row++;
          }
        if (row < 0) row = 0;
     }
   else
     {
        row++;
        if (pd->req_rows && (row >= pd->req_rows))
          {
             row = 0;
             col++;
          }
        if (col < 0) col = 0;
     }

   pd->last_col = col;
   pd->last_row = row;

   DBG("packing new obj at %d,%d", col, row);
   return _pack_at(obj, pd, subobj, col, row, 1, 1, EINA_TRUE);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_GRID_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_grid)

#include "efl_ui_grid.eo.c"
