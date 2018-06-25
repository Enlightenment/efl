#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_LAYOUT_PART_BOX_PROTECTED
#define EFL_UI_LAYOUT_PART_TABLE_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#include "../evas/canvas/evas_box.eo.h"
#include "../evas/canvas/evas_table.eo.h"

/* layout internals for box & table */
Eina_Bool    _efl_ui_layout_box_append(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _efl_ui_layout_box_prepend(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _efl_ui_layout_box_insert_before(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, const Evas_Object *reference);
Eina_Bool    _efl_ui_layout_box_insert_at(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, unsigned int pos);
Evas_Object *_efl_ui_layout_box_remove(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _efl_ui_layout_box_remove_all(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Eina_Bool clear);
Eina_Bool    _efl_ui_layout_table_pack(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
Evas_Object *_efl_ui_layout_table_unpack(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Evas_Object *child);
Eina_Bool    _efl_ui_layout_table_clear(Eo *obj, Efl_Ui_Layout_Object_Data *sd, const char *part, Eina_Bool clear);

#define BOX_CLASS   EFL_UI_LAYOUT_PART_BOX_CLASS
#define TABLE_CLASS EFL_UI_LAYOUT_PART_TABLE_CLASS

typedef struct _Layout_Part_Data   Efl_Ui_Layout_Box_Data;
typedef struct _Layout_Part_Data   Efl_Ui_Layout_Table_Data;

struct _Layout_Part_Data
{
   Efl_Ui_Layout_Object         *obj; // no ref
   Efl_Ui_Layout_Object_Data    *sd;  // data xref
   Eina_Stringshare      *part;
   unsigned char          temp;
};

Eo *
_efl_ui_layout_pack_proxy_get(Efl_Ui_Layout_Object *obj, Edje_Part_Type type, const char *part)
{
   if (type == EDJE_PART_TYPE_BOX)
     return efl_add(BOX_CLASS, obj,
                   efl_ui_layout_part_box_real_part_set(efl_added, obj, part));
   else if (type == EDJE_PART_TYPE_TABLE)
     return efl_add(TABLE_CLASS, obj,
                   efl_ui_layout_part_table_real_part_set(efl_added, obj, part));
   else
     return NULL;
}

EOLIAN static void
_efl_ui_layout_part_box_efl_object_destructor(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_HOOK;
   efl_data_xunref(pd->obj, pd->sd, obj);
   eina_stringshare_del(pd->part);
   efl_destructor(efl_super(obj, BOX_CLASS));
}

EOLIAN static void
_efl_ui_layout_part_box_real_part_set(Eo *obj, Efl_Ui_Layout_Box_Data *pd, Eo *layout, const char *part)
{
   pd->obj = layout;
   pd->sd = efl_data_xref(pd->obj, EFL_UI_LAYOUT_OBJECT_CLASS, obj);
   eina_stringshare_replace(&pd->part, part);
   pd->temp = 1;
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_part_box_efl_container_content_iterate(Eo *obj, Efl_Ui_Layout_Box_Data *pd)
{
   Eina_Iterator *it;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   it = evas_object_box_iterator_new(pack);
   return efl_canvas_iterator_create(obj, it, NULL);
}

EOLIAN static int
_efl_ui_layout_part_box_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   return evas_obj_box_count(pack);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   return _efl_ui_layout_box_remove_all(pd->obj, pd->sd, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   return _efl_ui_layout_box_remove_all(pd->obj, pd->sd, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_unpack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   return _efl_ui_layout_box_remove(pd->obj, pd->sd, pd->part, subobj) != NULL;
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_container_content_remove(Eo *obj, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *content)
{
   // alias for efl_pack_unpack
   return _efl_ui_layout_part_box_efl_pack_unpack(obj, pd, content);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_pack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   return _efl_ui_layout_box_append(pd->obj, pd->sd, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   return _efl_ui_layout_box_prepend(pd->obj, pd->sd, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   return _efl_ui_layout_box_append(pd->obj, pd->sd, pd->part, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   return _efl_ui_layout_box_insert_before(pd->obj, pd->sd, pd->part, subobj, existing);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   const Efl_Gfx_Entity *other;
   int index;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   index = efl_pack_index_get(pack, existing);
   if (index < 0) return EINA_FALSE;

   other = efl_pack_content_get(pack, index + 1);
   if (other)
     return _efl_ui_layout_box_insert_before(pd->obj, pd->sd, pd->part, subobj, other);

   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_box_efl_pack_linear_pack_at(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   return _efl_ui_layout_box_insert_at(pd->obj, pd->sd, pd->part, subobj, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_layout_part_box_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, int index)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);

   priv = efl_data_scope_get(pack, EVAS_BOX_CLASS);
   opt = eina_list_nth(priv->children, index);
   if (!opt) return NULL;
   return opt->obj;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_layout_part_box_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Layout_Box_Data *pd, int index)
{
   Efl_Gfx_Entity *subobj;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);

   subobj = efl_pack_content_get(pack, index);
   if (!subobj) return NULL;
   if (efl_pack_unpack(obj, subobj))
     return subobj;

   ERR("failed to remove %p from %p (item %d)", subobj, pd->obj, index);
   return NULL;
}

EOLIAN static int
_efl_ui_layout_part_box_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd, const Efl_Gfx_Entity *subobj)
{
   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   return efl_pack_index_get(pack, subobj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_layout_part_box_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Box_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_DIR_DEFAULT);

   return efl_ui_direction_get(efl_part(wd->resize_obj, pd->part));
}


/* Table proxy implementation */

EOLIAN static void
_efl_ui_layout_part_table_real_part_set(Eo *obj, Efl_Ui_Layout_Table_Data *pd, Eo *layout, const char *part)
{
   pd->obj = layout;
   pd->sd = efl_data_xref(pd->obj, EFL_UI_LAYOUT_OBJECT_CLASS, obj);
   eina_stringshare_replace(&pd->part, part);
   pd->temp = 1;
}

EOLIAN static void
_efl_ui_layout_part_table_efl_object_destructor(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   ELM_PART_HOOK;
   efl_data_xunref(pd->obj, pd->sd, obj);
   eina_stringshare_del(pd->part);
   efl_destructor(efl_super(obj, TABLE_CLASS));
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_part_table_efl_container_content_iterate(Eo *obj, Efl_Ui_Layout_Table_Data *pd)
{
   Eina_Iterator *it;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);

   it = evas_object_table_iterator_new(pack);

   return efl_canvas_iterator_create(obj, it, NULL);
}

EOLIAN static int
_efl_ui_layout_part_table_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   return evas_obj_table_count(pack);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   return _efl_ui_layout_table_clear(pd->obj, pd->sd, pd->part, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_pack_unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   return _efl_ui_layout_table_clear(pd->obj, pd->sd, pd->part, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_pack_unpack(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!subobj) return EINA_FALSE;
   return _efl_ui_layout_table_unpack(pd->obj, pd->sd, pd->part, subobj) == subobj;
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_container_content_remove(Eo *obj, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx_Entity *content)
{
   // alias for efl_pack_unpack
   return _efl_ui_layout_part_table_efl_pack_unpack(obj, pd, content);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_pack_table_pack_table(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx_Entity *subobj, int col, int row, int colspan, int rowspan)
{
   return _efl_ui_layout_table_pack(pd->obj, pd->sd, pd->part, subobj, col, row, colspan, rowspan);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_layout_part_table_efl_pack_table_table_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int col, int row)
{
   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   return evas_object_table_child_get(pack, col, row);
}

EOLIAN static Eina_Iterator *
_efl_ui_layout_part_table_efl_pack_table_table_contents_get(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int col, int row, Eina_Bool below)
{
   // contents at col,row - see also Efl.Ui.Table or edje_containers.c
   // not reusing edje's iterator because the container would be wrong

   Eina_List *list, *l = NULL;
   Evas_Object *sobj;
   unsigned short c, r, cs, rs;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);

   list = evas_object_table_children_get(pack);
   EINA_LIST_FREE(list, sobj)
     {
        evas_object_table_pack_get(pack, sobj, &c, &r, &cs, &rs);

        if (((int) c == col) && ((int) r == row))
          list = eina_list_append(list, sobj);
        else if (below)
          {
             if (((int) c <= col) && ((int) (c + cs) >= col) &&
                 ((int) r <= row) && ((int) (r + rs) >= row))
               list = eina_list_append(list, sobj);
          }
     }

   return efl_canvas_iterator_create(pd->obj, eina_list_iterator_new(l), l);
}

EOLIAN static Eina_Bool
_efl_ui_layout_part_table_efl_pack_table_table_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, Efl_Gfx_Entity * subobj, int *col, int *row, int *colspan, int *rowspan)
{
   unsigned short c, r, cs, rs;
   Eina_Bool ret;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);

   ret = evas_object_table_pack_get(pack, subobj, &c, &r, &cs, &rs);
   if (col) *col = c;
   if (row) *row = r;
   if (colspan) *colspan = cs;
   if (rowspan) *rowspan = rs;

   return ret;
}

EOLIAN static void
_efl_ui_layout_part_table_efl_pack_table_table_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd, int *cols, int *rows)
{
   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   evas_object_table_col_row_size_get(pack, cols, rows);
}

EOLIAN static int
_efl_ui_layout_part_table_efl_pack_table_table_columns_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   int cols, rows;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   evas_object_table_col_row_size_get(pack, &cols, &rows);
   return cols;
}

EOLIAN static int
_efl_ui_layout_part_table_efl_pack_table_table_rows_get(const Eo *obj EINA_UNUSED, Efl_Ui_Layout_Table_Data *pd)
{
   int cols, rows;

   edje_object_freeze(pd->obj);
   Eo *pack = (Eo *) edje_object_part_object_get(pd->obj, pd->part);
   edje_object_thaw(pd->obj);
   evas_object_table_col_row_size_get(pack, &cols, &rows);
   return rows;
}

#include "efl_ui_layout_part_box.eo.c"
#include "efl_ui_layout_part_table.eo.c"
