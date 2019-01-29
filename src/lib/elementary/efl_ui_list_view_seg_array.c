#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl.h>
#include <assert.h>

#include "efl_ui_list_view_private.h"
#include "efl_ui_list_view_seg_array.h"

static int
_search_lookup_cb(Eina_Rbtree const* rbtree, const void* key, int length EINA_UNUSED, void* data EINA_UNUSED)
{
  Efl_Ui_List_View_Seg_Array_Node const* node = (void const*)rbtree;
  int index = *(int*)key;
  if(index < node->first)
    {
      return 1;
    }
  else if(index < node->first + node->length)
    {
      return 0;
    }
  else
    {
      return -1;
    }
}

static int
_insert_lookup_cb(Eina_Rbtree const* rbtree, const void* key, int length EINA_UNUSED, void* data EINA_UNUSED)
{
  Efl_Ui_List_View_Seg_Array_Node const* node = (void const*)rbtree;
  int index = *(int*)key;
  if(index < node->first)
    {
      return 1;
    }
  else if(index < node->first + node->max)
    {
      return 0;
    }
  else
    {
       return -1;
    }
}

static Eina_Rbtree_Direction
_rbtree_compare(Efl_Ui_List_View_Seg_Array_Node const* left,
                                             Efl_Ui_List_View_Seg_Array_Node const* right, void* data EINA_UNUSED)
{
  if(left->first < right->first)
    return EINA_RBTREE_LEFT;
  else
    return EINA_RBTREE_RIGHT;
}

static void
_free_node(Efl_Ui_List_View_Seg_Array_Node* node, void* data EINA_UNUSED)
{
   int i = 0;

   while (i < node->length)
     {
        Efl_Ui_List_View_Layout_Item* item = node->pointers[i];
        efl_unref(item->children);
        free(item);
        ++i;
     }

   free(node);
}

static Efl_Ui_List_View_Seg_Array_Node*
_alloc_node(Efl_Ui_List_View_Seg_Array* pd, int first)
{
   Efl_Ui_List_View_Seg_Array_Node* node;
   node = calloc(1, sizeof(Efl_Ui_List_View_Seg_Array_Node) + pd->step_size*sizeof(Efl_Ui_List_View_Layout_Item*));
   node->first = first;
   node->max = pd->step_size;
   pd->root = (void*)eina_rbtree_inline_insert(EINA_RBTREE_GET(pd->root), EINA_RBTREE_GET(node),
                                                     EINA_RBTREE_CMP_NODE_CB(&_rbtree_compare), NULL);
   pd->node_count++;
   return node;
}

static Efl_Ui_List_View_Layout_Item*
_create_item_partial(Efl_Model* model)
{
   Efl_Ui_List_View_Layout_Item* item = calloc(1, sizeof(Efl_Ui_List_View_Layout_Item));
   item->children = efl_ref(model);
   return item;
}

static Efl_Ui_List_View_Layout_Item*
_create_item(Efl_Model* model, Efl_Ui_List_View_Seg_Array_Node* node, unsigned int index)
{
   Efl_Ui_List_View_Layout_Item* item =  _create_item_partial(model);
   item->index_offset = index - node->first;
   item->tree_node = node;
   return item;
}

static void
_efl_ui_list_view_seg_array_insert_at_node(Efl_Ui_List_View_Seg_Array* pd, int index,
                            Efl_Ui_List_View_Layout_Item* item, Efl_Ui_List_View_Seg_Array_Node* node)
{
   Eina_Iterator* iterator;
   int pos;

   if(node && node->length != node->max && (index - node->first) <= node->length)
    {
       pos = index - node->first;
       item->tree_node = node;
       item->index_offset = pos;
       if(pos < node->length)
         {
            assert(node->length != node->max);

            memmove(&node->pointers[pos], &node->pointers[pos+1], sizeof(node->pointers[pos])*(node->length - pos));
            node->pointers[pos] = item;
            node->length++;
         }
       else
         {
            assert(pos == node->length);

            assert(node->length != node->max);
            node->pointers[pos] = item;
            node->length++;
         }
    }
  else
    {
       node = _alloc_node(pd, index);
       node->pointers[0] = item;
       node->length++;
       item->index_offset = 0;
       item->tree_node = node;
    }

   node = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
   iterator = eina_rbtree_iterator_infix((void*)node);
   while(eina_iterator_next(iterator, (void**)&node))
     {
       node->first++;
     }

   eina_iterator_free(iterator);
}

static void
_efl_ui_list_view_seg_array_insert_object(Efl_Ui_List_View_Seg_Array *pd, unsigned int index, Efl_Model *children)
{
   Efl_Ui_List_View_Seg_Array_Node *node;

   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(pd->root),
                                           &index, sizeof(index), &_insert_lookup_cb, NULL);
   if (!node)
     {
        node = _alloc_node(pd, index);
     }

   assert(node->length < node->max);
   node->pointers[node->length] = _create_item(children, node, index);
   node->length++;
   pd->count++;
}

typedef struct _Efl_Ui_List_View_Segarray_Eina_Accessor
{
   Eina_Accessor vtable;
   Efl_Ui_List_View_Seg_Array* seg_array;
} Efl_Ui_List_View_Segarray_Eina_Accessor;

static Eina_Bool
_efl_ui_list_view_seg_array_accessor_get_at(Efl_Ui_List_View_Segarray_Eina_Accessor* acc,
                                      int idx, void** data)
{
   Efl_Ui_List_View_Seg_Array_Node* node;
   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(acc->seg_array->root),
                                           &idx, sizeof(idx), &_search_lookup_cb, NULL);
   if (node && (node->first <= idx && node->first + node->length > idx))
     {
         int i = idx - node->first;
         Efl_Ui_List_View_Layout_Item* item = node->pointers[i];
         *data = item;
         return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void*
_efl_ui_list_view_seg_array_accessor_get_container(Efl_Ui_List_View_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_list_view_seg_array_accessor_free(Efl_Ui_List_View_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   free(acc);
}

static Eina_Bool
_efl_ui_list_view_seg_array_accessor_lock(Efl_Ui_List_View_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_efl_ui_list_view_seg_array_accessor_unlock(Efl_Ui_List_View_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Accessor*
_efl_ui_list_view_seg_array_accessor_clone(Efl_Ui_List_View_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_list_view_seg_array_accessor_setup(Efl_Ui_List_View_Segarray_Eina_Accessor* acc, Efl_Ui_List_View_Seg_Array* seg_array)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_list_view_seg_array_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_list_view_seg_array_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_list_view_seg_array_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_list_view_seg_array_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_list_view_seg_array_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_list_view_seg_array_accessor_clone);
   acc->seg_array = seg_array;
}

typedef struct _Efl_Ui_List_View_Segarray_Node_Accessor
{
   Eina_Accessor vtable;
   Efl_Ui_List_View_Seg_Array* seg_array;
   Eina_Iterator* pre_iterator;
   Efl_Ui_List_View_Seg_Array_Node* current_node;
   int current_index;
} Efl_Ui_List_View_Segarray_Node_Accessor;

static Eina_Bool
_efl_ui_list_view_seg_array_node_accessor_get_at(Efl_Ui_List_View_Segarray_Node_Accessor* acc,
                                      int idx, void** data)
{
   if(idx == acc->current_index && acc->current_node)
     {
        (*data) = acc->current_node;
     }
   else
     {
       if(acc->current_index >= idx || !acc->current_node)
         {
            eina_iterator_free(acc->pre_iterator);
            acc->pre_iterator = NULL;
            acc->current_node = NULL;
            acc->current_index = -1;
         }

       if(!acc->pre_iterator)
         acc->pre_iterator = eina_rbtree_iterator_infix((void*)acc->seg_array->root);

       for(;acc->current_index != idx;++acc->current_index)
         {
         if(!eina_iterator_next(acc->pre_iterator, (void**)&acc->current_node))
           {
             --acc->current_index;
             return EINA_FALSE;
           }
         }
       (*data) = acc->current_node;
       return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void*
_efl_ui_list_view_seg_array_node_accessor_get_container(Efl_Ui_List_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_list_view_seg_array_node_accessor_free(Efl_Ui_List_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   if (acc->pre_iterator)
     eina_iterator_free(acc->pre_iterator);
   free(acc);
}

static Eina_Bool
_efl_ui_list_view_seg_array_node_accessor_lock(Efl_Ui_List_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_efl_ui_list_view_seg_array_node_accessor_unlock(Efl_Ui_List_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Accessor*
_efl_ui_list_view_seg_array_node_accessor_clone(Efl_Ui_List_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_list_view_seg_array_node_accessor_setup(Efl_Ui_List_View_Segarray_Node_Accessor* acc, Efl_Ui_List_View_Seg_Array* seg_array)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_list_view_seg_array_node_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_list_view_seg_array_node_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_list_view_seg_array_node_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_list_view_seg_array_node_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_list_view_seg_array_node_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_list_view_seg_array_node_accessor_clone);
   acc->seg_array = seg_array;
   acc->pre_iterator = NULL;
   acc->current_index = -1;
   acc->current_node = NULL;
}

/* External Functions */

Efl_Ui_List_View_Seg_Array *
efl_ui_list_view_seg_array_setup(int size)
{
   Efl_Ui_List_View_Seg_Array *pd = calloc(1, sizeof(Efl_Ui_List_View_Seg_Array));
   pd->step_size = size;

   return pd;
}

void
efl_ui_list_view_seg_array_free(Efl_Ui_List_View_Seg_Array *pd)
{
   if (pd->root)
     eina_rbtree_delete(EINA_RBTREE_GET(pd->root), EINA_RBTREE_FREE_CB(_free_node), NULL);

   pd->root = NULL;
   free(pd);
}

void
efl_ui_list_view_seg_array_flush(Efl_Ui_List_View_Seg_Array *pd)
{
   if (pd->root)
     eina_rbtree_delete(EINA_RBTREE_GET(pd->root), EINA_RBTREE_FREE_CB(_free_node), NULL);

   pd->root = NULL;
   pd->node_count = 0;
   pd->count = 0;
}

int
efl_ui_list_view_seg_array_count(Efl_Ui_List_View_Seg_Array* pd)
{
   return pd->count;
}

void
efl_ui_list_view_seg_array_insert(Efl_Ui_List_View_Seg_Array* pd, int index, Efl_Model* model)
{
  Efl_Ui_List_View_Seg_Array_Node* node, *next;
  Efl_Ui_List_View_Layout_Item* item;

  item = _create_item_partial(model);

  node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(pd->root),
                                          &index, sizeof(index), &_insert_lookup_cb, NULL);
  if(node)
    {
      next = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
      if(next && next->first <= index)
        _efl_ui_list_view_seg_array_insert_at_node(pd, index, item, next);
      else
        _efl_ui_list_view_seg_array_insert_at_node(pd, index, item, node);
    }
  else
    _efl_ui_list_view_seg_array_insert_at_node(pd, index, item, NULL);
}

void
efl_ui_list_view_seg_array_insert_value(Efl_Ui_List_View_Seg_Array *pd, int first, Eina_Value v)
{
   Efl_Model *children;
   unsigned int i, len;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_OBJECT)
     {
        children = eina_value_object_get(&v);
        _efl_ui_list_view_seg_array_insert_object(pd, first, children);
     }
   else if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ARRAY)
     {
        EINA_VALUE_ARRAY_FOREACH(&v, len, i, children)
          {
             unsigned int idx = first + i;

             _efl_ui_list_view_seg_array_insert_object(pd, idx, children);
          }
     }
   else
     {
        printf("Unknow type !\n");
     }
}


Efl_Ui_List_View_Layout_Item*
efl_ui_list_view_seg_array_remove(Efl_Ui_List_View_Seg_Array *pd, int index)
{
   Efl_Ui_List_View_Seg_Array_Node *node;
   Efl_Ui_List_View_Layout_Item *item, *rt;
   Eina_Iterator* iterator;
   int offset;

   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(pd->root),
                                        &index, sizeof(index), &_insert_lookup_cb, NULL);
   if (!node) return NULL;

   offset = index - node->first;
   if (offset >= node->length) return NULL;

   rt = node->pointers[offset];
   pd->count--;
   node->length--;

   while (offset < node->length)
     {
       node->pointers[offset] = node->pointers[offset+1];
       item = node->pointers[offset];
       --item->index_offset;
       ++offset;
     }

   node = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
   iterator = eina_rbtree_iterator_infix((void*)node);
   while(eina_iterator_next(iterator, (void**)&node))
     node->first--;

   return rt;
}

Eina_Accessor*
efl_ui_list_view_seg_array_accessor_get(Efl_Ui_List_View_Seg_Array* pd)
{
   Efl_Ui_List_View_Segarray_Eina_Accessor* acc = calloc(1, sizeof(Efl_Ui_List_View_Segarray_Eina_Accessor));
   _efl_ui_list_view_seg_array_accessor_setup(acc, pd);
   return &acc->vtable;
}

Eina_Accessor*
efl_ui_list_view_seg_array_node_accessor_get(Efl_Ui_List_View_Seg_Array* pd)
{
   Efl_Ui_List_View_Segarray_Node_Accessor* acc = calloc(1, sizeof(Efl_Ui_List_View_Segarray_Node_Accessor));
   _efl_ui_list_view_seg_array_node_accessor_setup(acc, pd);
   return &acc->vtable;
}

int
efl_ui_list_view_item_index_get(Efl_Ui_List_View_Layout_Item* item)
{
  Efl_Ui_List_View_Seg_Array_Node* node = item->tree_node;
  return item->index_offset + node->first;
}
