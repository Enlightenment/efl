#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_list_private.h"
#include "efl_ui_list_segarray.h"

#include <Efl.h>

#include <assert.h>

static int _search_lookup_cb(Eina_Rbtree const* rbtree, const void* key, int length EINA_UNUSED, void* data EINA_UNUSED)
{
  Efl_Ui_List_SegArray_Node const* node = (void const*)rbtree;
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

static int _insert_lookup_cb(Eina_Rbtree const* rbtree, const void* key, int length EINA_UNUSED, void* data EINA_UNUSED)
{
  Efl_Ui_List_SegArray_Node const* node = (void const*)rbtree;
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

static Eina_Rbtree_Direction _rbtree_compare(Efl_Ui_List_SegArray_Node const* left,
                                             Efl_Ui_List_SegArray_Node const* right, void* data EINA_UNUSED)
{
  if(left->first < right->first)
    return EINA_RBTREE_LEFT;
  else
    return EINA_RBTREE_RIGHT;
}


static void
_free_node(Efl_Ui_List_SegArray_Node* node, void* data EINA_UNUSED)
{
   int i = 0;

   while (i < node->length)
     {
       free(node->pointers[i]);
       ++i;
     }

   free(node);
}

static Efl_Ui_List_SegArray_Node*
_alloc_node(Efl_Ui_List_SegArray* segarray, int first, int max)
{
   Efl_Ui_List_SegArray_Node* node;
   node = calloc(1, sizeof(Efl_Ui_List_SegArray_Node) + max*sizeof(Efl_Ui_List_Item*));
   node->first = first;
   node->max = max;
   segarray->root = (void*)eina_rbtree_inline_insert(EINA_RBTREE_GET(segarray->root), EINA_RBTREE_GET(node),
                                                     EINA_RBTREE_CMP_NODE_CB(&_rbtree_compare), NULL);
   segarray->node_count++;
   return node;
}

void efl_ui_list_segarray_setup(Efl_Ui_List_SegArray* segarray, //int member_size,
                                int initial_step_size)
{
   segarray->root = NULL;
   segarray->array_initial_size = initial_step_size;
}

void efl_ui_list_segarray_flush(Efl_Ui_List_SegArray* segarray)
{
   eina_rbtree_delete(EINA_RBTREE_GET(segarray->root), EINA_RBTREE_FREE_CB(_free_node), NULL);
   segarray->root = NULL;
}

static Efl_Ui_List_Item* _create_item_partial(Efl_Model* model)
{
   Efl_Ui_List_Item* item = calloc(1, sizeof(Efl_Ui_List_Item));
   item->item.children = model;
   return item;
}

static Efl_Ui_List_Item* _create_item(Efl_Model* model, Efl_Ui_List_SegArray_Node* node, unsigned int index)
{
   Efl_Ui_List_Item* item =  _create_item_partial(model);
   item->item.index_offset = index - node->first;
   item->item.tree_node = node;
   return item;
}

Efl_Ui_List_Item*
efl_ui_list_segarray_remove(Efl_Ui_List_SegArray* segarray, int index)
{
   Efl_Ui_List_SegArray_Node *node;
   Efl_Ui_List_Item *item, *rt;
   Eina_Iterator* iterator;
   int offset;

   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(segarray->root),
                                        &index, sizeof(index), &_insert_lookup_cb, NULL);
   if (!node) return NULL;

   offset = index - node->first;
   if (offset >= node->length) return NULL;

   rt = node->pointers[offset];
   segarray->count--;
   node->length--;

   while (offset < node->length)
     {
       node->pointers[offset] = node->pointers[offset+1];
       item = node->pointers[offset];
       --item->item.index_offset;
       ++offset;
     }

   node = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
   iterator = eina_rbtree_iterator_infix((void*)node);
   while(eina_iterator_next(iterator, (void**)&node))
     node->first--;

   return rt;
}

static void
_efl_ui_list_segarray_insert_at_node(Efl_Ui_List_SegArray* segarray, int index, Efl_Ui_List_Item* item, Efl_Ui_List_SegArray_Node* node)
{
   Eina_Iterator* iterator;
   int pos;

   if(node && node->length != node->max && (index - node->first) <= node->length)
    {
       pos = index - node->first;
       item->item.tree_node = node;
       item->item.index_offset = pos;
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
       node = _alloc_node(segarray, index, segarray->array_initial_size);
       node->pointers[0] = item;
       node->length++;
       item->item.index_offset = 0;
       item->item.tree_node = node;
    }

   node = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
   iterator = eina_rbtree_iterator_infix((void*)node);
   while(eina_iterator_next(iterator, (void**)&node))
     {
       node->first++;
     }

   eina_iterator_free(iterator);
}


void efl_ui_list_segarray_insert(Efl_Ui_List_SegArray* segarray, int index, Efl_Model* model)
{
  Efl_Ui_List_SegArray_Node* node, *next;
  Efl_Ui_List_Item* item;

  item = _create_item_partial(model);
  
  node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(segarray->root),
                                          &index, sizeof(index), &_insert_lookup_cb, NULL);
  if(node)
    {
      next = (void*)EINA_RBTREE_GET(node)->son[EINA_RBTREE_LEFT];
      if(next && next->first <= index)
        _efl_ui_list_segarray_insert_at_node(segarray, index, item, next);
      else
        _efl_ui_list_segarray_insert_at_node(segarray, index, item, node);
    }
  else
    _efl_ui_list_segarray_insert_at_node(segarray, index, item, NULL);
}

void efl_ui_list_segarray_insert_accessor(Efl_Ui_List_SegArray* segarray, int first, Eina_Accessor* accessor)
{
   int i;
   Efl_Model* children;

   EINA_ACCESSOR_FOREACH(accessor, i, children)
     {
        Efl_Ui_List_SegArray_Node *node;
        int idx = first + i;

        node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(segarray->root),
                                                &idx, sizeof(idx), &_insert_lookup_cb, NULL);
        if (!node)
          {
             node = _alloc_node(segarray, idx, segarray->array_initial_size);
          }

        assert(node->length < node->max);
        node->pointers[node->length] = _create_item(children, node, idx);
        node->length++;
        segarray->count++;
     }
}

int efl_ui_list_segarray_count(Efl_Ui_List_SegArray const* segarray)
{
   return segarray->count;
}

typedef struct _Efl_Ui_List_Segarray_Eina_Accessor
{
   Eina_Accessor vtable;
   Efl_Ui_List_SegArray* segarray;
} Efl_Ui_List_Segarray_Eina_Accessor;

static Eina_Bool
_efl_ui_list_segarray_accessor_get_at(Efl_Ui_List_Segarray_Eina_Accessor* acc,
                                      int idx, void** data)
{
   Efl_Ui_List_SegArray_Node* node;
   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(acc->segarray->root),
                                           &idx, sizeof(idx), &_search_lookup_cb, NULL);
   if(node)
     {
        if(node->first <= idx && node->first + node->length > idx)
          {
             int i = idx - node->first;
             Efl_Ui_List_Item* item = node->pointers[i];
             *data = item;
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static void*
_efl_ui_list_segarray_accessor_get_container(Efl_Ui_List_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_list_segarray_accessor_free(Efl_Ui_List_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   free(acc);
}

static void
_efl_ui_list_segarray_accessor_lock(Efl_Ui_List_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
}

static void
_efl_ui_list_segarray_accessor_unlock(Efl_Ui_List_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
}

static Eina_Accessor*
_efl_ui_list_segarray_accessor_clone(Efl_Ui_List_Segarray_Eina_Accessor* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_list_segarray_accessor_setup(Efl_Ui_List_Segarray_Eina_Accessor* acc, Efl_Ui_List_SegArray* segarray)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_list_segarray_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_list_segarray_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_list_segarray_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_list_segarray_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_list_segarray_accessor_clone);
   acc->segarray = segarray;
}

Eina_Accessor* efl_ui_list_segarray_accessor_get(Efl_Ui_List_SegArray* segarray)
{
   Efl_Ui_List_Segarray_Eina_Accessor* acc = calloc(1, sizeof(Efl_Ui_List_Segarray_Eina_Accessor));
   _efl_ui_list_segarray_accessor_setup(acc, segarray);
   return &acc->vtable;
}

typedef struct _Efl_Ui_List_Segarray_Node_Accessor
{
   Eina_Accessor vtable;
   Efl_Ui_List_SegArray* segarray;
   Eina_Iterator* pre_iterator;
   Efl_Ui_List_SegArray_Node* current_node;
   int current_index;
} Efl_Ui_List_Segarray_Node_Accessor;

static Eina_Bool
_efl_ui_list_segarray_node_accessor_get_at(Efl_Ui_List_Segarray_Node_Accessor* acc,
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
         acc->pre_iterator = eina_rbtree_iterator_infix((void*)acc->segarray->root);

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
_efl_ui_list_segarray_node_accessor_get_container(Efl_Ui_List_Segarray_Node_Accessor* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_list_segarray_node_accessor_free(Efl_Ui_List_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   if (acc->pre_iterator)
     eina_iterator_free(acc->pre_iterator);
   free(acc);
}

static void
_efl_ui_list_segarray_node_accessor_lock(Efl_Ui_List_Segarray_Node_Accessor* acc EINA_UNUSED)
{
}

static void
_efl_ui_list_segarray_node_accessor_unlock(Efl_Ui_List_Segarray_Node_Accessor* acc EINA_UNUSED)
{
}

static Eina_Accessor*
_efl_ui_list_segarray_node_accessor_clone(Efl_Ui_List_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_list_segarray_node_accessor_setup(Efl_Ui_List_Segarray_Node_Accessor* acc, Efl_Ui_List_SegArray* segarray)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_list_segarray_node_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_list_segarray_node_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_list_segarray_node_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_list_segarray_node_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_list_segarray_node_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_list_segarray_node_accessor_clone);
   acc->segarray = segarray;
   acc->pre_iterator = NULL;
   acc->current_index = -1;
   acc->current_node = NULL;
}

Eina_Accessor* efl_ui_list_segarray_node_accessor_get(Efl_Ui_List_SegArray* segarray)
{
   Efl_Ui_List_Segarray_Node_Accessor* acc = calloc(1, sizeof(Efl_Ui_List_Segarray_Node_Accessor));
   _efl_ui_list_segarray_node_accessor_setup(acc, segarray);
   return &acc->vtable;
}

int efl_ui_list_item_index_get(Efl_Ui_List_Item const* item)
{
  Efl_Ui_List_SegArray_Node* node = item->item.tree_node;
  return item->item.index_offset + node->first;
}

