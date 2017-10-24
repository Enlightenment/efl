#ifndef EFL_UI_LIST_SEGARRAY_H
#define EFL_UI_LIST_SEGARRAY_H

#include "efl_ui_list_segarray.h"

typedef struct _Efl_Ui_List_SegArray_Node
{
   EINA_RBTREE;
  
   int length;
   int max;
   int first;

   // Eina_Position2D initial_position;

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
void efl_ui_list_segarray_insert_accessor(Efl_Ui_List_SegArray* segarray, int first, Eina_Accessor* accessor);
int efl_ui_list_segarray_count(Efl_Ui_List_SegArray const* segarray);
void efl_ui_list_segarray_setup(Efl_Ui_List_SegArray* segarray, //int member_size,
                                int initial_step_size);

#endif

