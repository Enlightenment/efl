#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_LAYOUT_INTERNAL_BOX_PROTECTED
#define EFL_UI_LAYOUT_INTERNAL_TABLE_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#include "efl_ui_layout_internal_box.eo.h"
#include "efl_ui_layout_internal_table.eo.h"
#include "../evas/canvas/evas_box.eo.h"
#include "../evas/canvas/evas_table.eo.h"

/* layout internals for box & table */
Eina_Bool    _elm_layout_box_append(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _elm_layout_box_prepend(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _elm_layout_box_insert_before(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, const Evas_Object *reference);
Eina_Bool    _elm_layout_box_insert_at(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, unsigned int pos);
Evas_Object *_elm_layout_box_remove(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _elm_layout_box_remove_all(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Eina_Bool clear);
Eina_Bool    _elm_layout_table_pack(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
Evas_Object *_elm_layout_table_unpack(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _elm_layout_table_clear(Eo *obj, Elm_Layout_Smart_Data *sd, const char *part, Eina_Bool clear);

#define BOX_CLASS   EFL_UI_LAYOUT_INTERNAL_BOX_CLASS
#define TABLE_CLASS EFL_UI_LAYOUT_INTERNAL_TABLE_CLASS

typedef struct _Layout_Part_Data   Efl_Ui_Layout_Box_Data;
typedef struct _Layout_Part_Data   Efl_Ui_Layout_Table_Data;
typedef struct _Part_Item_Iterator Part_Item_Iterator;

struct _Layout_Part_Data
{
   Elm_Layout            *obj; // no ref
   Elm_Layout_Smart_Data *sd;  // data xref
   Eina_Stringshare      *part;
   Evas_Object           *pack;
   unsigned char          temp;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

Eo *
_elm_layout_pack_proxy_get(Elm_Layout *obj, Evas_Object *pack, const char *part)
{
   Efl_Ui_Layout_Internal_Box *eo;

   if (eo_isa(pack, EVAS_BOX_CLASS))
     eo = eo_add(BOX_CLASS, obj,
                 efl_ui_layout_internal_box_real_part_set(eo_self, obj, pack, part));
   else if (eo_isa(pack, EVAS_TABLE_CLASS))
     eo = eo_add(TABLE_CLASS, obj,
                 efl_ui_layout_internal_table_real_part_set(eo_self, obj, pack, part));
   else
     return NULL;

   return eo;
}

EOLIAN static void
_efl_ui_layout_internal_box_eo_base_destructor(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_HOOK;
   eo_data_xunref(pd->obj, pd->sd, obj);
   eina_stringshare_del(pd->part);
   eo_destructor(eo_super(obj, BOX_CLASS));
}

EOLIAN static void
_efl_ui_layout_internal_box_real_part_set(Eo *obj, Efl_Ui_Layout_Box_Data *pd, Eo *layout, Eo *pack, const char *part)
{
   pd->obj = layout;
   pd->sd = eo_data_xref(pd->obj, ELM_LAYOUT_CLASS, obj);
   eina_stringshare_replace(&pd->part, part);
   pd->pack = pack;
   pd->temp = 1;
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
   eina_list_free(it->list);
   free(it);
}

static Eina_Iterator *
_part_item_iterator_create(Eo *obj, Eina_Iterator *real_iterator, Eina_List *list)
{
   Part_Item_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = list;
   it->real_iterator = real_iterator;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_part_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_part_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_part_item_iterator_free);
   eo_wref_add(obj, &it->object);

   return &it->iterator;
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_internal_box_efl_container_content_iterate(Eo *obj, Efl_Ui_Layout_Box_Data *pd)
{
   Eina_Iterator *it;

   it = evas_object_box_iterator_new(pd->pack);
   ELM_PART_RETURN_VAL(_part_item_iterator_create(obj, it, NULL));
}

EOLIAN static int
_efl_ui_layout_internal_box_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   ELM_PART_RETURN_VAL(evas_obj_box_count(pd->pack));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_remove_all(pd->obj, pd->sd, pd->part, EINA_TRUE));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_remove_all(pd->obj, pd->sd, pd->part, EINA_FALSE));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_unpack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_remove(pd->obj, pd->sd, pd->part, subobj) != NULL);
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_container_content_remove(Eo *obj, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *content)
{
   // alias for efl_pack_unpack
   return _efl_ui_layout_internal_box_efl_pack_unpack(obj, pd, content);
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_pack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_append(pd->obj, pd->sd, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_prepend(pd->obj, pd->sd, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_append(pd->obj, pd->sd, pd->part, subobj));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_insert_before(pd->obj, pd->sd, pd->part, subobj, existing));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj, const Efl_Gfx *existing)
{
   const Efl_Gfx *other;
   int index;

   index = efl_pack_index_get(pd->pack, existing);
   if (index < 0) ELM_PART_RETURN_VAL(EINA_FALSE);

   other = efl_pack_content_get(pd->pack, index + 1);
   if (other)
     ELM_PART_RETURN_VAL(_elm_layout_box_insert_before(pd->obj, pd->sd, pd->part, subobj, other));

   ELM_PART_CALL(efl_pack_end(obj, subobj));
   ELM_PART_RETURN_VAL(EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_box_efl_pack_linear_pack_at(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx *subobj, int index)
{
   ELM_PART_RETURN_VAL(_elm_layout_box_insert_at(pd->obj, pd->sd, pd->part, subobj, index));
}

EOLIAN static Efl_Gfx *
_efl_ui_layout_internal_box_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, int index)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;

   priv = eo_data_scope_get(pd->pack, EVAS_BOX_CLASS);
   opt = eina_list_nth(priv->children, index);
   if (!opt) ELM_PART_RETURN_VAL(NULL);
   ELM_PART_RETURN_VAL(opt->obj);
}

EOLIAN static Efl_Gfx *
_efl_ui_layout_internal_box_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Layout_Box_Data *pd, int index)
{
   Efl_Gfx *subobj;

   subobj = efl_pack_content_get(pd->pack, index);
   if (!subobj) ELM_PART_RETURN_VAL(NULL);
   if (efl_pack_unpack(obj, subobj))
     ELM_PART_RETURN_VAL(subobj);

   ERR("failed to remove %p from %p (item %d)", subobj, pd->obj, index);
   ELM_PART_RETURN_VAL(NULL);
}

EOLIAN static int
_efl_ui_layout_internal_box_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, const Efl_Gfx *subobj)
{
   ELM_PART_RETURN_VAL(efl_pack_index_get(pd->pack, subobj));
}

EOLIAN static Efl_Orient
_efl_ui_layout_internal_box_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_ORIENT_NONE);

   ELM_PART_RETURN_VAL(efl_orientation_get(efl_part(wd->resize_obj, pd->part)));
}


/* Table proxy implementation */

EOLIAN static void
_efl_ui_layout_internal_table_real_part_set(Eo *obj, Efl_Ui_Layout_Table_Data *pd, Eo *layout, Eo *pack, const char *part)
{
   pd->obj = layout;
   pd->sd = eo_data_xref(pd->obj, ELM_LAYOUT_CLASS, obj);
   eina_stringshare_replace(&pd->part, part);
   pd->pack = pack;
   pd->temp = 1;
}

EOLIAN static void
_efl_ui_layout_internal_table_eo_base_destructor(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_HOOK;
   eo_data_xunref(pd->obj, pd->sd, obj);
   eina_stringshare_del(pd->part);
   eo_destructor(eo_super(obj, TABLE_CLASS));
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_internal_table_efl_container_content_iterate(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   Eina_Iterator *it;

   it = evas_object_table_iterator_new(pd->pack);

   ELM_PART_RETURN_VAL(_part_item_iterator_create(obj, it, NULL));
}

EOLIAN static int
_efl_ui_layout_internal_table_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_RETURN_VAL(evas_obj_table_count(pd->pack));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_table_clear(pd->obj, pd->sd, pd->part, EINA_TRUE));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_RETURN_VAL(_elm_layout_table_clear(pd->obj, pd->sd, pd->part, EINA_FALSE));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_pack_unpack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx *subobj)
{
   if (!subobj) ELM_PART_RETURN_VAL(EINA_FALSE);
   ELM_PART_RETURN_VAL(_elm_layout_table_unpack(pd->obj, pd->sd, pd->part, subobj) == subobj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_container_content_remove(Eo *obj, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx *content)
{
   // alias for efl_pack_unpack
   return _efl_ui_layout_internal_table_efl_pack_unpack(obj, pd, content);
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_pack_grid_pack_grid(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx *subobj, int col, int row, int colspan, int rowspan)
{
   ELM_PART_RETURN_VAL(_elm_layout_table_pack(pd->obj, pd->sd, pd->part, subobj, col, row, colspan, rowspan));
}

EOLIAN static Efl_Gfx *
_efl_ui_layout_internal_table_efl_pack_grid_grid_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int col, int row)
{
   ELM_PART_RETURN_VAL(evas_object_table_child_get(pd->pack, col, row));
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_internal_table_efl_pack_grid_grid_contents_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int col, int row, Eina_Bool below)
{
   // contents at col,row - see also Efl.Ui.Grid or edje_containers.c
   // not reusing edje's iterator because the container would be wrong

   Eina_List *list, *l = NULL;
   Evas_Object *sobj;
   unsigned short c, r, cs, rs;

   list = evas_object_table_children_get(pd->pack);
   EINA_LIST_FREE(list, sobj)
     {
        evas_object_table_pack_get(pd->pack, sobj, &c, &r, &cs, &rs);

        if (((int) c == col) && ((int) r == row))
          list = eina_list_append(list, sobj);
        else if (below)
          {
             if (((int) c <= col) && ((int) (c + cs) >= col) &&
                 ((int) r <= row) && ((int) (r + rs) >= row))
               list = eina_list_append(list, sobj);
          }
     }

   ELM_PART_RETURN_VAL(_part_item_iterator_create(pd->obj, eina_list_iterator_new(l), l));
}

EOLIAN static Eina_Bool
_efl_ui_layout_internal_table_efl_pack_grid_grid_position_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx * subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short c, r, cs, rs;
   Eina_Bool ret;

   ret = evas_object_table_pack_get(pd->pack, subobj, &c, &r, &cs, &rs);
   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;

   ELM_PART_RETURN_VAL(ret);
}

EOLIAN static void
_efl_ui_layout_internal_table_efl_pack_grid_grid_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int *cols, int *rows)
{
   evas_object_table_col_row_size_get(pd->pack, cols, rows);
}

EOLIAN static int
_efl_ui_layout_internal_table_efl_pack_grid_grid_columns_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   int cols, rows;
   evas_object_table_col_row_size_get(pd->pack, &cols, &rows);
   ELM_PART_RETURN_VAL(cols);
}

EOLIAN static int
_efl_ui_layout_internal_table_efl_pack_grid_grid_rows_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   int cols, rows;
   evas_object_table_col_row_size_get(pd->pack, &cols, &rows);
   ELM_PART_RETURN_VAL(rows);
}

#include "efl_ui_layout_internal_box.eo.c"
#include "efl_ui_layout_internal_table.eo.c"
