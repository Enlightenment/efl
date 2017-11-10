#include "sg_node.h"
#include "sg_renderer.h"

#include <assert.h>

#define ERR printf

/* SG_Node class Start */

static SG_Node_Type  type(const SG_Node *obj);
static SG_Node* parent(const SG_Node *obj);
static void remove_child(SG_Node *obj, SG_Node *node);
static void remove_child_all(SG_Node *obj);
static void prepend_child(SG_Node *obj, SG_Node *node);
static void append_child(SG_Node *obj, SG_Node *node);
static void insert_child_before(SG_Node *obj, SG_Node *node, SG_Node *before);
static void insert_child_after(SG_Node *obj, SG_Node *node, SG_Node *after);
static void reparent_childs_to(SG_Node *obj, SG_Node *new_parent);
static int child_count(const SG_Node *obj);
static SG_Node *child_at(const SG_Node *obj, int i);
static void mark_dirty(SG_Node *obj, SG_Node_Dirty_State bits);
static void flags_set(SG_Node *obj, SG_Node_Flag flags, Eina_Bool enable);
static SG_Node_Flag flags(SG_Node *obj);
Eina_Bool is_subtree_blocked(const SG_Node *obj);
static void preprocess(SG_Node *obj);
void destroy(SG_Node *obj);

static void notify_node_change(SG_Root_Node *obj, SG_Node *node, SG_Node_Dirty_State state);
static inline void sg_node_dtr(SG_Node *obj);

static SG_Node_Func SG_Node_VTABLE = {
   sg_node_dtr,
   type,
   parent,
   remove_child,
   remove_child_all,
   prepend_child,
   append_child,
   insert_child_before,
   insert_child_after,
   reparent_childs_to,
   child_count,
   child_at,
   mark_dirty,
   flags_set,
   flags,
   destroy,
   is_subtree_blocked,
   preprocess,
   &SG_Node_VTABLE,
   NULL
};

inline static void
sg_node_dtr(SG_Node *obj)
{
   destroy(obj);
}

inline static void
sg_node_ctr(SG_Node *obj)
{
   obj->fn = &SG_Node_VTABLE;
   obj->m_node_flags = SG_NODE_OWNED_BY_PARENT;
}

EAPI void
sg_node_destroy(SG_Node *obj)
{
   if (!obj) return;

   obj->fn->dtr(obj);

   free (obj);
}

static SG_Node*
parent(const SG_Node *obj)
{
   return obj->m_parent;
}

/*!
    Removes \a node from this node's list of children.
 */
static void
remove_child(SG_Node *obj, SG_Node *node)
{
   assert(parent(node) == obj);

   SG_Node *previous = node->m_previous_sibling;
   SG_Node *next = node->m_next_sibling;
   if (previous)
     previous->m_next_sibling = next;
   else
     obj->m_first_child = next;
   if (next)
     next->m_previous_sibling = previous;
   else
     obj->m_last_child = previous;
   node->m_previous_sibling = 0;
   node->m_next_sibling = 0;

   mark_dirty(node, SG_NODE_DIRTY_NODE_REMOVED);
   node->m_parent = 0;
}

/*!
    Removes all child nodes from this node's list of children.
 */

static void remove_child_all(SG_Node *obj)
{
   while (obj->m_first_child)
     {
        SG_Node *node = obj->m_first_child;
        obj->m_first_child = node->m_next_sibling;
        node->m_next_sibling = 0;
        if (obj->m_first_child)
            obj->m_first_child->m_previous_sibling = 0;
        else
            obj->m_last_child = 0;
        mark_dirty(node, SG_NODE_DIRTY_NODE_REMOVED);
        node->m_parent = 0;
     }
}

/*!
    Prepends \a node to this node's the list of children.

    Ordering of nodes is important as geometry nodes will be rendered in the
    order they are added to the scene graph.
 */

static void prepend_child(SG_Node *obj, SG_Node *node)
{
   if (node->m_parent)
     {
        ERR("prepend_child :Node %p already has a parent", node);
        return;
     }

   if (obj->m_first_child)
     obj->m_first_child->m_previous_sibling = node;
   else
     obj->m_last_child = node;
   node->m_next_sibling = obj->m_first_child;
   obj->m_first_child = node;
   node->m_parent = obj;

   mark_dirty(node, SG_NODE_DIRTY_NODE_ADDED);
}

/*!
    Appends \a node to this node's list of children.

    Ordering of nodes is important as geometry nodes will be rendered in the
    order they are added to the scene graph.
 */

static void append_child(SG_Node *obj, SG_Node *node)
{
   if (node->m_parent)
     {
        ERR("append_child :Node %p already has a parent", node);
        return;
     }

   if (obj->m_last_child)
     obj->m_last_child->m_next_sibling = node;
   else
     obj->m_first_child = node;
   node->m_previous_sibling = obj->m_last_child;
   obj->m_last_child = node;
   node->m_parent = obj;

   mark_dirty(node, SG_NODE_DIRTY_NODE_ADDED);
}

/*!
    Inserts \a node to this node's list of children before the node specified with \a before.

    Ordering of nodes is important as geometry nodes will be rendered in the
    order they are added to the scene graph.
 */

static void insert_child_before(SG_Node *obj, SG_Node *node, SG_Node *before)
{
   if (node->m_parent)
     {
        ERR("insert_before:Node %p already has a parent", node);
        return;
     }

   if (before->m_parent != obj)
     {
        ERR("Before Node %p dosen't have same parent", before);
        return;
     }

   SG_Node *previous = before->m_previous_sibling;
   if (previous)
     previous->m_next_sibling = node;
   else
     obj->m_first_child = node;
   node->m_previous_sibling = previous;
   node->m_next_sibling = before;
   before->m_previous_sibling = node;
   node->m_parent = obj;

   mark_dirty(node, SG_NODE_DIRTY_NODE_ADDED);
}



/*!
    Inserts \a node to this node's list of children after the node specified with \a after.

    Ordering of nodes is important as geometry nodes will be rendered in the
    order they are added to the scene graph.
 */

static void insert_child_after(SG_Node *obj, SG_Node *node, SG_Node *after)
{
   if (node->m_parent)
     {
        ERR("insert_after:Node %p already has a parent", node);
        return;
     }

   if (after->m_parent != obj)
     {
        ERR("After Node %p dosen't have same parent", after);
        return;
     }

   SG_Node *next = after->m_next_sibling;
   if (next)
     next->m_previous_sibling = node;
   else
     obj->m_last_child = node;
   node->m_next_sibling = next;
   node->m_previous_sibling = after;
   after->m_next_sibling = node;
   node->m_parent = obj;

   mark_dirty(node, SG_NODE_DIRTY_NODE_ADDED);
}

/*!
 * \internal
 *
 * Reparents all nodes of this node to \a newParent.
 */
static void reparent_childs_to(SG_Node *obj, SG_Node *new_parent)
{
   // check if new parent is child of the object
   if (new_parent->m_parent == obj)
     {
        // not a valid operation
        return;
     }

   for (SG_Node *c = obj->m_first_child; c; c = obj->m_first_child)
   {
      remove_child(obj, c);
      append_child(new_parent, c);
   }
}

static int child_count(const SG_Node *obj)
{
   int count = 0;
   SG_Node *n = obj->m_first_child;

   while (n) {
     ++count;
     n = n->m_next_sibling;
   }
   return count;
}


static SG_Node *child_at(const SG_Node *obj, int i)
{
   SG_Node *n = obj->m_first_child;
   while (i && n)
   {
     --i;
     n = n->m_next_sibling;
   }
   return n;
}

static SG_Node_Type  type(const SG_Node *obj)
{
   return obj->m_type;
}


/*!
    Notifies all connected renderers that the node has dirty \a bits.
 */

static void
mark_dirty(SG_Node *obj, SG_Node_Dirty_State bits)
{
   int renderableCountDiff = 0;
   if (bits & SG_NODE_DIRTY_NODE_ADDED)
     renderableCountDiff += obj->m_subtree_renderable_count;
   if (bits & SG_NODE_DIRTY_NODE_REMOVED)
     renderableCountDiff -= obj->m_subtree_renderable_count;

   SG_Node *p = obj->m_parent;
   while (p)
   {
     p->m_subtree_renderable_count += renderableCountDiff;
     if (type(p) == SG_NODE_TYPE_ROOT)
       {
          SG_Root_Node *root = (SG_Root_Node *)p;
          if (root->m_in_dtr) return;
          notify_node_change((SG_Root_Node *)p, obj, bits);
       }
     p = p->m_parent;
   }
}

/*!
    Sets the flags \a f on this node if \a enabled is true;
    otherwise clears the flags.

    \sa flags()
*/

static void
flags_set(SG_Node *obj, SG_Node_Flag f, Eina_Bool enabled)
{
   SG_Node_Flag oldFlags = obj->m_node_flags;

   if (enabled)
     obj->m_node_flags |= f;
   else
     obj->m_node_flags &= ~f;
   int changedFlags = (oldFlags ^ obj->m_node_flags) & SG_NODE_USE_PREPROCESS;
   if (changedFlags)
     mark_dirty(obj, SG_NODE_USE_PREPROCESS);
}

static SG_Node_Flag
flags(SG_Node *obj)
{
   return obj->m_node_flags;
}


/*!
    \fn void preprocess()

    Override this function to do processing on the node before it is rendered.

    Preprocessing needs to be explicitly enabled by setting the flag
    QSGNode::UsePreprocess. The flag needs to be set before the node is added
    to the scene graph and will cause the preprocess() function to be called
    for every frame the node is rendered.

    \warning Beware of deleting nodes while they are being preprocessed. It is
    possible, with a small performance hit, to delete a single node during its
    own preprocess call. Deleting a subtree which has nodes that also use
    preprocessing may result in a segmentation fault. This is done for
    performance reasons.
 */

static void preprocess(SG_Node *obj EINA_UNUSED)
{
}

/*!
    Returns whether this node and its subtree is available for use.

    Blocked subtrees will not get their dirty states updated and they
    will not be rendered.

    The QSGOpacityNode will return a blocked subtree when accumulated opacity
    is 0, for instance.
 */

Eina_Bool is_subtree_blocked(const SG_Node *obj EINA_UNUSED)
{
   return EINA_FALSE;
}

/*!
    \internal
    Detaches the node from the scene graph and deletes any children it owns.

    This function is called from SG_Node's and SG_Root_Node's destructor. It
    should not be called explicitly in user code. SG_Root_Node needs to call
    destroy() because destroy() calls remove_child_node() which in turn calls
    mark_dirty() which type-casts the node to SG_Root_Node. This type-cast is not
    valid at the time QSGNode's destructor is called because the node will
    already be partially destroyed at that point.
*/

void destroy(SG_Node *obj)
{
   if (obj->m_parent)
     {
        remove_child(obj->m_parent, obj);
     }
   while (obj->m_first_child)
     {
        SG_Node *child = obj->m_first_child;
        remove_child(obj, child);
        if (flags(child) & SG_NODE_OWNED_BY_PARENT)
          sg_node_destroy(child);
     }
}

/* SG_Node class End */

/* SG_Root class Start */
static SG_Node_Func Root_VTABLE = {NULL};

/*!
    Called to notify all renderers that \a node has been marked as dirty
    with \a flags.
 */

static void notify_node_change(SG_Root_Node *obj, SG_Node *node, SG_Node_Dirty_State state)
{
   if (obj->m_renderer)
     obj->m_renderer->fn->node_changed(obj->m_renderer, node, state);
}

/*!
    Deletes the root node.

    When a root node is deleted it removes itself from all of renderers
    that are referencing it.
 */
inline static void
sg_node_root_dtr(SG_Node *obj)
{
   SG_Root_Node *root = (SG_Root_Node *) obj;

   root->m_in_dtr = EINA_TRUE;
   if (root->m_renderer)
     root->m_renderer->fn->root_node_set(root->m_renderer, NULL);
   destroy(obj);
   sg_node_dtr(obj);
   // cleanup root node data here
}

static inline void
sg_node_root_ctr(SG_Root_Node *obj)
{
   sg_node_ctr((SG_Node *) obj);
   // setup the table only once
   if (!Root_VTABLE.dtr)
    {
       // copy the base table
       Root_VTABLE = *obj->fn;
       Root_VTABLE.base = obj->fn;
       // update the override functions
       Root_VTABLE.dtr = sg_node_root_dtr;
    }
   // update data
   obj->fn = &Root_VTABLE;
   obj->m_type = SG_NODE_TYPE_ROOT;
}

EAPI SG_Root_Node *
sg_node_root_create(void)
{
   SG_Root_Node* obj = (SG_Root_Node *) calloc(1, sizeof(SG_Root_Node));
   sg_node_root_ctr(obj);
   return obj;
}

/* SG_Root class End */

/* SG_Opacity_Node class Start */
const float OPACITY_THRESHOLD = 0.001;

static SG_Node_Func Opacity_VTABLE = {NULL};

/*!
    Sets the opacity of this node to \a opacity.

    Before rendering the graph, the renderer will do an update pass
    over the subtree to propagate the opacity to its children.

    The value will be bounded to the range 0 to 1.
 */
static void opacity_set(SG_Opacity_Node *obj, float opacity)
{
   SG_Node_Dirty_State dirty_state = SG_NODE_DIRTY_OPACITY;

   if (opacity == obj->m_opacity) return;

   if ((obj->m_opacity < OPACITY_THRESHOLD && obj->m_opacity >= OPACITY_THRESHOLD)     // blocked to unblocked
       || (obj->m_opacity >= OPACITY_THRESHOLD && obj->m_opacity < OPACITY_THRESHOLD)) // unblocked to blocked
     dirty_state |= SG_NODE_DIRTY_SUBTREE_BLOCKED;

   obj->m_opacity = opacity;

   mark_dirty((SG_Node *)obj, dirty_state);
}

inline static float opacity_get(const SG_Opacity_Node *obj)
{
   return obj->m_opacity;
}

/*!
   Returns this node's accumulated opacity.

   This value is calculated during rendering and only stored
   in the opacity node temporarily.

   \internal
 */
inline static float combined_opacity_get(const SG_Opacity_Node *obj)
{
   return obj->m_combined_opacity;
}

/*!
   Sets the combined opacity of this node to \a opacity.

   This function is meant to be called by the node preprocessing
   prior to rendering the tree, so it will not mark the tree as
   dirty.

   \internal
 */
void combined_opacity_set(SG_Opacity_Node *obj, float opacity)
{
   obj->m_combined_opacity = opacity;
}

inline static void
sg_node_opacity_dtr(SG_Node *obj)
{
   Opacity_VTABLE.base->dtr(obj);
   // cleanup opacity node data here
}

static inline void
sg_node_opacity_ctr(SG_Opacity_Node *obj)
{
   sg_node_ctr((SG_Node *) obj);
   // setup the table only once
   if (!Opacity_VTABLE.dtr)
    {
       // copy the base table
       Opacity_VTABLE = *obj->fn;
       Opacity_VTABLE.base = obj->fn;
       // update the override functions
       Opacity_VTABLE.dtr = sg_node_opacity_dtr;
       // update transform class specific functions
       Opacity_VTABLE.opacity_set = opacity_set;
       Opacity_VTABLE.opacity_get = opacity_get;
       Opacity_VTABLE.combined_opacity_set = combined_opacity_set;
       Opacity_VTABLE.combined_opacity_get = combined_opacity_get;
    }
   // update data
   obj->fn = &Opacity_VTABLE;
   obj->m_type = SG_NODE_TYPE_OPACITY;
   obj->m_opacity = 1;
   obj->m_combined_opacity = 1;
}

EAPI SG_Opacity_Node *
sg_node_opacity_create(void)
{
   SG_Opacity_Node* obj = (SG_Opacity_Node *) calloc(1, sizeof(SG_Opacity_Node));
   sg_node_opacity_ctr(obj);
   return obj;
}

/* SG_Opacity_Node class  End */

/* SG_Node class End */

/* SG_Transform_Node class Start */

static SG_Node_Func Transform_VTABLE = {NULL};

/*!
   Sets this transform node's matrix to \a matrix.
 */
static void t_matrix_set(SG_Transform_Node *obj, const Eina_Matrix3 *matrix)
{
   if (!matrix && !obj->m_matrix) return;

   if (!matrix)
     {
        free(obj->m_matrix);
        obj->m_matrix = NULL;
     }
   else
     {
        if (!obj->m_matrix) obj->m_matrix = malloc(sizeof(Eina_Matrix3));

        if (eina_matrix3_equal(obj->m_matrix, matrix))
          return;

        eina_matrix3_copy(obj->m_matrix, matrix);
     }
   mark_dirty((SG_Node *)obj, SG_NODE_DIRTY_MATRIX);
}

static const Eina_Matrix3 * t_matrix_get(const SG_Transform_Node *obj)
{
   return obj->m_matrix;
}

/*!
   \fn const QMatrix4x4 &QSGTransformNode::combinedMatrix() const

   Set during rendering to the combination of all parent matrices for
   that rendering pass.

   \internal
 */
static inline const Eina_Matrix3* t_combined_matrix_get(const SG_Transform_Node *obj)
{
   return obj->m_combined_matrix;
}

/*!
   Sets the combined matrix of this matrix to \a transform.

   This function is meant to be called by the node preprocessing
   prior to rendering the tree, so it will not mark the tree as
   dirty.

   \internal
  */
void t_combined_matrix_set(SG_Transform_Node *obj, const Eina_Matrix3 *matrix)
{
   if (!matrix && !obj->m_combined_matrix) return;

   if (!matrix)
     {
        free(obj->m_combined_matrix);
        obj->m_combined_matrix = NULL;
     }
   else
     {
        if (!obj->m_combined_matrix) obj->m_combined_matrix = malloc(sizeof(Eina_Matrix3));
        eina_matrix3_copy(obj->m_combined_matrix, matrix);
     }
}

inline static void
sg_node_transform_dtr(SG_Node *obj)
{
   SG_Transform_Node *t = (SG_Transform_Node *) obj;
   Transform_VTABLE.base->dtr(obj);
   // cleanup transform node data here
   if (t->m_matrix) free(t->m_matrix);
   if (t->m_combined_matrix) free (t->m_combined_matrix);
}

static inline void
sg_node_transform_ctr(SG_Transform_Node *obj)
{
   sg_node_ctr((SG_Node *) obj);
   // setup the table only once
   if (!Transform_VTABLE.dtr)
    {
       // copy the base table
       Transform_VTABLE = *obj->fn;
       Transform_VTABLE.base = obj->fn;
       // update the override functions
       Transform_VTABLE.dtr = sg_node_transform_dtr;
       // update transform class specific functions
       Transform_VTABLE.matrix_set = t_matrix_set;
       Transform_VTABLE.matrix_get = t_matrix_get;
       Transform_VTABLE.combined_matrix_set = t_combined_matrix_set;
       Transform_VTABLE.combined_matrix_get = t_combined_matrix_get;
    }
   // update data
   obj->fn = &Transform_VTABLE;
   obj->m_type = SG_NODE_TYPE_TRANSFORM;
}

EAPI SG_Transform_Node *
sg_node_transform_create(void)
{
   SG_Transform_Node* obj = (SG_Transform_Node *) calloc(1, sizeof(SG_Transform_Node));
   sg_node_transform_ctr(obj);
   return obj;
}

/* SG_Transform_Node class  End */

/* SG_Basic_Geometry_Node class  Start */

static SG_Node_Func Basic_Geometry_VTABLE = {NULL};

static inline void clipper_set(SG_Node *obj, SG_Node *clipper)
{
   SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;
   gm->m_clipper = clipper;
}

static inline const SG_Node*
clipper_get(const SG_Node *obj)
{
   const SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;

   return gm->m_clipper;
}

static inline void render_matrix_set(SG_Node *obj, const Eina_Matrix3 *matrix)
{
   SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;
   gm->m_matrix = matrix;
}

static inline const Eina_Matrix3*
render_matrix_get(const SG_Node *obj)
{
   const SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;
   return gm->m_matrix;
}
static inline void geometry_set(SG_Node *obj, SG_Geometry *geometry)
{
   SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;
   gm->m_geometry = geometry;
}

static inline const SG_Geometry*
geometry_get(const SG_Node *obj)
{
   SG_Basic_Geometry_Node *gm = (SG_Basic_Geometry_Node *)obj;
   return gm->m_geometry;
}

static inline void
sg_node_basic_geometry_dtr(SG_Node *obj)
{
   Basic_Geometry_VTABLE.base->dtr(obj);
   // cleanup basic geometry node data here
}

void
sg_node_basic_geometry_ctr(SG_Basic_Geometry_Node *obj)
{
   sg_node_ctr((SG_Node *) obj);
   // setup the table only once
   if (!Basic_Geometry_VTABLE.dtr)
    {
       // copy the base table
       Basic_Geometry_VTABLE = *obj->fn;
       Basic_Geometry_VTABLE.base = obj->fn;
       // update the override functions
       Basic_Geometry_VTABLE.dtr = sg_node_basic_geometry_dtr;
       Basic_Geometry_VTABLE.clipper_set = clipper_set;
       Basic_Geometry_VTABLE.clipper_get = clipper_get;
       Basic_Geometry_VTABLE.render_matrix_set = render_matrix_set;
       Basic_Geometry_VTABLE.render_matrix_get = render_matrix_get;
       Basic_Geometry_VTABLE.geometry_set = geometry_set;
       Basic_Geometry_VTABLE.geometry_get = geometry_get;
    }
   // update data
   obj->fn = &Basic_Geometry_VTABLE;
   obj->m_type = SG_NODE_TYPE_BASIC;
}

/* SG_Basic_Geometry_Node class  End */

/* SG_Clip_Node class  Start */

static SG_Node_Func Clip_VTABLE = {NULL};

/*!
    Sets whether this clip node has a rectangular clip to \a rectHint.

    This is an optimization hint which means that the renderer can
    use scissoring instead of stencil, which is significantly faster.

    When this hint is set and it is applicable, the clip region will be
    generated from clipRect() rather than geometry().

    By default this property is \c false.
 */
inline static void rect_hint_set(SG_Clip_Node *obj, Eina_Bool rect_hint)
{
   obj->m_is_rectangular = rect_hint;
}

inline static Eina_Bool rect_hint_get(const SG_Clip_Node *obj)
{
   return obj->m_is_rectangular;
}

/*!
    Sets the clip rect of this clip node to \a rect.

    When a rectangular clip is set in combination with setIsRectangular
    the renderer may in some cases use a more optimal clip method.
 */
inline static void clip_rect_set(SG_Clip_Node *obj, Eina_Rect rect)
{
   obj->m_clip_rect = rect;
}

inline static Eina_Rect clip_rect_get(const SG_Clip_Node *obj)
{
   return obj->m_clip_rect;
}

inline static void
sg_node_clip_dtr(SG_Node *obj)
{
   Clip_VTABLE.base->dtr(obj);
   // cleanup clip node data here
}

static inline void
sg_node_clip_ctr(SG_Clip_Node *obj)
{
   sg_node_basic_geometry_ctr((SG_Basic_Geometry_Node *) obj);
   // setup the table only once
   if (!Clip_VTABLE.dtr)
    {
       // copy the base table
       Clip_VTABLE = *obj->fn;
       Clip_VTABLE.base = obj->fn;
       // update the override functions
       Clip_VTABLE.dtr = sg_node_clip_dtr;
       Clip_VTABLE.rect_hint_set = rect_hint_set;
       Clip_VTABLE.rect_hint_get = rect_hint_get;
       Clip_VTABLE.clip_rect_set = clip_rect_set;
       Clip_VTABLE.clip_rect_get = clip_rect_get;
    }
   // update data
   obj->fn = &Clip_VTABLE;
   obj->m_type = SG_NODE_TYPE_CLIP;
}

EAPI SG_Clip_Node *
sg_node_clip_create(void)
{
   SG_Clip_Node* obj = (SG_Clip_Node *) calloc(1, sizeof(SG_Clip_Node));
   sg_node_clip_ctr(obj);

   return obj;
}

/* SG_Clip_Node class  End */

/* SG_Geometry_Node class  Start */

static SG_Node_Func Geometry_VTABLE = {NULL};

/*!
    Sets the inherited opacity of this geometry to \a opacity.

    This function is meant to be called by the node preprocessing
    prior to rendering the tree, so it will not mark the tree as
    dirty.

    \internal
  */

inline static void
inherited_opacity_set(SG_Node *base, float opacity)
{
   SG_Geometry_Node *obj = (SG_Geometry_Node *)base;
   obj->m_opacity = opacity;
}

inline static float
inherited_opacity_get(const SG_Node *base)
{
   const SG_Geometry_Node *obj = (SG_Geometry_Node *)base;
   return obj->m_opacity;
}

static inline void
sg_node_geometry_dtr(SG_Node *obj)
{
   Geometry_VTABLE.base->dtr(obj);
   // cleanup geometry node data here
}

void
sg_node_geometry_ctr(SG_Geometry_Node *obj)
{
   sg_node_basic_geometry_ctr((SG_Basic_Geometry_Node *) obj);
   // setup the table only once
   if (!Geometry_VTABLE.dtr)
    {
       // copy the base table
       Geometry_VTABLE = *obj->fn;
       Geometry_VTABLE.base = obj->fn;
       // update the override functions
       Geometry_VTABLE.dtr = sg_node_geometry_dtr;
       Geometry_VTABLE.inherited_opacity_get = inherited_opacity_get;
       Geometry_VTABLE.inherited_opacity_set = inherited_opacity_set;
    }
   // update data
   obj->fn = &Geometry_VTABLE;
   obj->m_type = SG_NODE_TYPE_GEOMETRY;
   obj->m_opacity = 1;
}

// ctr and dtr end

EAPI SG_Geometry_Node *
sg_node_geometry_create(void)
{
   SG_Geometry_Node* obj = (SG_Geometry_Node *) calloc(1, sizeof(SG_Geometry_Node));
   sg_node_geometry_ctr(obj);
   return obj;
}

/* SG_Geometry_Node class  end */