#include "edje_private.h"

#define EFL_CANVAS_LAYOUT_INTERNAL_PROTECTED
#define EFL_CANVAS_LAYOUT_INTERNAL_BOX_PROTECTED
#define EFL_CANVAS_LAYOUT_INTERNAL_TABLE_PROTECTED
#define EFL_CANVAS_LAYOUT_INTERNAL_SWALLOW_PROTECTED

#include "efl_canvas_layout_internal.eo.h"
#include "efl_canvas_layout_internal_box.eo.h"
#include "efl_canvas_layout_internal_table.eo.h"
#include "efl_canvas_layout_internal_swallow.eo.h"

#include "../evas/canvas/evas_box.eo.h"
#include "../evas/canvas/evas_table.eo.h"

#define BOX_CLASS     EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS
#define TABLE_CLASS   EFL_CANVAS_LAYOUT_INTERNAL_TABLE_CLASS
#define SWALLOW_CLASS EFL_CANVAS_LAYOUT_INTERNAL_SWALLOW_CLASS

typedef struct _Edje_Part_Data     Edje_Box_Data;
typedef struct _Edje_Part_Data     Edje_Table_Data;
typedef struct _Edje_Part_Data     Edje_Swallow_Data;
typedef struct _Part_Item_Iterator Part_Item_Iterator;

struct _Edje_Part_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
   unsigned char   temp;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

#define PROXY_REF(obj, pd) do { if (!(pd->temp++)) eo_ref(obj); } while(0)
#define PROXY_UNREF(obj, pd) do { if (pd->temp) { if (!(--pd->temp)) eo_unref(obj); } } while(0)
#define RETURN_VAL(a) do { typeof(a) _ret = a; PROXY_UNREF(obj, pd); return _ret; } while(0)
#define RETURN_VOID do { PROXY_UNREF(obj, pd); return; } while(0)
#define PROXY_CALL(a) ({ PROXY_REF(obj, pd); a; })

/* ugly macros to avoid code duplication */

#define PROXY_RESET(type) \
   do { if (_ ## type ## _proxy) \
     { \
        eo_del_intercept_set(_ ## type ## _proxy, NULL); \
        eo_unref(_ ## type ## _proxy); \
        _ ## type ## _proxy = NULL; \
     } } while (0)

#define PROXY_DEL_CB(type) \
static void \
type ## _del_cb(Eo *proxy) \
{ \
   if (_ ## type ## _proxy) \
     { \
        eo_del_intercept_set(proxy, NULL); \
        eo_unref(proxy); \
        return; \
     } \
   if (eo_parent_get(proxy)) \
     { \
        eo_ref(proxy); \
        eo_parent_set(proxy, NULL); \
     } \
   _ ## type ## _proxy = proxy; \
}

#define PROXY_IMPLEMENTATION(type, TYPE, datatype) \
Eo * \
_edje_ ## type ## _internal_proxy_get(Edje_Object *obj EINA_UNUSED, Edje *ed, Edje_Real_Part *rp) \
{ \
   Edje_Box_Data *pd; \
   Eo *proxy; \
   \
   pd = eo_data_scope_get(_ ## type ## _proxy, TYPE ## _CLASS); \
   if (!pd) \
     { \
        if (_ ## type ## _proxy) \
          { \
             ERR("Found invalid handle for efl_part. Reset."); \
             _ ## type ## _proxy = NULL; \
          } \
        return eo_add(TYPE ## _CLASS, ed->obj, \
                      _edje_real_part_set(eo_self, ed, rp, rp->part->name)); \
     } \
   \
   if (EINA_UNLIKELY(pd->temp)) \
     { \
        /* warn about misuse, since non-implemented functions may trigger this \
         * misuse by accident. */ \
        ERR("Misuse of efl_part detected. Handles returned by efl_part() are " \
            "valid for a single function call! Did you call a non implemented " \
            "function?"); \
     } \
   proxy = _ ## type ## _proxy; \
   _ ## type ## _proxy = NULL; \
   _edje_real_part_set(proxy, ed, rp, rp->part->name); \
   return proxy; \
} \
\
EOLIAN static void \
_efl_canvas_layout_internal_ ## type ## _efl_canvas_layout_internal_real_part_set(Eo *obj, datatype *pd, void *ed, void *rp, const char *part) \
{ \
   pd->ed = ed; \
   pd->rp = rp; \
   pd->part = part; \
   pd->temp = 1; \
   eo_del_intercept_set(obj, type ## _del_cb); \
   eo_parent_set(obj, pd->ed->obj); \
} \
\
EOLIAN static Eo_Base * \
_efl_canvas_layout_internal_ ## type ## _eo_base_finalize(Eo *obj, datatype *pd) \
{ \
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL); \
   return eo_finalize(eo_super(obj, TYPE ## _CLASS)); \
}

static Eo *_box_proxy = NULL;
static Eo *_table_proxy = NULL;
static Eo *_swallow_proxy = NULL;

void
_edje_internal_proxy_shutdown(void)
{
   PROXY_RESET(box);
   PROXY_RESET(table);
   PROXY_RESET(swallow);
}

PROXY_DEL_CB(box)
PROXY_DEL_CB(table)
PROXY_DEL_CB(swallow)

PROXY_IMPLEMENTATION(box, BOX, Edje_Box_Data)
PROXY_IMPLEMENTATION(table, TABLE, Edje_Table_Data)
PROXY_IMPLEMENTATION(swallow, SWALLOW, Edje_Swallow_Data)

#undef PROXY_RESET
#undef PROXY_DEL_CB
#undef PROXY_IMPLEMENTATION


/* Legacy features */

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack_clear(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(_edje_part_box_remove_all(pd->ed, pd->part, EINA_TRUE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack_all(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(_edje_part_box_remove_all(pd->ed, pd->part, EINA_FALSE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_unpack(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   Evas_Object *removed;
   removed = _edje_part_box_remove(pd->ed, pd->part, subobj);
   RETURN_VAL((removed == subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_pack(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_begin(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_prepend(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_end(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_before(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   RETURN_VAL(_edje_part_box_insert_before(pd->ed, pd->part, subobj, existing));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_after(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   RETURN_VAL(_edje_part_box_insert_after(pd->ed, pd->part, subobj, existing));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_pack_linear_pack_at(Eo *obj, Edje_Box_Data *pd, Efl_Gfx *subobj, int index)
{
   int cnt = PROXY_CALL(efl_content_count(obj));
   if ((index < 0) && ((-index) <= (cnt + 1)))
     index = cnt + index + 1;
   if ((index >= 0) && (index < cnt))
     RETURN_VAL(_edje_part_box_insert_at(pd->ed, pd->part, subobj, index));
   else
     RETURN_VAL(_edje_part_box_append(pd->ed, pd->part, subobj));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_unpack_at(Eo *obj, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += PROXY_CALL(efl_content_count(obj));
   RETURN_VAL(_edje_part_box_remove_at(pd->ed, pd->part, index));
}

/* New APIs with Eo */

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_box_efl_pack_linear_pack_content_get(Eo *obj, Edje_Box_Data *pd, int index)
{
   if (index < 0) index += PROXY_CALL(efl_content_count(obj));
   RETURN_VAL(_edje_part_box_content_at(pd->ed, pd->part, index));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_box_efl_container_content_remove(Eo *obj, Edje_Box_Data *pd EINA_UNUSED, Efl_Gfx *subobj)
{
   RETURN_VAL(PROXY_CALL(efl_pack_unpack(obj, subobj)));
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_pack_linear_pack_index_get(Eo *obj, Edje_Box_Data *pd, const Efl_Gfx * subobj)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Eina_List *l;
   int k = 0;

   priv = eo_data_scope_get(pd->rp->object, EVAS_BOX_CLASS);
   if (!priv) RETURN_VAL(-1);
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == subobj)
          RETURN_VAL(k);
        k++;
     }
   RETURN_VAL(-1);
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

   if (!pd->rp->typedata.container) RETURN_VAL(NULL);
   it = evas_object_box_iterator_new(pd->rp->object);

   RETURN_VAL(_part_item_iterator_create(pd->rp->object, it));
}

EOLIAN static int
_efl_canvas_layout_internal_box_efl_container_content_count(Eo *obj, Edje_Box_Data *pd)
{
   RETURN_VAL(evas_obj_box_count(pd->rp->object));
}

EOLIAN static Efl_Orient
_efl_canvas_layout_internal_box_efl_pack_linear_pack_direction_get(Eo *obj, Edje_Box_Data *pd)
{
   const Edje_Part_Description_Box *desc =
         (Edje_Part_Description_Box *) pd->rp->chosen_description;

   if (!desc || !desc->box.layout)
     RETURN_VAL(EFL_ORIENT_NONE);

   if (!strncmp(desc->box.layout, "vertical", 8))
     RETURN_VAL(EFL_ORIENT_VERTICAL);
   else if (!strncmp(desc->box.layout, "horizontal", 10))
     RETURN_VAL(EFL_ORIENT_HORIZONTAL);

   WRN("unknown orientation '%s'", desc->box.layout);
   RETURN_VAL(EFL_ORIENT_NONE);
}

EOLIAN static Eina_Iterator *
_efl_canvas_layout_internal_table_efl_container_content_iterate(Eo *obj, Edje_Table_Data *pd)
{
   Eina_Iterator *it;

   if (!pd->rp->typedata.container) RETURN_VAL(NULL);
   it = evas_object_table_iterator_new(pd->rp->object);

   RETURN_VAL(_part_item_iterator_create(pd->rp->object, it));
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_container_content_count(Eo *obj, Edje_Table_Data *pd)
{
   RETURN_VAL(evas_obj_table_count(pd->rp->object));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_container_content_remove(Eo *obj, Edje_Table_Data *pd EINA_UNUSED, Efl_Gfx *content)
{
   RETURN_VAL(PROXY_CALL(efl_pack_unpack(obj, content)));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_pack_clear(Eo *obj, Edje_Table_Data *pd)
{
   RETURN_VAL(_edje_part_table_clear(pd->ed, pd->part, EINA_TRUE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_unpack_all(Eo *obj, Edje_Table_Data *pd)
{
   RETURN_VAL(_edje_part_table_clear(pd->ed, pd->part, EINA_FALSE));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_unpack(Eo *obj EINA_UNUSED, Edje_Table_Data *pd, Efl_Gfx *subobj)
{
   RETURN_VAL(_edje_part_table_unpack(pd->ed, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_grid_pack_grid(Eo *obj, Edje_Table_Data *pd, Efl_Gfx *subobj, int col, int row, int colspan, int rowspan)
{
   RETURN_VAL(_edje_part_table_pack(pd->ed, pd->part, subobj, col, row, colspan, rowspan));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_table_efl_pack_grid_grid_content_get(Eo *obj, Edje_Table_Data *pd, int col, int row)
{
   RETURN_VAL(_edje_part_table_child_get(pd->ed, pd->part, col, row));
}

EOLIAN static void
_efl_canvas_layout_internal_table_efl_pack_grid_grid_size_get(Eo *obj, Edje_Table_Data *pd, int *cols, int *rows)
{
   _edje_part_table_col_row_size_get(pd->ed, pd->part, cols, rows);
   RETURN_VOID;
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_pack_grid_grid_columns_get(Eo *obj, Edje_Table_Data *pd)
{
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   RETURN_VAL(cols);
}

EOLIAN static int
_efl_canvas_layout_internal_table_efl_pack_grid_grid_rows_get(Eo *obj, Edje_Table_Data *pd)
{
   int cols = 0, rows = 0;
   _edje_part_table_col_row_size_get(pd->ed, pd->part, &cols, &rows);
   RETURN_VAL(rows);
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
_efl_canvas_layout_internal_table_efl_pack_grid_grid_contents_get(Eo *obj, Edje_Table_Data *pd, int col, int row, Eina_Bool below)
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
   if (!pit) RETURN_VAL(NULL);

   EINA_MAGIC_SET(&pit->iterator, EINA_MAGIC_ITERATOR);

   pit->list = list;
   pit->real_iterator = eina_list_iterator_new(pit->list);
   pit->iterator.version = EINA_ITERATOR_VERSION;
   pit->iterator.next = FUNC_ITERATOR_NEXT(_table_item_iterator_next);
   pit->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_table_item_iterator_get_container);
   pit->iterator.free = FUNC_ITERATOR_FREE(_table_item_iterator_free);
   eo_wref_add(obj, &pit->object);

   RETURN_VAL(&pit->iterator);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_table_efl_pack_grid_grid_position_get(Eo *obj, Edje_Table_Data *pd, Efl_Gfx * subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short c, r, cs, rs;
   Eina_Bool ret;

   ret = evas_object_table_pack_get(pd->rp->object, subobj, &c, &r, &cs, &rs);
   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;

   RETURN_VAL(ret);
}

/* Swallow parts */
EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_get(Eo *obj, Edje_Swallow_Data *pd)
{
   RETURN_VAL(_edje_efl_container_content_get(pd->ed, pd->part));
}

EOLIAN static Eina_Bool
_efl_canvas_layout_internal_swallow_efl_container_content_set(Eo *obj, Edje_Swallow_Data *pd, Efl_Gfx *content)
{
   RETURN_VAL(_edje_efl_container_content_set(pd->ed, pd->part, content));
}

EOLIAN static Efl_Gfx *
_efl_canvas_layout_internal_swallow_efl_container_content_unset(Eo *obj, Edje_Swallow_Data *pd)
{
   Efl_Gfx *content = _edje_efl_container_content_get(pd->ed, pd->part);
   if (!content) RETURN_VAL(NULL);
   PROXY_CALL(efl_content_remove(obj, content));
   RETURN_VAL(content);
}


/* Legacy API implementation */

#ifdef DEGUG
#define PART_BOX_GET(obj, part, ...) ({ \
   Eo *__box = efl_part(obj, part); \
   if (!__box || !eo_isa(__box, EFL_CANVAS_LAYOUT_INTERNAL_BOX_CLASS)) \
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
   Eo *__table = efl_part(obj, part); \
   if (!__table || !eo_isa(__table, EFL_CANVAS_LAYOUT_INTERNAL_TABLE_CLASS)) \
     { \
        ERR("No such table part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __table; })
#else
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_part(obj, part); \
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

#include "efl_canvas_layout_internal.eo.c"
#include "efl_canvas_layout_internal_box.eo.c"
#include "efl_canvas_layout_internal_table.eo.c"
#include "efl_canvas_layout_internal_swallow.eo.c"
