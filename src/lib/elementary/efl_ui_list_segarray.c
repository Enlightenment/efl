#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_list_private.h"
#include "efl_ui_list_segarray.h"

#include <Efl.h>

static Eina_Rbtree_Direction _rbtree_compare(Eina_Rbtree const* left, Eina_Rbtree const* right)
{
  
}

void efl_ui_list_segarray_setup(Efl_Ui_List_SegArray* segarray, //int member_size,
                                int initial_step_size)
{
   segarray->root = NULL;
   /* segarray->member_size = member_size; */
   segarray->array_initial_size = initial_step_size;
}

void efl_ui_list_segarray_insert_accessor(int first, Eina_Accessor* accessor)
{
   int i;
   Efl_Model* children;
   EINA_ACCESSOR_FOREACH(accessor, i, children)
     {
        ;
     }
}

static void _insert(Efl_Ui_List_SegArray* segarray, Efl_Ui_List_Item* item)
{
  
}

static void _prepend(Efl_Ui_List_SegArray* segarray, Efl_Ui_List_Item* item)
{
   
}

/* static void */
/* _insert(int pos, Efl_Model* model) */
/* { */
/*    Efl_Ui_List_Item* item = malloc(sizeof(Efl_Ui_List_Item)); */
/*    item->model = model; */

   
/* } */

/* inline static Efl_Ui_List_Item** */
/* _back_empty_get_or_null(Efl_Ui_List_SegArray* array) */
/* { */
/*    /\* void* inlist_last = eina_rbtree_last(array->list); *\/ */
/*    /\* Efl_Ui_List_SegArray_Node* node = inlist_last; *\/ */
/*    /\* if(node && node->max == node->length) *\/ */
/*    /\*   return &node->pointers[node->length++]; *\/ */
/*    /\* else *\/ */
/*      return NULL; */
/* } */

/* inline static Efl_Ui_List_Item** */
/* _alloc_back_and_return_last(Efl_Ui_List_SegArray* array) */
/* { */
/*    Efl_Ui_List_SegArray_Node* new_node = calloc(1, sizeof(Efl_Ui_List_SegArray_Node) + array->array_initial_size); */
/*    new_node->length = 0; */
/*    new_node->max = array->array_initial_size; */

/*    array->list = eina_inlist_append(array->list, EINA_INLIST_GET(new_node)); */
/*    return &new_node->pointers[0]; */
/* } */


/* /\* void efl_ui_list_segarray_insert_at_index(Efl_Ui_List_SegArray* array EINA_UNUSED, int index EINA_UNUSED, *\/ */
/* /\*                                           Efl_Ui_List_Item* item EINA_UNUSED) *\/ */
/* /\* { *\/ */

/* /\* } *\/ */

/* void efl_ui_list_segarray_append(Efl_Ui_List_SegArray* array, Efl_Ui_List_Item* item) */
/* { */
/*    Efl_Ui_List_Item** new_item = _back_empty_get_or_null(array); */
/*    if(!new_item) */
/*      new_item = _alloc_back_and_return_last(array); */

/*    *new_item = item; */
/* } */

/* /\* void efl_ui_list_segarray_insert_at(Efl_Ui_List_SegArray* array, int position, Efl_Ui_List_Item* item) *\/ */
/* /\* { *\/ */
   
/* /\* } *\/ */


