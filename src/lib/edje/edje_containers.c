#include "edje_private.h"

#define EFL_CANVAS_LAYOUT_INTERNAL_BOX_PROTECTED
#define EFL_CANVAS_LAYOUT_INTERNAL_TABLE_PROTECTED

#include "efl_canvas_layout_internal_box.eo.h"
#include "efl_canvas_layout_internal_table.eo.h"

#include "../evas/canvas/evas_box.eo.h"
#include "../evas/canvas/evas_table.eo.h"

#define BOX_CLASS   EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS
#define TABLE_CLASS EFL_CANVAS_LAYOUT_INTERNAL_TABLE_CLASS

typedef struct _Edje_Part_Data     Edje_Box_Data;
typedef struct _Edje_Part_Data     Edje_Table_Data;
typedef struct _Part_Item_Iterator Part_Item_Iterator;

struct _Edje_Part_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

static Eina_Bool
_del_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   rp->typedata.container->eo_proxy = NULL;
   return EO_CALLBACK_CONTINUE;
}

Eo *
_edje_box_internal_proxy_get(Edje_Object *obj, Edje *ed, Edje_Real_Part *rp)
{
   Efl_Canvas_Layout_Internal_Box *eo = rp->typedata.container->eo_proxy;

   if (eo) return eo;

   eo = eo_add(BOX_CLASS, obj, efl_canvas_layout_internal_box_real_part_set(eo_self, ed, rp, rp->part->name));
   eo_event_callback_add(eo, EO_EVENT_DEL, _del_cb, rp);

   rp->typedata.container->eo_proxy = eo;
   return eo;
}

EOLIAN static void
_efl_canvas_layout_internal_box_real_part_set(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, void *ed, void *rp, const char *part)
{
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
}

EOLIAN static Eo_Base *
_efl_canvas_layout_internal_box_eo_base_finalize(Eo *obj, Edje_Box_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   return eo_finalize(eo_super(obj, BOX_CLASS));
}

/* Legacy features */

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   return _edje_part_box_remove_all(pd->ed, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   return _edje_part_box_remove_all(pd->ed, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   Evas_Object *removed;
   removed = _edje_part_box_remove(pd->ed, pd->part, subobj);
   return (removed == subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   return _edje_part_box_prepend(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   return _edje_part_box_insert_before(pd->ed, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   return _edje_part_box_insert_after(pd->ed, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_at(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, int index)
{
   int cnt = efl_content_count(obj);
   if ((index < 0) && ((-index) <= (cnt + 1)))
     index = cnt + index + 1;
   if ((index >= 0) && (index < cnt))
     return _edje_part_box_insert_at(pd->ed, pd->part, subobj, index);
   else
     return _edje_part_box_append(pd->ed, pd->part, subobj);
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_unpack_at(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_remove_at(pd->ed, pd->part, index);
}

/* New APIs with Eo */

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += efl_content_count(obj);
   return _edje_part_box_content_at(pd->ed, pd->part, index);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_container_content_remove(Eo *obj, Edje_Box_Data *pd EINA_UNUSED, Efl_Gfx *subobj)
{
   return efl_pack_unpack(obj, subobj);
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Edje_Box_Data *pd, const Efl_Gfx * subobj)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Eina_List *l;
   int k = 0;

   priv = eo_data_scope_get(pd->rp->object, EVAS_BOX_CLASS);
   if (!priv) return -1;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == subobj)
          return k;
        k++;
     }
   return -1;
}

/* this iterator is the same as efl_ui_box */
static Eina_Bool
_part_item_iterator_next(Part_Item_Iterator *it, void **data)
{
   Efl_Gfx *sub;

   if (!it->object) return EINA_FALSE;
   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_part_item_iterator_get_container(Part_Item_Iterator *it)
{
   return it->object;
}

static void
_part_item_iterator_free(Part_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eo_wref_del(it->object, &it->object);
   free(it);
}

static Eina_Iterator *
_part_item_iterator_create(Eo *obj, Eina_Iterator *real_iterator)
{
   Part_Item_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = real_iterator;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_part_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_part_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_part_item_iterator_free);
   eo_wref_add(obj, &it->object);

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_box_efl_container_content_iterate(Eo *obj, Edje_Box_Data *pd)
{
   Eina_Iterator *it;

   if (!pd->rp->typedata.container) return NULL;
   it = evas_object_box_iterator_new(pd->rp->object);

   return _part_item_iterator_create(obj, it);
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_container_content_count(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   return evas_obj_box_count(pd->rp->object);
}

EOLIAN static Efl_Orient
_efl_canvas_layout_internal_box_efl_pack_linear_pack_direction_get(Eo *obj EINA_UNUSED, Edje_Box_Data *pd)
{
   const Edje_Part_Description_Box *desc =
         (Edje_Part_Description_Box *) pd->rp->chosen_description;

   if (!desc || !desc->box.layout)
     return EFL_ORIENT_NONE;

   if (!strncmp(desc->box.layout, "vertical", 8))
     return EFL_ORIENT_VERTICAL;
   else if (!strncmp(desc->box.layout, "horizontal", 10))
     return EFL_ORIENT_HORIZONTAL;

   WRN("unknown orientation '%s'", desc->box.layout);
   return EFL_ORIENT_NONE;
}

/* Table */

Eo *
_edje_table_internal_proxy_get(Edje_Object *obj, Edje *ed, Edje_Real_Part *rp)
{
   Efl_Canvas_Layout_Internal_Box *eo = rp->typedata.container->eo_proxy;

   if (eo) return eo;

   eo = eo_add(TABLE_CLASS, obj, efl_canvas_layout_internal_table_real_part_set(eo_self, ed, rp, rp->part->name));
   eo_event_callback_add(eo, EO_EVENT_DEL, _del_cb, rp);

   rp->typedata.container->eo_proxy = eo;
   return eo;
}

EOLIAN static void
_efl_canvas_layout_internal_table_real_part_set(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, void *ed, void *rp, const char *part)
{
   pd->ed = ed;
   pd->rp = rp;
   pd->part = part;
}

EOLIAN static Eo_Base *
_efl_canvas_layout_internal_table_eo_base_finalize(Eo *obj, Edje_Table_Data *pd)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL);
   return eo_finalize(eo_super(obj, TABLE_CLASS));
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_table_efl_container_content_iterate(Eo *obj, Edje_Table_Data *pd)
{
   Eina_Iterator *it;

   if (!pd->rp->typedata.container) return NULL;
   it = evas_object_table_iterator_new(pd->rp->object);

   return _part_item_iterator_create(obj, it);
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_container_content_count(Eo *obj EINA_UNUSED, Edje_Table_Data *pd)
{
   return evas_obj_table_count(pd->rp->object);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_container_content_remove(Eo *obj, Edje_Table_Data *pd EINA_UNUSED, Efl_Gfx *content)
{
   return efl_pack_unpack(obj, content);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Edje_Table_Data *pd)
{
   return _edje_part_table_clear(pd->ed, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Edje_Table_Data *pd)
{
   return _edje_part_table_clear(pd->ed, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_unpack(Eo *obj EINA_UNUSED EINA_UNUSED, Edje_Table_Data *pd, Efl_Gfx *subobj)
{
   return _edje_part_table_unpack(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_grid_pack_grid(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, Efl_Gfx *subobj, int col, int row, int colspan, int rowspan)
{
   return _edje_part_table_pack(pd->ed, pd->part, subobj, col, row, colspan, rowspan);
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_table_efl_pack_grid_grid_content_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, int col, int row)
{
   return _edje_part_table_child_get(pd->ed, pd->part, col, row);
}

EOLIAN static void
_efl_canvas_layout_internal_table_efl_pack_grid_grid_size_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, int *cols, int *rows)
{
   _edje_part_table_col_row_size_get(pd->ed, pd->part, cols, rows);
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_pack_grid_grid_columns_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd)
{
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   return cols;
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_pack_grid_grid_rows_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd)
{
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   return rows;
}

/* New table apis with eo */

static Eina_Bool
_table_item_iterator_next(Part_Item_Iterator *it, void **data)
{
   Efl_Gfx *sub;

   if (!it->object) return EINA_FALSE;
   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_table_item_iterator_get_container(Part_Item_Iterator *it)
{
   return it->object;
}

static void
_table_item_iterator_free(Part_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   eo_wref_del(it->object, &it->object);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_table_efl_pack_grid_grid_contents_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, int col, int row, Eina_Bool below)
{
   Evas_Object *sobj;
   Eina_Iterator *it;
   Part_Item_Iterator *pit;
   Eina_List *list = NULL;
   unsigned short c, r, cs, rs;

   /* FIXME: terrible performance because there is no proper evas table api */

   it = evas_object_table_iterator_new(pd->rp->object);
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        evas_object_table_pack_get(pd->rp->object, sobj, &c, &r, &cs, &rs);
        if (((int) c == col) && ((int) r == row))
          list = eina_list_append(list, sobj);
        else if (below)
          {
             if (((int) c <= col) && ((int) (c + cs) >= col) &&
                 ((int) r <= row) && ((int) (r + rs) >= row))
               list = eina_list_append(list, sobj);
          }
     }
   eina_iterator_free(it);

   pit = calloc(1, sizeof(*pit));
   if (!pit) return NULL;

   EINA_MAGIC_SET(&pit->iterator, EINA_MAGIC_ITERATOR);

   pit->list = list;
   pit->real_iterator = eina_list_iterator_new(pit->list);
   pit->iterator.version = EINA_ITERATOR_VERSION;
   pit->iterator.next = FUNC_ITERATOR_NEXT(_table_item_iterator_next);
   pit->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_table_item_iterator_get_container);
   pit->iterator.free = FUNC_ITERATOR_FREE(_table_item_iterator_free);
   eo_wref_add(obj, &pit->object);

   return &pit->iterator;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_grid_grid_position_get(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, Efl_Gfx * subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short c, r, cs, rs;
   Eina_Bool ret;

   ret = evas_object_table_pack_get(pd->rp->object, subobj, &c, &r, &cs, &rs);
   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;

   return ret;
}

/* Legacy API implementation */

#ifdef DEGUG
#define PART_BOX_GET(obj, part, ...) ({ \
   Eo *__box = efl_content_get(obj, part); \
   if (!__box || !eo_isa(__box, EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS)) \
     { \
        ERR("No such box part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __box; })
#else
#define PART_BOX_GET(obj, part, ...) ({ \
   Eo *__box = efl_content_get(obj, part); \
   if (!__box) return __VA_ARGS__; \
   __box; })
#endif

EAPI Eina_Bool
edje_object_part_box_append(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_end(box, child);
}

EAPI Eina_Bool
edje_object_part_box_prepend(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   efl_pack_begin(box, child);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_box_insert_before(Edje_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_before(box, child, reference);
}

EAPI Eina_Bool
edje_object_part_box_insert_after(Edje_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_after(box, child, reference);
}

EAPI Eina_Bool
edje_object_part_box_insert_at(Edje_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   return efl_pack_at(box, child, pos);
}

EAPI Evas_Object *
edje_object_part_box_remove_at(Edje_Object *obj, const char *part, unsigned int pos)
{
   Eo *box = PART_BOX_GET(obj, part, NULL);
   return efl_pack_unpack_at(box, pos);
}

EAPI Evas_Object *
edje_object_part_box_remove(Edje_Object *obj, const char *part, Evas_Object *child)
{
   Eo *box = PART_BOX_GET(obj, part, NULL);
   if (efl_pack_unpack(box, child))
     return child;
   return NULL;
}

EAPI Eina_Bool
edje_object_part_box_remove_all(Edje_Object *obj, const char *part, Eina_Bool clear)
{
   Eo *box = PART_BOX_GET(obj, part, EINA_FALSE);
   if (clear)
     return efl_pack_clear(box);
   else
     return efl_pack_unpack_all(box);
}

#ifdef DEBUG
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_content_get(obj, part); \
   if (!__table || !eo_isa(__table, EFL_CANVAS_LAYOUT_INTERNAL_TABLE_CLASS)) \
     { \
        ERR("No such table part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __table; })
#else
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_content_get(obj, part); \
   if (!__table) return __VA_ARGS__; \
   __table; })
#endif

EAPI Eina_Bool
edje_object_part_table_pack(Edje_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   return efl_pack_grid(table, child_obj, col, row, colspan, rowspan);
}

EAPI Eina_Bool
edje_object_part_table_col_row_size_get(const Edje_Object *obj, const char *part, int *cols, int *rows)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   efl_pack_grid_size_get(table, cols, rows);
   return EINA_TRUE;
}

EAPI Evas_Object *
edje_object_part_table_child_get(const Edje_Object *obj, const char *part, unsigned int col, unsigned int row)
{
   Eo *table = PART_TABLE_GET(obj, part, NULL);
   return efl_pack_grid_content_get(table, col, row);
}

EAPI Eina_Bool
edje_object_part_table_unpack(Edje_Object *obj, const char *part, Evas_Object *child_obj)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   return efl_pack_unpack(table, child_obj);
}

EAPI Eina_Bool
edje_object_part_table_clear(Edje_Object *obj, const char *part, Eina_Bool clear)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   if (clear)
     return efl_pack_clear(table);
   else
     return efl_pack_unpack_all(table);
}

#include "efl_canvas_layout_internal_box.eo.c"
#include "efl_canvas_layout_internal_table.eo.c"
