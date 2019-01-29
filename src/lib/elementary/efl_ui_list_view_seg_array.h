#ifndef EFL_UI_LIST_VIEW_SEG_ARRAY_H
#define EFL_UI_LIST_VIEW_SEG_ARRAY_H

typedef struct _Efl_Ui_List_View_SegArray_Node
{
   EINA_RBTREE;

   int length;
   int max;
   int first;

   void* layout_data;

   Efl_Ui_List_View_Layout_Item* pointers[0];
} Efl_Ui_List_View_Seg_Array_Node;

typedef struct _Efl_Ui_List_View_Seg_Array
{
   Efl_Ui_List_View_Seg_Array_Node *root;

   int step_size;
   int node_count;
   int count;
} Efl_Ui_List_View_Seg_Array;


Efl_Ui_List_View_Seg_Array * efl_ui_list_view_seg_array_setup(int size);
void efl_ui_list_view_seg_array_free(Efl_Ui_List_View_Seg_Array *seg_array);
void efl_ui_list_view_seg_array_flush(Efl_Ui_List_View_Seg_Array *seg_array);
int efl_ui_list_view_seg_array_count(Efl_Ui_List_View_Seg_Array* seg_array);
int efl_ui_list_view_item_index_get(Efl_Ui_List_View_Layout_Item* item);


void efl_ui_list_view_seg_array_insert(Efl_Ui_List_View_Seg_Array* seg_array, int index, Efl_Model* model);
void efl_ui_list_view_seg_array_insert_value(Efl_Ui_List_View_Seg_Array *seg_array, int first, Eina_Value value);
Efl_Ui_List_View_Layout_Item* efl_ui_list_view_seg_array_remove(Efl_Ui_List_View_Seg_Array *seg_array, int index);

Eina_Accessor* efl_ui_list_view_seg_array_accessor_get(Efl_Ui_List_View_Seg_Array* seg_array);
Eina_Accessor* efl_ui_list_view_seg_array_node_accessor_get(Efl_Ui_List_View_Seg_Array* seg_array);


#endif
