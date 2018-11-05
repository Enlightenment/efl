#ifndef EFL_UI_LIST_VIEW_SEG_ARRAY_H
#define EFL_UI_LIST_VIEW_SEG_ARRAY_H

typedef struct _Efl_Ui_List_View_Item Efl_Ui_List_View_Item;

typedef struct _Efl_Ui_List_View_SegArray_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int first;

   void* layout_data;

   Efl_Ui_List_View_Layout_Item* pointers[0];
} Efl_Ui_List_View_Seg_Array_Node;

typedef struct _Efl_Ui_List_View_Seg_Array_Data
{
   Efl_Ui_List_View_Seg_Array_Node *root;

   int step_size;
   int node_count;
   int count;
} Efl_Ui_List_View_Seg_Array_Data;

#endif
