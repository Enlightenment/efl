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

void _efl_ui_table_custom_layout(Efl_Ui_Table *ui_table, Efl_Ui_Table_Data *pd);

#define TABLE_ITEM_KEY "__table_item"

struct _Table_Item
{
   EINA_INLIST;

   Efl_Gfx_Entity *object;
   int col_span, row_span;
   int col, row;
};

struct _Efl_Ui_Table_Data
{
   Table_Item *items;
   Eo *clipper;
   int count;

   int cols, rows;
   int req_cols, req_rows; // requested - 0 means infinite
   int last_col, last_row; // only used by pack api
   Efl_Ui_Dir fill_dir;    // direction in which cells are added when using pack()
   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;
   struct {
      double h, v;
   } align;
   Eina_Bool cols_recalc : 1;
   Eina_Bool rows_recalc : 1;
   Eina_Bool linear_recalc : 1;
   Eina_Bool homogeneoush : 1;
   Eina_Bool homogeneousv : 1;
};

struct _Table_Item_Iterator
{
   Eina_Iterator  iterator;
   Efl_Ui_Table  *object;
   Eina_Inlist   *cur;
};

#endif
