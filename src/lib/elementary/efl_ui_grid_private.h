#ifndef EFL_UI_GRID_PRIVATE_H
#define EFL_UI_GRID_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Grid_Data Efl_Ui_Grid_Data;
typedef struct _Grid_Item_Iterator Grid_Item_Iterator;
typedef struct _Grid_Item Grid_Item;

#define GRID_ITEM_KEY "__grid_item"

struct _Grid_Item
{
   EINA_INLIST;

   Efl_Gfx *object;
   int col_span, row_span;
   int col, row;

   Eina_Bool linear : 1;
};

struct _Efl_Ui_Grid_Data
{
   const Efl_Class *layout_engine;
   const void     *layout_data;

   Grid_Item *items;
   int count;

   int req_cols, req_rows; // requested - 0 means infinite
   int last_col, last_row; // only used by linear apis
   Efl_Orient dir1, dir2;  // must be orthogonal (H,V or V,H)
   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;
   Eina_Bool linear_recalc : 1;
};

struct _Grid_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Eina_List     *list;
   Efl_Ui_Grid    *object;
};

static inline Eina_Bool
_horiz(Efl_Orient dir)
{
   return dir % 180 == EFL_ORIENT_RIGHT;
}

#endif
