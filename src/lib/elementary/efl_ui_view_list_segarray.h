#ifndef EFL_UI_LIST_SEGARRAY_H
#define EFL_UI_LIST_SEGARRAY_H

typedef struct _Efl_Ui_View_List_Item Efl_Ui_View_List_Item;

typedef struct _Efl_Ui_View_List_SegArray_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int first;

   void* layout_data;

   Efl_Ui_View_List_LayoutItem* pointers[0];
} Efl_Ui_View_List_SegArray_Node;

typedef struct _Efl_Ui_View_List_SegArray_Data
{
   Efl_Ui_View_List_SegArray_Node *root;

   int step_size;
   int node_count;
   int count;
} Efl_Ui_View_List_SegArray_Data;

#endif
