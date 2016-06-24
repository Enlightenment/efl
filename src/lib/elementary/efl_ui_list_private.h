#ifndef EFL_UI_LIST_PRIVATE_H
#define EFL_UI_LIST_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_List_Data Efl_Ui_List_Data;
typedef struct _Item_Size Item_Size;
typedef struct _Item_Block Item_Block;
typedef struct _Item_Private Item_Private;

struct _Efl_Ui_List_Data
{
   Eo           *obj;
   Efl_Model    *model;
   Eina_List    *item_blocks;
   int           count;
   Eina_Promise *count_promise;
   Eina_Promise *child_promise;
   Evas_Coord    x, y, w, h;
   Eina_Bool     v_scroll: 1;
   Eina_Bool     h_scroll: 1;
   Eina_Bool     homogeneous: 1;
   Eina_Bool     calculated: 1;
   Eina_Bool     changed: 1;
};

struct _Item_Size
{
   Evas_Coord w, h;
}

struct _Item_Block
{
   int               count;
   Evas_Coord        x, y, w, h;
   Efl_Ui_List_Data *wd;
   Eina_List        *items;
   Eina_Bool         calculated: 1;
   Eina_Bool         realized: 1;
}

struct _Item_Private;
{
   int               index; // index from the block
   Evas_Coord        x, y; // x, y position from the block
   Evas_Coord        w, h;
   Item_Block       *block;
   Efl_Ui_List_Data *pd;
   Eo               *layout;
   Efl_Model        *my_model;
   Eina_Bool         queued: 1;
   Eina_Bool         calc_once: 1;
   Eina_Bool         calculated: 1;
   Eina_Bool         realized: 1;
   Eina_Bool         disabled: 1;
   Eina_Bool         selected: 1;
   Eina_Bool         focused: 1;
};

#endif
