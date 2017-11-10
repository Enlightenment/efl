#include "sg_node.h"

#define SG_UPDATER_DEBUG
#define DBG printf

static void destructor(SG_Node_Updater* obj);
static void update_states(SG_Node_Updater *obj, SG_Node *n);
static Eina_Bool is_node_blocked(const SG_Node_Updater *obj, SG_Node *n, SG_Node *root);

static void visit_node(SG_Node_Updater *obj, SG_Node *n);

static SG_Node_Updater_Func VTABLE =
{
   destructor,
   update_states,
   is_node_blocked,

};
void sg_node_updater_ctr(SG_Node_Updater *obj)
{
   float opacity = 1;
   obj->fn = &VTABLE;
   obj->m_combined_matrix_stack = eina_array_new(20);
   obj->m_opacity_stack = eina_inarray_new(sizeof(float), 20);
   eina_inarray_push(obj->m_opacity_stack, &opacity);
}

static void destructor(SG_Node_Updater* obj)
{
   eina_inarray_free(obj->m_opacity_stack);
   eina_array_free(obj->m_combined_matrix_stack);
}

SG_Node_Updater * sg_node_updater_create(void)
{
   SG_Node_Updater *obj = calloc(1, sizeof(SG_Node_Updater));
   sg_node_updater_ctr(obj);
   return obj;
}

void  sg_node_updater_destroy(SG_Node_Updater *obj)
{
   if (!obj) return;
   obj->fn->dtr(obj);
   free(obj);
}

static void update_states(SG_Node_Updater *obj, SG_Node *n)
{
    obj->m_current_clip = 0;
    obj->m_force_update = 0;
    visit_node(obj, n);
}

static inline void visit_children(SG_Node_Updater *obj, SG_Node *n)
{
    for (SG_Node *c = n->m_first_child; c; c = c->m_next_sibling)
        visit_node(obj, c);
}

static void enter_transform_node(SG_Node_Updater *obj, SG_Transform_Node *t)
{
   const Eina_Matrix3 *m = t->fn->matrix_get(t);
   Eina_Matrix3 tmp;
   Eina_Matrix3 *combine = NULL;

#ifdef SG_UPDATER_DEBUG
    DBG("enter transform: %p  force= %d\n", t, obj->m_force_update);
#endif

   if (eina_array_count_get(obj->m_combined_matrix_stack))
     combine = eina_array_data_get(obj->m_combined_matrix_stack,
                                   eina_array_count_get(obj->m_combined_matrix_stack));

   if (m && (eina_matrix3_type_get(m) != EINA_MATRIX_TYPE_IDENTITY ))
     {
        if (combine)
          {
             eina_matrix3_multiply(&tmp, combine , m);
             t->fn->combined_matrix_set(t, &tmp);
          }
        else
          {
             t->fn->combined_matrix_set(t, m);
          }
        eina_array_push(obj->m_combined_matrix_stack, t->fn->combined_matrix_get(t));
     }
   else
     {
        if (combine)
          {
             t->fn->combined_matrix_set(t, combine);
          }
        else
          {
             t->fn->combined_matrix_set(t, NULL);
          }
     }
}

static void leave_transform_node(SG_Node_Updater *obj, SG_Transform_Node *t)
{
   const Eina_Matrix3 *m = t->fn->matrix_get(t);

#ifdef SG_UPDATER_DEBUG
    DBG("leave transform: %p\n",t);
#endif

   if (m && (eina_matrix3_type_get(m) != EINA_MATRIX_TYPE_IDENTITY ))
     {
        eina_array_pop(obj->m_combined_matrix_stack);
     }
}

static void enter_clip_node(SG_Node_Updater *obj, SG_Clip_Node *c)
{
   Eina_Matrix3 *m = NULL;

#ifdef SG_UPDATER_DEBUG
    DBG("enter clip: %p\n", c);
#endif

   if (eina_array_count_get(obj->m_combined_matrix_stack))
     m = eina_array_data_get(obj->m_combined_matrix_stack, eina_array_count_get(obj->m_combined_matrix_stack));

   c->m_matrix = m;
   c->m_clipper = obj->m_current_clip;
   obj->m_current_clip = (SG_Node *)c;
}

static void leave_clip_node(SG_Node_Updater *obj, SG_Clip_Node *c)
{
#ifdef SG_UPDATER_DEBUG
    DBG("leave clip: %p\n", c);
#endif
   obj->m_current_clip = c->m_clipper;
}

static void enter_opacity_node(SG_Node_Updater *obj, SG_Opacity_Node *o)
{
   float opacity = *((float *)eina_inarray_nth(obj->m_opacity_stack , eina_inarray_count(obj->m_opacity_stack)));

#ifdef SG_UPDATER_DEBUG
    DBG("enter opacity: %p\n", o);
#endif
   opacity = opacity * o->m_opacity;
   o->fn->combined_opacity_set(o, opacity);
   eina_inarray_push(obj->m_opacity_stack, &opacity);
}

static void leave_opacity_node(SG_Node_Updater *obj, SG_Opacity_Node *o)
{

#ifdef SG_UPDATER_DEBUG
    DBG("leave opacity: %p\n", o);
#endif

   if (o->fn->flags((SG_Node *)o) & SG_NODE_DIRTY_OPACITY)
     --obj->m_force_update;

   eina_inarray_pop(obj->m_opacity_stack);
}

static void enter_geometry_node(SG_Node_Updater *obj, SG_Geometry_Node *g)
{
   float opacity = *((float *)eina_inarray_nth(obj->m_opacity_stack , eina_inarray_count(obj->m_opacity_stack)));
   Eina_Matrix3 *m = NULL;
#ifdef SG_UPDATER_DEBUG
    DBG("enter geometry: %p\n", g);
#endif
   if (eina_array_count_get(obj->m_combined_matrix_stack))
     m = eina_array_data_get(obj->m_combined_matrix_stack, eina_array_count_get(obj->m_combined_matrix_stack));

   g->m_matrix = m;
   g->m_clipper = obj->m_current_clip;
   g->fn->inherited_opacity_set(g, opacity);
}

inline static void leave_geometry_node(SG_Node_Updater *obj EINA_UNUSED, SG_Geometry_Node *g EINA_UNUSED)
{
#ifdef SG_UPDATER_DEBUG
    DBG("leave geometry: %p\n", g);
#endif
}

void visit_node(SG_Node_Updater *obj, SG_Node *n)
{

    if (!obj->m_force_update)
        return;
    if (n->fn->is_subtree_blocked(n))
        return;

   switch (n->fn->type(n))
     {
       case SG_NODE_TYPE_TRANSFORM:
         {
            SG_Transform_Node *t = (SG_Transform_Node *)n;
            enter_transform_node(obj, t);
            visit_children(obj, n);
            leave_transform_node(obj, t);
            break;
         }
       case SG_NODE_TYPE_GEOMETRY:
         {
             SG_Geometry_Node *g = (SG_Geometry_Node *)n;
             enter_geometry_node(obj, g);
             visit_children(obj, n);
             leave_geometry_node(obj, g);
             break;
         }
       case SG_NODE_TYPE_CLIP:
         {
             SG_Clip_Node *c = (SG_Clip_Node *)n;
             enter_clip_node(obj, c);
             visit_children(obj, n);
             leave_clip_node(obj, c);
             break;
         }
       case SG_NODE_TYPE_OPACITY:
         {
             SG_Opacity_Node *o = (SG_Opacity_Node *)n;
             enter_opacity_node(obj, o);
             visit_children(obj, n);
             leave_opacity_node(obj, o);
             break;
         }
       default:
           visit_children(obj, n);
           break;
     }
}

/*!
   Returns true if \a node is has something that blocks it in the chain from
   \a node to \a root doing a full state update pass.

   This function does not process dirty states, simply does a simple traversion
   up to the top.

   The function assumes that \a root exists in the parent chain of \a node.
 */
static Eina_Bool
is_node_blocked(const SG_Node_Updater *obj EINA_UNUSED,  SG_Node *node, SG_Node *root)
{
   while ((node != root) && (node != 0))
     {
        if (node->fn->is_subtree_blocked(node))
          return EINA_TRUE;
        node = node->fn->parent(node);
     }
   return EINA_FALSE;
}

