#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_part_table.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_TABLE_CLASS

#include "../evas/canvas/evas_table.eo.h"

PROXY_IMPLEMENTATION(table, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

typedef struct _Part_Item_Iterator Part_Item_Iterator;

EOLIAN static Eina_Iterator *
_efl_canvas_layout_part_table_efl_container_content_iterate(Eo *obj, void *_pd EINA_UNUSED)
{
   Eina_Iterator *it;

   PROXY_DATA_GET(obj, pd);
   if (!pd->rp->typedata.container) return NULL;
   it = evas_object_table_iterator_new(pd->rp->object);

   return efl_canvas_iterator_create(pd->rp->object, it, NULL);
}

EOLIAN static int
_efl_canvas_layout_part_table_efl_container_content_count(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return evas_obj_table_count(pd->rp->object);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_container_content_remove(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED EINA_UNUSED, Efl_Gfx_Entity *content)
{
   return efl_pack_unpack(obj, content);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_pack_pack_clear(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_table_clear(pd->ed, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_pack_unpack_all(Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_table_clear(pd->ed, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_pack_unpack(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_table_unpack(pd->ed, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_pack_table_pack_table(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int col, int row, int colspan, int rowspan)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_table_pack(pd->ed, pd->part, subobj, col, row, colspan, rowspan);
}

EOLIAN static Efl_Gfx_Entity *
_efl_canvas_layout_part_table_efl_pack_table_table_content_get(Eo *obj, void *_pd EINA_UNUSED, int col, int row)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_part_table_child_get(pd->ed, pd->part, col, row);
}

EOLIAN static void
_efl_canvas_layout_part_table_efl_pack_table_table_size_get(const Eo *obj, void *_pd EINA_UNUSED, int *cols, int *rows)
{
   PROXY_DATA_GET(obj, pd);
   _edje_part_table_col_row_size_get(pd->ed, pd->part, cols, rows);
}

EOLIAN static int
_efl_canvas_layout_part_table_efl_pack_table_table_columns_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   return cols;
}

EOLIAN static int
_efl_canvas_layout_part_table_efl_pack_table_table_rows_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   return rows;
}

/* New table apis with eo */

static Eina_Bool
_table_item_iterator_next(Part_Item_Iterator *it, void **data)
{
   Efl_Gfx_Entity *sub;

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
   efl_wref_del(it->object, &it->object);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_part_table_efl_pack_table_table_contents_get(Eo *obj, void *_pd EINA_UNUSED, int col, int row, Eina_Bool below)
{
   Evas_Object *sobj;
   Eina_Iterator *it;
   Part_Item_Iterator *pit;
   Eina_List *list = NULL;
   unsigned short c, r, cs, rs;

   /* FIXME: terrible performance because there is no proper evas table api */

   PROXY_DATA_GET(obj, pd);
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
   efl_wref_add(obj, &pit->object);

   return &pit->iterator;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_table_efl_pack_table_table_position_get(const Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity * subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short c, r, cs, rs;
   Eina_Bool ret;

   PROXY_DATA_GET(obj, pd);
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
   Eo *__box = efl_part(obj, part); \
   if (!__box || !efl_isa(__box, EFL_CANVAS_LAYOUT_PART_BOX_CLASS)) \
     { \
        ERR("No such box part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __box; })
#else
#define PART_BOX_GET(obj, part, ...) ({ \
   Eo *__box = efl_part(obj, part); \
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
   return efl_pack_begin(box, child);
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

EAPI Eina_Bool
edje_object_part_table_pack(Edje_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   return efl_pack_table(table, child_obj, col, row, colspan, rowspan);
}

EAPI Eina_Bool
edje_object_part_table_col_row_size_get(const Edje_Object *obj, const char *part, int *cols, int *rows)
{
   Eo *table = PART_TABLE_GET(obj, part, EINA_FALSE);
   efl_pack_table_size_get(table, cols, rows);
   return EINA_TRUE;
}

EAPI Evas_Object *
edje_object_part_table_child_get(const Edje_Object *obj, const char *part, unsigned int col, unsigned int row)
{
   Eo *table = PART_TABLE_GET(obj, part, NULL);
   return efl_pack_table_content_get(table, col, row);
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

#include "efl_canvas_layout_part_table.eo.c"
