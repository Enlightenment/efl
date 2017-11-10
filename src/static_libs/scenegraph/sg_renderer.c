#include "sg_renderer.h"

static void add_nodes_topreprocess(SG_Renderer *obj, SG_Node *node);
static void remove_nodes_topreprocess(SG_Renderer *obj, SG_Node *node);

static void root_node_set(SG_Renderer *obj, SG_Root_Node *node);
static SG_Root_Node* root_node_get(const SG_Renderer *obj);
static SG_Node_Updater* node_updater_get(const SG_Renderer *obj);
static void node_updater_set(SG_Renderer *obj, SG_Node_Updater *updater);
static void node_changed(SG_Renderer *obj, SG_Node *node, SG_Node_Dirty_State state);
static void preprocess(SG_Renderer *obj);
static void render_scene(SG_Renderer *obj);
static void render(SG_Renderer *obj);
static void release_cached_resources(SG_Renderer *obj);

/*!
    \class SG_Renderer
    \brief The renderer class is the abstract baseclass used for rendering the
    scene graph.

    The renderer is not tied to any particular surface. It expects a context to
    be current and will render into that surface according to how the device rect,
    viewport rect and projection transformation are set up.

    Rendering is a sequence of steps initiated by calling renderScene(). This will
    effectively draw the scene graph starting at the root node. The SG_Node::preprocess()
    function will be called for all the nodes in the graph, followed by an update
    pass which updates all matrices, opacity, clip states and similar in the graph.
    Because the update pass is called after preprocess, it is safe to modify the graph
    during preprocess. To run a custom update pass over the graph, install a custom
    SGNodeUpdater using node_updater_set(). Once all the graphs dirty states are updated,
    the virtual render() function is called.

    The render() function is implemented by SG_Renderer subclasses to render the graph
    in the most optimal way for a given hardware.

    The renderer can make use of stencil, depth and color buffers in addition to the
    scissor rect.

    \internal
 */

static SG_Renderer_Func VTABLE =
{
   sg_renderer_dtr,
   root_node_set,
   root_node_get,
   node_updater_set,
   node_updater_get,
   node_changed,
   preprocess,
   render,
   render_scene,
   release_cached_resources,
   &VTABLE
};

void sg_renderer_ctr(SG_Renderer* obj)
{
   obj->fn = &VTABLE;
}

void
sg_renderer_dtr(SG_Renderer* obj EINA_UNUSED)
{

}

SG_Renderer *
sg_renderer_create(void)
{
   SG_Renderer *obj = calloc(1, sizeof(SG_Renderer));
   sg_renderer_ctr(obj);
   return obj;
}

void
sg_renderer_destroy(SG_Renderer* obj)
{
   if (obj)
     {
        obj->fn->dtr(obj);
        free(obj);
     }
}

static void release_cached_resources(SG_Renderer *obj EINA_UNUSED)
{
   // FIXME No implementation
}


/*!
    Sets the \a node as the root of the SG_Node scene
    that you want to render. You need to provide a \a node
    before trying to render the scene.

    \note This doesn't take ownership of \a node.

    \sa root_node_get()
*/

static void root_node_set(SG_Renderer *obj, SG_Root_Node *node)
{
   if (obj->m_root_node == node)
     return;
   if (obj->m_root_node)
     {
        // remove it from the old renderer
        obj->m_root_node->m_renderer = NULL;
        node_changed(obj, (SG_Node *)obj->m_root_node, SG_NODE_DIRTY_NODE_REMOVED);
     }
   obj->m_root_node = node;
   if (obj->m_root_node)
     {
        obj->m_root_node->m_renderer = obj;
        node_changed(obj, (SG_Node *)obj->m_root_node, SG_NODE_DIRTY_NODE_ADDED);
     }
}

/*!
    Returns the root of the SG_Node scene.

    \sa root_node_set()
*/
static SG_Root_Node *root_node_get(const SG_Renderer *obj)
{
   return obj->m_root_node;
}

/*!
    Returns the node updater that this renderer uses to update states in the
    scene graph.

    If no updater is specified a default one is constructed.
 */
static SG_Node_Updater* node_updater_get(const SG_Renderer *obj)
{
   return obj->m_node_updater;
}

/*!
    Sets the node updater that this renderer uses to update states in the
    scene graph.

    This will delete and override any existing node updater
*/
static void node_updater_set(SG_Renderer *obj, SG_Node_Updater *updater)
{
   if (obj->m_node_updater)
     sg_node_updater_destroy(obj->m_node_updater);
   obj->m_node_updater = updater;
}

/*!
    Updates internal data structures and emits the sceneGraphChanged() signal.

    If \a flags contains the SG_NODE_DIRTY_NODE_REMOVED flag, the node might be
    in the process of being destroyed. It is then not safe to downcast the node
    pointer.
*/

static void node_changed(SG_Renderer *obj, SG_Node *node, SG_Node_Dirty_State state)
{
   if (state & SG_NODE_DIRTY_NODE_ADDED)
     add_nodes_topreprocess(obj, node);
   if (state & SG_NODE_DIRTY_NODE_REMOVED)
     remove_nodes_topreprocess(obj, node);
   if (state & SG_NODE_DIRTY_USE_PREPROCESS)
     {
        if (node->fn->flags(node) & SG_NODE_USE_PREPROCESS)
          obj->m_nodes_to_preprocess = eina_list_append(obj->m_nodes_to_preprocess, node);
        else
          obj->m_nodes_to_preprocess = eina_list_remove(obj->m_nodes_to_preprocess, node);
     }

   if (!obj->m_changed_emitted && !obj->m_is_rendering)
     {
        obj->m_changed_emitted = EINA_TRUE;
        //emit sceneGraphChanged();
     }
}

static void add_nodes_topreprocess(SG_Renderer *obj, SG_Node *node)
{
   for (SG_Node *c = node->m_first_child; c; c = c->m_next_sibling)
     add_nodes_topreprocess(obj, c);
   if (node->fn->flags(node) & SG_NODE_USE_PREPROCESS)
     obj->m_nodes_to_preprocess = eina_list_append(obj->m_nodes_to_preprocess, node);
}

static void remove_nodes_topreprocess(SG_Renderer *obj, SG_Node *node)
{
   for (SG_Node *c = node->m_first_child; c; c = c->m_next_sibling)
     remove_nodes_topreprocess(obj, c);
   if (node->fn->flags(node) & SG_NODE_USE_PREPROCESS)
     {
        obj->m_nodes_to_preprocess = eina_list_remove(obj->m_nodes_to_preprocess, node);
        // If preprocessing *now*, mark the node as gone.
        if (obj->m_is_preprocessing)
          obj->m_nodes_dont_preprocess = eina_list_append(obj->m_nodes_dont_preprocess, node);
     }
}

void render_scene(SG_Renderer *obj)
{
   if (!root_node_get(obj))
     return;

   obj->m_is_rendering = EINA_TRUE;

   obj->fn->preprocess(obj);

   obj->fn->render(obj);

   obj->m_is_rendering = EINA_FALSE;
   obj->m_changed_emitted = EINA_FALSE;
}

void render(SG_Renderer *obj EINA_UNUSED)
{
   //ERR("You must subclass the SG_Renderer and implement");
}

void preprocess(SG_Renderer *obj)
{
   Eina_List *l, *l_next;
   SG_Node *n;
   SG_Root_Node *root = root_node_get(obj);

   obj->m_is_preprocessing = EINA_TRUE;

   // We need to use EINA_LIST_FOREACH_SAFE, in case any of the preprocess calls deletes a node that
   // is in the preprocess list and thus, changes the m_nodes_to_preprocess behind our backs
   // For the default case, when this does not happen, the cost is negligible.

   // 1. Preprocess phase
   EINA_LIST_FOREACH_SAFE(obj->m_nodes_to_preprocess, l, l_next, n)
     {
        // If we are currently preprocessing, check this node hasn't been
        // deleted or something. we don't want a use-after-free!
        if (eina_list_data_find(obj->m_nodes_dont_preprocess, n)) // skip
          continue;
        if (!obj->m_node_updater->fn->is_node_blocked(obj->m_node_updater, n, (SG_Node *)root))
          {
             n->fn->preprocess(n);
          }
     }

   // 2. update phase
   obj->m_node_updater->fn->update_states(obj->m_node_updater, (SG_Node *)root);

   obj->m_is_preprocessing = EINA_FALSE;
   eina_list_free(obj->m_nodes_dont_preprocess);
}

