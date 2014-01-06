#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

static inline Evas_3D_Node_Mesh *
_node_mesh_new(Evas_3D_Node *node, Evas_3D_Mesh *mesh)
{
   Evas_3D_Node_Mesh *nm = (Evas_3D_Node_Mesh *)malloc(sizeof(Evas_3D_Node_Mesh));

   if (nm == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   nm->node = node;
   nm->mesh = mesh;
   nm->frame = 0;

   return nm;
}

static inline void
_node_mesh_free(Evas_3D_Node_Mesh *nm)
{
   free(nm);
}

static void
_node_mesh_free_func(void *data)
{
   _node_mesh_free((Evas_3D_Node_Mesh *)data);
}

static Eina_Bool
_node_scene_root_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                               void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Scene *s = *(Evas_3D_Scene **)key;
   evas_3d_object_change(&s->base, EVAS_3D_STATE_SCENE_ROOT_NODE, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static Eina_Bool
_node_scene_camera_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                 void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Scene *s = *(Evas_3D_Scene **)key;
   evas_3d_object_change(&s->base, EVAS_3D_STATE_SCENE_CAMERA_NODE, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_node_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED, Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Node *node = (Evas_3D_Node *)obj;
   Eina_List    *l;
   Evas_3D_Node *n;

   /* Notify all scenes using this node that it has changed. */
   if (node->scenes_root)
     eina_hash_foreach(node->scenes_root, _node_scene_root_change_notify, obj);

   if (node->scenes_camera)
     eina_hash_foreach(node->scenes_camera, _node_scene_camera_change_notify, obj);

   /* Notify parent that a member has changed. */
   if (node->parent)
     evas_3d_object_change(&node->parent->base, EVAS_3D_STATE_NODE_MEMBER, obj);

   /* Notify members that the parent has changed. */
   EINA_LIST_FOREACH(node->members, l, n)
     {
        evas_3d_object_change(&n->base, EVAS_3D_STATE_NODE_PARENT, obj);
     }
}

static Eina_Bool
_node_transform_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   if (evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_TRANSFORM) ||
       evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_PARENT))
     {
        if (node->parent)
          {
             const Evas_Vec3 *scale_parent = &node->parent->scale_world;
             const Evas_Vec4 *orientation_parent = &node->parent->orientation_world;

             /* Orienatation */
             if (node->orientation_inherit)
               {
                  evas_vec4_quaternion_multiply(&node->orientation_world,
                                                orientation_parent, &node->orientation);
               }
             else
               {
                  node->orientation_world = node->orientation;
               }

             /* Scale */
             if (node->scale_inherit)
               evas_vec3_multiply(&node->scale_world, scale_parent, &node->scale);
             else
               node->scale_world = node->scale;

             /* Position */
             if (node->position_inherit)
               {
                  evas_vec3_multiply(&node->position_world, &node->position, scale_parent);
                  evas_vec3_quaternion_rotate(&node->position_world, &node->position_world,
                                              orientation_parent);
                  evas_vec3_add(&node->position_world, &node->position_world,
                                &node->parent->position_world);
               }
             else
               {
                  node->position_world = node->position;
               }
          }
        else
          {
             node->position_world = node->position;
             node->orientation_world = node->orientation;
             node->scale_world = node->scale;
          }

        if (node->type == EVAS_3D_NODE_TYPE_CAMERA)
          {
             evas_mat4_inverse_build(&node->data.camera.matrix_world_to_eye,
                                     &node->position_world, &node->orientation_world,
                                     &node->scale_world);
          }
        else if (node->type == EVAS_3D_NODE_TYPE_LIGHT)
          {
          }
        else if (node->type == EVAS_3D_NODE_TYPE_MESH)
          {
             evas_mat4_build(&node->data.mesh.matrix_local_to_world,
                             &node->position_world, &node->orientation_world, &node->scale_world);
          }
/*
        if (node->parent)
          {
             evas_mat4_nocheck_multiply(&node->matrix_local_to_world,
                                        &node->parent->matrix_local_to_world,
                                        &node->matrix_local_to_parent);
          }
        else
          {
             evas_mat4_copy(&node->matrix_local_to_world, &node->matrix_local_to_parent);
          }*/
     }

   return EINA_TRUE;
}

static Eina_Bool
_node_item_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   if (node->type == EVAS_3D_NODE_TYPE_CAMERA)
     {
        if (node->data.camera.camera)
          evas_3d_object_update(&node->data.camera.camera->base);
     }
   else if (node->type == EVAS_3D_NODE_TYPE_LIGHT)
     {
        if (node->data.light.light)
          evas_3d_object_update(&node->data.light.light->base);
     }
   else if (node->type == EVAS_3D_NODE_TYPE_MESH)
     {
        Eina_List *l;
        Evas_3D_Mesh *m;

        EINA_LIST_FOREACH(node->data.mesh.meshes, l, m)
          {
             evas_3d_object_update(&m->base);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_node_aabb_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   if (evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_TRANSFORM) ||
       evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_MESH_GEOMETRY) ||
       evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_MESH_FRAME) ||
       evas_3d_object_dirty_get(&node->base, EVAS_3D_STATE_NODE_MEMBER))
     {
        Eina_List *l;
        Evas_3D_Node *n;

        /* Update AABB of this node. */
        evas_box3_empty_set(&node->aabb);

        EINA_LIST_FOREACH(node->members, l, n)
          {
             evas_box3_union(&node->aabb, &node->aabb, &n->aabb);
          }

        if (node->type == EVAS_3D_NODE_TYPE_MESH)
          {
             /* TODO: */
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_node_update_done(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   evas_3d_object_update_done(&node->base);
   return EINA_TRUE;
}

static void
_node_update(Evas_3D_Object *obj)
{
   Evas_3D_Node *node = (Evas_3D_Node *)obj;

   /* Update transform. */
   evas_3d_node_tree_traverse(node, EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_FALSE,
                              _node_transform_update, NULL);

   /* Update AABB. */
   evas_3d_node_tree_traverse(node, EVAS_3D_TREE_TRAVERSE_POST_ORDER, EINA_FALSE,
                              _node_aabb_update, NULL);

   /* Update node item. */
   evas_3d_node_tree_traverse(node, EVAS_3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_item_update, NULL);

   /* Mark all nodes in the tree as up-to-date. */
   evas_3d_node_tree_traverse(node, EVAS_3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_update_done, NULL);
}

static void
_node_free(Evas_3D_Object *obj)
{
   Evas_3D_Node *node = (Evas_3D_Node *)obj;

   if (node->members)
     {
        Eina_List *l;
        Evas_3D_Node *n;

        EINA_LIST_FOREACH(node->members, l, n)
          {
             evas_3d_object_unreference(&n->base);
          }

        eina_list_free(node->members);
     }

   if (node->data.mesh.meshes)
     {
        Eina_List *l;
        Evas_3D_Mesh *m;

        EINA_LIST_FOREACH(node->data.mesh.meshes, l, m)
          {
             evas_3d_mesh_node_del(m, node);
             evas_3d_object_unreference(&m->base);
          }

        eina_list_free(node->data.mesh.meshes);
     }

   if (node->data.mesh.node_meshes)
     eina_hash_free(node->data.mesh.node_meshes);

   if (node->scenes_root)
     eina_hash_free(node->scenes_root);

   if (node->scenes_camera)
     eina_hash_free(node->scenes_camera);

   free(node);
}

static const Evas_3D_Object_Func node_func =
{
   _node_free,
   _node_change,
   _node_update,
};

void
evas_3d_node_scene_root_add(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;

   if (node->scenes_root == NULL)
     {
        node->scenes_root = eina_hash_pointer_new(NULL);

        if (node->scenes_root == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(node->scenes_root, &scene);

   eina_hash_set(node->scenes_root, &scene, (const void *)(count + 1));
}

void
evas_3d_node_scene_root_del(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;

   if (node->scenes_root == NULL)
     {
        ERR("No scene to delete.");
        return;
     }

   count = (int)eina_hash_find(node->scenes_root, &scene);

   if (count == 1)
     eina_hash_del(node->scenes_root, &scene, NULL);
   else
     eina_hash_set(node->scenes_root, &scene, (const void *)(count - 1));
}

void
evas_3d_node_scene_camera_add(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;

   if (node->scenes_camera == NULL)
     {
        node->scenes_camera = eina_hash_pointer_new(NULL);

        if (node->scenes_camera == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(node->scenes_camera, &scene);

   eina_hash_set(node->scenes_camera, &scene, (const void *)(count + 1));
}

void
evas_3d_node_scene_camera_del(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;

   if (node->scenes_camera == NULL)
     {
        ERR("No scene to delete.");
        return;
     }

   count = (int)eina_hash_find(node->scenes_camera, &scene);

   if (count == 1)
     eina_hash_del(node->scenes_camera, &scene, NULL);
   else
     eina_hash_set(node->scenes_camera, &scene, (const void *)(count - 1));
}

Evas_3D_Node *
evas_3d_node_new(Evas *e, Evas_3D_Node_Type type)
{
   Evas_3D_Node *node = NULL;

   node = (Evas_3D_Node *)calloc(1, sizeof(Evas_3D_Node));

   if (node == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&node->base, e, EVAS_3D_OBJECT_TYPE_NODE, &node_func);

   evas_vec3_set(&node->position, 0.0, 0.0, 0.0);
   evas_vec4_set(&node->orientation, 0.0, 0.0, 0.0, 0.0);
   evas_vec3_set(&node->scale, 1.0, 1.0, 1.0);

   evas_vec3_set(&node->position_world, 0.0, 0.0, 0.0);
   evas_vec4_set(&node->orientation_world, 0.0, 0.0, 0.0, 1.0);
   evas_vec3_set(&node->scale_world, 1.0, 1.0, 1.0);

   node->position_inherit = EINA_TRUE;
   node->orientation_inherit = EINA_TRUE;
   node->scale_inherit = EINA_TRUE;

   evas_box3_set(&node->aabb, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

   node->type = type;

   if (type == EVAS_3D_NODE_TYPE_MESH)
     {
        node->data.mesh.node_meshes = eina_hash_pointer_new(_node_mesh_free_func);

        if (node->data.mesh.node_meshes == NULL)
          {
             ERR("Failed to create node mesh table.");
             _node_free(&node->base);
             return NULL;
          }
     }

   return node;
}

void
evas_3d_node_traverse(Evas_3D_Node *from, Evas_3D_Node *to, Evas_3D_Node_Traverse_Type type,
                      Eina_Bool skip, Evas_3D_Node_Func func, void *data)
{
   Eina_List *nodes = NULL, *n;
   Evas_3D_Node *node = NULL;

   if (from == NULL || func == NULL)
     goto error;

   if (type == EVAS_3D_NODE_TRAVERSE_DOWNWARD)
     {
        if (to == NULL)
          goto error;

        node = to;

        do {
             nodes = eina_list_prepend(nodes, (const void *)node);

             if (node == from)
               break;

             node = node->parent;

             if (node == NULL)
               goto error;
        } while (1);
     }
   else if (type == EVAS_3D_NODE_TRAVERSE_UPWARD)
     {
        node = from;

        do {
             nodes = eina_list_append(nodes, (const void *)node);

             if (node == to)
               break;

             node = node->parent;

             if (node == NULL)
               {
                  if (to == NULL)
                    break;

                  goto error;
               }
        } while (1);
     }

   EINA_LIST_FOREACH(nodes, n, node)
     {
        if (!func(node, data) && skip)
          break;
     }

   eina_list_free(nodes);
   return;

error:
   ERR("Node traverse error.");

   if (nodes)
     eina_list_free(nodes);
}

void
evas_3d_node_tree_traverse(Evas_3D_Node *root, Evas_3D_Tree_Traverse_Type type,
                           Eina_Bool skip, Evas_3D_Node_Func func, void *data)
{
   Eina_List *nodes = NULL, *l;
   Evas_3D_Node *node = NULL, *n, *last;

   if (root == NULL || func == NULL)
     return;

   if (type == EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER)
     {
        /* Put the root node in the queue. */
        nodes = eina_list_append(nodes, root);

        while (eina_list_count(nodes) > 0)
          {
             /* Dequeue a node. */
             node = eina_list_data_get(nodes);
             nodes = eina_list_remove_list(nodes, nodes);

             /* Call node function on the node. */
             if (func(node, data) || !skip)
               {
                  /* Enqueue member nodes. */
                  EINA_LIST_FOREACH(node->members, l, n)
                    {
                       nodes = eina_list_append(nodes, n);
                    }
               }
          }
     }
   else if (type == EVAS_3D_TREE_TRAVERSE_PRE_ORDER)
     {
        /* Put the root node in the stack. */
        nodes = eina_list_append(nodes, root);

        while (eina_list_count(nodes) > 0)
          {
             /* Pop a node from the stack. */
             node = eina_list_data_get(nodes);
             nodes = eina_list_remove_list(nodes, nodes);

             /* Call node function on the node. */
             if (func(node, data) || !skip)
               {
                  /* Push member nodes into the stack. */
                  EINA_LIST_REVERSE_FOREACH(node->members, l, n)
                    {
                       nodes = eina_list_prepend(nodes, n);
                    }
               }
          }
     }
   else if (type == EVAS_3D_TREE_TRAVERSE_POST_ORDER)
     {
        if (skip)
          {
             ERR("Using skip with post order traversal has no effect.");
             return;
          }

        /* Put the root node in the stack. */
        nodes = eina_list_append(nodes, root);
        last = NULL;

        while (eina_list_count(nodes) > 0)
          {
             /* Peek a node from the stack. */
             node = eina_list_data_get(nodes);

             if (eina_list_count(node->members) == 0)
               {
                  /* The peeked node is a leaf node,
                   * so visit it and pop from the stack. */
                  func(node, data);
                  nodes = eina_list_remove_list(nodes, nodes);

                  /* Save the last visited node. */
                  last = node;
               }
             else
               {
                  /* If the peeked node is not a leaf node,
                   * there can be only two possible cases.
                   *
                   * 1. the parent of the last visited node.
                   * 2. a sibling of the last visited node.
                   *
                   * If the last visited node is a direct child of the peeked node,
                   * we have no unvisted child nodes for the peeked node, so we have to visit
                   * the peeked node and pop from the stack.
                   *
                   * Otherwise it should be a sibling of the peeked node, so we have to push
                   * its childs into the stack. */

                  if (last && last->parent == node)
                    {
                       /* Visit the node as it doesn't have any unvisited child node. */
                       func(node, data);
                       nodes = eina_list_remove_list(nodes, nodes);

                       /* Save the last visited node. */
                       last = node;
                    }
                  else
                    {
                       /* Push child nodes into the stack. */
                       EINA_LIST_REVERSE_FOREACH(node->members, l, n)
                         {
                            nodes = eina_list_prepend(nodes, n);
                         }
                    }
               }
          }
     }

   if (nodes != NULL)
     eina_list_free(nodes);
}

Eina_Bool
_node_is_visible(Evas_3D_Node *node EINA_UNUSED, Evas_3D_Node *camera_node EINA_UNUSED)
{
   /* TODO: */
   return EINA_TRUE;
}

Eina_Bool
evas_3d_node_mesh_collect(Evas_3D_Node *node, void *data)
{
   Evas_3D_Scene_Data *scene_data = (Evas_3D_Scene_Data *)data;

   if (!_node_is_visible(node, scene_data->camera_node))
     {
        /* Skip entire sub-tree of this node. */
        return EINA_FALSE;
     }

   if (node->type == EVAS_3D_NODE_TYPE_MESH)
     scene_data->mesh_nodes = eina_list_append(scene_data->mesh_nodes, node);

   return EINA_TRUE;
}

Eina_Bool
evas_3d_node_light_collect(Evas_3D_Node *node, void *data)
{
   Evas_3D_Scene_Data *scene_data = (Evas_3D_Scene_Data *)data;

   if (node->type == EVAS_3D_NODE_TYPE_LIGHT)
     scene_data->light_nodes = eina_list_append(scene_data->light_nodes, node);

   return EINA_TRUE;
}

EAPI Evas_3D_Node *
evas_3d_node_add(Evas *e, Evas_3D_Node_Type type)
{
   return evas_3d_node_new(e, type);
}

EAPI void
evas_3d_node_del(Evas_3D_Node *node)
{
   evas_3d_object_unreference(&node->base);
}

EAPI Evas_3D_Node_Type
evas_3d_node_type_get(const Evas_3D_Node *node)
{
   return node->type;
}

EAPI Evas *
evas_3d_node_evas_get(const Evas_3D_Node *node)
{
   return node->base.evas;
}

EAPI void
evas_3d_node_member_add(Evas_3D_Node *node, Evas_3D_Node *member)
{
   if (node == member)
     {
        ERR("Failed to add a member node (adding to itself).");
        return;
     }

   if (member->parent == node)
     return;

   if (member->parent)
     {
        /* Detaching from previous parent. */
        member->parent->members = eina_list_remove(member->parent->members, member);

        /* Mark changed. */
        evas_3d_object_change(&member->parent->base, EVAS_3D_STATE_NODE_MEMBER, NULL);
     }
   else
     {
        /* Should get a new reference. */
        evas_3d_object_reference(&member->base);
     }

   /* Add the member node. */
   node->members = eina_list_append(node->members, (const void *)member);
   member->parent = node;

   /* Mark changed. */
   evas_3d_object_change(&member->base, EVAS_3D_STATE_NODE_PARENT, NULL);
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MEMBER, NULL);
}

EAPI void
evas_3d_node_member_del(Evas_3D_Node *node, Evas_3D_Node *member)
{
   if (member->parent != node)
     {
        ERR("Failed to delete a member node (not a member of the given node)");
        return;
     }

   /* Delete the member node. */
   node->members = eina_list_remove(node->members, member);
   member->parent = NULL;

   /* Mark modified object as changed. */
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MEMBER, NULL);
   evas_3d_object_change(&member->base, EVAS_3D_STATE_NODE_PARENT, NULL);

   /* Decrease reference count. */
   evas_3d_object_unreference(&member->base);
}

EAPI Evas_3D_Node *
evas_3d_node_parent_get(const Evas_3D_Node *node)
{
   return node->parent;
}

EAPI const Eina_List *
evas_3d_node_member_list_get(const Evas_3D_Node *node)
{
   return node->members;
}

EAPI void
evas_3d_node_position_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z)
{
   node->position.x = x;
   node->position.y = y;
   node->position.z = z;

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_orientation_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w)
{
   node->orientation.x = x;
   node->orientation.y = y;
   node->orientation.z = z;
   node->orientation.w = w;

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_orientation_angle_axis_set(Evas_3D_Node *node,
                                        Evas_Real angle, Evas_Real x, Evas_Real y, Evas_Real z)
{
   Evas_Real half_angle = 0.5 * DEGREE_TO_RADIAN(angle);
   Evas_Real s = sin(half_angle);
   Evas_Vec3 axis;

   evas_vec3_set(&axis, x, y, z);
   evas_vec3_normalize(&axis, &axis);

   node->orientation.w = cos(half_angle);
   node->orientation.x = s * axis.x;
   node->orientation.y = s * axis.y;
   node->orientation.z = s * axis.z;

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_scale_set(Evas_3D_Node *node, Evas_Real x, Evas_Real y, Evas_Real z)
{
   node->scale.x = x;
   node->scale.y = y;
   node->scale.z = z;

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_position_get(const Evas_3D_Node *node, Evas_3D_Space space,
                          Evas_Real *x, Evas_Real *y, Evas_Real *z)
{
   if (space == EVAS_3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
     }
   else if (space == EVAS_3D_SPACE_PARENT)
     {
        if (x) *x = node->position.x;
        if (y) *y = node->position.y;
        if (z) *z = node->position.z;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        evas_3d_object_update((Evas_3D_Object *)&node->base);

        if (x) *x = node->position_world.x;
        if (y) *y = node->position_world.y;
        if (z) *z = node->position_world.z;
     }
}

EAPI void
evas_3d_node_orientation_get(const Evas_3D_Node *node, Evas_3D_Space space,
                             Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *w)
{
   if (space == EVAS_3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
        if (w) *w = 0.0;
     }
   else if (space == EVAS_3D_SPACE_PARENT)
     {
        if (x) *x = node->orientation.x;
        if (y) *y = node->orientation.y;
        if (z) *z = node->orientation.z;
        if (w) *w = node->orientation.w;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        evas_3d_object_update((Evas_3D_Object *)&node->base);

        if (x) *x = node->orientation_world.x;
        if (y) *y = node->orientation_world.y;
        if (z) *z = node->orientation_world.z;
        if (w) *w = node->orientation_world.w;
     }

}

EAPI void
evas_3d_node_scale_get(const Evas_3D_Node *node, Evas_3D_Space space,
                       Evas_Real *x, Evas_Real *y, Evas_Real *z)
{
   if (space == EVAS_3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
     }
   else if (space == EVAS_3D_SPACE_PARENT)
     {
        if (x) *x = node->scale.x;
        if (y) *y = node->scale.y;
        if (z) *z = node->scale.z;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        evas_3d_object_update((Evas_3D_Object *)&node->base);

        if (x) *x = node->scale_world.x;
        if (y) *y = node->scale_world.y;
        if (z) *z = node->scale_world.z;
     }
}

EAPI void
evas_3d_node_position_inherit_set(Evas_3D_Node *node, Eina_Bool inherit)
{
   node->position_inherit = inherit;
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_orientation_inherit_set(Evas_3D_Node *node, Eina_Bool inherit)
{
   node->orientation_inherit = inherit;
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_scale_inherit_set(Evas_3D_Node *node, Eina_Bool inherit)
{
   node->scale_inherit = inherit;
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI Eina_Bool
evas_3d_node_position_inherit_get(const Evas_3D_Node *node)
{
   return node->position_inherit;
}

EAPI Eina_Bool
evas_3d_node_orientation_inherit_get(const Evas_3D_Node *node)
{
   return node->orientation_inherit;
}

EAPI Eina_Bool
evas_3d_node_scale_inherit_get(const Evas_3D_Node *node)
{
   return node->scale_inherit;
}

EAPI void
evas_3d_node_look_at_set(Evas_3D_Node *node,
                         Evas_3D_Space target_space, Evas_Real tx, Evas_Real ty, Evas_Real tz,
                         Evas_3D_Space up_space, Evas_Real ux, Evas_Real uy, Evas_Real uz)
{
   Evas_Vec3   target;
   Evas_Vec3   up;
   Evas_Vec3   x, y, z;

   /* Target position in parent space. */
   if (target_space == EVAS_3D_SPACE_LOCAL)
     {
        ERR("TODO:");
        return;
     }
   else if (target_space == EVAS_3D_SPACE_PARENT)
     {
        evas_vec3_set(&target, tx, ty, tz);
     }
   else if (target_space == EVAS_3D_SPACE_WORLD)
     {
        ERR("TODO:");
        return;
     }
   else
     {
        ERR("Invalid coordinate space.");
        return;
     }

   if (up_space == EVAS_3D_SPACE_LOCAL)
     {
        ERR("TODO:");
        return;
     }
   else if (up_space == EVAS_3D_SPACE_PARENT)
     {
        evas_vec3_set(&up, ux, uy, uz);
     }
   else if (up_space == EVAS_3D_SPACE_WORLD)
     {
        ERR("TODO:");
        return;
     }
   else
     {
        ERR("Invalid coordinate space.");
        return;
     }

   /* From now on, everything takes place in parent space. */
   evas_vec3_subtract(&z, &node->position, &target);
   evas_vec3_normalize(&z, &z);

   evas_vec3_cross_product(&x, &up, &z);
   evas_vec3_normalize(&x, &x);

   evas_vec3_cross_product(&y, &z, &x);
   evas_vec3_normalize(&y, &y);

   /* Below matrix to quaternion conversion code taken from
    * http://fabiensanglard.net/doom3_documentation/37726-293748.pdf
    * When any license issue occurs, use ken shoemake's algorithm instead.
    */

   if (x.x + y.y + z.z > 0.0)
     {
        Evas_Real t = x.x + y.y + z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        node->orientation.w = s * t;
        node->orientation.z = (x.y - y.x) * s;
        node->orientation.y = (z.x - x.z) * s;
        node->orientation.x = (y.z - z.y) * s;
     }
   else if (x.x > y.y && x.x > z.z)
     {
        Evas_Real t = x.x - y.y - z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        node->orientation.x = s * t;
        node->orientation.y = (x.y + y.x) * s;
        node->orientation.z = (z.x + x.z) * s;
        node->orientation.w = (y.z - z.y) * s;
     }
   else if (y.y > z.z)
     {
        Evas_Real t = -x.x + y.y - z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        node->orientation.y = s * t;
        node->orientation.x = (x.y + y.x) * s;
        node->orientation.w = (z.x - x.z) * s;
        node->orientation.z = (y.z + z.y) * s;
     }
   else
     {
        Evas_Real t = -x.x - y.y + z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        node->orientation.z = s * t;
        node->orientation.w = (x.y - y.x) * s;
        node->orientation.x = (z.x + x.z) * s;
        node->orientation.y = (y.z + z.y) * s;
     }

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_TRANSFORM, NULL);
}

EAPI void
evas_3d_node_camera_set(Evas_3D_Node *node, Evas_3D_Camera *camera)
{
   if (node->type != EVAS_3D_NODE_TYPE_CAMERA)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (node->data.camera.camera == camera)
     return;

   if (node->data.camera.camera)
     {
        /* Detach previous camera object. */
        evas_3d_camera_node_del(node->data.camera.camera, node);
        evas_3d_object_unreference(&node->data.camera.camera->base);
     }

   node->data.camera.camera = camera;
   evas_3d_object_reference(&camera->base);

   /* Register change notification on the camera for this node. */
   evas_3d_camera_node_add(camera, node);

   /* Mark changed. */
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_CAMERA, NULL);
}

EAPI Evas_3D_Camera *
evas_3d_node_camera_get(const Evas_3D_Node *node)
{
   return node->data.camera.camera;
}

EAPI void
evas_3d_node_light_set(Evas_3D_Node *node, Evas_3D_Light *light)
{
   if (node->type != EVAS_3D_NODE_TYPE_LIGHT)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (node->data.light.light == light)
     return;

   if (node->data.light.light)
     {
        /* Detach previous light object. */
        evas_3d_light_node_del(node->data.light.light, node);
        evas_3d_object_unreference(&node->data.light.light->base);
     }

   node->data.light.light = light;
   evas_3d_object_reference(&light->base);

   /* Register change notification on the light for this node. */
   evas_3d_light_node_add(light, node);

   /* Mark changed. */
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_LIGHT, NULL);
}

EAPI Evas_3D_Light *
evas_3d_node_light_get(const Evas_3D_Node *node)
{
   return node->data.light.light;
}

EAPI void
evas_3d_node_mesh_add(Evas_3D_Node *node, Evas_3D_Mesh *mesh)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (node->type != EVAS_3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (eina_hash_find(node->data.mesh.node_meshes, mesh) != NULL)
     {
        ERR("The mesh is already added to the node.");
        return;
     }

   if ((nm = _node_mesh_new(node, mesh)) == NULL)
     {
        ERR("Failed to create node mesh.");
        return;
     }

   /* TODO: Find node mesh and add if it does not exist. */
   if (!eina_hash_add(node->data.mesh.node_meshes, mesh, nm))
     {
        ERR("Failed to add a mesh to mesh table.");
        _node_mesh_free(nm);
        return;
     }

   node->data.mesh.meshes = eina_list_append(node->data.mesh.meshes, mesh);
   evas_3d_object_reference(&mesh->base);

   /* Register change notification. */
   evas_3d_mesh_node_add(mesh, node);

   /* Mark changed. */
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MESH_GEOMETRY, NULL);
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MESH_MATERIAL, NULL);
}

EAPI void
evas_3d_node_mesh_del(Evas_3D_Node *node, Evas_3D_Mesh *mesh)
{
   if (node->type != EVAS_3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (!eina_hash_del(node->data.mesh.node_meshes, mesh, NULL))
     {
        ERR("The given mesh doesn't belong to this node.");
        return;
     }

   node->data.mesh.meshes = eina_list_remove(node->data.mesh.meshes, mesh);
   evas_3d_mesh_node_del(mesh, node);
   evas_3d_object_unreference(&mesh->base);

   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MESH_GEOMETRY, NULL);
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MESH_MATERIAL, NULL);
}

EAPI const Eina_List *
evas_3d_node_mesh_list_get(const Evas_3D_Node *node)
{
   return node->data.mesh.meshes;
}

EAPI void
evas_3d_node_mesh_frame_set(Evas_3D_Node *node, Evas_3D_Mesh *mesh, int frame)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (node->type != EVAS_3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if ((nm = eina_hash_find(node->data.mesh.node_meshes, mesh)) == NULL)
     {
        ERR("The given mesh doesn't belongs to this node.");
        return;
     }

   nm->frame = frame;
   evas_3d_object_change(&node->base, EVAS_3D_STATE_NODE_MESH_FRAME, NULL);
}

EAPI int
evas_3d_node_mesh_frame_get(const Evas_3D_Node *node, Evas_3D_Mesh *mesh)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (node->type != EVAS_3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return 0;
     }

   if ((nm = eina_hash_find(node->data.mesh.node_meshes, mesh)) == NULL)
     {
        ERR("The given mesh doesn't belongs to this node.");
        return 0;
     }

   return nm->frame;
}
