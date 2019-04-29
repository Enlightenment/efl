#include "efl_ui_table_private.h"

#define MY_CLASS EFL_UI_TABLE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Table"

#define EFL_UI_TABLE_DATA_GET(o, pd) \
   Efl_Ui_Table_Data *pd = efl_data_scope_get(o, EFL_UI_TABLE_CLASS)

inline static Table_Item *
_efl_ui_table_item_date_get(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   Table_Item *gi;
   if (!efl_invalidated_get(subobj) &&
       (obj != efl_canvas_object_render_parent_get(subobj)))
     {
        ERR("%p is not a child of %p", subobj, obj);
        return NULL;
     }
   gi = efl_key_data_get(subobj, TABLE_ITEM_KEY);
   if (!gi)
     {
        WRN("item %p has no table internal data", subobj);
        EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), gi)
          {
             if (gi->object == subobj)
               break;
          }
        if (!gi)
          {
             ERR("item %p was not found in this table", subobj);
             return NULL;
          }
     }
   return gi;
}

static void
_on_child_size_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *table = data;
   efl_pack_layout_request(table);
}

static void
_on_child_hints_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *table = data;
   efl_pack_layout_request(table);
}

static void
_on_child_del(void *data, const Efl_Event *event)
{
   Eo *table = data;
   Table_Item *gi;
   EFL_UI_TABLE_DATA_GET(table, pd);

   gi = _efl_ui_table_item_date_get(table, pd, event->object);
   if (!gi) return;

   pd->items = (Table_Item *)
         eina_inlist_remove(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));
   free(gi);

   pd->count--;
   efl_key_data_set(event->object, TABLE_ITEM_KEY, NULL);

   efl_pack_layout_request(table);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_ui_table_callbacks,
  { EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _on_child_size_changed },
  { EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _on_child_hints_changed },
  { EFL_EVENT_DEL, _on_child_del }
);

static void
_efl_ui_table_last_position_get(Eo * obj, Efl_Ui_Table_Data *pd, int *last_col, int *last_row)
{
   Table_Item *gi;
   int col = -1, row  = -1;
   int req_cols, req_rows;

   if (!pd->linear_recalc)
     {
        *last_col = pd->last_col;
        *last_row = pd->last_row;
        return;
     }

   efl_pack_table_size_get(obj, &req_cols, &req_rows);

   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
     {
        EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(pd->items), gi)
          {
             if ((gi->row < row) || (req_cols < gi->col) || (req_rows < gi->row))
               continue;

             if (gi->row > row)
               {
                  row = gi->row;
                  col = gi->col;
               }
             else if (gi->col > col)
               {
                  col = gi->col;
               }
          }
     }
   else
     {
        EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(pd->items), gi)
          {
             if ((gi->col < col) || (req_cols < gi->col) || (req_rows < gi->row))
               continue;

             if (gi->col > col)
               {
                  col = gi->col;
                  row = gi->row;
               }
             else if (gi->row > row)
               {
                  row = gi->row;
               }
          }
     }
   *last_col = col;
   *last_row = row;
   pd->linear_recalc = EINA_FALSE;
}

static void
_efl_ui_table_size_hints_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_pack_layout_request(ev->object);
}

EOLIAN static void
_efl_ui_table_homogeneous_set(Eo *obj, Efl_Ui_Table_Data *pd, Eina_Bool homogeneoush, Eina_Bool homogeneousv)
{
   homogeneoush = !!homogeneoush;
   homogeneousv = !!homogeneousv;

   if ((pd->homogeneoush == homogeneoush) && (pd->homogeneousv == homogeneousv))
     return;

   pd->homogeneoush = homogeneoush;
   pd->homogeneousv = homogeneousv;
   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_homogeneous_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, Eina_Bool *homogeneoush, Eina_Bool *homogeneousv)
{
   if (homogeneoush) *homogeneoush = pd->homogeneoush;
   if (homogeneousv) *homogeneousv = pd->homogeneousv;
}

EOLIAN static void
_efl_ui_table_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Table_Data *pd)
{
   _efl_ui_table_custom_layout(obj, pd);
}

EOLIAN void
_efl_ui_table_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Table_Data *_pd EINA_UNUSED)
{
   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_table_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Table_Data *_pd EINA_UNUSED, Eina_Size2D sz)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_table_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Table_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_table_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Table_Data *pd)
{
   pd->clipper = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj);
   evas_object_static_clip_set(pd->clipper, EINA_TRUE);
   efl_gfx_entity_geometry_set(pd->clipper, EINA_RECT(-49999, -49999, 99999, 99999));
   efl_canvas_group_member_add(obj, pd->clipper);
   efl_ui_widget_sub_object_add(obj, pd->clipper);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_ui_widget_focus_allow_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_table_size_hints_changed_cb, NULL);
}

EOLIAN static void
_efl_ui_table_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Table_Data *_pd EINA_UNUSED)
{
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_table_size_hints_changed_cb, NULL);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_table_efl_object_constructor(Eo *obj, Efl_Ui_Table_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->dir1 = EFL_UI_DIR_RIGHT;
   pd->dir2 = EFL_UI_DIR_DOWN;
   pd->last_col = -1;
   pd->last_row = -1;
   pd->req_cols = 0;
   pd->req_rows = 0;
   pd->cols = 0;
   pd->rows = 0;
   pd->align.h = 0.5;
   pd->align.v = 0.5;

   return obj;
}

EOLIAN static void
_efl_ui_table_efl_object_invalidate(Eo *obj, Efl_Ui_Table_Data *pd)
{
   Table_Item *gi;

   efl_invalidate(efl_super(obj, MY_CLASS));

   EINA_INLIST_FREE(EINA_INLIST_GET(pd->items), gi)
     {
        efl_event_callback_array_del(gi->object, efl_ui_table_callbacks(), obj);
     }
}

EOLIAN static void
_efl_ui_table_efl_gfx_arrangement_content_padding_set(Eo *obj, Efl_Ui_Table_Data *pd, double h, double v, Eina_Bool scalable)
{
   scalable = !!scalable;
   if (h < 0) h = 0;
   if (v < 0) v = 0;

   if (EINA_DBL_EQ(pd->pad.h, h) && EINA_DBL_EQ(pd->pad.v, v) &&
       (pd->pad.scalable == scalable))
     return;

   pd->pad.h = h;
   pd->pad.v = v;
   pd->pad.scalable = scalable;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_efl_gfx_arrangement_content_padding_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, double *h, double *v, Eina_Bool *scalable)
{
   if (scalable) *scalable = pd->pad.scalable;
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

EOLIAN static void
_efl_ui_table_efl_gfx_arrangement_content_align_set(Eo *obj, Efl_Ui_Table_Data *pd, double h, double v)
{
   if (h < 0) h = -1;
   else if (h > 1) h = 1;
   if (v < 0) v = -1;
   else if (v > 1) v = 1;

   if (EINA_DBL_EQ(pd->align.h, h) && EINA_DBL_EQ(pd->align.v, v))
     return;

   pd->align.h = h;
   pd->align.v = v;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_efl_gfx_arrangement_content_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, double *h, double *v)
{
   if (h) *h = pd->align.h;
   if (v) *v = pd->align.v;
}

static Eina_Bool
_pack_at(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj, int col, int row, int colspan, int rowspan)
{
   Table_Item *gi;

   if (efl_key_data_get(subobj, TABLE_ITEM_KEY))
     {
        if (efl_canvas_object_render_parent_get(subobj) == obj)
          ERR("subobj %p %s is already added to this", subobj, efl_class_name_get(subobj));
        else
          ERR("subobj %p %s currently belongs to different table", subobj, efl_class_name_get(subobj));

        return EINA_FALSE;
     }

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

   if (!efl_ui_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   gi = calloc(1, sizeof(*gi));
   if (!gi) return EINA_FALSE;
   gi->col = col;
   gi->row = row;
   gi->col_span = colspan;
   gi->row_span = rowspan;
   gi->object = subobj;
   pd->count++;
   pd->items = (Table_Item *)
       eina_inlist_append(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));

   efl_key_data_set(subobj, TABLE_ITEM_KEY, gi);
   efl_key_data_set(subobj, "_elm_leaveme", obj);
   efl_canvas_object_clipper_set(subobj, pd->clipper);
   efl_event_callback_array_add(subobj, efl_ui_table_callbacks(), obj);

   efl_canvas_group_member_add(obj, subobj);
   efl_event_callback_call(obj, EFL_CONTAINER_EVENT_CONTENT_ADDED, subobj);

   if ((gi->col > pd->last_col) || (gi->row > pd->last_row))
     pd->linear_recalc = EINA_TRUE;

   if (pd->cols < gi->col + gi->col_span)
     pd->cols = gi->col + gi->col_span;
   if (pd->rows < gi->row + gi->row_span)
     pd->rows = gi->row + gi->row_span;

   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_table_pack_table(Eo *obj, Efl_Ui_Table_Data *pd,
                                     Efl_Gfx_Entity *subobj,
                                     int col, int row, int colspan, int rowspan)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   return _pack_at(obj, pd, subobj, col, row, colspan, rowspan);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_table_table_position_get(const Eo *obj, Efl_Ui_Table_Data *pd, Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan)
{
   int c = -1, r = -1, cs = 0, rs = 0;
   Table_Item *gi;
   Eina_Bool ret = EINA_FALSE;

   gi = _efl_ui_table_item_date_get((Eo *)obj, pd, subobj);
   if (gi)
     {
        c = gi->col;
        r = gi->row;
        cs = gi->col_span;
        rs = gi->row_span;
        ret = EINA_TRUE;
     }

   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;
   return ret;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_table_efl_pack_table_table_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, int col, int row)
{
   Table_Item *gi;
   EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), gi)
     {
        if (gi->col == col && gi->row == row)
          return gi->object;
     }

   return NULL;
}

static Eina_Bool
_item_remove(Efl_Ui_Table *obj, Efl_Ui_Table_Data *pd, Table_Item *gi)
{
   Efl_Gfx_Entity *subobj = gi->object;

   if (!subobj || !_elm_widget_sub_object_redirect_to_top(obj, subobj))
     return EINA_FALSE;

   efl_canvas_object_clipper_set(subobj, NULL);
   efl_key_data_set(subobj, "_elm_leaveme", NULL);
   efl_key_data_set(subobj, TABLE_ITEM_KEY, NULL);
   efl_event_callback_array_del(subobj, efl_ui_table_callbacks(), obj);

   efl_canvas_group_member_remove(obj, subobj);
   efl_event_callback_call(obj, EFL_CONTAINER_EVENT_CONTENT_REMOVED, subobj);

   if ((gi->col == pd->last_col) && (gi->row == pd->last_row))
     pd->linear_recalc = EINA_TRUE;

   if (gi->col + gi->col_span >= pd->cols)
     pd->cols_recalc = EINA_TRUE;
   if (gi->row + gi->row_span >= pd->rows)
     pd->rows_recalc = EINA_TRUE;

   pd->items = (Table_Item *)
         eina_inlist_remove(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));
   free(gi);

   pd->count--;
   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_unpack(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   Table_Item *gi = _efl_ui_table_item_date_get(obj, pd, subobj);
   if (!gi) return EINA_FALSE;

   return _item_remove(obj, pd, gi);
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_pack_clear(Eo *obj, Efl_Ui_Table_Data *pd)
{
   Table_Item *gi;

   EINA_INLIST_FREE(EINA_INLIST_GET(pd->items), gi)
     {
        efl_event_callback_array_del(gi->object, efl_ui_table_callbacks(), obj);
        efl_del(gi->object);

        pd->items = (Table_Item *)
            eina_inlist_remove(EINA_INLIST_GET(pd->items), EINA_INLIST_GET(gi));
        free(gi);
     }

   pd->linear_recalc = EINA_TRUE;
   pd->cols_recalc = EINA_TRUE;
   pd->rows_recalc = EINA_TRUE;

   pd->last_col = -1;
   pd->last_row = -1;
   pd->cols = 0;
   pd->rows = 0;
   pd->count = 0;
   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_unpack_all(Eo *obj, Efl_Ui_Table_Data *pd)
{
   Table_Item *gi;
   Eina_Bool ret = EINA_TRUE;

   EINA_INLIST_FREE(EINA_INLIST_GET(pd->items), gi)
     ret &= _item_remove(obj, pd, gi);

   pd->linear_recalc = EINA_TRUE;
   pd->cols_recalc = EINA_TRUE;
   pd->rows_recalc = EINA_TRUE;

   pd->last_col = -1;
   pd->last_row = -1;
   pd->cols = 0;
   pd->rows = 0;

   return ret;
}

EOLIAN static void
_efl_ui_table_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);
}

static Eina_Bool
_efl_ui_table_item_iterator_next(Table_Item_Iterator *it, void **data)
{
   Table_Item *gi;

   if (!it->cur)
     return EINA_FALSE;

   if (data)
     {
        gi = EINA_INLIST_CONTAINER_GET(it->cur, Table_Item);
        *data = gi->object;
     }

   it->cur = it->cur->next;

   return EINA_TRUE;
}

static Eo *
_efl_ui_table_item_iterator_get_container(Table_Item_Iterator *it)
{
   return it->object;
}

static void
_efl_ui_table_item_iterator_free(Table_Item_Iterator *it)
{
   free(it);
}

static inline Eina_Iterator *
_efl_ui_table_item_iterator_create(Eo *obj, Eina_Inlist *list)
{
   Table_Item_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->object = obj;
   it->cur = list;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_ui_table_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
     _efl_ui_table_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_ui_table_item_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_ui_table_efl_container_content_iterate(Eo *obj, Efl_Ui_Table_Data *pd)
{
   Table_Item_Iterator *it;

   if (!pd->count)
     return NULL;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->object = obj;
   it->cur = EINA_INLIST_GET(pd->items);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_ui_table_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
     _efl_ui_table_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_ui_table_item_iterator_free);

   return &it->iterator;
}

EOLIAN static int
_efl_ui_table_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
{
   return pd->count;
}

EOLIAN static Eina_Iterator *
_efl_ui_table_efl_pack_table_table_contents_get(Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd, int col, int row, Eina_Bool below)
{
   Table_Item *gi;
   Eina_List *atlist = NULL;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), gi)
     {
        if ((gi->col == col) && (gi->row == row))
          atlist = eina_list_append(atlist, gi->object);
        else if (below)
          {
             if ((gi->col <= col) && (gi->col + gi->col_span >= col) &&
                 (gi->row <= row) && (gi->row + gi->row_span >= row))
               atlist = eina_list_append(atlist, gi->object);
          }
     }

   return eina_list_iterator_new(atlist);
}

EOLIAN static void
_efl_ui_table_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Ui_Dir dir)
{
   if (pd->dir1 == dir)
     return;

   if (dir == EFL_UI_DIR_DEFAULT)
     dir = EFL_UI_DIR_RIGHT;

   pd->dir1 = dir;

   /* if both directions are either horizontal or vertical, need to adjust
    * secondary direction (dir2) */
   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE) ==
       efl_ui_dir_is_horizontal(pd->dir2, EINA_FALSE))
     {
        if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
          pd->dir2 = EFL_UI_DIR_DOWN;
        else
          pd->dir2 = EFL_UI_DIR_RIGHT;
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
   if ((pd->dir1 == primary) && (pd->dir2 == secondary))
     return;

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
_efl_ui_table_efl_pack_table_table_size_set(Eo *obj, Efl_Ui_Table_Data *pd, int cols, int rows)
{
   if (cols < 0) cols = 0;
   if (rows < 0) rows = 0;

   if ((pd->req_cols == cols) && (pd->req_rows == rows))
     return;

   pd->req_cols = cols;
   pd->req_rows = rows;
   pd->linear_recalc = EINA_TRUE;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_size_get(const Eo *obj, Efl_Ui_Table_Data *pd EINA_UNUSED, int *cols, int *rows)
{
   if (cols) *cols = efl_pack_table_columns_get(obj);
   if (rows) *rows = efl_pack_table_rows_get(obj);
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_columns_set(Eo *obj, Efl_Ui_Table_Data *pd, int columns)
{
   if (pd->req_cols == columns)
     return;

   pd->req_cols = columns;
   pd->linear_recalc = EINA_TRUE;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_table_efl_pack_table_table_columns_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
{
   Table_Item *gi;
   if (pd->cols_recalc)
     {
        pd->cols = 0;
        EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(pd->items), gi)
          {
             if (pd->cols < gi->col + gi->col_span)
               pd->cols = gi->col+ gi->col_span;
          }
        pd->cols_recalc = EINA_FALSE;
     }
   return pd->req_cols ? : pd->cols;
}

EOLIAN static void
_efl_ui_table_efl_pack_table_table_rows_set(Eo *obj, Efl_Ui_Table_Data *pd, int rows)
{
   if (pd->req_rows == rows)
     return;

   pd->req_rows = rows;
   pd->linear_recalc = EINA_TRUE;

   efl_pack_layout_request(obj);
}

EOLIAN static int
_efl_ui_table_efl_pack_table_table_rows_get(const Eo *obj EINA_UNUSED, Efl_Ui_Table_Data *pd)
{
   Table_Item *gi;
   if (pd->rows_recalc)
     {
        pd->rows = 0;
        EINA_INLIST_REVERSE_FOREACH(EINA_INLIST_GET(pd->items), gi)
          {
             if (pd->rows < gi->row + gi->row_span)
               pd->rows = gi->row + gi->row_span;
          }
        pd->rows_recalc = EINA_FALSE;
     }
   return pd->req_rows ? : pd->rows;
}

EOLIAN static Eina_Bool
_efl_ui_table_efl_pack_pack(Eo *obj, Efl_Ui_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   int last_col, last_row;

   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);

   _efl_ui_table_last_position_get(obj, pd, &last_col, &last_row);

   if (efl_ui_dir_is_horizontal(pd->dir1, EINA_TRUE))
     {
        last_col++;
        if (pd->req_cols && (last_col >= pd->req_cols))
          {
             last_col = 0;
             last_row++;
          }
        if (last_row < 0) last_row = 0;
     }
   else
     {
        last_row++;
        if (pd->req_rows && (last_row >= pd->req_rows))
          {
             last_row = 0;
             last_col++;
          }
        if (last_col < 0) last_col = 0;
     }

   pd->last_col = last_col;
   pd->last_row = last_row;

   DBG("packing new obj at %d,%d", last_col, last_row);
   return _pack_at(obj, pd, subobj, last_col, last_row, 1, 1);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_TABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_table)

#include "efl_ui_table.eo.c"
