#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl.h>
#include <assert.h>

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TREE_VIEW_SEG_ARRAY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tree_View_Seg_Array"

#include "efl_ui_tree_view_seg_array.h"

static int _shallow_search_lookup_cb(Eina_Rbtree const* rbtree, const void* key, int length EINA_UNUSED, void* data EINA_UNUSED)
{
  Efl_Ui_Tree_View_Seg_Array_Node const* node = (void const*)rbtree;
  int index = *(int*)key;
  if (index < node->shallow_first_index)
    {
      return 1;
    }
  else if (index < node->shallow_first_index + node->length)
    {
      return 0;
    }
  else
    {
      return -1;
    }
}

static Eina_Rbtree_Direction _shallow_rbtree_compare(Efl_Ui_Tree_View_Seg_Array_Node const* left,
                                             Efl_Ui_Tree_View_Seg_Array_Node const* right, void* data EINA_UNUSED)
{
  if (left->shallow_first_index < right->shallow_first_index)
    return EINA_RBTREE_LEFT;
  else
    return EINA_RBTREE_RIGHT;
}

static void
_free_node(Efl_Ui_Tree_View_Seg_Array_Node* node, void* data EINA_UNUSED)
{
   int i = 0;

   while (i < node->length)
     {
        Efl_Ui_Tree_View_Item* item = node->pointers[i];
        if (item->segarray)
          {
             efl_ui_tree_view_seg_array_free(item->segarray);
          }
        efl_unref(item->model);
        free(item);
        ++i;
     }

   free(node);
}

static Efl_Ui_Tree_View_Seg_Array_Node*
_alloc_node(Efl_Ui_Tree_View_Seg_Array* seg_array, int shallow_first_index, int linearized_first_index)
{
   Efl_Ui_Tree_View_Seg_Array_Node* node;
   node = calloc(1, sizeof(Efl_Ui_Tree_View_Seg_Array_Node) + seg_array->step_size*sizeof(Efl_Ui_Tree_View_Item*));
   node->linearized_first_index = linearized_first_index;
   node->shallow_first_index = shallow_first_index;
   node->max = seg_array->step_size;
   node->seg_array = seg_array;
   seg_array->root = (void*)eina_rbtree_inline_insert(EINA_RBTREE_GET(seg_array->root), EINA_RBTREE_GET(node),
                                                     EINA_RBTREE_CMP_NODE_CB(&_shallow_rbtree_compare), NULL);
   seg_array->node_count++;
   return node;
}

static Efl_Ui_Tree_View_Item* _create_item_partial(Efl_Model* model, Efl_Ui_Tree_View_Item* parent)
{
   Eina_Value *v;
   Efl_Ui_Tree_View_Item* item;

   item = calloc(1, sizeof(Efl_Ui_Tree_View_Item));
   item->parent = parent;
   item->depth = 1;

   if (parent)
     item->depth += parent->depth;

   v = eina_value_uint_new(item->depth);
   item->model = efl_new(EFL_UI_TREE_VIEW_SEG_ARRAY_DEPTH_MODEL_CLASS,
                            efl_ui_view_model_set(efl_added, model),
                            efl_model_property_set(efl_added, "depth", v));
   eina_value_free(v);

   return item;
}

static void
_efl_ui_tree_view_seg_array_insert_at_node(Efl_Ui_Tree_View_Seg_Array* seg_array, int shallow_index, Efl_Ui_Tree_View_Item* item, Efl_Ui_Tree_View_Seg_Array_Node* node)
{
   Efl_Ui_Tree_View_Seg_Array_Node* before_node;
   int shallow_index_pred, pos = 0;
   Eina_Iterator* iterator;

   assert (item->shallow_offset == 0);
   assert (item->linearized_offset == 0);

   if (node && node->length != node->max && (shallow_index - node->shallow_first_index) <= node->length)
     {
        assert(node->length != node->max);

        pos = shallow_index - node->shallow_first_index;
        item->tree_node = node;
        item->shallow_offset = pos;

        assert(pos == node->length);
        if (pos < node->length)
          memmove(&node->pointers[pos], &node->pointers[pos+1], sizeof(node->pointers[pos])*(node->length - pos));

        node->pointers[pos] = item;
        if (pos)
          item->linearized_offset = node->pointers[pos-1]->linearized_offset + node->pointers[pos-1]->children_count + 1;

        node->length++;
     }
   else
     {
        node = _alloc_node(seg_array, shallow_index, 0u);
        node->pointers[0] = item;
        node->length++;
        item->shallow_offset = 0;
        item->tree_node = node;

        shallow_index_pred = shallow_index ? shallow_index - 1 : 0;

        before_node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(seg_array->root),
                                                       &shallow_index_pred, sizeof(shallow_index_pred),
                                                       &_shallow_search_lookup_cb, NULL);
        if (before_node == node)
          {
             node->linearized_first_index = 0u;
             assert (node->shallow_first_index == 0);
          }
        else
          {
             assert (node->shallow_first_index > before_node->shallow_first_index);
             assert (node->shallow_first_index == before_node->shallow_first_index
                    + before_node->pointers[before_node->length-1]->shallow_offset + 1);
             node->linearized_first_index = before_node->linearized_first_index;
             if (before_node->length)
               node->linearized_first_index += before_node->pointers[before_node->length-1]->linearized_offset
                     + before_node->pointers[before_node->length-1]->children_count + 1;
          }
     }

   int i = node->linearized_first_index;
   iterator = eina_rbtree_iterator_infix((void*)seg_array->root);
   while (eina_iterator_next(iterator, (void**)&node))
     if (node->linearized_first_index > i)
       {
          node->shallow_first_index++;
          node->linearized_first_index++;
       }
   eina_iterator_free(iterator);
   seg_array->count++;
}

typedef struct _Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow
{
   Eina_Accessor vtable;
   Efl_Ui_Tree_View_Seg_Array* segarray;
} Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow;

static Eina_Bool
_efl_ui_tree_view_seg_array_accessor_shallow_get_at(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc,
                                      int idx, void** data)
{
   Efl_Ui_Tree_View_Seg_Array_Node* node;
   node = (void*)eina_rbtree_inline_lookup(EINA_RBTREE_GET(acc->segarray->root),
                                           &idx, sizeof(idx), &_shallow_search_lookup_cb, NULL);
   if (node && (node->shallow_first_index <= idx && node->shallow_first_index + node->length > idx))
     {
        int i = idx - node->shallow_first_index;
        Efl_Ui_Tree_View_Item* item = node->pointers[i];
        *data = item;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void*
_efl_ui_tree_view_seg_array_accessor_shallow_get_container(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_tree_view_seg_array_accessor_shallow_free(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc EINA_UNUSED)
{
   free(acc);
}

static Eina_Bool
_efl_ui_tree_view_seg_array_accessor_shallow_lock(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_efl_ui_tree_view_seg_array_accessor_shallow_unlock(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Accessor*
_efl_ui_tree_view_seg_array_accessor_shallow_clone(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_tree_view_seg_array_accessor_shallow_setup(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc, Efl_Ui_Tree_View_Seg_Array* segarray)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_tree_view_seg_array_accessor_shallow_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_tree_view_seg_array_accessor_shallow_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_tree_view_seg_array_accessor_shallow_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_tree_view_seg_array_accessor_shallow_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_tree_view_seg_array_accessor_shallow_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_tree_view_seg_array_accessor_shallow_clone);
   acc->segarray = segarray;
}

typedef struct _Efl_Ui_Tree_View_Segarray_Node_Accessor
{
   Eina_Accessor vtable;
   Efl_Ui_Tree_View_Seg_Array* segarray;
   Eina_Iterator* pre_iterator;
   Efl_Ui_Tree_View_Seg_Array_Node* current_node;
   int current_index;
} Efl_Ui_Tree_View_Segarray_Node_Accessor;

static Eina_Bool
_efl_ui_tree_view_seg_array_node_accessor_get_at(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc,
                                      int idx, void** data)
{
   if (idx == acc->current_index && acc->current_node)
     {
        (*data) = acc->current_node;
     }
   else
     {
        if (acc->current_index >= idx || !acc->current_node)
          {
             eina_iterator_free(acc->pre_iterator);
             acc->pre_iterator = NULL;
             acc->current_node = NULL;
             acc->current_index = -1;
          }

        if (!acc->pre_iterator)
          acc->pre_iterator = eina_rbtree_iterator_infix((void*)acc->segarray->root);

        for (;acc->current_index != idx;++acc->current_index)
          {
             if (!eina_iterator_next(acc->pre_iterator, (void**)&acc->current_node))
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
_efl_ui_tree_view_seg_array_node_accessor_get_container(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
  return NULL;
}

static void
_efl_ui_tree_view_seg_array_node_accessor_free(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   if (acc->pre_iterator)
     eina_iterator_free(acc->pre_iterator);
   free(acc);
}

static Eina_Bool
_efl_ui_tree_view_seg_array_node_accessor_lock(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_efl_ui_tree_view_seg_array_node_accessor_unlock(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Accessor*
_efl_ui_tree_view_seg_array_node_accessor_clone(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc EINA_UNUSED)
{
   return &acc->vtable;
}

static void
_efl_ui_tree_view_seg_array_node_accessor_setup(Efl_Ui_Tree_View_Segarray_Node_Accessor* acc, Efl_Ui_Tree_View_Seg_Array* segarray)
{
   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_ui_tree_view_seg_array_node_accessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_ui_tree_view_seg_array_node_accessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_ui_tree_view_seg_array_node_accessor_free);
   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_ui_tree_view_seg_array_node_accessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_ui_tree_view_seg_array_node_accessor_unlock);
   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_ui_tree_view_seg_array_node_accessor_clone);
   acc->segarray = segarray;
   acc->pre_iterator = NULL;
   acc->current_index = -1;
   acc->current_node = NULL;
}

int efl_ui_tree_view_item_index_get(Efl_Ui_Tree_View_Item* item)
{
  Efl_Ui_Tree_View_Seg_Array_Node* node = item->tree_node;
  return item->shallow_offset + node->shallow_first_index;
}

static void
_efl_ui_tree_view_seg_array_fix_offset (Efl_Ui_Tree_View_Item* current)
{
   Efl_Ui_Tree_View_Seg_Array_Node *node;
   Efl_Ui_Tree_View_Item* item;
   Eina_Iterator* iterator;
   int i;
   EINA_SAFETY_ON_NULL_RETURN(current);

   // need to fix size
   current->children_count++;
   node = current->tree_node;

   EINA_SAFETY_ON_NULL_RETURN(node);
   EINA_SAFETY_ON_NULL_RETURN(node->seg_array);

   i = current->shallow_offset + 1;
   while (i < node->length)
     {
        item = node->pointers[i];
        item->linearized_offset++;
        i++;
     }

   i = node->linearized_first_index;
   iterator = eina_rbtree_iterator_infix((void*)node->seg_array->root);
   while (eina_iterator_next(iterator, (void**)&node))
     if (node->linearized_first_index > i)
       node->linearized_first_index++;

   eina_iterator_free(iterator);
   if (current->parent)
     _efl_ui_tree_view_seg_array_fix_offset(current->parent);
}

/* External methods */

Efl_Ui_Tree_View_Seg_Array*
efl_ui_tree_view_seg_array_setup(int size)
{

   Efl_Ui_Tree_View_Seg_Array *seg_array = calloc(1, sizeof(Efl_Ui_Tree_View_Seg_Array));
   seg_array->step_size = size;

   return seg_array;
}

void
efl_ui_tree_view_seg_array_free(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   if (seg_array->root)
     eina_rbtree_delete(EINA_RBTREE_GET(seg_array->root), EINA_RBTREE_FREE_CB(_free_node), NULL);

   free(seg_array);
}

void
efl_ui_tree_view_seg_array_flush(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   if (seg_array->root)
     eina_rbtree_delete(EINA_RBTREE_GET(seg_array->root), EINA_RBTREE_FREE_CB(_free_node), NULL);

   seg_array->root = NULL;
   seg_array->node_count = 0;
   seg_array->count = 0;
}

int
efl_ui_tree_view_seg_array_count(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   return seg_array->count;
}

void
efl_ui_tree_view_seg_array_insert(Efl_Ui_Tree_View_Seg_Array *seg_array, Efl_Ui_Tree_View_Item *parent, int shallow_index, Efl_Model *model)
{
   Efl_Ui_Tree_View_Seg_Array_Node *next, *node = NULL;
   Efl_Ui_Tree_View_Seg_Array* priv = seg_array;
   Efl_Ui_Tree_View_Item* item;
   Eina_Iterator* iterator;

   item = _create_item_partial(model, parent);

   if (parent)
     {
        if (!parent->segarray)
          parent->segarray = efl_ui_tree_view_seg_array_setup(seg_array->step_size);

        _efl_ui_tree_view_seg_array_fix_offset (parent);
        seg_array->count++;
        priv = parent->segarray;
     }

   iterator = eina_rbtree_iterator_infix((void*)priv->root);
   while (eina_iterator_next(iterator, (void**)&next))
     {
        if (next->shallow_first_index > shallow_index)
          break;
        node = next;
     }
   eina_iterator_free(iterator);

   _efl_ui_tree_view_seg_array_insert_at_node(priv, shallow_index, item, node);
}

/* TODO */
Efl_Ui_Tree_View_Item*
efl_ui_tree_view_seg_array_remove(Efl_Ui_Tree_View_Seg_Array *seg_array EINA_UNUSED, int index EINA_UNUSED)
{
  return NULL;
}

Eina_Accessor*
efl_ui_tree_view_seg_array_shallow_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow* acc = calloc(1, sizeof(Efl_Ui_Tree_View_Segarray_Eina_Accessor_Shallow));
   _efl_ui_tree_view_seg_array_accessor_shallow_setup(acc, seg_array);
   return &acc->vtable;
}

Eina_Accessor*
efl_ui_tree_view_seg_array_linearized_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   return efl_ui_tree_view_seg_array_shallow_accessor_get(seg_array);
}

Eina_Accessor*
efl_ui_tree_view_seg_array_node_accessor_get(Efl_Ui_Tree_View_Seg_Array *seg_array)
{
   Efl_Ui_Tree_View_Segarray_Node_Accessor* acc = calloc(1, sizeof(Efl_Ui_Tree_View_Segarray_Node_Accessor));
   _efl_ui_tree_view_seg_array_node_accessor_setup(acc, seg_array);
   return &acc->vtable;
}

