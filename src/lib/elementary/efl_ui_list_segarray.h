#ifndef EFL_UI_LIST_SEGARRAY_H
#define EFL_UI_LIST_SEGARRAY_H

#include "efl_ui_list_segarray.h"

typedef struct _Efl_Ui_List_Item Efl_Ui_List_Item;

typedef struct _Efl_Ui_List_SegArray_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int first;

   void* layout_data;

   Efl_Ui_List_Item* pointers[0];
} Efl_Ui_List_SegArray_Node;

typedef struct _Efl_Ui_List_SegArray
{
   Efl_Ui_List_SegArray_Node *root; // of Efl_Ui_List_SegArray_Nodea

   int array_initial_size;
   int node_count;
   int count;
} Efl_Ui_List_SegArray;

Eina_Accessor* efl_ui_list_segarray_accessor_get(Efl_Ui_List_SegArray* segarray);
Eina_Accessor* efl_ui_list_segarray_node_accessor_get(Efl_Ui_List_SegArray* segarray);
Efl_Ui_List_SegArray_Node* efl_ui_list_segarray_item_node_get(Efl_Ui_List_SegArray* segarray,
                                                              Efl_Ui_List_Item* item);
void efl_ui_list_segarray_insert_accessor(Efl_Ui_List_SegArray* segarray, int first, Eina_Accessor* accessor);
int efl_ui_list_segarray_count(Efl_Ui_List_SegArray const* segarray);
void efl_ui_list_segarray_setup(Efl_Ui_List_SegArray* segarray, int initial_step_size);
void efl_ui_list_segarray_flush(Efl_Ui_List_SegArray* segarray);

void efl_ui_list_segarray_insert(Efl_Ui_List_SegArray* segarray, int index, Efl_Model* model);
Efl_Ui_List_Item *efl_ui_list_segarray_remove(Efl_Ui_List_SegArray* segarray, int index);

#endif
