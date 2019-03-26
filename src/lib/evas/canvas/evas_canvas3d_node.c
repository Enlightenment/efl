#include "eo_internal.h"
#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_canvas3d_node_callback.h"

#define MY_CLASS EVAS_CANVAS3D_NODE_CLASS
#define MY_CLASS_NAME "Evas_Canvas3D_Node"

Evas_Canvas3D_Mesh_Frame *evas_canvas3d_mesh_frame_find(Evas_Canvas3D_Mesh_Data *pd, int frame);

static void
_look_at_set(Evas_Canvas3D_Node_Data *pd, Eina_Vector3 *target, Eina_Vector3 *up);

static Eina_Stringshare *
_generate_unic_color_key(Evas_Color *color, Evas_Color *bg_color, Evas_Canvas3D_Node *node, Evas_Canvas3D_Mesh *mesh,
                         Eina_Bool init)
{
   static unsigned char red = 0;
   static unsigned char green = 0;
   static unsigned char blue = 0;
   if (init) red = green = blue = 0;

#define GET_NEXT_COLOR    \
   red++;                 \
   if (red == 255)        \
     {                    \
        red = 0;          \
        green++;          \
        if (green == 255) \
         {                \
            green = 0;    \
            blue++;       \
         }                \
     }

   GET_NEXT_COLOR
   /*Get another color if color equal with background color*/
   if ((EINA_DBL_EQ(bg_color->r, (double)red)) &&
       (EINA_DBL_EQ(bg_color->g, (double)green)) &&
       (EINA_DBL_EQ(bg_color->b, (double)blue)))
     {
        GET_NEXT_COLOR
     }

   color->r = (double)red / 255;
   color->g = (double)green / 255;
   color->b = (double)blue / 255;

#undef GET_NEXT_COLOR

   return eina_stringshare_printf("%p %p", node, mesh);
}

static void
_evas_canvas3d_node_private_callback_collision(void *data, const Efl_Event *event)
{
   Eina_List *collision_list = NULL, *l = NULL;
   Evas_Canvas3D_Node *target_node = NULL, *n = NULL;
   Evas_Canvas3D_Node_Data *pd_target = NULL, *pd = NULL;
   const Efl_Event_Description *eo_desc = NULL;
   Eina_Bool ret = EINA_FALSE;

   target_node = (Evas_Canvas3D_Node *)event->info;
   pd_target = efl_data_scope_get(target_node, EVAS_CANVAS3D_NODE_CLASS);
   collision_list = (Eina_List *)data;
   eo_desc = efl_object_legacy_only_event_description_get("collision");

   if (collision_list)
     {
        EINA_LIST_FOREACH(collision_list, l, n)
          {
             pd = efl_data_scope_get(n, EVAS_CANVAS3D_NODE_CLASS);
             if (box_intersection_box(&pd_target->aabb, &pd->aabb))
               ret = efl_event_callback_legacy_call(target_node, eo_desc, n);
          }
        if (!ret)
          {
             /* XXX: Putting it like this because that's how the logic was,
              * but it seems absolutely wrong that it only checks the last
              * and decides based on that. */
             efl_event_callback_stop(event->object);
          }
     }
}
static void
_evas_canvas3d_node_private_callback_clicked(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Bool ret = EINA_FALSE;
   const Efl_Event_Description *eo_desc = efl_object_legacy_only_event_description_get("clicked");
   ret = efl_event_callback_legacy_call((Eo *)event->info, eo_desc, event->info);

   if (!ret)
      efl_event_callback_stop(event->object);
}

static inline Evas_Canvas3D_Node_Mesh *
_node_mesh_new(Evas_Canvas3D_Node *node, Evas_Canvas3D_Mesh *mesh)
{
   Evas_Canvas3D_Node_Mesh *nm = (Evas_Canvas3D_Node_Mesh *)malloc(sizeof(Evas_Canvas3D_Node_Mesh));

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
_node_mesh_free(Evas_Canvas3D_Node_Mesh *nm)
{
   free(nm);
}

static void
_node_mesh_free_func(void *data)
{
   _node_mesh_free((Evas_Canvas3D_Node_Mesh *)data);
}

static Eina_Bool
_node_scene_root_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                               void *data EINA_UNUSED, void *fdata)
{
   Evas_Canvas3D_Scene *s = *(Evas_Canvas3D_Scene **)key;
   evas_canvas3d_object_change(s, EVAS_CANVAS3D_STATE_SCENE_ROOT_NODE, (Evas_Canvas3D_Object *)fdata);
   return EINA_TRUE;
}

static Eina_Bool
_node_scene_camera_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                 void *data EINA_UNUSED, void *fdata)
{
   Evas_Canvas3D_Scene *s = *(Evas_Canvas3D_Scene **)key;
   evas_canvas3d_object_change(s, EVAS_CANVAS3D_STATE_SCENE_CAMERA_NODE, (Evas_Canvas3D_Object *)fdata);
   return EINA_TRUE;
}

EOLIAN static void
_evas_canvas3d_node_evas_canvas3d_object_change_notify(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_State state EINA_UNUSED , Evas_Canvas3D_Object *ref EINA_UNUSED)
{
   Eina_List    *l;
   Evas_Canvas3D_Node *n;
   Eina_Bool orientation;
   Eina_Bool position;
   Eina_Bool scale;
   Eina_Bool parent_change;

   /* Notify all scenes using this node that it has changed. */
   if (pd->scenes_root)
     eina_hash_foreach(pd->scenes_root, _node_scene_root_change_notify, obj);

   if (pd->scenes_camera)
     eina_hash_foreach(pd->scenes_camera, _node_scene_camera_change_notify, obj);

   parent_change = (state == EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION)
                     || (state == EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION)
                     || (state == EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE);

   /* Notify parent that a member has changed. */
   if (pd->parent && !parent_change)
    {
       evas_canvas3d_object_change(pd->parent, EVAS_CANVAS3D_STATE_NODE_MEMBER, obj);
    }

   orientation = (state == EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION)
                   || (state == EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION);
   position = (state == EVAS_CANVAS3D_STATE_NODE_TRANSFORM_POSITION)
                || (state == EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION);
   scale = (state == EVAS_CANVAS3D_STATE_NODE_TRANSFORM_SCALE)
             || (state == EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE);

   /* Notify members that the parent has changed. */
   if (scale)
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          evas_canvas3d_object_change(n, EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE, obj);
       }
   if (orientation && !(pd->billboard_target))
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          /*Skip change orientation if node is billboard*/
          Evas_Canvas3D_Node_Data *pdm = efl_data_scope_get(n, EVAS_CANVAS3D_NODE_CLASS);
          if (pdm->billboard_target)
            continue;
          evas_canvas3d_object_change(n, EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION, obj);
       }
   if (position)
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          evas_canvas3d_object_change(n, EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION, obj);
       }
}

EOLIAN static void
_evas_canvas3d_node_evas_canvas3d_object_callback_register(Eo *obj, Evas_Canvas3D_Node_Data *pd EINA_UNUSED,
                                               const char *event, const void *data)
{
   Evas_Canvas3D_Node_Private_Callback_Type tcb;

   GET_CALLBACK_TYPE(tcb, event)

   if (tcb != PRIVATE_CALLBACK_NONE)
     efl_event_callback_add(obj, &evas_canvas3d_node_private_event_desc[tcb], evas_canvas3d_node_private_callback_functions[tcb], data);

}

EOLIAN static void
_evas_canvas3d_node_evas_canvas3d_object_callback_unregister(Eo *obj, Evas_Canvas3D_Node_Data *pd EINA_UNUSED,
                                                 const char *event)
{
   Evas_Canvas3D_Node_Private_Callback_Type tcb;

   GET_CALLBACK_TYPE(tcb, event)

   if (tcb != PRIVATE_CALLBACK_NONE)
     efl_event_callback_del(obj, &evas_canvas3d_node_private_event_desc[tcb], evas_canvas3d_node_private_callback_functions[tcb], NULL);
}

static Eina_Bool
_node_transform_update(Evas_Canvas3D_Node *node, void *data EINA_UNUSED)
{
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   Eina_Bool transform_dirty = EINA_FALSE, parent_dirty = EINA_FALSE;

   transform_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION);
   transform_dirty|= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_POSITION);
   transform_dirty|= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_SCALE);
   parent_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION);
   parent_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION);
   parent_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE);

   if (transform_dirty || parent_dirty)
     {
        if (pd->parent)
          {
             Evas_Canvas3D_Node_Data *pdparent = efl_data_scope_get(pd->parent, MY_CLASS);
             const Eina_Vector3 *scale_parent = &pdparent->scale_world;
             const Eina_Quaternion *orientation_parent = &pdparent->orientation_world;

             /* Orienatation */
             if (pd->orientation_inherit)
               {
                  eina_quaternion_mul(&pd->orientation_world,
                                      orientation_parent, &pd->orientation);
               }
             else
               {
                  pd->orientation_world = pd->orientation;
               }

             /* Scale */
             if (pd->scale_inherit)
               eina_vector3_multiply(&pd->scale_world, scale_parent, &pd->scale);
             else
               pd->scale_world = pd->scale;

             /* Position */
             if (pd->position_inherit)
               {
                  eina_vector3_multiply(&pd->position_world, &pd->position, scale_parent);
                  eina_vector3_quaternion_rotate(&pd->position_world, &pd->position_world,
                                              orientation_parent);
                  eina_vector3_add(&pd->position_world, &pd->position_world,
                                &pdparent->position_world);
               }
             else
               {
                  pd->position_world = pd->position;
               }
          }
        else
          {
             pd->position_world = pd->position;
             pd->orientation_world = pd->orientation;
             pd->scale_world = pd->scale;
          }

        if (pd->type == EVAS_CANVAS3D_NODE_TYPE_CAMERA)
          {
             evas_mat4_inverse_build(&pd->data.camera.matrix_world_to_eye,
                                     &pd->position_world, &pd->orientation_world,
                                     &pd->scale_world);
          }
        else if (pd->type == EVAS_CANVAS3D_NODE_TYPE_LIGHT)
          {
          }
        else if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
          {
             evas_mat4_build(&pd->data.mesh.matrix_local_to_world,
                             &pd->position_world, &pd->orientation_world, &pd->scale_world);
          }
/*
        if (pd->parent)
          {
             evas_mat4_nocheck_multiply(&pd->matrix_local_to_world,
                                        &pd->parent->matrix_local_to_world,
                                        &pd->matrix_local_to_parent);
          }
        else
          {
             evas_mat4_copy(&pd->matrix_local_to_world, &pd->matrix_local_to_parent);
          }*/
     }

   return EINA_TRUE;
}

static Eina_Bool
_node_billboard_update(Evas_Canvas3D_Node *node, void *data EINA_UNUSED)
{
   Eina_Vector3   target;
   Eina_Vector3   up;
   Evas_Canvas3D_Node_Data *pd_node = efl_data_scope_get(node, MY_CLASS);
   if (pd_node->billboard_target)
     {
        Evas_Canvas3D_Node_Data *pd_target = efl_data_scope_get(pd_node->billboard_target,
                                                       MY_CLASS);
        eina_vector3_set(&target, pd_target->position.x, pd_target->position.y,
                      pd_target->position.z);
        eina_vector3_set(&up, 0, 1, 0);

        _look_at_set(pd_node, &target, &up);

        if (pd_node->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
          {
             evas_mat4_build(&pd_node->data.mesh.matrix_local_to_world,
                             &pd_node->position, &pd_node->orientation,
                             &pd_node->scale);
          }
        else if (pd_node->type == EVAS_CANVAS3D_NODE_TYPE_LIGHT)
          {
             evas_mat4_build(&pd_node->data.light.matrix_local_to_world,
                             &pd_node->position, &pd_node->orientation,
                             &pd_node->scale);
          }
        else if (pd_node->type == EVAS_CANVAS3D_NODE_TYPE_CAMERA)
          {
             evas_mat4_inverse_build(&pd_node->data.light.matrix_local_to_world,
                                     &pd_node->position, &pd_node->orientation,
                                     &pd_node->scale);
          }
        else
          ERR("Not supported type of node: line %d in file %s", __LINE__ , __FILE__);
     }
   return EINA_TRUE;
}

static Eina_Bool
_node_item_update(Evas_Canvas3D_Node *node, void *data EINA_UNUSED)
{
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, EVAS_CANVAS3D_NODE_CLASS);
   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_CAMERA)
     {
        if (pd->data.camera.camera)
          {
             evas_canvas3d_object_update(pd->data.camera.camera);
          }
     }
   else if (pd->type == EVAS_CANVAS3D_NODE_TYPE_LIGHT)
     {
        if (pd->data.light.light)
          {
             evas_canvas3d_object_update(pd->data.light.light);
          }
     }
   else if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        Eina_List *l;
        Evas_Canvas3D_Mesh *m;
        EINA_LIST_FOREACH(pd->data.mesh.meshes, l, m)
          {
             evas_canvas3d_object_update(m);
          }
     }

   return EINA_TRUE;
}

static void
_pack_meshes_vertex_data(Evas_Canvas3D_Node *node, Eina_Vector3 **vertices, int *count)
{
    const Eina_List *m, *l;
    Evas_Canvas3D_Mesh *mesh;
    int j;
    int frame;
    Eina_Vector3 *it;
    Evas_Canvas3D_Vertex_Buffer   pos0, pos1;
    Evas_Real               pos_weight;

    *count = 0;
    m = (Eina_List *)evas_canvas3d_node_mesh_list_get(node);
    EINA_LIST_FOREACH(m, l, mesh)
      {
         frame = evas_canvas3d_node_mesh_frame_get(node, mesh);
         evas_canvas3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION,
                                                    &pos0, &pos1, &pos_weight);
         if(!pos0.data) continue;
         if(!pos0.stride)
           {
             *count += pos0.size / (sizeof(float) * 3);
           }
         else
           {
             *count += pos0.size / pos0.stride;
           }
      }
    *vertices = (Eina_Vector3*)malloc(*count * sizeof(Eina_Vector3));
    it = *vertices;
    if(!*vertices)
      {
         ERR("Not enough memory.");
         return;
      }

    EINA_LIST_FOREACH(m, l, mesh)
      {
         frame = evas_canvas3d_node_mesh_frame_get(node, mesh);
         evas_canvas3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION,
                                                    &pos0, &pos1, &pos_weight);
         if(!pos0.data) continue;
         int stride = 0;
         if(!pos0.stride)
           {
             stride = sizeof(float) * 3;
           }
         else
          {
            stride = pos0.stride;
          }
         for (j = 0; j < pos0.size / stride; j++)
           {
             evas_canvas3d_mesh_interpolate_position_get(it, &pos0, &pos1, pos_weight, j);
             it++;
           }
      }
}

static void
_update_node_shapes(Evas_Canvas3D_Node *node)
{
   int i;
   int count;
   Eina_Vector3 *vertices = NULL;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, EVAS_CANVAS3D_NODE_CLASS);
   Eina_Bool transform_orientation_dirty;
   Eina_Bool transform_position_dirty;
   Eina_Bool transform_scale_dirty;
   Eina_Bool mesh_geom_dirty;
   Eina_Vector3 position = pd->position_world;

   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        pd->aabb.p0 = position;
        pd->aabb.p1 = position;
        pd->obb.p0 = position;
        pd->obb.p1 = position;
        pd->bsphere.radius = 0;
        pd->bsphere.center = position;
        return;
     }

   transform_orientation_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION);
   transform_orientation_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION);
   transform_position_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_POSITION);
   transform_position_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION);
   transform_scale_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_SCALE);
   transform_scale_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE);
   mesh_geom_dirty = evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_MESH_GEOMETRY);
   mesh_geom_dirty |= evas_canvas3d_object_dirty_get(node, EVAS_CANVAS3D_STATE_NODE_MESH_FRAME);


        _pack_meshes_vertex_data(node, &vertices, &count);
        if (count > 0)
          {
            calculate_box(&pd->obb, count, vertices);
            for (i = 0; i < count; i++)
            {
                eina_vector3_homogeneous_position_transform(&vertices[i], &pd->data.mesh.matrix_local_to_world, &vertices[i]);
            }
            calculate_box(&pd->aabb, count, vertices);
            if (transform_position_dirty || transform_scale_dirty || mesh_geom_dirty)
            {
                calculate_sphere(&pd->bsphere, count, vertices);
            }
            eina_vector3_homogeneous_position_transform(&pd->obb.p0, &pd->data.mesh.matrix_local_to_world, &pd->obb.p0);
            eina_vector3_homogeneous_position_transform(&pd->obb.p0, &pd->data.mesh.matrix_local_to_world, &pd->obb.p0);
          }
        free(vertices);

}

Eina_Bool
node_aabb_update(Evas_Canvas3D_Node *node, void *data EINA_UNUSED)
{
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, EVAS_CANVAS3D_NODE_CLASS);
   Eina_List *current;
   Evas_Canvas3D_Node *datanode;
   const Efl_Event_Description *eo_desc = NULL;
   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH &&
       pd->type != EVAS_CANVAS3D_NODE_TYPE_NODE)
     return EINA_TRUE;
   _update_node_shapes(node);
   EINA_LIST_FOREACH(pd->members, current, datanode)
     {
        Evas_Canvas3D_Node_Data *datapd = efl_data_scope_get(datanode, EVAS_CANVAS3D_NODE_CLASS);
        evas_box3_union(&pd->obb, &pd->obb, &datapd->obb);
        evas_box3_union(&pd->aabb, &pd->aabb, &datapd->aabb);
     }

   evas_build_sphere(&pd->aabb, &pd->bsphere);
   eo_desc = efl_object_legacy_only_event_description_get("collision,private");
   efl_event_callback_legacy_call(node, eo_desc, (void *)node);

   return EINA_TRUE;
}

static Eina_Bool
_node_update_done(Evas_Canvas3D_Node *obj, void *data EINA_UNUSED)
{
   //@FIXME
   Evas_Canvas3D_Object_Data *pdobject = efl_data_scope_get(obj, EVAS_CANVAS3D_OBJECT_CLASS);
   memset(&pdobject->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_CANVAS3D_STATE_MAX);
   return EINA_TRUE;
}

EOLIAN static void
_evas_canvas3d_node_evas_canvas3d_object_update_notify(Eo *obj, Evas_Canvas3D_Node_Data *pd EINA_UNUSED)
{
   /* Update transform. */
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_FALSE,
                              _node_transform_update, NULL);
   /*Update billboard*/
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_billboard_update, NULL);
   /* Update AABB. */
   if(efl_object_legacy_only_event_description_get("collision,private"))
   {
      evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_POST_ORDER, EINA_FALSE,
              node_aabb_update, NULL);
   }

   /* Update node item. */
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_item_update, NULL);

   /* Mark all nodes in the tree as up-to-date. */
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_update_done, NULL);
}

static void
_node_free(Evas_Canvas3D_Object *obj)
{
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   if (pd->members)
     {
        Eina_List *l;
        Evas_Canvas3D_Node *n;

        EINA_LIST_FOREACH(pd->members, l, n)
          {
             efl_unref(n);
          }

        eina_list_free(pd->members);
     }

   if (pd->parent)
     {
        evas_canvas3d_node_member_del(pd->parent, obj);
     }

   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH && pd->data.mesh.meshes)
     {
        Eina_List *l;
        Evas_Canvas3D_Mesh *m;

        EINA_LIST_FOREACH(pd->data.mesh.meshes, l, m)
          {
             efl_unref(m);
          }

        eina_list_free(pd->data.mesh.meshes);

        if (pd->data.mesh.node_meshes)
          eina_hash_free(pd->data.mesh.node_meshes);
     }

   if (pd->scenes_root)
     eina_hash_free(pd->scenes_root);

   if (pd->scenes_camera)
     eina_hash_free(pd->scenes_camera);
}

void
evas_canvas3d_node_scene_root_add(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene)
{
   int count = 0;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   if (pd->scenes_root == NULL)
     {
        pd->scenes_root = eina_hash_pointer_new(NULL);

        if (pd->scenes_root == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)(uintptr_t)eina_hash_find(pd->scenes_root, &scene);

   eina_hash_set(pd->scenes_root, &scene, (const void *)(uintptr_t)(count + 1));
}

void
evas_canvas3d_node_scene_root_del(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene)
{
   int count = 0;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   if (pd->scenes_root == NULL)
     {
        ERR("No scene to delete.");
        return;
     }

   count = (int)(uintptr_t)eina_hash_find(pd->scenes_root, &scene);

   if (count == 1)
     eina_hash_del(pd->scenes_root, &scene, NULL);
   else
     eina_hash_set(pd->scenes_root, &scene, (const void *)(uintptr_t)(count - 1));
}

EOLIAN static Eina_Hash*
_evas_canvas3d_node_scene_root_get(Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->scenes_root;
}

void
evas_canvas3d_node_scene_camera_add(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene)
{
   int count = 0;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   if (pd->scenes_camera == NULL)
     {
        pd->scenes_camera = eina_hash_pointer_new(NULL);

        if (pd->scenes_camera == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)(uintptr_t)eina_hash_find(pd->scenes_camera, &scene);

   eina_hash_set(pd->scenes_camera, &scene, (const void *)(uintptr_t)(count + 1));
}

void
evas_canvas3d_node_scene_camera_del(Evas_Canvas3D_Node *node, Evas_Canvas3D_Scene *scene)
{
   int count = 0;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   if (pd->scenes_camera == NULL)
     {
        ERR("No scene to delete.");
        return;
     }

   count = (int)(uintptr_t)eina_hash_find(pd->scenes_camera, &scene);

   if (count == 1)
     eina_hash_del(pd->scenes_camera, &scene, NULL);
   else
     eina_hash_set(pd->scenes_camera, &scene, (const void *)(uintptr_t)(count - 1));
}

void
evas_canvas3d_node_traverse(Evas_Canvas3D_Node *from, Evas_Canvas3D_Node *to, Evas_Canvas3D_Node_Traverse_Type type,
                      Eina_Bool skip, Evas_Canvas3D_Node_Func func, void *data)
{
   Eina_List *nodes = NULL, *n;
   Evas_Canvas3D_Node *node = NULL;

   if (from == NULL || func == NULL)
     goto error;

   if (type == EVAS_CANVAS3D_NODE_TRAVERSE_DOWNWARD)
     {
        if (to == NULL)
          goto error;

        node = to;

        do {
             nodes = eina_list_prepend(nodes, (const void *)node);

             if (node == from)
               break;
             Evas_Canvas3D_Node_Data *pdnode = efl_data_scope_get(node, MY_CLASS);
             node = pdnode->parent;

             if (node == NULL)
               goto error;
        } while (1);
     }
   else if (type == EVAS_CANVAS3D_NODE_TRAVERSE_UPWARD)
     {
        node = from;

        do {
             nodes = eina_list_append(nodes, (const void *)node);

             if (node == to)
               break;
             Evas_Canvas3D_Node_Data *pdnode = efl_data_scope_get(node, MY_CLASS);
             node = pdnode->parent;

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
evas_canvas3d_node_tree_traverse(Evas_Canvas3D_Node *root, Evas_Canvas3D_Tree_Traverse_Type type,
                           Eina_Bool skip, Evas_Canvas3D_Node_Func func, void *data)
{
   Eina_List *nodes = NULL, *l;
   Evas_Canvas3D_Node *node = NULL, *n, *last;

   if (root == NULL || func == NULL)
     return;

   if (type == EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER)
     {
        /* Put the root node in the queue. */
        nodes = eina_list_append(nodes, root);

        while (eina_list_count(nodes) > 0)
          {
             /* Dequeue a node. */
             node = eina_list_data_get(nodes);
             Evas_Canvas3D_Node_Data *pdnode = efl_data_scope_get(node, MY_CLASS);

             nodes = eina_list_remove_list(nodes, nodes);

             /* Call node function on the node. */
             if (func(node, data) || !skip)
               {
                  /* Enqueue member nodes. */
                  EINA_LIST_FOREACH(pdnode->members, l, n)
                    {
                       nodes = eina_list_append(nodes, n);
                    }
               }
          }
     }
   else if (type == EVAS_CANVAS3D_TREE_TRAVERSE_PRE_ORDER)
     {
        /* Put the root node in the stack. */
        nodes = eina_list_append(nodes, root);

        while (eina_list_count(nodes) > 0)
          {
             /* Pop a node from the stack. */
             node = eina_list_data_get(nodes);
             Evas_Canvas3D_Node_Data *pdnode = efl_data_scope_get(node, MY_CLASS);
             nodes = eina_list_remove_list(nodes, nodes);

             /* Call node function on the node. */
             if (func(node, data) || !skip)
               {
                  /* Push member nodes into the stack. */
                  EINA_LIST_REVERSE_FOREACH(pdnode->members, l, n)
                    {
                       nodes = eina_list_prepend(nodes, n);
                    }
               }
          }
     }
   else if (type == EVAS_CANVAS3D_TREE_TRAVERSE_POST_ORDER)
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
             Evas_Canvas3D_Node_Data *pdnode = efl_data_scope_get(node, MY_CLASS);
             if (eina_list_count(pdnode->members) == 0)
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
                   Evas_Canvas3D_Node_Data *pdlast;
                   if (last )
                     pdlast= efl_data_scope_get(last, MY_CLASS);
                  if (last && pdlast->parent == node)
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
                       EINA_LIST_REVERSE_FOREACH(pdnode->members, l, n)
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
_node_is_visible(Evas_Canvas3D_Node *node EINA_UNUSED, Evas_Canvas3D_Node *camera_node EINA_UNUSED)
{
   /* TODO: */
   return EINA_TRUE;
}

Eina_Bool
evas_canvas3d_node_mesh_collect(Evas_Canvas3D_Node *node, void *data)
{
   Eina_List *list_meshes, *l;
   Evas_Canvas3D_Mesh *mesh = NULL;
   Evas_Canvas3D_Mesh_Data *mesh_pd;
   Evas_Canvas3D_Mesh_Frame *f;
   Evas_Canvas3D_Scene_Public_Data *scene_data = (Evas_Canvas3D_Scene_Public_Data *)data;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   unsigned short int *index;
   int stride, tex_stride, normal_stride;

   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        scene_data->mesh_nodes = eina_list_append(scene_data->mesh_nodes, node);
        /*In case LOD calculate distance to the camera node*/
        if (pd->lod)
          {
              Evas_Canvas3D_Node_Data *pd_camera = efl_data_scope_get(scene_data->camera_node, MY_CLASS);
              scene_data->lod_distance = eina_vector3_distance_get(&pd_camera->position, &pd->position_world);
          }
        /* calculation of tangent space for all meshes */
        list_meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(node);
        EINA_LIST_FOREACH(list_meshes, l, mesh)
          {
             mesh_pd = efl_data_scope_get(mesh, MY_CLASS);
             f = evas_canvas3d_mesh_frame_find(mesh_pd, 0);
             if (!f)
               {
                  ERR("Not existing mesh frame.");
                  continue;
               }

             float *tangent_data = (float *)f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT].data;
             if (!tangent_data && ((mesh_pd->shader_mode == EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP) ||
                 (mesh_pd->shader_mode == EVAS_CANVAS3D_SHADER_MODE_PARALLAX_OCCLUSION)))
               {
                  index = (unsigned short int *)mesh_pd->indices;
                  tangent_data = (float*) malloc((3 * mesh_pd->vertex_count) * sizeof(float));

                  float *vertex_data = (float *)f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].data;
                  if (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].stride != 0)
                    stride = f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].stride / sizeof(float);
                  else
                    stride = 3;

                  float *tex_data = (float *)f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].data;
                  if (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].stride != 0)
                    tex_stride = f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].stride / sizeof(float);
                  else
                    tex_stride = 2;

                  float *normal_data = (float *)f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].data;
                  if (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].stride != 0)
                    normal_stride = f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].stride / sizeof(float);
                  else
                    normal_stride = 2;

                  evas_tangent_space_get(vertex_data, tex_data, normal_data, index, mesh_pd->vertex_count,
                                         mesh_pd->index_count, stride, tex_stride, normal_stride, &tangent_data);

                  evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT, 3 * sizeof(float), tangent_data);
                  free(tangent_data);
               }
          }
     }
   if (!_node_is_visible(node, scene_data->camera_node))
     {
        /* Skip entire sub-tree of this node. */
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
evas_canvas3d_node_color_node_mesh_collect(Evas_Canvas3D_Node *node, void *data)
{
   Evas_Canvas3D_Scene_Public_Data *scene_data = (Evas_Canvas3D_Scene_Public_Data *)data;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   Evas_Canvas3D_Node_Data *pd_camera = efl_data_scope_get(scene_data->camera_node, MY_CLASS);
   Evas_Canvas3D_Camera *camera = (Evas_Canvas3D_Camera*)pd_camera->data.camera.camera;

   Eina_List *list_meshes, *l;
   Evas_Canvas3D_Mesh *mesh;
   Eina_Stringshare *key, *datakey;
   Evas_Color *color;
   Eina_Bool visible = EINA_FALSE;
   Eina_Array *arr;
   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        visible = evas_canvas3d_camera_node_visible_get(camera, scene_data->camera_node, node, EVAS_CANVAS3D_FRUSTUM_MODE_BSPHERE);
        if (visible)
          {
             list_meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(node);
             EINA_LIST_FOREACH(list_meshes, l, mesh)
               {
                 if (evas_canvas3d_mesh_color_pick_enable_get(mesh))
                   {
                      color = calloc(1, sizeof(Evas_Color));

                      if (!eina_hash_population(scene_data->node_mesh_colors))
                        key = _generate_unic_color_key(color, &scene_data->bg_color,
                                                       node, mesh, EINA_TRUE);
                      else
                        key = _generate_unic_color_key(color, &scene_data->bg_color,
                                                       node, mesh, EINA_FALSE);

                      datakey = eina_stringshare_printf("%f %f %f", color->r, color->g, color->b);
                      eina_hash_add(scene_data->node_mesh_colors, key, color);
                      arr = eina_array_new(2);
                      eina_array_push(arr, (void *)node);
                      eina_array_push(arr, (void *)mesh);
                      eina_hash_add(scene_data->colors_node_mesh, datakey, arr);
                   }
               }
           }
        else
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
evas_canvas3d_node_light_collect(Evas_Canvas3D_Node *node, void *data)
{
   Evas_Canvas3D_Scene_Public_Data *scene_data = (Evas_Canvas3D_Scene_Public_Data *)data;
   Evas_Canvas3D_Node_Data *pd = efl_data_scope_get(node, MY_CLASS);
   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_LIGHT)
     scene_data->light_nodes = eina_list_append(scene_data->light_nodes, node);

   return EINA_TRUE;
}

EAPI Evas_Canvas3D_Node *
evas_canvas3d_node_add(Evas *e, Evas_Canvas3D_Node_Type type)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return efl_add(MY_CLASS, e, evas_canvas3d_node_type_set(efl_added, type));
}

EOLIAN static Efl_Object *
_evas_canvas3d_node_efl_object_finalize(Eo *obj, Evas_Canvas3D_Node_Data *pd)
{
   if (pd->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        pd->data.mesh.node_meshes = eina_hash_pointer_new(_node_mesh_free_func);
        if (pd->data.mesh.node_meshes == NULL)
          {
             ERR("Failed to create node mesh table.");
             _node_free(obj);
             return NULL;
          }
     }
   return obj;
}

EOLIAN static Efl_Object *
_evas_canvas3d_node_efl_object_constructor(Eo *obj, Evas_Canvas3D_Node_Data *pd)
{
   efl_constructor(efl_super(obj, MY_CLASS));
   evas_canvas3d_object_type_set(obj, EVAS_CANVAS3D_OBJECT_TYPE_NODE);

   eina_vector3_set(&pd->position, 0.0, 0.0, 0.0);
   eina_quaternion_set(&pd->orientation, 0.0, 0.0, 0.0, 1.0);
   eina_vector3_set(&pd->scale, 1.0, 1.0, 1.0);

   eina_vector3_set(&pd->position_world, 0.0, 0.0, 0.0);
   eina_quaternion_set(&pd->orientation_world, 0.0, 0.0, 0.0, 1.0);
   eina_vector3_set(&pd->scale_world, 1.0, 1.0, 1.0);

   pd->position_inherit = EINA_TRUE;
   pd->orientation_inherit = EINA_TRUE;
   pd->scale_inherit = EINA_TRUE;
   pd->data.mesh.node_meshes = 0;
   pd->billboard_target = NULL;
   pd->lod = EINA_FALSE;

   evas_box3_set(&pd->aabb, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
   return obj;
}

EOLIAN static void
_evas_canvas3d_node_efl_object_destructor(Eo *obj, Evas_Canvas3D_Node_Data *pd EINA_UNUSED)
{
   _node_free(obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Evas_Canvas3D_Node_Type
_evas_canvas3d_node_node_type_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->type;
}

EOLIAN static void
_evas_canvas3d_node_node_type_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Node_Type type)
{
   pd->type = type;
}

EOLIAN static void
_evas_canvas3d_node_member_add(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Node *member)
{
   if (obj == member)
     {
        ERR("Failed to add a member node (adding to itself).");
        return;
     }
   Evas_Canvas3D_Node_Data *pdmember = efl_data_scope_get(member, MY_CLASS);
   if (!pdmember || pdmember->parent == obj)
     return;

   if (pdmember->parent)
     {
        /* Detaching from previous parent. */
         Evas_Canvas3D_Node_Data *pdmemberparent = efl_data_scope_get(pdmember->parent, MY_CLASS);
        pdmemberparent->members = eina_list_remove(pdmemberparent->members, member);

        /* Mark changed. */
        evas_canvas3d_object_change(pdmember->parent, EVAS_CANVAS3D_STATE_NODE_MEMBER, NULL);
     }
   else
     {
        /* Should get a new reference. */
        efl_ref(member);
     }

   /* Add the member node. */
   pd->members = eina_list_append(pd->members, (const void *)member);
   pdmember->parent = obj;

   /* Mark changed. */
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MEMBER, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE, NULL);
}

EOLIAN static void
_evas_canvas3d_node_member_del(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Node *member)
{
   Evas_Canvas3D_Node_Data *pdmember = efl_data_scope_get(member, MY_CLASS);
   if (!pdmember || pdmember->parent != obj)
     {
        ERR("Failed to delete a member node (not a member of the given node)");
        return;
     }

   /* Delete the member node. */
   pd->members = eina_list_remove(pd->members, member);
   pdmember->parent = NULL;

   /* Mark modified object as changed. */
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MEMBER, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_ORIENTATION, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_POSITION, NULL);
   evas_canvas3d_object_change(member, EVAS_CANVAS3D_STATE_NODE_PARENT_SCALE, NULL);

   /* Decrease reference count. */
   efl_unref(member);
}

EOLIAN static Evas_Canvas3D_Node *
_evas_canvas3d_node_parent_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->parent;
}

EOLIAN static const Eina_List *
_evas_canvas3d_node_member_list_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->members;
}

EOLIAN static void
_evas_canvas3d_node_position_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z)
{
   pd->position.x = x;
   pd->position.y = y;
   pd->position.z = z;

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_POSITION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_orientation_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w)
{
   pd->orientation.x = x;
   pd->orientation.y = y;
   pd->orientation.z = z;
   pd->orientation.w = w;

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_orientation_angle_axis_set(Eo *obj, Evas_Canvas3D_Node_Data *pd,
                                        Evas_Real angle, Evas_Real x, Evas_Real y, Evas_Real z)
{
   Evas_Real half_angle = 0.5 * DEGREE_TO_RADIAN(angle);
   Evas_Real s = sin(half_angle);
   Eina_Vector3 axis;

   eina_vector3_set(&axis, x, y, z);
   eina_vector3_normalize(&axis, &axis);

   pd->orientation.w = cos(half_angle);
   pd->orientation.x = s * axis.x;
   pd->orientation.y = s * axis.y;
   pd->orientation.z = s * axis.z;

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_scale_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z)
{
   pd->scale.x = x;
   pd->scale.y = y;
   pd->scale.z = z;

  evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_SCALE, NULL);
}

EOLIAN static void
_evas_canvas3d_node_position_get(const Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Space space,
                          Evas_Real *x, Evas_Real *y, Evas_Real *z)
{
   if (space == EVAS_CANVAS3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
     }
   else if (space == EVAS_CANVAS3D_SPACE_PARENT)
     {
        if (x) *x = pd->position.x;
        if (y) *y = pd->position.y;
        if (z) *z = pd->position.z;
     }
   else if (space == EVAS_CANVAS3D_SPACE_WORLD)
     {
        evas_canvas3d_object_update((Eo *) obj);

        if (x) *x = pd->position_world.x;
        if (y) *y = pd->position_world.y;
        if (z) *z = pd->position_world.z;
     }
}

EOLIAN static void
_evas_canvas3d_node_orientation_get(const Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Space space,
                             Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *w)
{
   if (space == EVAS_CANVAS3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
        if (w) *w = 0.0;
     }
   else if (space == EVAS_CANVAS3D_SPACE_PARENT)
     {
        if (x) *x = pd->orientation.x;
        if (y) *y = pd->orientation.y;
        if (z) *z = pd->orientation.z;
        if (w) *w = pd->orientation.w;
     }
   else if (space == EVAS_CANVAS3D_SPACE_WORLD)
     {
        evas_canvas3d_object_update((Eo *) obj);

        if (x) *x = pd->orientation_world.x;
        if (y) *y = pd->orientation_world.y;
        if (z) *z = pd->orientation_world.z;
        if (w) *w = pd->orientation_world.w;
     }

}

EOLIAN static void
_evas_canvas3d_node_scale_get(const Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Space space,
                       Evas_Real *x, Evas_Real *y, Evas_Real *z)
{
   if (space == EVAS_CANVAS3D_SPACE_LOCAL)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        if (z) *z = 0.0;
     }
   else if (space == EVAS_CANVAS3D_SPACE_PARENT)
     {
        if (x) *x = pd->scale.x;
        if (y) *y = pd->scale.y;
        if (z) *z = pd->scale.z;
     }
   else if (space == EVAS_CANVAS3D_SPACE_WORLD)
     {
        evas_canvas3d_object_update((Eo *) obj);

        if (x) *x = pd->scale_world.x;
        if (y) *y = pd->scale_world.y;
        if (z) *z = pd->scale_world.z;
     }
}

EOLIAN static void
_evas_canvas3d_node_position_inherit_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->position_inherit = inherit;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_POSITION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_orientation_inherit_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->orientation_inherit = inherit;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_scale_inherit_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->scale_inherit = inherit;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_SCALE, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas3d_node_position_inherit_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->position_inherit;
}

EOLIAN static Eina_Bool
_evas_canvas3d_node_orientation_inherit_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->orientation_inherit;
}

EOLIAN static Eina_Bool
_evas_canvas3d_node_scale_inherit_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->scale_inherit;
}

static void
_look_at_set(Evas_Canvas3D_Node_Data *pd, Eina_Vector3 *target, Eina_Vector3 *up)
{
   Eina_Vector3   x, y, z;

   eina_vector3_subtract(&z, &pd->position, target);
   eina_vector3_normalize(&z, &z);

   eina_vector3_cross_product(&x, up, &z);
   eina_vector3_normalize(&x, &x);

   eina_vector3_cross_product(&y, &z, &x);
   eina_vector3_normalize(&y, &y);

   /* Below matrix to quaternion conversion code taken from
    * http://fabiensanglard.net/doom3_documentation/37726-293748.pdf
    * When any license issue occurs, use ken shoemake's algorithm instead.
    */

   if (x.x + y.y + z.z > 0.0)
     {
        Evas_Real t = x.x + y.y + z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        pd->orientation.w = s * t;
        pd->orientation.z = (x.y - y.x) * s;
        pd->orientation.y = (z.x - x.z) * s;
        pd->orientation.x = (y.z - z.y) * s;
     }
   else if (x.x > y.y && x.x > z.z)
     {
        Evas_Real t = x.x - y.y - z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        pd->orientation.x = s * t;
        pd->orientation.y = (x.y + y.x) * s;
        pd->orientation.z = (z.x + x.z) * s;
        pd->orientation.w = (y.z - z.y) * s;
     }
   else if (y.y > z.z)
     {
        Evas_Real t = -x.x + y.y - z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        pd->orientation.y = s * t;
        pd->orientation.x = (x.y + y.x) * s;
        pd->orientation.w = (z.x - x.z) * s;
        pd->orientation.z = (y.z + z.y) * s;
     }
   else
     {
        Evas_Real t = -x.x - y.y + z.z + 1.0;
        Evas_Real s = evas_reciprocal_sqrt(t) * 0.5;

        pd->orientation.z = s * t;
        pd->orientation.w = (x.y - y.x) * s;
        pd->orientation.x = (z.x + x.z) * s;
        pd->orientation.y = (y.z + z.y) * s;
     }
}

EOLIAN static void
_evas_canvas3d_node_look_at_set(Eo *obj, Evas_Canvas3D_Node_Data *pd,
                         Evas_Canvas3D_Space target_space, Evas_Real tx, Evas_Real ty, Evas_Real tz,
                         Evas_Canvas3D_Space up_space, Evas_Real ux, Evas_Real uy, Evas_Real uz)
{
   Eina_Vector3   target;
   Eina_Vector3   up;

   /* Target position in parent space. */
   if (target_space == EVAS_CANVAS3D_SPACE_LOCAL)
     {
        ERR("TODO:");
        return;
     }
   else if (target_space == EVAS_CANVAS3D_SPACE_PARENT)
     {
        eina_vector3_set(&target, tx, ty, tz);
     }
   else if (target_space == EVAS_CANVAS3D_SPACE_WORLD)
     {
        ERR("TODO:");
        return;

     }
   else
     {
        ERR("Invalid coordinate space.");
        return;
     }

   if (up_space == EVAS_CANVAS3D_SPACE_LOCAL)
     {
       eina_vector3_set(&up, ux, uy, uz);
        //ERR("TODO:");
        //return;
     }
   else if (up_space == EVAS_CANVAS3D_SPACE_PARENT)
     {
        eina_vector3_set(&up, ux, uy, uz);
     }
   else if (up_space == EVAS_CANVAS3D_SPACE_WORLD)
     {
        ERR("TODO:");
        return;
     }
   else
     {
        ERR("Invalid coordinate space.");
        return;
     }

   _look_at_set(pd, &target, &up);

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL);
}

EOLIAN static void
_evas_canvas3d_node_camera_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Camera *camera)
{
   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_CAMERA)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (pd->data.camera.camera == camera)
     return;

   if (pd->data.camera.camera)
     {
        /* Detach previous camera object. */
        evas_canvas3d_camera_node_del(pd->data.camera.camera, obj);
        efl_unref(pd->data.camera.camera);
     }

   pd->data.camera.camera = camera;
   efl_ref(camera);

   /* Register change notification on the camera for this node. */
   evas_canvas3d_camera_node_add(camera, obj);

   /* Mark changed. */
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_CAMERA, NULL);
}

EOLIAN static Evas_Canvas3D_Camera *
_evas_canvas3d_node_camera_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->data.camera.camera;
}

EOLIAN static void
_evas_canvas3d_node_light_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Light *light)
{
   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_LIGHT)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (pd->data.light.light == light)
     return;

   if (pd->data.light.light)
     {
        /* Detach previous light object. */
        evas_canvas3d_light_node_del(pd->data.light.light, obj);
        efl_unref(pd->data.light.light);
     }

   pd->data.light.light = light;
   efl_ref(light);

   /* Register change notification on the light for this node. */
   evas_canvas3d_light_node_add(light, obj);

   /* Mark changed. */
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_LIGHT, NULL);
}

EOLIAN static Evas_Canvas3D_Light *
_evas_canvas3d_node_light_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->data.light.light;
}

EOLIAN static void
_evas_canvas3d_node_mesh_add(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Mesh *mesh)
{
   Evas_Canvas3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (eina_hash_find(pd->data.mesh.node_meshes, &mesh) != NULL)
     {
        ERR("The mesh is already added to the node.");
        return;
     }

   if ((nm = _node_mesh_new(obj, mesh)) == NULL)
     {
        ERR("Failed to create node mesh.");
        return;
     }

   /* TODO: Find node mesh and add if it does not exist. */
   if (!eina_hash_add(pd->data.mesh.node_meshes, &mesh, nm))
     {
        ERR("Failed to add a mesh to mesh table.");
        _node_mesh_free(nm);
        return;
     }

   pd->data.mesh.meshes = eina_list_append(pd->data.mesh.meshes, mesh);
   efl_ref(mesh);

   /* Register change notification. */
   evas_canvas3d_mesh_node_add(mesh, obj);

   /* Mark changed. */
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MESH_GEOMETRY, NULL);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MESH_MATERIAL, NULL);
}

EOLIAN static void
_evas_canvas3d_node_mesh_del(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Mesh *mesh)
{
   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (!eina_hash_del(pd->data.mesh.node_meshes, &mesh, NULL))
     {
        ERR("The given mesh doesn't belong to this node.");
        return;
     }

   pd->data.mesh.meshes = eina_list_remove(pd->data.mesh.meshes, mesh);
   evas_canvas3d_mesh_node_del(mesh, obj);
   efl_unref(mesh);

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MESH_GEOMETRY, NULL);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MESH_MATERIAL, NULL);
}

EOLIAN static const Eina_List *
_evas_canvas3d_node_mesh_list_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->data.mesh.meshes;
}

EOLIAN static void
_evas_canvas3d_node_mesh_frame_set(Eo *obj, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Mesh *mesh, int frame)
{
   Evas_Canvas3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return;
     }

   if ((nm = eina_hash_find(pd->data.mesh.node_meshes, &mesh)) == NULL)
     {
        ERR("The given mesh doesn't belongs to this node.");
        return;
     }

   nm->frame = frame;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_MESH_FRAME, NULL);
}

EOLIAN static int
_evas_canvas3d_node_mesh_frame_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd, Evas_Canvas3D_Mesh *mesh)
{
   Evas_Canvas3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        ERR("Node type mismatch.");
        return 0;
     }

   if ((nm = eina_hash_find(pd->data.mesh.node_meshes, &mesh)) == NULL)
     {
        ERR("The given mesh doesn't belongs to this node.");
        return 0;
     }

   return nm->frame;
}

EOLIAN static void
_evas_canvas3d_node_bounding_box_get(Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *x2, Evas_Real *y2, Evas_Real *z2)
{
   evas_canvas3d_object_update(obj);
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_FALSE,
                                 node_aabb_update, NULL);

   if (x) *x = pd->aabb.p0.x;
   if (y) *y = pd->aabb.p0.y;
   if (z) *z = pd->aabb.p0.z;
   if (x2) *x2 = pd->aabb.p1.x;
   if (y2) *y2 = pd->aabb.p1.y;
   if (z2) *z2 = pd->aabb.p1.z;
}

EOLIAN static void
_evas_canvas3d_node_bounding_sphere_get(Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *r)
{
   evas_canvas3d_object_update(obj);
   evas_canvas3d_node_tree_traverse(obj, EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_FALSE,
                                 node_aabb_update, NULL);

   if (x) *x = pd->bsphere.center.x;
   if (y) *y = pd->bsphere.center.y;
   if (z) *z = pd->bsphere.center.z;
   if (r) *r = pd->bsphere.radius;
}

EOLIAN static void
_evas_canvas3d_node_billboard_target_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd,
                                   Eo *target)
{
   if (pd->billboard_target != target)
     {
        pd->billboard_target = target;
        evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_PARENT_BILLBOARD, NULL);
     }
}

EOLIAN static Evas_Canvas3D_Node *
_evas_canvas3d_node_billboard_target_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->billboard_target;
}

EOLIAN static void
_evas_canvas3d_node_lod_enable_set(Eo *obj, Evas_Canvas3D_Node_Data *pd,
                                   Eina_Bool enable)
{
   pd->lod = enable;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_NODE_LOD, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas3d_node_lod_enable_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Node_Data *pd)
{
   return pd->lod;
}
#include "canvas/evas_canvas3d_node.eo.c"
