#ifndef EFL_UI_TREE_SEG_ARRAY_H
#define EFL_UI_TREE_SEG_ARRAY_H

#include <Elementary.h>

typedef struct _Efl_Ui_Tree_View_Item Efl_Ui_Tree_View_Item;
typedef struct _Efl_Ui_Tree_View_Seg_Array_Node Efl_Ui_Tree_View_Seg_Array_Node;

typedef struct _Efl_Ui_Tree_View_Seg_Array
{
   Efl_Ui_Tree_View_Seg_Array_Node* root;

   int step_size;
   int node_count;
   int count;
} Efl_Ui_Tree_View_Seg_Array;

struct _Efl_Ui_Tree_View_Seg_Array_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int linearized_first_index, shallow_first_index;

   void* layout_data;
   Efl_Ui_Tree_View_Seg_Array* seg_array;
   Efl_Ui_Tree_View_Item* pointers[0];
};


Efl_Ui_Tree_View_Seg_Array* efl_ui_tree_view_seg_array_setup(int size);
void efl_ui_tree_view_seg_array_free(Efl_Ui_Tree_View_Seg_Array *seg_array);
void efl_ui_tree_view_seg_array_flush(Efl_Ui_Tree_View_Seg_Array *seg_array);
int efl_ui_tree_view_seg_array_count(Efl_Ui_Tree_View_Seg_Array *seg_array);

void efl_ui_tree_view_seg_array_insert(Efl_Ui_Tree_View_Seg_Array *seg_array, Efl_Ui_Tree_View_Item *parent, int shallow_index, Efl_Model *model);
Efl_Ui_Tree_View_Item* efl_ui_tree_view_seg_array_remove(Efl_Ui_Tree_View_Seg_Array *seg_array, int index);

Eina_Accessor* efl_ui_tree_view_seg_array_shallow_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array);
Eina_Accessor* efl_ui_tree_view_seg_array_linearized_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array);
Eina_Accessor* efl_ui_tree_view_seg_array_node_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array);

#define EFL_UI_TREE_VIEW_SEG_ARRAY_POS_GET(_item)                      \
  _item->shallow_offset + ((Efl_Ui_Tree_View_Seg_Array_Node*)_item->tree_node)->shallow_first_index

#define EFL_UI_TREE_VIEW_SEG_ARRAY_LINEAR_POS_GET(_item)                      \
  _item->linearized_offset + ((Efl_Ui_Tree_View_Seg_Array_Node*)_item->tree_node)->linearized_first_index

#endif
