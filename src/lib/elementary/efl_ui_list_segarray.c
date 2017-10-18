#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_list_private.h"
#include "efl_ui_list_segarray.h"

#include <Efl.h>

static Eina_Rbtree_Direction _rbtree_compare(Eina_Rbtree const* left, Eina_Rbtree const* right)
{
   Efl_Ui_List_SegArray_Node *nl = left, *nr = right;
   return !nl ? EINA_RBTREE_LEFT :
     (
       !nr ? EINA_RBTREE_RIGHT :
       (
        nl->first < nr->first ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT
       )
     );
}

static Efl_Ui_List_SegArray_Node*
_alloc_node(Efl_Ui_List_SegArray* segarray, int first, int max)
{
   Efl_Ui_List_SegArray_Node* node;
   node = malloc(sizeof(Efl_Ui_List_SegArray_Node) + max*sizeof(Efl_Ui_List_Item*));
   node->first = first;
   node->max = max;
   segarray->root = eina_rbtree_inline_insert(segarray->root, EINA_RBTREE_GET(node), &_rbtree_compare, NULL);
   segarray->node_count++;
   return node;
}

void efl_ui_list_segarray_setup(Efl_Ui_List_SegArray* segarray, //int member_size,
                                int initial_step_size)
{
   segarray->root = NULL;
   /* segarray->member_size = member_size; */
   segarray->array_initial_size = initial_step_size;
}

void efl_ui_list_segarray_insert_accessor(Efl_Ui_List_SegArray* segarray, int first, Eina_Accessor* accessor)
{
   int i;
   Efl_Model* children;
   Efl_Ui_List_SegArray_Node* node = NULL;
   Efl_Ui_List_SegArray_Node* first_node = NULL;
   int array_first = 0;

   if(segarray->root)
     {
        Eina_Iterator* pre_iterator = eina_rbtree_iterator_prefix(segarray->root);
        eina_iterator_next(pre_iterator, &first_node);
        array_first = first_node->first;
     }

   EINA_ACCESSOR_FOREACH(accessor, i, children)
     {
        if((first + i < array_first) || !efl_ui_list_segarray_count(segarray))
          {
             if(!node)
               {
                  node = _alloc_node(segarray, i + first, segarray->array_initial_size);
               }
             else
               {
               }
          }
        else if(first + i < array_first + efl_ui_list_segarray_count(segarray))
          {
          }
        else
          {
          }
     }
}

int efl_ui_list_segarray_count(Efl_Ui_List_SegArray const* segarray)
{
   return segarray->count;
}

typedef struct _Efl_Ui_List_Segarray_Eina_Accessor
{
   Eina_Accessor vtable;
   
} Efl_Ui_List_Segarray_Eina_Accessor;

static Eina_Bool
_efl_ui_list_segarray_accessor_get_at(Efl_Ui_List_Segarray_Eina_Accessor* acc,
                                      unsigned int idx, void** data)
{
}

static void*
_efl_ui_list_segarray_accessor_get_container(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
}

static void
_efl_ui_list_segarray_accessor_free(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
}

static void
_efl_ui_list_segarray_accessor_lock(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
}

static void
_efl_ui_list_segarray_accessor_unlock(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
}

static Eina_Accessor*
_efl_ui_list_segarray_accessor_clone(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
   return acc;
}

static void
_efl_ui_list_segarray_accessor_setup(Efl_Ui_List_Segarray_Eina_Accessor* acc)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_clone);
}

Eina_Accessor* efl_ui_list_segarray_accessor_get(Efl_Ui_List_SegArray* segarray)
{
   Efl_Ui_List_Segarray_Eina_Accessor* acc = calloc(1, sizeof(Efl_Ui_List_Segarray_Eina_Accessor));
   _efl_ui_list_segarray_accessor_setup(acc);
   return &acc->vtable;
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


