#ifndef EFL_UI_TREE_SEG_ARRAY_H
#define EFL_UI_TREE_SEG_ARRAY_H

#include <Elementary.h>

typedef struct _Efl_Ui_Tree_View_Item Efl_Ui_Tree_View_Item;
typedef struct _Efl_Ui_Tree_View_Seg_Array_Node Efl_Ui_Tree_View_Seg_Array_Node;

typedef struct _Efl_Ui_Tree_View_Seg_Array_Data
{
   Efl_Ui_Tree_View_Seg_Array_Node *root;

   int step_size;
   int node_count;
   int count;
} Efl_Ui_Tree_View_Seg_Array_Data;

struct _Efl_Ui_Tree_View_Seg_Array_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int linearized_first_index, shallow_first_index;

   void* layout_data;
   Efl_Ui_Tree_View_Seg_Array_Data *pd;
   Efl_Ui_Tree_View_Item* pointers[0];
};


#define EFL_UI_TREE_VIEW_SEG_ARRAY_POS_GET(_item)                      \
  _item->shallow_offset + ((Efl_Ui_Tree_View_Seg_Array_Node*)_item->tree_node)->shallow_first_index

#define EFL_UI_TREE_VIEW_SEG_ARRAY_LINEAR_POS_GET(_item)                      \
  _item->linearized_offset + ((Efl_Ui_Tree_View_Seg_Array_Node*)_item->tree_node)->linearized_first_index

#endif
