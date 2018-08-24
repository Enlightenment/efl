#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_UI_FOCUS_OBJECT_PROTECTED


#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_focus_graph.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_CALC_CLASS
#define FOCUS_DATA(obj) Efl_Ui_Focus_Manager_Calc_Data *pd = efl_data_scope_get(obj, MY_CLASS);

#define NODE_DIRECTIONS_COUNT 4

#define DIRECTION_IS_LOGICAL(dir) (dir >= EFL_UI_FOCUS_DIRECTION_PREVIOUS && dir < EFL_UI_FOCUS_DIRECTION_UP)
#define DIRECTION_IS_2D(dir) (dir >= EFL_UI_FOCUS_DIRECTION_UP && dir < EFL_UI_FOCUS_DIRECTION_LAST)
#define DIRECTION_CHECK(dir) (dir >= EFL_UI_FOCUS_DIRECTION_PREVIOUS && dir < EFL_UI_FOCUS_DIRECTION_LAST)

//#define CALC_DEBUG
#define DEBUG_TUPLE(obj) efl_name_get(obj), efl_class_name_get(obj)

static int _focus_log_domain = -1;

#define F_CRI(...) EINA_LOG_DOM_CRIT(_focus_log_domain, __VA_ARGS__)
#define F_ERR(...) EINA_LOG_DOM_ERR(_focus_log_domain, __VA_ARGS__)
#define F_WRN(...) EINA_LOG_DOM_WARN(_focus_log_domain, __VA_ARGS__)
#define F_INF(...) EINA_LOG_DOM_INFO(_focus_log_domain, __VA_ARGS__)
#define F_DBG(...) EINA_LOG_DOM_DBG(_focus_log_domain, __VA_ARGS__)

#define DIRECTION_ACCESS(V, ID) ((V)->graph.directions[(ID) - 2])

typedef enum {
    DIMENSION_X = 0,
    DIMENSION_Y = 1,
} Dimension;

typedef struct _Border Border;
typedef struct _Node Node;

struct _Border {
  Eina_List *one_direction; //partners that are linked in one direction
  Eina_List *cleanup_nodes; //a list of nodes that needs to be cleaned up when this node is deleted
};

typedef enum {
  NODE_TYPE_NORMAL = 0,
  NODE_TYPE_ONLY_LOGICAL = 2,
} Node_Type;

struct _Node{
  Node_Type type; //type of the node

  Efl_Ui_Focus_Object *focusable;
  Efl_Ui_Focus_Manager *manager;
  Efl_Ui_Focus_Manager *redirect_manager;

  struct _Tree_Node{
    Node *parent; //the parent of the tree
    Eina_List *children; //this saves the original set of elements
    Eina_List *saved_order;
  }tree;

  struct _Graph_Node {
    Border directions[NODE_DIRECTIONS_COUNT];
  } graph;

  Eina_Bool on_list : 1;
};

#define T(n) (n->tree)
#define G(n) (n->graph)

typedef struct {
    Eina_List *focus_stack;
    Eina_Hash *node_hash;
    Efl_Ui_Focus_Manager *redirect;
    Efl_Ui_Focus_Object *redirect_entry;
    Eina_List *dirty;
    Efl_Ui_Focus_Graph_Context graph_ctx;

    Node *root;
} Efl_Ui_Focus_Manager_Calc_Data;


static Node* _request_subchild(Node *node);
static void dirty_add(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Node *dirty);
static Node* _next(Node *node);
static void _prepare_node(Node *root);

static Eina_Bool
_focus_manager_active_get(Eo *obj)
{
   Eo *root, *manager, *comp_parent, *redirect;

   if (efl_isa(obj, EFL_UI_FOCUS_MANAGER_WINDOW_ROOT_INTERFACE)) return EINA_TRUE;

   root = efl_ui_focus_manager_root_get(obj);
   manager = efl_ui_focus_object_focus_manager_get(root);

   if (!manager) return EINA_FALSE;

   redirect = efl_ui_focus_manager_redirect_get(manager);

   if (!redirect) return EINA_FALSE;

   if (efl_composite_part_is(obj))
     comp_parent = efl_parent_get(obj); //a focus manager can be attached to something via composition
   else
     comp_parent = NULL;

   if (redirect == obj ||
      (redirect == comp_parent))
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_manager_in_chain_set(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Eo *manager, *root;

   root = efl_ui_focus_manager_root_get(obj);
   manager = efl_ui_focus_object_focus_manager_get(pd->root->focusable);

   EINA_SAFETY_ON_NULL_RETURN(root);

   if (!efl_isa(root, EFL_UI_FOCUS_MANAGER_WINDOW_ROOT_INTERFACE))
     EINA_SAFETY_ON_NULL_RETURN(manager);

   //so we dont run infinitly this does not fix it, but at least we only have a error
   EINA_SAFETY_ON_TRUE_RETURN(manager == obj);

   if (manager)
     efl_ui_focus_manager_focus_set(manager, root);
   else
      DBG("No focus manager for focusable %s@%p",
          efl_class_name_get(pd->root->focusable), root);
}

static void
border_onedirection_set(Node *node, Efl_Ui_Focus_Direction direction, Eina_List *list)
{
   Node *partner;
   Eina_List *lnode;
   Border *border;

   EINA_SAFETY_ON_FALSE_RETURN(DIRECTION_IS_2D(direction));

   border = &DIRECTION_ACCESS(node, direction);

   EINA_LIST_FREE(border->one_direction, partner)
     {
        Border *b = &DIRECTION_ACCESS(partner, efl_ui_focus_util_direction_complement(EFL_UI_FOCUS_UTIL_CLASS,direction));
        b->cleanup_nodes = eina_list_remove(b->cleanup_nodes, node);
     }

   border->one_direction = list;

   EINA_LIST_FOREACH(border->one_direction, lnode, partner)
     {
        Border *comp_border = &DIRECTION_ACCESS(partner,efl_ui_focus_util_direction_complement(EFL_UI_FOCUS_UTIL_CLASS,direction));

        comp_border->cleanup_nodes = eina_list_append(comp_border->cleanup_nodes, node);
     }
}

static void
border_onedirection_cleanup(Node *node, Efl_Ui_Focus_Direction direction)
{
   Node *partner;
   Border *border;

   EINA_SAFETY_ON_FALSE_RETURN(DIRECTION_IS_2D(direction));

   border = &DIRECTION_ACCESS(node, direction);

   EINA_LIST_FREE(border->cleanup_nodes, partner)
     {
        Border *b = &DIRECTION_ACCESS(partner, efl_ui_focus_util_direction_complement(EFL_UI_FOCUS_UTIL_CLASS,direction));
        b->one_direction = eina_list_remove(b->one_direction, node);
     }
}
/**
 * Create a new node
 */
static Node*
node_new(Efl_Ui_Focus_Object *focusable, Efl_Ui_Focus_Manager *manager)
{
    Node *node;

    node = calloc(1, sizeof(Node));

    node->focusable = focusable;
    node->manager = manager;

    return node;
}

/**
 * Looks up given focus object from the focus manager.
 *
 * @returns node found, or NULL if focusable was not found in the manager.
 */
static Node*
node_get(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *focusable)
{
   Node *ret;

   ret = eina_hash_find(pd->node_hash, &focusable);

   if (ret) return ret;

   ERR("Focusable %p (%s) not registered in manager %p", focusable, efl_class_name_get(focusable), obj);

   return NULL;
}

/**
 * Free a node item and unlink this item from all direction
 */
static void
node_item_free(Node *item)
{
   Node *n;
   Eina_List *l;
   Eo *obj = item->manager;
   FOCUS_DATA(obj);

   /*cleanup graph parts*/

   //add all neighbors of the node to the dirty list
   for(int i = EFL_UI_FOCUS_DIRECTION_UP; i < EFL_UI_FOCUS_DIRECTION_LAST; i++)
     {
        Node *partner;

#define MAKE_LIST_DIRTY(node, field) \
        EINA_LIST_FOREACH(DIRECTION_ACCESS(node, i).field, l, partner) \
          { \
             dirty_add(obj, pd, partner); \
          }

        MAKE_LIST_DIRTY(item, one_direction)
        MAKE_LIST_DIRTY(item, cleanup_nodes)

        border_onedirection_cleanup(item, i);
        border_onedirection_set(item, i, NULL);
     }

   /*cleanup manager householdings*/

   //remove from the focus stack
   pd->focus_stack = eina_list_remove(pd->focus_stack, item);

   //if this is the entry for redirect, NULL them out!
   if (pd->redirect_entry == item->focusable)
     pd->redirect_entry = NULL;

   //remove from the dirty parts
   pd->dirty = eina_list_remove(pd->dirty, item);
   item->on_list = EINA_FALSE;

   /*merge tree items*/

   //free the tree items
   if (!item->tree.parent && item->tree.children)
     {
        ERR("Freeing the root with children is going to break the logical tree!");
     }

   if (item->tree.parent && item->tree.children)
     {
        Node *parent;

        parent = item->tree.parent;
        //reparent everything into the next layer
        EINA_LIST_FOREACH(item->tree.children, l, n)
          {
             n->tree.parent = item->tree.parent;
          }
        parent->tree.children = eina_list_merge(parent->tree.children , item->tree.children);
     }

   if (item->tree.parent)
     {
        Node *parent;

        parent = item->tree.parent;
        T(parent).children = eina_list_remove(T(parent).children, item);
     }

   //free the safed order
   ELM_SAFE_FREE(T(item).saved_order, eina_list_free);

   free(item);
}
//FOCUS-STACK HELPERS

static Efl_Ui_Focus_Object*
_focus_stack_unfocus_last(Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Efl_Ui_Focus_Object *focusable = NULL;
   Node *n;

   n = eina_list_last_data_get(pd->focus_stack);

   if (n)
     focusable = n->focusable;

   pd->focus_stack = eina_list_remove(pd->focus_stack, n);

   if (n)
     efl_ui_focus_object_focus_set(n->focusable, EINA_FALSE);

   return focusable;
}

#ifdef CALC_DEBUG
static void
_debug_node(Node *node)
{
   Eina_List *tmp = NULL;

   if (!node) return;

   printf("NODE %s-%s\n", DEBUG_TUPLE(node->focusable));

#define DIR_LIST(dir) DIRECTION_ACCESS(node,dir).partners

#define DIR_OUT(dir)\
   tmp = DIR_LIST(dir); \
   { \
      Eina_List *list_node; \
      Node *partner; \
      printf("-"#dir"-> ("); \
      EINA_LIST_FOREACH(tmp, list_node, partner) \
        printf("%s-%s,", DEBUG_TUPLE(partner->focusable)); \
      printf(")\n"); \
   }

   DIR_OUT(EFL_UI_FOCUS_DIRECTION_RIGHT)
   DIR_OUT(EFL_UI_FOCUS_DIRECTION_LEFT)
   DIR_OUT(EFL_UI_FOCUS_DIRECTION_UP)
   DIR_OUT(EFL_UI_FOCUS_DIRECTION_DOWN)

}
#endif

static Eina_Bool
_no_logical(const void *iter EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Node *n = data;

   return n->type != NODE_TYPE_ONLY_LOGICAL;
}

static void
dirty_flush_node(Efl_Ui_Focus_Manager *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Node *node)
{
   Efl_Ui_Focus_Graph_Calc_Result result;

   efl_ui_focus_graph_calc(&pd->graph_ctx, eina_iterator_filter_new(eina_hash_iterator_data_new(pd->node_hash), _no_logical, NULL, NULL) ,  (Opaque_Graph_Member*) node, &result);

   for (int i = 0; i < 4; ++i)
     {
        Efl_Ui_Focus_Direction direction = -1;
        Efl_Ui_Focus_Graph_Calc_Direction_Result *res = NULL;

        if (i == 0)
          {
             direction = EFL_UI_FOCUS_DIRECTION_RIGHT;
             res = &result.right;
          }
        else if (i == 1)
          {
             direction = EFL_UI_FOCUS_DIRECTION_LEFT;
             res = &result.left;
          }
        else if (i == 2)
          {
             direction = EFL_UI_FOCUS_DIRECTION_UP;
             res = &result.top;
          }
        else if (i == 3)
          {
             direction = EFL_UI_FOCUS_DIRECTION_DOWN;
             res = &result.bottom;
          }

       border_onedirection_set(node, direction, res->relation);
     }

#ifdef CALC_DEBUG
   _debug_node(node);
#endif
}

static void
dirty_flush(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Node *node)
{
   if (!node->on_list) return;

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);

   pd->dirty = eina_list_remove(pd->dirty, node);
   node->on_list = EINA_FALSE;

   dirty_flush_node(obj, pd, node);
}

static void
dirty_flush_all(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Node *node;

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);

   EINA_LIST_FREE(pd->dirty, node)
     {
        node->on_list = EINA_FALSE;
        dirty_flush_node(obj, pd, node);
     }
}

static void
dirty_add(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Node *dirty)
{
   if (dirty->type == NODE_TYPE_ONLY_LOGICAL)
     {
        ERR("Only not only logical nodes can be marked dirty");
        return;
     }
   if (dirty->on_list) return;

   //if (eina_list_data_find(pd->dirty, dirty)) return;
   pd->dirty = eina_list_append(pd->dirty, dirty);
   dirty->on_list = EINA_TRUE;

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, NULL);
}


static void
_node_new_geometry_cb(void *data, const Efl_Event *event)
{
   Node *node;
   FOCUS_DATA(data)

   node = node_get(data, pd, event->object);
   if (!node)
      return;

   dirty_add(data, pd, node);

   return;
}

static void
_object_del_cb(void *data, const Efl_Event *event)
{
   /*
    * Lets just implicitly delete items that are deleted
    * Otherwise we have later just a bunch of errors
    */
   efl_ui_focus_manager_calc_unregister(data, event->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(regular_node,
    {EFL_GFX_ENTITY_EVENT_RESIZE, _node_new_geometry_cb},
    {EFL_GFX_ENTITY_EVENT_MOVE, _node_new_geometry_cb},
    {EFL_EVENT_DEL, _object_del_cb},
);

EFL_CALLBACKS_ARRAY_DEFINE(logical_node,
    {EFL_EVENT_DEL, _object_del_cb},
);


//=============================

static Node*
_register(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child, Node *parent, Node_Type type, Efl_Ui_Focus_Manager *redirect)
{
   Node *node;
   node = eina_hash_find(pd->node_hash, &child);
   if (node && !(node->type == type && T(node).parent == parent && node->redirect_manager == redirect))
     {
        ERR("Child %p is already registered in the graph (%s)", child, node->type == NODE_TYPE_ONLY_LOGICAL ? "logical" : "regular");
        return NULL;
     }

   node = node_new(child, obj);
   eina_hash_add(pd->node_hash, &child, node);

   //add the parent
   if (parent)
     {
        T(node).parent = parent;
        T(parent).children = eina_list_append(T(parent).children, node);
     }
   node->type = NODE_TYPE_ONLY_LOGICAL;
   node->redirect_manager = redirect;

   return node;
}
EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_register_logical(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent,  Efl_Ui_Focus_Manager *redirect)
{
   Node *node = NULL;
   Node *pnode = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, EFL_UI_FOCUS_MANAGER_INTERFACE), EINA_FALSE);

   F_DBG("Manager: %p register %p %p %p", obj, child, parent, redirect);

   pnode = node_get(obj, pd, parent);
   if (!pnode) return EINA_FALSE;

   node = _register(obj, pd, child, pnode, NODE_TYPE_ONLY_LOGICAL, redirect);
   if (!node) return EINA_FALSE;

   //listen to deletion
   efl_event_callback_array_add(child, logical_node(), obj);

   //set again
   if (T(pnode).saved_order)
     {
        Eina_List *tmp;

        tmp = eina_list_clone(T(pnode).saved_order);
        efl_ui_focus_manager_calc_update_order(obj, parent, tmp);
     }

   return EINA_TRUE;
}


EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_register(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent, Efl_Ui_Focus_Manager *redirect)
{
   Node *node = NULL;
   Node *pnode = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, EFL_UI_FOCUS_MANAGER_INTERFACE), EINA_FALSE);

   F_DBG("Manager: %p register %p %p %p", obj, child, parent, redirect);

   pnode = node_get(obj, pd, parent);
   if (!pnode) return EINA_FALSE;

   node = _register(obj, pd, child, pnode, NODE_TYPE_NORMAL, redirect);
   if (!node) return EINA_FALSE;

   //listen to changes
   efl_event_callback_array_add(child, regular_node(), obj);

   node->type = NODE_TYPE_NORMAL;
   node->redirect_manager = redirect;

   //mark dirty
   dirty_add(obj, pd, node);

   //set again
   if (T(pnode).saved_order)
     {
        Eina_List *tmp;

        tmp = eina_list_clone(T(pnode).saved_order);
        efl_ui_focus_manager_calc_update_order(obj, parent, tmp);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_update_redirect(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Manager *redirect)
{
   Node *node = node_get(obj, pd, child);
   if (!node) return EINA_FALSE;

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, MY_CLASS), EINA_FALSE);

   node->redirect_manager = redirect;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_update_parent(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent_obj)
{
   Node *node;
   Node *parent;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent_obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);

   node = node_get(obj, pd, child);
   parent = node_get(obj, pd, parent_obj);

   if (!node || !parent) return EINA_FALSE;

   if (T(node).parent)
     {
        Node *old_parent;

        old_parent = T(node).parent;

        T(old_parent).children = eina_list_remove(T(old_parent).children, node);
     }

   T(node).parent = parent;

   if (T(node).parent)
     {
        T(parent).children = eina_list_append(T(parent).children, node);
     }

   return EINA_TRUE;
}

static Eina_List*
_set_a_without_b(Eina_List *a, Eina_List *b)
{
   Eina_List *a_out = NULL, *node;
   void *data;

   a_out = eina_list_clone(a);

   EINA_LIST_FOREACH(b, node, data)
     {
        a_out = eina_list_remove(a_out, data);
     }

   return a_out;
}

static Eina_Bool
_equal_set(Eina_List *none_nodes, Eina_List *nodes)
{
   Eina_List *n;
   Node *node;

   if (eina_list_count(nodes) != eina_list_count(none_nodes)) return EINA_FALSE;

   EINA_LIST_FOREACH(nodes, n, node)
     {
        if (!eina_list_data_find(none_nodes, node))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_focus_manager_calc_update_order(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *parent, Eina_List *order)
{
   Node *pnode;
   Efl_Ui_Focus_Object *o;
   Eina_List *node_order = NULL, *not_ordered, *trash, *node_order_clean, *n;

   F_DBG("Manager_update_order on %p %p", obj, parent);

   pnode = node_get(obj, pd, parent);
   if (!pnode)
     return;

   ELM_SAFE_FREE(T(pnode).saved_order, eina_list_free);
   T(pnode).saved_order = order;

   //get all nodes from the subset
   EINA_LIST_FOREACH(order, n, o)
     {
        Node *tmp;

        tmp = eina_hash_find(pd->node_hash, &o);

        if (!tmp) continue;

        node_order = eina_list_append(node_order, tmp);
     }

   not_ordered = _set_a_without_b(T(pnode).children, node_order);
   trash = _set_a_without_b(node_order, T(pnode).children);
   node_order_clean = _set_a_without_b(node_order, trash);

   eina_list_free(node_order);
   eina_list_free(trash);

   eina_list_free(T(pnode).children);
   T(pnode).children = eina_list_merge(node_order_clean, not_ordered);

   return;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_update_children(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *parent, Eina_List *order)
{
   Node *pnode;
   Efl_Ui_Focus_Object *o;
   Eina_Bool fail = EINA_FALSE;
   Eina_List *node_order = NULL;

   pnode = node_get(obj, pd, parent);
   if (!pnode)
     return EINA_FALSE;

   //get all nodes from the subset
   EINA_LIST_FREE(order, o)
     {
        Node *tmp;

        tmp = node_get(obj, pd, o);
        if (!tmp)
          fail = EINA_TRUE;
        node_order = eina_list_append(node_order, tmp);
     }

   if (fail)
     {
        eina_list_free(node_order);
        return EINA_FALSE;
     }

   if (!_equal_set(node_order, T(pnode).children))
     {
        ERR("Set of children is not equal");
        return EINA_FALSE;
     }

   eina_list_free(T(pnode).children);
   T(pnode).children = node_order;

   return EINA_TRUE;
}

static inline Node*
_request_subchild_except(Node *n, Node *except)
{
   n = _request_subchild(n);
   while (n == except)
     {
        n = _next(n);
     }

   return n;
}

EOLIAN static void
_efl_ui_focus_manager_calc_unregister(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child)
{
   Node *node;

   node = eina_hash_find(pd->node_hash, &child);

   if (!node) return;

   F_DBG("Manager: %p unregister %p", obj, child);

   if (eina_list_last_data_get(pd->focus_stack) == node)
     {
        if (!efl_invalidated_get(pd->root->focusable))
          {
             Node *n = NULL;

             // if there is no redirect manager, then try to recover the focus property to a different element
             //allow the removal of the redirect when we are removing the child that is the redirect manager
             if (!pd->redirect || pd->redirect == child)
               {
                  n = eina_list_nth(pd->focus_stack, eina_list_count(pd->focus_stack) - 2);
                  if (!n)
                    n = _request_subchild_except(pd->root, node);

                  if (n)
                    efl_ui_focus_manager_focus_set(obj, n->focusable);
               }
             // if there is a redirect manager, then remove the flag from the child
             if (!n)
               {
                  efl_ui_focus_object_focus_set(child, EINA_FALSE);
               }
          }
     }

   eina_hash_del_by_key(pd->node_hash, &child);
}

static void
_redirect_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   efl_ui_focus_manager_redirect_set(data, NULL);
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_ui_focus_manager_redirect_set(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Manager *redirect)
{
   Efl_Ui_Focus_Manager *old_manager;

   if (pd->redirect == redirect) return;

   F_DBG("Manager: %p setting redirect from %p to %p", obj, pd->redirect, redirect);

   old_manager = pd->redirect;

   if (pd->redirect)
     efl_event_callback_del(pd->redirect, EFL_EVENT_DEL, _redirect_del, obj);

   pd->redirect = redirect;

   if (pd->redirect)
     efl_event_callback_add(pd->redirect, EFL_EVENT_DEL, _redirect_del, obj);

   efl_ui_focus_manager_reset_history(old_manager);

   //adjust focus property of the most upper element
   if (_focus_manager_active_get(obj))
     {
        Node *n = NULL;

        n = eina_list_last_data_get(pd->focus_stack);

        if (!pd->redirect && old_manager)
          {
             if (n)
               {
                  efl_ui_focus_object_focus_set(n->focusable, EINA_TRUE);
               }
             else
               {
                  n = _request_subchild(pd->root);
                  if (n)
                    efl_ui_focus_manager_focus_set(obj, n->focusable);
               }
          }
        else if (pd->redirect && !old_manager)
          {
             if (n)
               efl_ui_focus_object_focus_set(n->focusable, EINA_FALSE);
          }
     }

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED , old_manager);

   //just set the root of the new redirect as focused, so it is in a known state
   if (redirect)
     {
        efl_ui_focus_manager_setup_on_first_touch(redirect, EFL_UI_FOCUS_DIRECTION_LAST, NULL);
     }
}

EOLIAN static Efl_Ui_Focus_Manager *
_efl_ui_focus_manager_calc_efl_ui_focus_manager_redirect_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   return pd->redirect;
}

static void
_free_node(void *data)
{
   Node *node = data;
   FOCUS_DATA(node->manager);

   if (node->type == NODE_TYPE_ONLY_LOGICAL)
     efl_event_callback_array_del(node->focusable, logical_node(), node->manager);
   else
     efl_event_callback_array_del(node->focusable, regular_node(), node->manager);

   if (pd->root != data)
     {
        node_item_free(node);
     }
}

EOLIAN static Efl_Object *
_efl_ui_focus_manager_calc_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->node_hash = eina_hash_pointer_new(_free_node);

   pd->graph_ctx.offset_focusable = offsetof(Node, focusable);

   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_focus_manager_calc_efl_object_provider_find(const Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd EINA_UNUSED, const Efl_Object *klass)
{
   if (klass == MY_CLASS)
     return (Efl_Object *) obj;

   return efl_provider_find(efl_super(obj, MY_CLASS), klass);
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Node *n;

   pd->focus_stack = eina_list_free(pd->focus_stack);
   EINA_LIST_FREE(pd->dirty, n)
     {
        n->on_list = EINA_FALSE;
     }
   eina_hash_free(pd->node_hash);

   efl_ui_focus_manager_redirect_set(obj, NULL);

   if (pd->root)
     node_item_free(pd->root);
   pd->root = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

typedef struct {
   Eina_Iterator iterator;
   Eina_Iterator *real_iterator;
   Efl_Ui_Focus_Manager *object;
} Border_Elements_Iterator;

static Eina_Bool
_iterator_next(Border_Elements_Iterator *it, void **data)
{
   Node *node;

   EINA_ITERATOR_FOREACH(it->real_iterator, node)
     {
        if (node->type == NODE_TYPE_ONLY_LOGICAL) continue;

        for(int i = EFL_UI_FOCUS_DIRECTION_UP ;i < EFL_UI_FOCUS_DIRECTION_LAST; i++)
          {
             if (!DIRECTION_ACCESS(node, i).one_direction)
               {
                  *data = node->focusable;
                  return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}

static Eo *
_iterator_get_container(Border_Elements_Iterator *it)
{
   return it->object;
}

static void
_iterator_free(Border_Elements_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static void
_prepare_node(Node *root)
{
   Eina_List *n;
   Node *node;

   efl_ui_focus_object_prepare_logical(root->focusable);

   EINA_LIST_FOREACH(root->tree.children, n, node)
     {
        _prepare_node(node);
     }
}

EOLIAN static Eina_Iterator*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_border_elements_get(const Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Border_Elements_Iterator *it;

   /* XXX const */
   dirty_flush_all((Eo *)obj, pd);

   //now call prepare on every node
   _prepare_node(pd->root);

   it = calloc(1, sizeof(Border_Elements_Iterator));

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_hash_iterator_data_new(pd->node_hash);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_iterator_free);
   it->object = (Eo *)obj;

   return (Eina_Iterator*) it;
}

static Node*
_no_history_element(Eina_Hash *node_hash)
{
   //nothing is selected yet, just try to use the first element in the iterator
   Eina_Iterator *iter;
   Node *upper;

   iter = eina_hash_iterator_data_new(node_hash);

   EINA_ITERATOR_FOREACH(iter, upper)
     {
        if (upper->type == NODE_TYPE_NORMAL)
          break;
     }

   eina_iterator_free(iter);

   if (upper->type != NODE_TYPE_NORMAL)
     return NULL;

   return upper;
}

static void
_get_middle(Evas_Object *obj, Eina_Vector2 *elem)
{
   Eina_Rect geom;

   geom = efl_ui_focus_object_focus_geometry_get(obj);
   elem->x = geom.x + geom.w/2;
   elem->y = geom.y + geom.h/2;
}

static Node*
_coords_movement(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Node *upper, Efl_Ui_Focus_Direction direction)
{
   Node *candidate;
   Eina_List *node_list;
   Eina_List *lst;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_IS_2D(direction), NULL);

   //flush the node and prepare all the nodes
   _prepare_node(pd->root);
   dirty_flush(obj, pd, upper);

   //decide which direction we take
   lst = DIRECTION_ACCESS(upper, direction).one_direction;

   //we are searching which of the partners is lower to the history
   EINA_LIST_REVERSE_FOREACH(pd->focus_stack, node_list, candidate)
     {
        //we only calculate partners for normal nodes
        if (candidate->type == NODE_TYPE_NORMAL) continue;

        if (eina_list_data_find(lst, candidate))
          {
             //this is the next accessible part
             return candidate;
          }
     }

   //if we haven't found anything in the history, use the widget with the smallest distance
   {
      Eina_List *n;
      Node *node, *min = NULL;
      Eina_Vector2 elem, other;
      float min_distance = 0.0;

      _get_middle(upper->focusable, &elem);

      EINA_LIST_FOREACH(lst, n, node)
        {
           _get_middle(node->focusable, &other);
           float tmp = eina_vector2_distance_get(&other, &elem);
           if (!min || tmp < min_distance)
             {
                min = node;
                min_distance = tmp;
             }
        }
      candidate = min;
   }
   return candidate;
}


static Node*
_prev_item(Node *node)
{
   Node *parent;
   Eina_List *lnode;

   parent = T(node).parent;

   //we are accessing the parents children, prepare!
   efl_ui_focus_object_prepare_logical(parent->focusable);

   lnode = eina_list_data_find_list(T(parent).children, node);
   lnode = eina_list_prev(lnode);

   if (lnode)
     return eina_list_data_get(lnode);
   return NULL;
}

static Node*
_next(Node *node)
{
   Node *n;

   //Case 1 we are having children
   //But only enter the children if it does NOT have a redirect manager
   if (T(node).children && !node->redirect_manager)
     {
        return eina_list_data_get(T(node).children);
     }

   //case 2 we are the root and we don't have children, return ourself
   if (!T(node).parent)
     {
        return node;
     }

   //case 3 we are not at the end of the parents list
   n = node;
   while(T(n).parent)
     {
        Node *parent;
        Eina_List *lnode;

        parent = T(n).parent;

        //we are accessing the parents children, prepare!
        efl_ui_focus_object_prepare_logical(parent->focusable);

        lnode = eina_list_data_find_list(T(parent).children, n);
        lnode = eina_list_next(lnode);

        if (lnode)
          {
             return eina_list_data_get(lnode);
          }

        n = parent;
     }

   //this is then the root again
   return NULL;
}

static Node*
_prev(Node *node)
{
   Node *n = NULL;

   //this is the root there is no parent
   if (!T(node).parent)
     return NULL;

   n =_prev_item(node);

   //we are accessing prev items children, prepare them!
   if (n && n->focusable)
     efl_ui_focus_object_prepare_logical(n->focusable);

   //case 1 there is a item in the parent previous to node, which has children
   if (n && T(n).children && !n->redirect_manager)
     {
        do
          {
              n = eina_list_last_data_get(T(n).children);
          }
        while (T(n).children && !n->redirect_manager);

        return n;
     }

   //case 2 there is a item in the parent previous to node, which has no children
   if (n)
     return n;

   //case 3 there is a no item in the parent previous to this one
   return T(node).parent;
}


static Node*
_logical_movement(Efl_Ui_Focus_Manager_Calc_Data *pd EINA_UNUSED, Node *upper, Efl_Ui_Focus_Direction direction, Eina_Bool accept_logical)
{
   Node* (*deliver)(Node *n);
   Node *result;
   Eina_List *stack = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_IS_LOGICAL(direction), NULL);

   if (direction == EFL_UI_FOCUS_DIRECTION_NEXT)
     deliver = _next;
   else
     deliver = _prev;

   //search as long as we have a none logical parent
   result = upper;
   do
     {
        //give up, if we have already been here
        if (!!eina_list_data_find(stack, result))
          {
             eina_list_free(stack);
             ERR("Warning cycle detected\n");
             return NULL;
          }

        stack = eina_list_append(stack, result);

        if (direction == EFL_UI_FOCUS_DIRECTION_NEXT)
          efl_ui_focus_object_prepare_logical(result->focusable);

        result = deliver(result);
        if (accept_logical)
          break;
   } while(result && result->type != NODE_TYPE_NORMAL && !result->redirect_manager);

   eina_list_free(stack);

   return result;
}

static Efl_Ui_Focus_Object*
_request_move(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Direction direction, Node *upper, Eina_Bool accept_logical)
{
   Node *dir = NULL;

   if (!upper)
     upper = eina_list_last_data_get(pd->focus_stack);

   if (!upper)
     {
        upper = _no_history_element(pd->node_hash);
        if (upper)
          return upper->focusable;
        return NULL;

     }

   if (direction == EFL_UI_FOCUS_DIRECTION_PREVIOUS
    || direction == EFL_UI_FOCUS_DIRECTION_NEXT)
      dir = _logical_movement(pd, upper, direction, accept_logical);
   else
      dir = _coords_movement(obj, pd, upper, direction);

   //return the widget
   if (dir)
     return dir->focusable;
   else
     return NULL;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_request_move(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *child, Eina_Bool logical)
{
   Node *child_node;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_CHECK(direction), NULL);

   if (!child)
     child_node = eina_list_last_data_get(pd->focus_stack);
   else
     child_node = node_get(obj, pd, child);

   if (!child_node)
     return NULL;

   return _request_move(obj, pd, direction, child_node, logical);
}

static int
_node_depth(Node *node)
{
  int i = 0;

  while (node && node->tree.parent)
    {
      node = node->tree.parent;
      i++;
    }

  return i;
}

static Node*
_request_subchild(Node *node)
{
   //important! if there are no children _next would return the parent of node which will exceed the limit of children of node
   Node *target = NULL;

   if (node->tree.children)
     {
        int new_depth, old_depth = _node_depth(node);

        target = node;
        //try to find a child that is not logical or has a redirect manager
        do
          {
             if (target != node)
               efl_ui_focus_object_prepare_logical(target->focusable);

             target = _next(target);
             //abort if we are exceeding the childrens of node
             new_depth = _node_depth(target);

             if (new_depth <= old_depth) target = NULL;
          }
        while (target && target->type == NODE_TYPE_ONLY_LOGICAL && !target->redirect_manager);
        F_DBG("Found node %p", target);
     }

   return target;
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_ui_focus_manager_manager_focus_set(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *focus)
{
   Node *node, *last;
   Efl_Ui_Focus_Object *last_focusable = NULL, *new_focusable;
   Efl_Ui_Focus_Manager *redirect_manager;
   Node_Type node_type;

   EINA_SAFETY_ON_NULL_RETURN(focus);

   //check if node is part of this manager object
   node = node_get(obj, pd, focus);
   if (!node) return;

   if (node->type == NODE_TYPE_ONLY_LOGICAL && !node->redirect_manager)
     {
        Node *target = NULL;

        F_DBG(" %p is logical, fetching the next subnode that is either a redirect or a regular", obj);

        //important! if there are no children _next would return the parent of node which will exceed the limit of children of node
        efl_ui_focus_object_prepare_logical(node->focusable);

        target = _request_subchild(node);

        //check if we have found anything
        if (target)
          {
             node = target;
          }
        else
          {
             ERR("Could not fetch a node located at %p", node->focusable);
             return;
          }
     }

   F_DBG("Manager: %p focusing object %p %s", obj, node->focusable, efl_class_name_get(node->focusable));


   if (eina_list_last_data_get(pd->focus_stack) == node)
     {
        //the correct one is focused
        if (node->redirect_manager == pd->redirect)
          return;
     }

   //make sure this manager is in the chain of redirects
   _manager_in_chain_set(obj, pd);

   node_type = node->type;
   new_focusable = node->focusable;

   redirect_manager = node->redirect_manager;

   last = eina_list_last_data_get(pd->focus_stack);
   if (last)
     last_focusable = last->focusable;

   //remove the object from the list and add it again
   if (node_type == NODE_TYPE_NORMAL)
     {
        pd->focus_stack = eina_list_remove(pd->focus_stack, node);
        pd->focus_stack = eina_list_append(pd->focus_stack, node);
     }

   //capture how we came to the redirect manager
   if (redirect_manager)
     {
        pd->redirect_entry = new_focusable;
     }

   //set to NULL here, from the event earlier this pointer could be dead.
   node = NULL;

   //unset redirect manager for the case that its a different one to the one we want
   if (pd->redirect && pd->redirect != redirect_manager)
     {
        Efl_Ui_Focus_Manager *m = obj;

        //completely unset the current redirect chain
        while (efl_ui_focus_manager_redirect_get(m))
         {
            Efl_Ui_Focus_Manager *old = m;

            m = efl_ui_focus_manager_redirect_get(m);
            efl_ui_focus_manager_redirect_set(old, NULL);
          }
     }

   //now check if this is also a listener object
   if (redirect_manager)
     {
        //set the redirect
        efl_ui_focus_manager_redirect_set(obj, redirect_manager);
     }

   /*
     Only emit those signals if we are already at the top of the focus stack.
     Otherwise focus_get in the callback to that signal might return false.
    */
   if (node_type == NODE_TYPE_NORMAL)
     {
        //populate the new change
        if (last_focusable)
          efl_ui_focus_object_focus_set(last_focusable, EINA_FALSE);
        if (new_focusable)
          efl_ui_focus_object_focus_set(new_focusable, EINA_TRUE);
        efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, last_focusable);
     }
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_ui_focus_manager_setup_on_first_touch(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *entry)
{
   if (direction == EFL_UI_FOCUS_DIRECTION_PREVIOUS && entry)
     {
        Efl_Ui_Focus_Manager_Logical_End_Detail last;
        Efl_Ui_Focus_Manager *rec_manager = obj;
        do
          {
             last = efl_ui_focus_manager_logical_end(rec_manager);
             EINA_SAFETY_ON_NULL_RETURN(last.element);
             efl_ui_focus_manager_focus_set(rec_manager, last.element);

             rec_manager = efl_ui_focus_manager_redirect_get(rec_manager);
          }
        while (rec_manager);
     }
   else if (DIRECTION_IS_2D(direction) && entry)
     efl_ui_focus_manager_focus_set(obj, entry);
   else
     efl_ui_focus_manager_focus_set(obj, pd->root->focusable);
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_move(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Efl_Ui_Focus_Object *candidate = NULL;
   Efl_Ui_Focus_Manager *early, *late;

   // for the case that focus is set to a new element, a new redirect
   // manager could have been set, to adjust everything
   // in the new reidirect manager we have to call the first touch function.
   Eina_Bool adjust_redirect_manager = EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_CHECK(direction), NULL);

   early = efl_ui_focus_manager_redirect_get(obj);

   if (pd->redirect)
     {
        Efl_Ui_Focus_Object *old_candidate = NULL;
        candidate = efl_ui_focus_manager_move(pd->redirect, direction);

        if (!candidate)
          {
             Efl_Ui_Focus_Object *new_candidate = NULL;

             if (DIRECTION_IS_LOGICAL(direction))
               {
                  // lets just take the redirect_entry
                  Node *n = node_get(obj, pd, pd->redirect_entry);
                  new_candidate = _request_move(obj, pd, direction, n, EINA_FALSE);

                  if (new_candidate)
                    {
                       efl_ui_focus_manager_focus_set(obj, new_candidate);
                       adjust_redirect_manager = EINA_TRUE;
                    }
                  else
                    {
                       //we set the redirect to NULL since it cannot
                       //help us, later on the redirect manager can be
                       //set to the same again, and it is strictly new setted up.
                       efl_ui_focus_manager_redirect_set(obj, NULL);
                       pd->redirect_entry = NULL;
                    }

                  candidate = new_candidate;
               }
             else
               {
                  Node *n;

                  old_candidate = efl_ui_focus_manager_focus_get(pd->redirect);
                  n = eina_hash_find(pd->node_hash, &old_candidate);

                  if (n)
                    new_candidate = _request_move(obj, pd, direction, n, EINA_FALSE);

                  if (new_candidate)
                    {
                       //redirect does not have smth. but we do have.
                       efl_ui_focus_manager_focus_set(obj, new_candidate);
                       adjust_redirect_manager = EINA_TRUE;
                    }
                  else
                    {
                       /*
                         DO NOT unset the redirect here, the value has to stay,
                         if one direction is not continuing in one direction, the
                         other can continue thus we need to safe the redirect for this case.
                        */

                    }

                  candidate = new_candidate;
               }
          }
     }
   else
     {
        Efl_Ui_Focus_Object *child = NULL;

        if (!pd->focus_stack)
          {
             Node *child_node;

             child_node = _no_history_element(pd->node_hash);
             if (child_node)
               child = child_node->focusable;
          }

        candidate = efl_ui_focus_manager_request_move(obj, direction, child, EINA_FALSE);

        F_DBG("Manager: %p moved to %p %s in direction %d", obj, candidate, efl_class_name_get(candidate), direction);

        if (candidate)
          {
             efl_ui_focus_manager_focus_set(obj, candidate);
             adjust_redirect_manager = EINA_TRUE;
          }
     }

   if (adjust_redirect_manager)
     {
        late = efl_ui_focus_manager_redirect_get(obj);

        if (early != late)
          {
             //this is a new manager, we have to init its case!
             if (late)
               efl_ui_focus_manager_setup_on_first_touch(late, direction, candidate);
          }
     }

   return candidate;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_calc_efl_ui_focus_manager_root_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *root)
{
   Node *node;

   if (pd->root)
     {
        ERR("Root element can only be set once!");
        return EINA_FALSE;
     }

   node = _register(obj, pd, root, NULL, NODE_TYPE_ONLY_LOGICAL, NULL);

   pd->root = node;

   return EINA_TRUE;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_root_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   if (!pd->root) return NULL;

   return pd->root->focusable;
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_calc_efl_object_finalize(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Efl_Object *result;

   if (!pd->root)
     {
        ERR("Constructing failed. No root element set.");
        return NULL;
     }

   result = efl_finalize(efl_super(obj, MY_CLASS));

   return result;
}

static Eina_List*
_convert(Border b)
{
   Eina_List *n, *par = NULL;
   Node *node;

   EINA_LIST_FOREACH(b.one_direction, n, node)
     par = eina_list_append(par, node->focusable);

   return par;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_manager_focus_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Node *upper = NULL;

   if (pd->redirect && pd->redirect_entry) return pd->redirect_entry;

   upper = eina_list_last_data_get(pd->focus_stack);

   if (!upper)
     return NULL;
   return upper->focusable;
}

EOLIAN static Efl_Ui_Focus_Relations*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_fetch(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child)
{
   Efl_Ui_Focus_Relations *res;
   Node *n, *tmp;

   n = node_get(obj, pd, child);
   if (!n)
      return NULL;

   res = calloc(1, sizeof(Efl_Ui_Focus_Relations));

   dirty_flush(obj, pd, n);

   //make sure to prepare_logical so next and prev are correctly
   if (n->tree.parent)
     efl_ui_focus_object_prepare_logical(n->tree.parent->focusable);
   efl_ui_focus_object_prepare_logical(n->focusable);

#define DIR_CLONE(dir) _convert(DIRECTION_ACCESS(n,dir));

   res->right = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_RIGHT);
   res->left = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_LEFT);
   res->top = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_UP);
   res->down = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_DOWN);
   res->next = (tmp = _next(n)) ? tmp->focusable : NULL;
   res->prev = (tmp = _prev(n)) ? tmp->focusable : NULL;
   res->position_in_history = eina_list_data_idx(pd->focus_stack, n);
   res->node = child;

   res->logical = (n->type == NODE_TYPE_ONLY_LOGICAL);

   if (T(n).parent)
     res->parent = T(n).parent->focusable;
   res->redirect = n->redirect_manager;
#undef DIR_CLONE

   return res;
}

EOLIAN static void
_efl_ui_focus_manager_calc_class_constructor(Efl_Class *c EINA_UNUSED)
{
   _focus_log_domain = eina_log_domain_register("elementary-focus", EINA_COLOR_CYAN);
}

EOLIAN static void
_efl_ui_focus_manager_calc_class_destructor(Efl_Class *c EINA_UNUSED)
{
   eina_log_domain_unregister(_focus_log_domain);
   _focus_log_domain = -1;
}

EOLIAN static Efl_Ui_Focus_Manager_Logical_End_Detail
_efl_ui_focus_manager_calc_efl_ui_focus_manager_logical_end(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
   Node *child = pd->root;
   Efl_Ui_Focus_Manager_Logical_End_Detail ret = { 0, NULL};
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, ret);

   //we need to return the most lower right element

   while ((child) && (T(child).children) && (!child->redirect_manager))
     child = eina_list_last_data_get(T(child).children);
   while ((child) && (child->type != NODE_TYPE_NORMAL) && (!child->redirect_manager))
     child = _prev(child);

   if (child)
     {
        ret.is_regular_end = child->type == NODE_TYPE_NORMAL;
        ret.element = child->focusable;
     }
   return ret;
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_ui_focus_manager_reset_history(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
  Efl_Ui_Focus_Object *last_focusable;

  if (!pd->focus_stack) return;

  last_focusable = _focus_stack_unfocus_last(pd);

  pd->focus_stack = eina_list_free(pd->focus_stack);

  efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, last_focusable);
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_ui_focus_manager_pop_history_stack(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Calc_Data *pd)
{
  Efl_Ui_Focus_Object *last_focusable;
  Node *last = NULL;

  if (pd->redirect)
    {
       Eina_List *n;

       last_focusable = pd->redirect_entry;
       n = eina_list_last(pd->focus_stack);

       while (n && (eina_list_data_get(n) != last_focusable))
         {
            n = eina_list_prev(n);
         }

       last = eina_list_data_get(n);
    }
  else
    {
       last_focusable = _focus_stack_unfocus_last(pd);
       //get now the highest, and unfocus that!
       last = eina_list_last_data_get(pd->focus_stack);
    }

  if (last)
    {
       efl_ui_focus_object_focus_set(last->focusable, EINA_TRUE);
    }
  else
    {
       last = _request_subchild_except(pd->root, node_get(obj, pd, last_focusable));
       if (last)
         efl_ui_focus_manager_focus_set(obj, last->focusable);
    }

  efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, last_focusable);
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_calc_efl_ui_focus_manager_request_subchild(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Ui_Focus_Object *child_obj)
{
   Node *child, *target;

   child = node_get(obj, pd, child_obj);
   if (!child) return NULL;

   target = _request_subchild(child);

   if (target) return target->focusable;
   return NULL;
}

EOLIAN static void
_efl_ui_focus_manager_calc_efl_object_dbg_info_get(Eo *obj, Efl_Ui_Focus_Manager_Calc_Data *pd, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(obj, MY_CLASS), root);
   Efl_Dbg_Info *append, *group = EFL_DBG_INFO_LIST_APPEND(root, "Efl.Ui.Focus.Manager");
   Eina_Iterator *iter;
   Node *node;

   append = EFL_DBG_INFO_LIST_APPEND(group, "children");

   iter = eina_hash_iterator_data_new(pd->node_hash);
   EINA_ITERATOR_FOREACH(iter, node)
     {
        EFL_DBG_INFO_APPEND(append, "-", EINA_VALUE_TYPE_UINT64, node->focusable);
     }
   eina_iterator_free(iter);
}

#define EFL_UI_FOCUS_MANAGER_CALC_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _efl_ui_focus_manager_calc_efl_object_dbg_info_get)

#include "efl_ui_focus_manager_calc.eo.c"
