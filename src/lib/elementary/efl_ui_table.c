#include "efl_ui_table_private.h"

#include "../evas/canvas/evas_table.eo.h"

#define MY_CLASS EFL_UI_TABLE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Table"
#define MY_CLASS_NAME_LEGACY "elm_grid"

typedef struct _Custom_Table_Data Custom_Table_Data;

static void _subobj_del_cb(void *data, const Efl_Event *event);
static void _item_remove(Efl_Ui_Table *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj);

struct _Custom_Table_Data
{
   Efl_Ui_Table      *parent;
   Efl_Ui_Table_Data *gd;
};

EFL_CALLBACKS_ARRAY_DEFINE(subobj_callbacks,
                          { EFL_EVENT_DEL, _subobj_del_cb });

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_mirrored_set(wd->resize_obj, rtl);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_table_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   return int_ret;
}

static void
_layout_updated_emit(Efl_Ui_Table *obj)
{
   efl_event_callback_legacy_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (!efl_alive_get(obj)) return;

   evas_object_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
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
   Efl_Ui_Table_Data *pd = efl_data_scope_get(data, MY_CLASS);

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
   EO_VERSION, "Efl.Ui.Table.Internal", EFL_CLASS_TYPE_REGULAR,
   sizeof(Custom_Table_Data), _custom_table_initializer, NULL, NULL
};

EFL_DEFINE_CLASS(_efl_ui_table_custom_table_class_get, &custom_table_class_desc,
                EVAS_TABLE_CLASS, NULL)

#define CUSTOM_TABLE_CLASS _efl_ui_table_custom_table_class_get()

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
_efl_ui_table_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Table_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   _sizing_eval(obj, pd);
   efl_canvas_group_calculate(efl_super(wd->resize_obj, CUSTOM_TABLE_CLASS));
}

EOLIAN void
_efl_ui_table_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_table_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Table_Data *pd)
{
   Custom_Table_Data *custom;
   Evas_Object *table;

   elm_widget_sub_object_parent_add(obj);

   table = efl_add(CUSTOM_TABLE_CLASS, obj);
   custom = efl_data_scope_get(table, CUSTOM_TABLE_CLASS);
   custom->gd = pd;
   custom->parent = obj;

   evas_object_table_homogeneous_set(table, EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE);
   elm_widget_resize_object_set(obj, table);

   evas_object_event_callback_add
         (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _table_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static void
_efl_ui_table_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
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
_efl_ui_table_efl_object_constructor(Eo *obj, Efl_Ui_Table_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->dir1 = EFL_UI_DIR_RIGHT;
   pd->dir2 = EFL_UI_DIR_DOWN;
   pd->last_col = -1;
   pd->last_row = -1;
   pd->req_cols = 0;
   pd->req_rows = 0;

   return obj;
}



EOLIAN static void
_efl_ui_table_efl_pack_pack_padding_set(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, double h, double v, Eina_Bool scalable)
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
_efl_ui_table_efl_pack_pack_padding_get(const Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, double *h, double *v, Eina_Bool *scalable)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (scalable) *scalable = pd->pad.scalable;
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

static void
_subobj_del_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Table *obj = data;
   Efl_Ui_Table_Data *pd = efl_data_scope_get(obj, EFL_UI_TABLE_CLASS);

   efl_event_callback_array_del(event->object, subobj_callbacks(), data);
   _item_remove(obj, pd, event->object);

   if (!elm_widget_sub_object_del(obj, event->object))
     WRN("failed to remove child from its parent");
}

static Eina_Bool
_pack_at(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj,
         int col, int row, int colspan, int rowspan, Eina_Bool linear)
{
   Table_Item *gi = NULL;

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
        ERR("table requested size exceeded! packing in extra cell at "
            "%d,%d %dx%d (table: %dx%d)",
            col, row, colspan, rowspan, pd->req_cols, pd->req_rows);
     }

   if (obj == elm_widget_parent_widget_get(subobj))
     {
        gi = efl_key_data_get(subobj, TABLE_ITEM_KEY);
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
        pd->items = (Table_Item *)
              eina_inlist_append(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));

        efl_key_data_set(subobj, TABLE_ITEM_KEY, gi);
        elm_widget_sub_object_add(obj, subobj);
        efl_event_callback_legacy_call(obj, EFL_CONTAINER_EVENT_CONTENT_ADDED, subobj);
        efl_event_callback_array_add(subobj, subobj_callbacks(), obj);
     }

   return evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_table_pack_table(Eo *obj, Efl_Ui_Table_Data *pd,
                                     Efl_Gfx_Entity *subobj,
                                     int col, int row, int colspan, int rowspan)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   return _pack_at(obj, pd, subobj, col, row, colspan, rowspan, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_table_table_position_get(const Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan)
{
   int c = -1, r = -1, cs = 0, rs = 0;
   Table_Item *gi;
   Eina_Bool ret = EINA_FALSE;

   if (obj != elm_widget_parent_widget_get(subobj))
     {
        if (efl_invalidated_get(subobj)) goto end;
        ERR("%p is not a child of %p", subobj, obj);
        goto end;
     }

   gi = efl_key_data_get(subobj, TABLE_ITEM_KEY);
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

EOLIAN static Efl_Gfx_Entity *
_efl_ui_table_efl_pack_table_table_content_get(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, int col, int row)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   return evas_object_table_child_get(wd->resize_obj, col, row);
}

static void
_item_remove(Efl_Ui_Table *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   Table_Item *gi = efl_key_data_get(subobj, TABLE_ITEM_KEY);
   Table_Item *gi2, *last = NULL;

   if (!gi)
     {
        WRN("item %p has no table internal data", subobj);
        EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), gi)
          if (gi->object == subobj)
            break;
        if (!gi)
          {
             ERR("item %p was not found in this table", subobj);
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
        if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
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
   pd->items = (Table_Item *)
         eina_inlist_remove(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));
   pd->count--;
   efl_key_data_set(subobj, TABLE_ITEM_KEY, NULL);
   free(gi);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_unpack(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
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
_efl_ui_table_efl_pack_pack_clear(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_table_clear(wd->resize_obj, EINA_TRUE);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_unpack_all(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   evas_object_table_clear(wd->resize_obj, EINA_FALSE);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_table_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   evas_object_smart_need_recalculate_set(obj, EINA_TRUE);
}

static Eina_Bool
_table_item_iterator_next(Table_Item_Iterator *it, void **data)
{
   Efl_Gfx_Entity *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_table_item_iterator_get_container(Table_Item_Iterator *it)
{
   return it->object;
}

static void
_table_item_iterator_free(Table_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   free(it);
}

static inline Eina_Iterator *
_table_item_iterator_create(Eo *obj, Eina_List *list)
{
   Table_Item_Iterator *it;
   if (!list) return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = list;
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_table_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_table_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_table_item_iterator_free);
   it->object = obj;

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_ui_table_efl_container_content_iterate(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   Eina_List *list;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   list = evas_object_table_children_get(wd->resize_obj);
   return _table_item_iterator_create(obj, list);
}

EOLIAN static int
_efl_ui_table_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
{
   return pd->count;
}

EOLIAN static Eina_Iterator *
_efl_ui_table_efl_pack_table_table_contents_get(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED,
                                                int col, int row, Eina_Bool below)
{
   Eina_List *list, *atlist = NULL;
   Evas_Object *sobj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   list = evas_object_table_children_get(wd->resize_obj);
   EINA_LIST_FREE(list, sobj)
     {
        Table_Item *gi = efl_key_data_get(sobj, TABLE_ITEM_KEY);
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

   return _table_item_iterator_create(obj, atlist);
}

EOLIAN static void
_efl_ui_table_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Ui_Dir dir)
{
   pd->dir1 = dir;

   /* if both directions are either horizontal or vertical, need to adjust
    * secondary direction (dir2) */
   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE) ==
       efl_ui_dir_is_horizontal(pd->dir2, EINA_FALSE))
     {
        if (!efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
          pd->dir2 = EFL_UI_DIR_RIGHT;
        else
          pd->dir2 = EFL_UI_DIR_DOWN;
     }

   efl_pack_layout_request(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_table_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
{
   return pd->dir1;
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_direction_set(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Ui_Dir primary, Efl_Ui_Dir secondary)
{
   pd->dir1 = primary;
   pd->dir2 = secondary;

   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE) ==
       efl_ui_dir_is_horizontal(pd->dir2, EINA_FALSE))
     {
        ERR("specified two directions in the same axis, secondary directions "
            " is reset to a valid default");
        if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
          pd->dir2 = EFL_UI_DIR_DOWN;
        else
          pd->dir2 = EFL_UI_DIR_RIGHT;
     }

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, Efl_Ui_Dir *primary, Efl_Ui_Dir *secondary)
{
   if (primary) *primary = pd->dir1;
   if (secondary) *secondary = pd->dir2;
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_size_set(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, int cols, int rows)
{
   if (cols < 0) cols = 0;
   if (rows < 0) rows = 0;

   pd->req_cols = cols;
   pd->req_rows = rows;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd EINA_UNUSED, int *cols, int *rows)
{
   if (cols) *cols = efl_pack_table_columns_get(obj);
   if (rows) *rows = efl_pack_table_rows_get(obj);
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_columns_set(Eo *obj, Efl_Ui_Table_Data *pd, int columns)
{
   pd->req_cols = columns;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_table_efl_pack_table_table_columns_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
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
_efl_ui_table_efl_pack_table_table_rows_set(Eo *obj, Efl_Ui_Table_Data *pd, int rows)
{
   pd->req_rows = rows;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_table_efl_pack_table_table_rows_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
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
_efl_ui_table_efl_pack_pack(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   /* this is just an alias */
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   int col = pd->last_col;
   int row = pd->last_row;

   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
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

#define EFL_UI_TABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_table)

#include "efl_ui_table.eo.c"
