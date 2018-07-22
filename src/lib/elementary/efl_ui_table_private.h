#ifndef EFL_UI_TABLE_PRIVATE_H
#define EFL_UI_TABLE_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Table_Data Efl_Ui_Table_Data;
typedef struct _Table_Item_Iterator Table_Item_Iterator;
typedef struct _Table_Item Table_Item;

#define TABLE_ITEM_KEY "__table_item"

struct _Table_Item
{
   EINA_INLIST;

   Efl_Gfx_Entity *object;
   int col_span, row_span;
   int col, row;

   Eina_Bool linear : 1;
};

struct _Efl_Ui_Table_Data
{
   Table_Item *items;
   int count;

   int req_cols, req_rows; // requested - 0 means infinite
   int last_col, last_row; // only used by linear apis
   Efl_Ui_Dir dir1, dir2;  // must be orthogonal (H,V or V,H)
   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;
   Eina_Bool linear_recalc : 1;
};

struct _Table_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Eina_List     *list;
   Efl_Ui_Table    *object;
};

#endif
