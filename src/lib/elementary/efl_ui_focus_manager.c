#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_CLASS
#define FOCUS_DATA(obj) Efl_Ui_Focus_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

#define DIM_EFL_UI_FOCUS_DIRECTION(dim,neg) dim*2+neg
#define NODE_DIRECTIONS_COUNT 4

#define DIRECTION_CHECK(dir) (dir >= 0 && dir < EFL_UI_FOCUS_DIRECTION_LAST)

//#define CALC_DEBUG
#define DEBUG_TUPLE(obj) efl_name_get(obj), efl_class_name_get(obj)

static int _focus_log_domain = -1;

#define F_CRI(...) EINA_LOG_DOM_CRIT(_focus_log_domain, __VA_ARGS__)
#define F_ERR(...) EINA_LOG_DOM_ERR(_focus_log_domain, __VA_ARGS__)
#define F_WRN(...) EINA_LOG_DOM_WARN(_focus_log_domain, __VA_ARGS__)
#define F_INF(...) EINA_LOG_DOM_INFO(_focus_log_domain, __VA_ARGS__)
#define F_DBG(...) EINA_LOG_DOM_DBG(_focus_log_domain, __VA_ARGS__)

typedef struct {
    Eina_Bool positive;
    Efl_Ui_Focus_Object *anchor;
} Anchor;

typedef enum {
    DIMENSION_X = 0,
    DIMENSION_Y = 1,
} Dimension;

typedef struct _Border Border;
typedef struct _Node Node;

struct _Border {
  Eina_List *partners;
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
    Eina_List *safed_order;
  }tree;

  struct _Graph_Node {
    Border directions[NODE_DIRECTIONS_COUNT];
  } graph;
};

#define T(n) (n->tree)
#define G(n) (n->graph)

typedef struct {
    Eina_List *focus_stack;
    Eina_Hash *node_hash;
    Efl_Ui_Focus_Manager *redirect;
    Eina_List *dirty;

    Node *root;
} Efl_Ui_Focus_Manager_Data;

static Efl_Ui_Focus_Direction
_complement(Efl_Ui_Focus_Direction dir)
{
    #define COMP(a,b) \
        if (dir == a) return b; \
        if (dir == b) return a;

    COMP(EFL_UI_FOCUS_DIRECTION_RIGHT, EFL_UI_FOCUS_DIRECTION_LEFT)
    COMP(EFL_UI_FOCUS_DIRECTION_UP, EFL_UI_FOCUS_DIRECTION_DOWN)
    COMP(EFL_UI_FOCUS_DIRECTION_PREV, EFL_UI_FOCUS_DIRECTION_NEXT)

    #undef COMP

    return EFL_UI_FOCUS_DIRECTION_LAST;
}

/*
 * Set this new list of partners to the border.
 * All old partners will be deleted
 */
static void
border_partners_set(Node *node, Efl_Ui_Focus_Direction direction, Eina_List *list)
{
   Node *partner;
   Eina_List *lnode;
   Border *border = &G(node).directions[direction];

   EINA_LIST_FREE(border->partners, partner)
     {
        Border *comp_border = &G(partner).directions[_complement(direction)];

        comp_border->partners = eina_list_remove(comp_border->partners, node);
     }

   border->partners = list;

   EINA_LIST_FOREACH(border->partners, lnode, partner)
     {
        Border *comp_border = &G(partner).directions[_complement(direction)];

        comp_border->partners = eina_list_append(comp_border->partners, node);
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
node_get(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *focusable)
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
   //free the graph items
   for(int i = 0;i < NODE_DIRECTIONS_COUNT; i++)
     {
        border_partners_set(item, i, NULL);
     }

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

   free(item);
}


//CALCULATING STUFF

static inline int
_distance(Eina_Rectangle node, Eina_Rectangle op, Dimension dim)
{
    int min, max, point;
    int v1, v2;

    if (dim == DIMENSION_X)
      {
         min = op.x;
         max = eina_rectangle_max_x(&op);
         point = node.x + node.w/2;
      }
    else
      {
         min = op.y;
         max = eina_rectangle_max_y(&op);
         point = node.y + node.h/2;
      }

    v1 = min - point;
    v2 = max - point;

    if (abs(v1) < abs(v2))
      return v1;
    else
      return v2;
}

static inline void
_calculate_node(Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *node, Dimension dim, Eina_List **pos, Eina_List **neg)
{
   Eina_Rectangle rect = EINA_RECTANGLE_INIT;
   Efl_Ui_Focus_Object *op;
   int dim_min, dim_max;
   Eina_Iterator *nodes;
   int cur_pos_min = 0, cur_neg_min = 0;
   Node *n;

   nodes = eina_hash_iterator_data_new(pd->node_hash);
   rect = efl_ui_focus_object_focus_geometry_get(node);

   *pos = NULL;
   *neg = NULL;

   if (dim == DIMENSION_X)
     {
        dim_min = rect.y;
        dim_max = rect.y + rect.h;
     }
   else
     {
        dim_min = rect.x;
        dim_max = rect.x + rect.w;
     }

   EINA_ITERATOR_FOREACH(nodes, n)
     {
        Eina_Rectangle op_rect = EINA_RECTANGLE_INIT;
        int min, max;

        op = n->focusable;
        if (op == node) continue;

        if (n->type == NODE_TYPE_ONLY_LOGICAL) continue;

        op_rect = efl_ui_focus_object_focus_geometry_get(op);

        if (dim == DIMENSION_X)
          {
             min = op_rect.y;
             max = eina_rectangle_max_y(&op_rect);
          }
        else
          {
             min = op_rect.x;
             max = eina_rectangle_max_x(&op_rect);
          }


        /* The only way the calculation does make sense is if the two number
         * lines are not disconnected.
         * If they are connected one point of the 4 lies between the min and max of the other line
         */
        if (!((min <= max && max <= dim_min && dim_min <= dim_max) ||
              (dim_min <= dim_max && dim_max <= min && min <= max)) &&
              !eina_rectangle_intersection(&op_rect, &rect))
          {
             //this thing hits horizontal
             int tmp_dis;

             tmp_dis = _distance(rect, op_rect, dim);

             if (tmp_dis < 0)
               {
                  if (tmp_dis == cur_neg_min)
                    {
                       //add it
                       *neg = eina_list_append(*neg, op);
                    }
                  else if (tmp_dis > cur_neg_min
                    || cur_neg_min == 0) //init case
                    {
                       //nuke the old and add
#ifdef CALC_DEBUG
                       printf("CORRECTION FOR %s-%s\n found anchor %s-%s in distance %d\n (%d,%d,%d,%d)\n (%d,%d,%d,%d)\n\n", DEBUG_TUPLE(node), DEBUG_TUPLE(op),
                         tmp_dis,
                         op_rect.x, op_rect.y, op_rect.w, op_rect.h,
                         rect.x, rect.y, rect.w, rect.h);
#endif
                       *neg = eina_list_free(*neg);
                       *neg = eina_list_append(NULL, op);
                       cur_neg_min = tmp_dis;
                    }
               }
             else
               {
                  if (tmp_dis == cur_pos_min)
                    {
                       //add it
                       *pos = eina_list_append(*pos, op);
                    }
                  else if (tmp_dis < cur_pos_min
                    || cur_pos_min == 0) //init case
                    {
                       //nuke the old and add
#ifdef CALC_DEBUG
                       printf("CORRECTION FOR %s-%s\n found anchor %s-%s in distance %d\n (%d,%d,%d,%d)\n (%d,%d,%d,%d)\n\n", DEBUG_TUPLE(node), DEBUG_TUPLE(op),
                         tmp_dis,
                         op_rect.x, op_rect.y, op_rect.w, op_rect.h,
                         rect.x, rect.y, rect.w, rect.h);
#endif
                       *pos = eina_list_free(*pos);
                       *pos = eina_list_append(NULL, op);
                       cur_pos_min = tmp_dis;
                    }
               }


#if 0
             printf("(%d,%d,%d,%d)%s vs(%d,%d,%d,%d)%s\n", rect.x, rect.y, rect.w, rect.h, elm_widget_part_text_get(node, NULL), op_rect.x, op_rect.y, op_rect.w, op_rect.h, elm_widget_part_text_get(op, NULL));
             printf("(%d,%d,%d,%d)\n", min, max, dim_min, dim_max);
             printf("Candidate %d\n", tmp_dis);
             if (anchor->anchor == NULL || abs(tmp_dis) < abs(distance)) //init case
               {
                  distance = tmp_dis;
                  anchor->positive = tmp_dis > 0 ? EINA_FALSE : EINA_TRUE;
                  anchor->anchor = op;
                  //Helper for debugging wrong calculations

               }
#endif
         }

     }
}

#ifdef CALC_DEBUG
static void
_debug_node(Node *node)
{
   Eina_List *tmp = NULL;

   if (!node) return;

   printf("NODE %s-%s\n", DEBUG_TUPLE(node->focusable));

#define DIR_LIST(dir) G(node).directions[dir].partners

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

static void
convert_border_set(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Data *pd, Node *node, Eina_List *focusable_list, Efl_Ui_Focus_Direction dir)
{
   Eina_List *partners = NULL;
   Efl_Ui_Focus_Object *fobj;

   EINA_LIST_FREE(focusable_list, fobj)
     {
        Node *entry;

        entry = node_get(obj, pd, fobj);
        if (!entry)
          {
             CRI("Found a obj in graph without node-entry!");
             return;
          }
        partners = eina_list_append(partners, entry);
     }

   border_partners_set(node, dir, partners);
}

static void
dirty_flush_node(Efl_Ui_Focus_Manager *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Node *node)
{
   Eina_List *x_partners_pos, *x_partners_neg;
   Eina_List *y_partners_pos, *y_partners_neg;

   _calculate_node(pd, node->focusable, DIMENSION_X, &x_partners_pos, &x_partners_neg);
   _calculate_node(pd, node->focusable, DIMENSION_Y, &y_partners_pos, &y_partners_neg);

   convert_border_set(obj, pd, node, x_partners_pos, EFL_UI_FOCUS_DIRECTION_RIGHT);
   convert_border_set(obj, pd, node, x_partners_neg, EFL_UI_FOCUS_DIRECTION_LEFT);
   convert_border_set(obj, pd, node, y_partners_neg, EFL_UI_FOCUS_DIRECTION_UP);
   convert_border_set(obj, pd, node, y_partners_pos, EFL_UI_FOCUS_DIRECTION_DOWN);

#ifdef CALC_DEBUG
   _debug_node(node);
#endif
}

static void
dirty_flush(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Data *pd, Node *node)
{
   if (!eina_list_data_find(pd->dirty, node)) return;

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);

   pd->dirty = eina_list_remove(pd->dirty, node);

   dirty_flush_node(obj, pd, node);
}

static void
dirty_flush_all(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager_Data *pd)
{
   Node *node;

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, NULL);

   EINA_LIST_FREE(pd->dirty, node)
     {
        dirty_flush_node(obj, pd, node);
     }
}

static void
dirty_add(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Node *dirty)
{
   if (dirty->type == NODE_TYPE_ONLY_LOGICAL)
     {
        ERR("Only not only logical nodes can be marked dirty");
        return;
     }

   //if (eina_list_data_find(pd->dirty, dirty)) return;
   pd->dirty = eina_list_remove(pd->dirty, dirty);
   pd->dirty = eina_list_append(pd->dirty, dirty);

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, NULL);
}


static void
_node_new_geometery_cb(void *data, const Efl_Event *event)
{
   Node *node;
   FOCUS_DATA(data)

   node = node_get(data, pd, event->object);
   if (!node)
      return;

   dirty_add(data, pd, node);

   return;
}

EFL_CALLBACKS_ARRAY_DEFINE(focusable_node,
    {EFL_GFX_EVENT_RESIZE, _node_new_geometery_cb},
    {EFL_GFX_EVENT_MOVE, _node_new_geometery_cb},
);

//=============================

static Node*
_register(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child, Node *parent)
{
   Node *node;
   if (!!eina_hash_find(pd->node_hash, &child))
     {
        ERR("Child %p is already registered in the graph", child);
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

   return node;
}
EOLIAN static Eina_Bool
_efl_ui_focus_manager_register_logical(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent,  Efl_Ui_Focus_Manager *redirect)
{
   Node *node = NULL;
   Node *pnode = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, MY_CLASS), EINA_FALSE);

   F_DBG("Manager: %p register %p %p %p", obj, child, parent, redirect);

   pnode = node_get(obj, pd, parent);
   if (!pnode) return EINA_FALSE;

   node = _register(obj, pd, child, pnode);
   if (!node) return EINA_FALSE;

   node->type = NODE_TYPE_ONLY_LOGICAL;
   node->redirect_manager = redirect;

   //set again
   if (T(pnode).safed_order)
     {
        Eina_List *tmp;

        tmp = eina_list_clone(T(pnode).safed_order);
        efl_ui_focus_manager_update_order(obj, parent, tmp);
     }

   return EINA_TRUE;
}


EOLIAN static Eina_Bool
_efl_ui_focus_manager_register(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent, Efl_Ui_Focus_Manager *redirect)
{
   Node *node = NULL;
   Node *pnode = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, MY_CLASS), EINA_FALSE);

   F_DBG("Manager: %p register %p %p %p", obj, child, parent, redirect);

   pnode = node_get(obj, pd, parent);
   if (!pnode) return EINA_FALSE;

   node = _register(obj, pd, child, pnode);
   if (!node) return EINA_FALSE;

   //listen to changes
   efl_event_callback_array_add(child, focusable_node(), obj);

   node->type = NODE_TYPE_NORMAL;
   node->redirect_manager = redirect;

   //mark dirty
   dirty_add(obj, pd, node);

   //set again
   if (T(pnode).safed_order)
     {
        Eina_List *tmp;

        tmp = eina_list_clone(T(pnode).safed_order);
        efl_ui_focus_manager_update_order(obj, parent, tmp);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_update_redirect(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Manager *redirect)
{
   Node *node = node_get(obj, pd, child);
   if (!node) return EINA_FALSE;

   if (redirect)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(redirect, MY_CLASS), EINA_FALSE);

   node->redirect_manager = redirect;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_update_parent(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent_obj)
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
_efl_ui_focus_manager_update_order(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *parent, Eina_List *order)
{
   Node *pnode;
   Efl_Ui_Focus_Object *o;
   Eina_List *node_order = NULL, *not_ordered, *trash, *node_order_clean, *n;

   F_DBG("Manager_update_order on %p %p", obj, parent);

   pnode = node_get(obj, pd, parent);
   if (!pnode)
     return;

   ELM_SAFE_FREE(T(pnode).safed_order, eina_list_free);
   T(pnode).safed_order = order;

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

   T(pnode).children = eina_list_merge(node_order_clean, not_ordered);

   return;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_update_children(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *parent, Eina_List *order)
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

   T(pnode).children = node_order;

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_focus_manager_unregister(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child)
{
   Node *node;

   node = eina_hash_find(pd->node_hash, &child);

   if (!node) return;

   F_DBG("Manager: %p unregister %p", obj, child);


   //remove the object from the stack if it hasn't done that until now
   //after this it's not at the top anymore
   //elm_widget_focus_set(node->focusable, EINA_FALSE);
   //delete again from the list, for the case it was not at the top
   pd->focus_stack = eina_list_remove(pd->focus_stack, node);

   //add all neighbors of the node to the dirty list
   for(int i = 0; i < 4; i++)
     {
        Node *partner;
        Eina_List *n;

        EINA_LIST_FOREACH(node->graph.directions[i].partners, n, partner)
          {
             dirty_add(obj, pd, partner);
          }
     }

   //remove from the dirty parts
   pd->dirty = eina_list_remove(pd->dirty, node);

   eina_hash_del_by_key(pd->node_hash, &child);
}

EOLIAN static void
_efl_ui_focus_manager_redirect_set(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Manager *redirect)
{
   Efl_Ui_Focus_Manager *old_manager;

   if (pd->redirect == redirect) return;

   F_DBG("Manager: %p setting redirect from %p to %p", obj, pd->redirect, redirect);

   if (pd->redirect)
     efl_wref_del(pd->redirect, &pd->redirect);

   old_manager = pd->redirect;
   pd->redirect = redirect;

   if (pd->redirect)
     efl_wref_add(pd->redirect, &pd->redirect);

   efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED , old_manager);
}

EOLIAN static Efl_Ui_Focus_Manager *
_efl_ui_focus_manager_redirect_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd)
{
   return pd->redirect;
}

static void
_free_node(void *data)
{
   Node *node = data;
   FOCUS_DATA(node->manager);

   efl_event_callback_array_del(node->focusable, focusable_node(), node->manager);

   if (pd->root != data)
     {
        node_item_free(node);
     }
}

EOLIAN static Efl_Object *
_efl_ui_focus_manager_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Manager_Data *pd)
{
   pd->node_hash = eina_hash_pointer_new(_free_node);
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_ui_focus_manager_efl_object_provider_find(Eo *obj, Efl_Ui_Focus_Manager_Data *pd EINA_UNUSED, const Efl_Object *klass)
{
   if (klass == MY_CLASS)
     return obj;

   return efl_provider_find(efl_super(obj, MY_CLASS), klass);
}

EOLIAN static void
_efl_ui_focus_manager_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Manager_Data *pd)
{
   eina_list_free(pd->focus_stack);
   eina_list_free(pd->dirty);

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
        for(int i = 0 ;i < NODE_DIRECTIONS_COUNT; i++)
          {
             if (node->type != NODE_TYPE_ONLY_LOGICAL &&
                 !node->graph.directions[i].partners)
               {
                  *data = node->focusable;
                  return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}

static Elm_Layout *
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

EOLIAN static Eina_Iterator*
_efl_ui_focus_manager_border_elements_get(Eo *obj, Efl_Ui_Focus_Manager_Data *pd)
{
   Border_Elements_Iterator *it;

   dirty_flush_all(obj, pd);

   it = calloc(1, sizeof(Border_Elements_Iterator));

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_hash_iterator_data_new(pd->node_hash);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_iterator_free);
   it->object = obj;

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
   Eina_Rectangle geom;

   geom = efl_ui_focus_object_focus_geometry_get(obj);
   elem->x = geom.x + geom.w/2;
   elem->y = geom.y + geom.h/2;
}

static Node*
_coords_movement(Efl_Ui_Focus_Manager_Data *pd, Node *upper, Efl_Ui_Focus_Direction direction)
{
   Node *candidate;
   Eina_List *node_list;

   //we are searching which of the partners is lower to the history
   EINA_LIST_REVERSE_FOREACH(pd->focus_stack, node_list, candidate)
     {
        if (eina_list_data_find(G(upper).directions[direction].partners, candidate))
          {
             //this is the next accessable part
             return candidate;
          }
     }

   //if we haven't found anything in the history, use the widget with the smallest distance
   {
      Eina_List *lst = G(upper).directions[direction].partners;
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
   //case 1 there is a item in the parent previous to node, which has children
   if (n && T(n).children)
     {
        do
          {
              n = eina_list_last_data_get(T(n).children);
          }
        while (T(n).children);

        return n;
     }

   //case 2 there is a item in the parent previous to node, which has no children
   if (n)
     return n;

   //case 3 there is a no item in the parent previous to this one
   return T(node).parent;
}


static Node*
_logical_movement(Efl_Ui_Focus_Manager_Data *pd EINA_UNUSED, Node *upper, Efl_Ui_Focus_Direction direction)
{
   Node* (*deliver)(Node *n);
   Node *result;
   Eina_List *stack = NULL;

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
        result = deliver(result);
   } while(result && result->type != NODE_TYPE_NORMAL && !result->redirect_manager);

   eina_list_free(stack);

   return result;
}

static Efl_Ui_Focus_Object*
_request_move(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Direction direction, Node *upper)
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

   dirty_flush(obj, pd, upper);

   if (direction == EFL_UI_FOCUS_DIRECTION_PREV
    || direction == EFL_UI_FOCUS_DIRECTION_NEXT)
      dir = _logical_movement(pd, upper, direction);
   else
      dir = _coords_movement(pd, upper, direction);

   //return the widget
   if (dir)
     return dir->focusable;
   else
     return NULL;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_request_move(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Direction direction)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_CHECK(direction), NULL);

   if (pd->redirect)
     return efl_ui_focus_manager_request_move(pd->redirect, direction);
   else
     {
        Node *upper = NULL;

        upper = eina_list_last_data_get(pd->focus_stack);

        if (!upper)
          {
             upper = _no_history_element(pd->node_hash);
             if (upper)
               return upper->focusable;
             return NULL;
          }

        return _request_move(obj, pd, direction, upper);
     }
}

EOLIAN static void
_efl_ui_focus_manager_focus(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *focus)
{
   Node *node;
   Node *old_focus;
   Efl_Ui_Focus_Manager *redirect_manager;

   EINA_SAFETY_ON_NULL_RETURN(focus);

   //if we want to focus the root then just spin to the first normal
   if (focus == pd->root->focusable)
     {
        Node *f = _logical_movement(pd, pd->root, EFL_UI_FOCUS_DIRECTION_NEXT);

        if (f)
          focus = f->focusable;

        if (!focus) return;
     }

   //check if node is part of this manager object
   node = node_get(obj, pd, focus);
   if (!node) return;

   F_DBG("Manager: %p focusing object %p %s", obj, focus, efl_class_name_get(focus));

   if (node->type == NODE_TYPE_ONLY_LOGICAL && !node->redirect_manager && pd->root != node)
     {
        ERR(" %p is logical, cannot be focused", obj);
        return;
     }

   if (pd->redirect)
     {
        //first unset the redirect
        efl_ui_focus_manager_redirect_set(obj, NULL);
     }

   redirect_manager = node->redirect_manager;

   if (node->type == NODE_TYPE_NORMAL)
     {
        Eo *focusable;
        //check if this is already the focused object
        old_focus = eina_list_last_data_get(pd->focus_stack);

        //check if this is already at the top
        if (old_focus && old_focus->focusable == focus) return;

        //remove the object from the list and add it again
        pd->focus_stack = eina_list_remove(pd->focus_stack, node);
        pd->focus_stack = eina_list_append(pd->focus_stack, node);

        //save fields we later need
        focusable = node->focusable;

        //populate the new change
        if (old_focus) efl_ui_focus_object_focus_set(old_focus->focusable, EINA_FALSE);
        efl_ui_focus_object_focus_set(node->focusable, EINA_TRUE);
        efl_event_callback_call(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, focusable);
        node = NULL;
     }
   else if (node->redirect_manager)
     {
        Efl_Ui_Focus_Object *root;

        root = efl_ui_focus_manager_root_get(node->redirect_manager);
        efl_ui_focus_manager_focus(node->redirect_manager, root);
     }

   //now check if this is also a listener object
   if (redirect_manager)
     {
        efl_ui_focus_manager_redirect_set(obj, redirect_manager);
     }
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_move(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Efl_Ui_Focus_Object *candidate = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(DIRECTION_CHECK(direction), NULL);

   if (pd->redirect)
     {
        Efl_Ui_Focus_Object *old_candidate = NULL;
        candidate = efl_ui_focus_manager_move(pd->redirect, direction);
        old_candidate = efl_ui_focus_manager_focused(pd->redirect);

        if (!candidate)
          {
             Efl_Ui_Focus_Object *new_candidate = NULL;
             Node *n;

             //there is no candidate check if we have something for that direction
             new_candidate = NULL;
             n = eina_hash_find(pd->node_hash, &old_candidate);

             if (direction == EFL_UI_FOCUS_DIRECTION_NEXT ||
                 direction == EFL_UI_FOCUS_DIRECTION_PREV)
               {
                 if (n)
                   {
                      n = T(n).parent;
                      new_candidate = _request_move(obj, pd, direction, n);
                      efl_ui_focus_manager_focus(obj, new_candidate);
                      candidate = new_candidate;
                   }
               }
             else
               {

                  if (n)
                    new_candidate = _request_move(obj, pd, direction, n);

                  if (new_candidate)
                    {
                       //redirect does not have smth. but we do have.
                       efl_ui_focus_manager_focus(obj, new_candidate);
                    }
                 candidate = new_candidate;
               }

          }
     }
   else
     {
        candidate = efl_ui_focus_manager_request_move(obj, direction);

        if (candidate)
          efl_ui_focus_manager_focus(obj, candidate);
     }

   F_DBG("Manager: %p moved to %p %s in direction %d", obj, candidate, efl_class_name_get(candidate), direction);

   return candidate;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_root_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *root)
{
   Node *node;

   if (pd->root)
     {
        ERR("Root element can only be set once!");
        return EINA_FALSE;
     }

   node = _register(obj, pd, root, NULL);
   node->type = NODE_TYPE_ONLY_LOGICAL;

   pd->root = node;

   return EINA_TRUE;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_root_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd)
{
   if (!pd->root) return NULL;

   return pd->root->focusable;
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_efl_object_finalize(Eo *obj, Efl_Ui_Focus_Manager_Data *pd)
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
_convert(Eina_List *node_list)
{
   Eina_List *n, *par = NULL;
   Node *node;

   EINA_LIST_FOREACH(node_list, n, node)
     par = eina_list_append(par, node->focusable);

   return par;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_focused(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd)
{
   Node *upper = NULL;

   upper = eina_list_last_data_get(pd->focus_stack);

   if (!upper)
     return NULL;
   return upper->focusable;
}

EOLIAN static Efl_Ui_Focus_Relations*
_efl_ui_focus_manager_fetch(Eo *obj, Efl_Ui_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *child)
{
   Efl_Ui_Focus_Relations *res;
   Node *n, *tmp;

   n = node_get(obj, pd, child);
   if (!n)
      return NULL;

   res = calloc(1, sizeof(Efl_Ui_Focus_Relations));

   dirty_flush(obj, pd, n);

#define DIR_CLONE(dir) _convert(G(n).directions[dir].partners);

   res->right = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_RIGHT);
   res->left = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_LEFT);
   res->top = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_UP);
   res->down = DIR_CLONE(EFL_UI_FOCUS_DIRECTION_DOWN);
   res->next = (tmp = _next(n)) ? tmp->focusable : NULL;
   res->prev = (tmp = _prev(n)) ? tmp->focusable : NULL;
   switch(n->type)
     {
        case NODE_TYPE_ONLY_LOGICAL:
          res->type = "logical";
        break;
        case NODE_TYPE_NORMAL:
          res->type = "normal";
        break;
     }
   res->parent = T(n).parent->focusable;
   res->redirect = n->redirect_manager;
#undef DIR_CLONE

   return res;
}

EOLIAN static void
_efl_ui_focus_manager_class_constructor(Efl_Class *c EINA_UNUSED)
{
   _focus_log_domain = eina_log_domain_register("elementary-focus", EINA_COLOR_CYAN);
}

EOLIAN static void
_efl_ui_focus_manager_class_destructor(Efl_Class *c EINA_UNUSED)
{
   eina_log_domain_unregister(_focus_log_domain);
   _focus_log_domain = -1;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_logical_end(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Data *pd)
{
   Node *child = pd->root;

   EINA_SAFETY_ON_NULL_RETURN_VAL(child, NULL);

   //we need to return the most lower right element

   while(T(child).children)
     child = eina_list_last_data_get(T(child).children);
   while (child->type != NODE_TYPE_NORMAL)
     child = _prev(child);

  return child ? child->focusable : NULL;
}

#include "efl_ui_focus_manager.eo.c"
