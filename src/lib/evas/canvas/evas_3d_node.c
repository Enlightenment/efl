#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_3d_node_callback.h"

#define MY_CLASS EVAS_3D_NODE_CLASS
#define MY_CLASS_NAME "Evas_3D_Node"

Evas_3D_Mesh_Frame *evas_3d_mesh_frame_find(Evas_3D_Mesh_Data *pd, int frame);

static Eina_Stringshare *
_generate_unic_color_key(Evas_Color *color, Evas_Color *bg_color, Evas_3D_Node *node, Evas_3D_Mesh *mesh,
                         Eina_Bool init)
{
   static unsigned short red = USHRT_MAX;

   if (init) red = USHRT_MAX;

   if (fabs(bg_color->r - (double)red) <= DBL_EPSILON) red--;

   color->r = (double)red / USHRT_MAX;
   color->g = 0.0;
   color->b = 0.0;

   red--;

   if (red < 1) red = USHRT_MAX;

   return eina_stringshare_printf("%p %p", node, mesh);
}

static Eina_Bool
_evas_3d_node_private_callback_collision(void *data, Eo *obj EINA_UNUSED,
                                            const Eo_Event_Description *desc EINA_UNUSED,
                                            void *event_info)
{
   Eina_List *collision_list = NULL, *l = NULL;
   Evas_3D_Node *target_node = NULL, *n = NULL;
   Evas_3D_Node_Data *pd_target = NULL, *pd = NULL;
   const Eo_Event_Description *eo_desc = NULL;
   Eina_Bool ret = EINA_FALSE;

   target_node = (Evas_3D_Node *)event_info;
   pd_target = eo_data_scope_get(target_node, EVAS_3D_NODE_CLASS);
   collision_list = (Eina_List *)data;
   eo_desc = eo_base_legacy_only_event_description_get("collision");

   if (collision_list)
     {
        EINA_LIST_FOREACH(collision_list, l, n)
          {
             pd = eo_data_scope_get(n, EVAS_3D_NODE_CLASS);
             if (box_intersection_box(&pd_target->aabb, &pd->aabb))
               eo_do(target_node, ret = eo_event_callback_call(eo_desc, n));
          }
        return ret;
     }
   return ret;
}
static Eina_Bool
_evas_3d_node_private_callback_clicked(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
                                          const Eo_Event_Description *desc EINA_UNUSED,
                                          void *event_info)
{
   Eina_Bool ret = EINA_FALSE;
   const Eo_Event_Description *eo_desc = eo_base_legacy_only_event_description_get("clicked");
   eo_do((Eo *)event_info, ret = eo_event_callback_call(eo_desc, event_info));

   return ret;
}

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
   eo_do(s, evas_3d_object_change(EVAS_3D_STATE_SCENE_ROOT_NODE, (Evas_3D_Object *)fdata));
   return EINA_TRUE;
}

static Eina_Bool
_node_scene_camera_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                 void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Scene *s = *(Evas_3D_Scene **)key;
   eo_do(s, evas_3d_object_change(EVAS_3D_STATE_SCENE_CAMERA_NODE, (Evas_3D_Object *)fdata));
   return EINA_TRUE;
}

EOLIAN static void
_evas_3d_node_evas_3d_object_change_notify(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_State state EINA_UNUSED , Evas_3D_Object *ref EINA_UNUSED)
{
   Eina_List    *l;
   Evas_3D_Node *n;
   Eina_Bool orientation;
   Eina_Bool position;
   Eina_Bool scale;
   Eina_Bool parent_change;

   /* Notify all scenes using this node that it has changed. */
   if (pd->scenes_root)
     eina_hash_foreach(pd->scenes_root, _node_scene_root_change_notify, obj);

   if (pd->scenes_camera)
     eina_hash_foreach(pd->scenes_camera, _node_scene_camera_change_notify, obj);

   parent_change = (state == EVAS_3D_STATE_NODE_PARENT_ORIENTATION)
                     || (state == EVAS_3D_STATE_NODE_PARENT_POSITION)
                     || (state == EVAS_3D_STATE_NODE_PARENT_SCALE);

   /* Notify parent that a member has changed. */
   if (pd->parent && !parent_change)
    {
       eo_do(pd->parent, evas_3d_object_change(EVAS_3D_STATE_NODE_MEMBER, obj));
    }

   orientation = (state == EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION)
                   || (state == EVAS_3D_STATE_NODE_PARENT_ORIENTATION);
   position = (state == EVAS_3D_STATE_NODE_TRANSFORM_POSITION)
                || (state == EVAS_3D_STATE_NODE_PARENT_POSITION);
   scale = (state == EVAS_3D_STATE_NODE_TRANSFORM_SCALE)
             || (state == EVAS_3D_STATE_NODE_PARENT_SCALE);

   /* Notify members that the parent has changed. */
   if (scale)
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          eo_do(n, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_SCALE, obj));
       }
   if (orientation)
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          eo_do(n, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_ORIENTATION, obj));
       }
   if (position)
     EINA_LIST_FOREACH(pd->members, l, n)
       {
          eo_do(n, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_POSITION, obj));
       }
}

EOLIAN static void
_evas_3d_node_evas_3d_object_callback_register(Eo *obj, Evas_3D_Node_Data *pd EINA_UNUSED,
                                               const char *event, const void *data)
{
   Evas_3D_Node_Private_Callback_Type tcb;

   GET_CALLBACK_TYPE(tcb, event)

   if (tcb != PRIVATE_CALLBACK_NONE)
     eo_do(obj, eo_event_callback_add(&evas_3d_node_private_event_desc[tcb],
                                      evas_3d_node_private_callback_functions[tcb], data));

}

EOLIAN static void
_evas_3d_node_evas_3d_object_callback_unregister(Eo *obj, Evas_3D_Node_Data *pd EINA_UNUSED,
                                                 const char *event)
{
   Evas_3D_Node_Private_Callback_Type tcb;

   GET_CALLBACK_TYPE(tcb, event)

   if (tcb != PRIVATE_CALLBACK_NONE)
     eo_do(obj, eo_event_callback_del(&evas_3d_node_private_event_desc[tcb],
                                      evas_3d_node_private_callback_functions[tcb], NULL));
}

static Eina_Bool
_node_transform_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
   Eina_Bool transform_dirty = EINA_FALSE, parent_dirty = EINA_FALSE;

   eo_do(node,
         transform_dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION),
         transform_dirty|= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_POSITION),
         transform_dirty|= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_SCALE),
         parent_dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_ORIENTATION),
         parent_dirty |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_POSITION),
         parent_dirty |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_SCALE));

   if (transform_dirty || parent_dirty)
     {
        if (pd->parent)
          {
             Evas_3D_Node_Data *pdparent = eo_data_scope_get(pd->parent, MY_CLASS);
             const Evas_Vec3 *scale_parent = &pdparent->scale_world;
             const Evas_Vec4 *orientation_parent = &pdparent->orientation_world;

             /* Orienatation */
             if (pd->orientation_inherit)
               {
                  evas_vec4_quaternion_multiply(&pd->orientation_world,
                                                orientation_parent, &pd->orientation);
               }
             else
               {
                  pd->orientation_world = pd->orientation;
               }

             /* Scale */
             if (pd->scale_inherit)
               evas_vec3_multiply(&pd->scale_world, scale_parent, &pd->scale);
             else
               pd->scale_world = pd->scale;

             /* Position */
             if (pd->position_inherit)
               {
                  evas_vec3_multiply(&pd->position_world, &pd->position, scale_parent);
                  evas_vec3_quaternion_rotate(&pd->position_world, &pd->position_world,
                                              orientation_parent);
                  evas_vec3_add(&pd->position_world, &pd->position_world,
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

        if (pd->type == EVAS_3D_NODE_TYPE_CAMERA)
          {
             evas_mat4_inverse_build(&pd->data.camera.matrix_world_to_eye,
                                     &pd->position_world, &pd->orientation_world,
                                     &pd->scale_world);
          }
        else if (pd->type == EVAS_3D_NODE_TYPE_LIGHT)
          {
          }
        else if (pd->type == EVAS_3D_NODE_TYPE_MESH)
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
_node_item_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, EVAS_3D_NODE_CLASS);
   if (pd->type == EVAS_3D_NODE_TYPE_CAMERA)
     {
        if (pd->data.camera.camera)
          {
             eo_do(pd->data.camera.camera, evas_3d_object_update());
          }
     }
   else if (pd->type == EVAS_3D_NODE_TYPE_LIGHT)
     {
        if (pd->data.light.light)
          {
             eo_do(pd->data.light.light, evas_3d_object_update());
          }
     }
   else if (pd->type == EVAS_3D_NODE_TYPE_MESH)
     {
        Eina_List *l;
        Evas_3D_Mesh *m;
        EINA_LIST_FOREACH(pd->data.mesh.meshes, l, m)
          {
             eo_do(m, evas_3d_object_update());
          }
     }

   return EINA_TRUE;
}

static void
_rotate_vertices(Evas_Vec4* orientation, int vertex_count, Evas_Vec3* vertex_position)
{
   int i;
   if (orientation->x || orientation->y || orientation->z)
     for (i = 0; i < vertex_count; i++)
        evas_vec3_quaternion_rotate(&vertex_position[i], &vertex_position[i], orientation);
}


static void
_scale_vertices(Evas_Vec3* scale, int vertex_count, Evas_Vec3* vertex_position)
{
   int i;
   if ((scale->x != 1) || (scale->y != 1) || (scale->z != 1))
     for (i = 0; i < vertex_count; i++)
        evas_vec3_multiply(&vertex_position[i], &vertex_position[i], scale);
}

static void
_calculate_sphere(Evas_Sphere *sphere, int vertex_count,  Evas_Vec3 *vertex_position)
{
   float radius = 0.0001f;
   Evas_Vec3 center, pos, diff;
   float len, alpha, alpha2;
   int  i, k;

   // shuffle array for averaging algorithms error
   for (i = 0; i < vertex_count; i++)
     {
        k = i + rand()%(vertex_count-i);
        pos = vertex_position[i];
        vertex_position[i] = vertex_position[k];
        vertex_position[k] = pos;
     }

   center = vertex_position[0];

   for (k = 0; k < 2; k++)
     {
        for (i = 0; i < vertex_count; ++i)
          {
             pos = vertex_position[i];
             evas_vec3_subtract(&diff, &pos, &center);
             len = evas_vec3_length_get(&diff);
             if (len > radius)
               {
                  alpha = len / radius;
                  alpha2 = alpha * alpha;
                  radius = 0.5f * (alpha + 1 / alpha) * radius;
                  evas_vec3_scale(&pos, &pos, 1 - 1 / alpha2);
                  evas_vec3_scale(&center, &center, (1 + 1 / alpha2));
                  evas_vec3_add(&center, &center, &pos);
                  evas_vec3_scale(&center, &center, 0.5f);
               }
          }
     }

   for (i = 0; i < vertex_count; ++i)
     {
        pos = vertex_position[i];
        evas_vec3_subtract(&diff, &pos, &center);
        len = evas_vec3_length_get(&diff);

        if (len > radius)
          {
             radius = (radius + len) / 2.0f;
             evas_vec3_scale(&diff, &diff, (len - radius) / len);
             evas_vec3_add(&center, &center, &diff);
          }
     }

   sphere->radius = radius;
   sphere->center = center;
}

static void
_calculate_box(Evas_Box3 *box3, int vertex_count, Evas_Vec3 *vertex_position)
{
   int i = 0;
   float vxmin, vymin, vzmin, vxmax, vymax, vzmax;

   vxmax = vxmin = vertex_position[0].x;
   vymax = vymin = vertex_position[0].y;
   vzmax = vzmin = vertex_position[0].z;

   for (i = 1; i < vertex_count; ++i)
     {
        if (vxmin > vertex_position[i].x) vxmin = vertex_position[i].x;
        if (vxmax < vertex_position[i].x) vxmax = vertex_position[i].x;
        if (vymin > vertex_position[i].y) vymin = vertex_position[i].y;
        if (vymax < vertex_position[i].y) vymax = vertex_position[i].y;
        if (vzmin > vertex_position[i].z) vzmin = vertex_position[i].z;
        if (vzmax < vertex_position[i].z) vzmax = vertex_position[i].z;
     }
   evas_box3_set(box3, vxmin, vymin, vzmin, vxmax, vymax, vzmax);
}

static void
_pack_meshes_vertex_data(Evas_3D_Node *node, Evas_Vec3 **vertices, int *count)
{
   const Eina_List *m, *l;
   Evas_3D_Mesh *mesh;
   Evas_3D_Mesh_Frame *f;
   int j;
   int frame;
   Evas_3D_Mesh_Data *mpd;
   Evas_Vec3 *it;

   *count = 0;
   eo_do(node, m = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(m, l, mesh)
     {
        eo_do(node, frame = evas_3d_node_mesh_frame_get(mesh));
        mpd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);
        f = evas_3d_mesh_frame_find(mpd, frame);
        if (f) *count += mpd->vertex_count;
     }

   *vertices = (Evas_Vec3*)malloc(*count * sizeof(Evas_Vec3));
   it = *vertices;
   if (!*vertices)
     {
        ERR("Not enough memory.");
        return;
     }

   EINA_LIST_FOREACH(m, l, mesh)
     {
        eo_do(node, frame = evas_3d_node_mesh_frame_get(mesh));
        mpd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);
        f = evas_3d_mesh_frame_find(mpd, frame);
        if (f)
          {
             float *src = (float *)f->vertices[EVAS_3D_VERTEX_POSITION].data;
             int stride = f->vertices[EVAS_3D_VERTEX_POSITION].stride;
             if (!stride) stride = sizeof(float) * 3;
             for (j = 0; j < mpd->vertex_count; j++)
               {
                  it->x = src[0];
                  it->y = src[1];
                  it->z = src[2];
                  it++;
                  src = (float *)((char *)src + stride);
               }
          }
     }
}

static void
_update_node_shapes(Evas_3D_Node *node)
{
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, EVAS_3D_NODE_CLASS);
   Eina_Bool transform_orientation_dirty;
   Eina_Bool transform_scale_dirty;
   Eina_Bool mesh_geom_dirty;
   Evas_Vec3 scale;
   Evas_Vec3 position = pd->position_world;
   evas_vec3_set(&scale, pd->scale_world.x, pd->scale.y, pd->scale.z);

   if (pd->type != EVAS_3D_NODE_TYPE_MESH)
     {
        evas_box3_empty_set(&pd->local_aabb);
        evas_box3_empty_set(&pd->local_obb);
        pd->local_bsphere.radius = 0;
        evas_vec3_set(&pd->local_bsphere.center, 0, 0, 0);

        pd->aabb.p0 = position;
        pd->aabb.p1 = position;
        pd->obb.p0 = position;
        pd->obb.p1 = position;
        pd->bsphere.radius = 0;
        pd->bsphere.center = position;
        return;
     }

   eo_do(node,
         transform_orientation_dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION),
         transform_orientation_dirty |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_ORIENTATION),
         transform_scale_dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_SCALE),
         transform_scale_dirty |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_SCALE),
         mesh_geom_dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_MESH_GEOMETRY),
         mesh_geom_dirty |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_MESH_FRAME));

   if ( transform_orientation_dirty || transform_scale_dirty || mesh_geom_dirty)
     {
        int count;
        Evas_Vec3 *vertices = NULL;
        _pack_meshes_vertex_data(node, &vertices, &count);
        if (count > 0)
          {
             _scale_vertices(&pd->scale_world, count, vertices);
             if (mesh_geom_dirty)
               _calculate_box(&pd->local_obb, count, vertices);

             if (transform_scale_dirty || mesh_geom_dirty)
               {
                  _calculate_sphere(&pd->local_bsphere, count, vertices);
               }
             if (transform_orientation_dirty || mesh_geom_dirty)
               {
                  _rotate_vertices(&pd->orientation_world, count, vertices);
                  _calculate_box(&pd->local_aabb, count, vertices);
               }
             free(vertices);
          }
     }

   pd->bsphere.radius = pd->local_bsphere.radius;
   evas_vec3_quaternion_rotate(&pd->bsphere.center, &pd->local_bsphere.center, &pd->orientation_world);
   evas_vec3_add(&pd->obb.p0, &position, &pd->local_obb.p0);
   evas_vec3_add(&pd->obb.p1, &position, &pd->local_obb.p1);
   evas_vec3_add(&pd->aabb.p0, &position, &pd->local_aabb.p0);
   evas_vec3_add(&pd->aabb.p1, &position, &pd->local_aabb.p1);
   evas_vec3_add(&pd->bsphere.center, &position, &pd->bsphere.center);
}

static Eina_Bool
_node_aabb_update(Evas_3D_Node *node, void *data EINA_UNUSED)
{
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, EVAS_3D_NODE_CLASS);
   Eina_Bool need_recalc;
   Eina_List *current;
   Evas_3D_Node *datanode;
   const Eo_Event_Description *eo_desc = NULL;

  eo_do(node,
        need_recalc = evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_ORIENTATION),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_POSITION),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_POSITION),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_TRANSFORM_SCALE),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_PARENT_SCALE),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_MESH_GEOMETRY),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_MESH_FRAME),
        need_recalc |= evas_3d_object_dirty_get(EVAS_3D_STATE_NODE_MEMBER));

   if (!need_recalc) return EINA_TRUE;

   _update_node_shapes(node);

   EINA_LIST_FOREACH(pd->members, current, datanode)
     {
        Evas_3D_Node_Data *datapd = eo_data_scope_get(datanode, EVAS_3D_NODE_CLASS);
        evas_box3_union(&pd->obb, &pd->obb, &datapd->obb);
        evas_box3_union(&pd->aabb, &pd->aabb, &datapd->aabb);
        evas_build_sphere(&pd->aabb, &pd->bsphere);
     }

   eo_desc = eo_base_legacy_only_event_description_get("collision,private");
   eo_do(node, eo_event_callback_call(eo_desc, (void *)node));

   return EINA_TRUE;
}

static Eina_Bool
_node_update_done(Evas_3D_Node *obj, void *data EINA_UNUSED)
{
   //@FIXME
   Evas_3D_Object_Data *pdobject = eo_data_scope_get(obj, EVAS_3D_OBJECT_CLASS);
   memset(&pdobject->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_3D_STATE_MAX);
   return EINA_TRUE;
}

EOLIAN static void
_evas_3d_node_evas_3d_object_update_notify(Eo *obj, Evas_3D_Node_Data *pd EINA_UNUSED)
{
   /* Update transform. */
   evas_3d_node_tree_traverse(obj, EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_FALSE,
                              _node_transform_update, NULL);

   /* Update AABB. */
   evas_3d_node_tree_traverse(obj, EVAS_3D_TREE_TRAVERSE_POST_ORDER, EINA_FALSE,
                              _node_aabb_update, NULL);

   /* Update node item. */
   evas_3d_node_tree_traverse(obj, EVAS_3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_item_update, NULL);

   /* Mark all nodes in the tree as up-to-date. */
   evas_3d_node_tree_traverse(obj, EVAS_3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              _node_update_done, NULL);
}

static void
_node_free(Evas_3D_Object *obj)
{
   Evas_3D_Node_Data *pd = eo_data_scope_get(obj, MY_CLASS);

   if (pd->members)
     {
        Eina_List *l;
        Evas_3D_Node *n;

        EINA_LIST_FOREACH(pd->members, l, n)
          {
             eo_unref(n);
          }

        eina_list_free(pd->members);
     }

   if (pd->data.mesh.meshes)
     {
        Eina_List *l;
        Evas_3D_Mesh *m;

        EINA_LIST_FOREACH(pd->data.mesh.meshes, l, m)
          {
             //evas_3d_mesh_node_del(m, node);
             eo_unref(m);
          }

        eina_list_free(pd->data.mesh.meshes);
     }

   if (pd->data.mesh.node_meshes)
     eina_hash_free(pd->data.mesh.node_meshes);

   if (pd->scenes_root)
     eina_hash_free(pd->scenes_root);

   if (pd->scenes_camera)
     eina_hash_free(pd->scenes_camera);

   //free(node);
}

void
evas_3d_node_scene_root_add(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
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
evas_3d_node_scene_root_del(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
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

void
evas_3d_node_scene_camera_add(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
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
evas_3d_node_scene_camera_del(Evas_3D_Node *node, Evas_3D_Scene *scene)
{
   int count = 0;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
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
             Evas_3D_Node_Data *pdnode = eo_data_scope_get(node, MY_CLASS);
             node = pdnode->parent;

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
             Evas_3D_Node_Data *pdnode = eo_data_scope_get(node, MY_CLASS);
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
             Evas_3D_Node_Data *pdnode = eo_data_scope_get(node, MY_CLASS);

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
   else if (type == EVAS_3D_TREE_TRAVERSE_PRE_ORDER)
     {
        /* Put the root node in the stack. */
        nodes = eina_list_append(nodes, root);

        while (eina_list_count(nodes) > 0)
          {
             /* Pop a node from the stack. */
             node = eina_list_data_get(nodes);
             Evas_3D_Node_Data *pdnode = eo_data_scope_get(node, MY_CLASS);
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
             Evas_3D_Node_Data *pdnode = eo_data_scope_get(node, MY_CLASS);
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
                   Evas_3D_Node_Data *pdlast;
                   if (last )
                     pdlast= eo_data_scope_get(last, MY_CLASS);
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
_node_is_visible(Evas_3D_Node *node EINA_UNUSED, Evas_3D_Node *camera_node EINA_UNUSED)
{
   /* TODO: */
   return EINA_TRUE;
}

Eina_Bool
evas_3d_node_mesh_collect(Evas_3D_Node *node, void *data)
{
   Evas_3D_Scene_Public_Data *scene_data = (Evas_3D_Scene_Public_Data *)data;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
   if (!_node_is_visible(node, scene_data->camera_node))
     {
        /* Skip entire sub-tree of this node. */
        return EINA_FALSE;
     }

   if (pd->type == EVAS_3D_NODE_TYPE_MESH)
     scene_data->mesh_nodes = eina_list_append(scene_data->mesh_nodes, node);

   return EINA_TRUE;
}

Eina_Bool
evas_3d_node_color_node_mesh_collect(Evas_3D_Node *node, void *data)
{
   Evas_3D_Scene_Public_Data *scene_data = (Evas_3D_Scene_Public_Data *)data;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
   Evas_3D_Node_Data *pd_camera = eo_data_scope_get(scene_data->camera_node, MY_CLASS);
   Evas_3D_Camera *camera = (Evas_3D_Camera*)pd_camera->data.camera.camera;

   Eina_List *list_meshes, *l;
   Evas_3D_Mesh *mesh;
   Eina_Stringshare *key, *datakey;
   Evas_Color *color;
   Eina_Bool visible = EINA_FALSE;
   Eina_Array *arr;
   if (pd->type == EVAS_3D_NODE_TYPE_MESH)
     {
        eo_do(camera,
              visible = evas_3d_camera_node_visible_get(scene_data->camera_node,
                                                        node, EVAS_3D_FRUSTUM_MODE_BSPHERE));
        if (visible)
          {
             eo_do (node, list_meshes = (Eina_List *)evas_3d_node_mesh_list_get());
             EINA_LIST_FOREACH(list_meshes, l, mesh)
               {
                 if (eo_do(mesh, evas_3d_mesh_color_pick_enable_get()))
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
evas_3d_node_light_collect(Evas_3D_Node *node, void *data)
{
   Evas_3D_Scene_Public_Data *scene_data = (Evas_3D_Scene_Public_Data *)data;
   Evas_3D_Node_Data *pd = eo_data_scope_get(node, MY_CLASS);
   if (pd->type == EVAS_3D_NODE_TYPE_LIGHT)
     scene_data->light_nodes = eina_list_append(scene_data->light_nodes, node);

   return EINA_TRUE;
}

EAPI Evas_3D_Node *
evas_3d_node_add(Evas *e, Evas_3D_Node_Type type)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(MY_CLASS, e, evas_3d_node_constructor(type));
   return eo_obj;
}

EOLIAN static void
_evas_3d_node_constructor(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Node_Type type)
{
   eo_do(obj, evas_3d_object_type_set(EVAS_3D_OBJECT_TYPE_NODE));

   evas_vec3_set(&pd->position, 0.0, 0.0, 0.0);
   evas_vec4_set(&pd->orientation, 0.0, 0.0, 0.0, 1.0);
   evas_vec3_set(&pd->scale, 1.0, 1.0, 1.0);

   evas_vec3_set(&pd->position_world, 0.0, 0.0, 0.0);
   evas_vec4_set(&pd->orientation_world, 0.0, 0.0, 0.0, 1.0);
   evas_vec3_set(&pd->scale_world, 1.0, 1.0, 1.0);

   pd->position_inherit = EINA_TRUE;
   pd->orientation_inherit = EINA_TRUE;
   pd->scale_inherit = EINA_TRUE;
   pd->data.mesh.node_meshes = 0;

   evas_box3_set(&pd->aabb, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

   pd->type = type;

   if (type == EVAS_3D_NODE_TYPE_MESH)
     {
        pd->data.mesh.node_meshes = eina_hash_pointer_new(_node_mesh_free_func);
        if (pd->data.mesh.node_meshes == NULL)
          {
             ERR("Failed to create node mesh table.");
             _node_free(obj);
          }
     }
}

EOLIAN static Evas_3D_Node_Type
_evas_3d_node_type_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->type;
}

EOLIAN static void
_evas_3d_node_member_add(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Node *member)
{
   if (obj == member)
     {
        ERR("Failed to add a member node (adding to itself).");
        return;
     }
   Evas_3D_Node_Data *pdmember = eo_data_scope_get(member, MY_CLASS);
   if (pdmember->parent == obj)
     return;

   if (pdmember->parent)
     {
        /* Detaching from previous parent. */
         Evas_3D_Node_Data *pdmemberparent = eo_data_scope_get(pdmember->parent, MY_CLASS);
        pdmemberparent->members = eina_list_remove(pdmemberparent->members, member);

        /* Mark changed. */
        eo_do(pdmember->parent, evas_3d_object_change(EVAS_3D_STATE_NODE_MEMBER, NULL));
     }
   else
     {
        /* Should get a new reference. */
        eo_ref(member);
     }

   /* Add the member node. */
   pd->members = eina_list_append(pd->members, (const void *)member);
   pdmember->parent = obj;

   /* Mark changed. */
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MEMBER, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_ORIENTATION, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_POSITION, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_SCALE, NULL));
}

EOLIAN static void
_evas_3d_node_member_del(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Node *member)
{
   Evas_3D_Node_Data *pdmember = eo_data_scope_get(member, MY_CLASS);
   if (pdmember->parent != obj)
     {
        ERR("Failed to delete a member node (not a member of the given node)");
        return;
     }

   /* Delete the member node. */
   pd->members = eina_list_remove(pd->members, member);
   pdmember->parent = NULL;

   /* Mark modified object as changed. */
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MEMBER, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_ORIENTATION, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_POSITION, NULL));
   eo_do(member, evas_3d_object_change(EVAS_3D_STATE_NODE_PARENT_SCALE, NULL));

   /* Decrease reference count. */
   eo_unref(member);
}

EOLIAN static Evas_3D_Node *
_evas_3d_node_parent_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->parent;
}

EOLIAN static const Eina_List *
_evas_3d_node_member_list_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->members;
}

EOLIAN static void
_evas_3d_node_position_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z)
{
   pd->position.x = x;
   pd->position.y = y;
   pd->position.z = z;

   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_POSITION, NULL));
}

EOLIAN static void
_evas_3d_node_orientation_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w)
{
   pd->orientation.x = x;
   pd->orientation.y = y;
   pd->orientation.z = z;
   pd->orientation.w = w;

   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL));
}

EOLIAN static void
_evas_3d_node_orientation_angle_axis_set(Eo *obj, Evas_3D_Node_Data *pd,
                                        Evas_Real angle, Evas_Real x, Evas_Real y, Evas_Real z)
{
   Evas_Real half_angle = 0.5 * DEGREE_TO_RADIAN(angle);
   Evas_Real s = sin(half_angle);
   Evas_Vec3 axis;

   evas_vec3_set(&axis, x, y, z);
   evas_vec3_normalize(&axis, &axis);

   pd->orientation.w = cos(half_angle);
   pd->orientation.x = s * axis.x;
   pd->orientation.y = s * axis.y;
   pd->orientation.z = s * axis.z;

   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL));
}

EOLIAN static void
_evas_3d_node_scale_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_Real x, Evas_Real y, Evas_Real z)
{
   pd->scale.x = x;
   pd->scale.y = y;
   pd->scale.z = z;

  eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_SCALE, NULL));
}

EOLIAN static void
_evas_3d_node_position_get(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Space space,
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
        if (x) *x = pd->position.x;
        if (y) *y = pd->position.y;
        if (z) *z = pd->position.z;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        eo_do(obj, evas_3d_object_update());

        if (x) *x = pd->position_world.x;
        if (y) *y = pd->position_world.y;
        if (z) *z = pd->position_world.z;
     }
}

EOLIAN static void
_evas_3d_node_orientation_get(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Space space,
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
        if (x) *x = pd->orientation.x;
        if (y) *y = pd->orientation.y;
        if (z) *z = pd->orientation.z;
        if (w) *w = pd->orientation.w;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        eo_do(obj, evas_3d_object_update());

        if (x) *x = pd->orientation_world.x;
        if (y) *y = pd->orientation_world.y;
        if (z) *z = pd->orientation_world.z;
        if (w) *w = pd->orientation_world.w;
     }

}

EOLIAN static void
_evas_3d_node_scale_get(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Space space,
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
        if (x) *x = pd->scale.x;
        if (y) *y = pd->scale.y;
        if (z) *z = pd->scale.z;
     }
   else if (space == EVAS_3D_SPACE_WORLD)
     {
        eo_do(obj, evas_3d_object_update());

        if (x) *x = pd->scale_world.x;
        if (y) *y = pd->scale_world.y;
        if (z) *z = pd->scale_world.z;
     }
}

EOLIAN static void
_evas_3d_node_position_inherit_set(Eo *obj, Evas_3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->position_inherit = inherit;
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_POSITION, NULL));
}

EOLIAN static void
_evas_3d_node_orientation_inherit_set(Eo *obj, Evas_3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->orientation_inherit = inherit;
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL));
}

EOLIAN static void
_evas_3d_node_scale_inherit_set(Eo *obj, Evas_3D_Node_Data *pd, Eina_Bool inherit)
{
   pd->scale_inherit = inherit;
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_SCALE, NULL));
}

EOLIAN static Eina_Bool
_evas_3d_node_position_inherit_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->position_inherit;
}

EOLIAN static Eina_Bool
_evas_3d_node_orientation_inherit_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->orientation_inherit;
}

EOLIAN static Eina_Bool
_evas_3d_node_scale_inherit_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->scale_inherit;
}

EOLIAN static void
_evas_3d_node_look_at_set(Eo *obj, Evas_3D_Node_Data *pd,
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
       evas_vec3_set(&up, ux, uy, uz);
        //ERR("TODO:");
        //return;
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
   evas_vec3_subtract(&z, &pd->position, &target);
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

   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_TRANSFORM_ORIENTATION, NULL));
}

EOLIAN static void
_evas_3d_node_camera_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Camera *camera)
{
   if (pd->type != EVAS_3D_NODE_TYPE_CAMERA)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (pd->data.camera.camera == camera)
     return;

   if (pd->data.camera.camera)
     {
        /* Detach previous camera object. */
        evas_3d_camera_node_del(pd->data.camera.camera, obj);
        eo_unref(pd->data.camera.camera);
     }

   pd->data.camera.camera = camera;
   eo_ref(camera);

   /* Register change notification on the camera for this node. */
   evas_3d_camera_node_add(camera, obj);

   /* Mark changed. */
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_CAMERA, NULL));
}

EOLIAN static Evas_3D_Camera *
_evas_3d_node_camera_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->data.camera.camera;
}

EOLIAN static void
_evas_3d_node_light_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Light *light)
{
   if (pd->type != EVAS_3D_NODE_TYPE_LIGHT)
     {
        ERR("Node type mismatch.");
        return;
     }

   if (pd->data.light.light == light)
     return;

   if (pd->data.light.light)
     {
        /* Detach previous light object. */
        evas_3d_light_node_del(pd->data.light.light, obj);
        eo_unref(pd->data.light.light);
     }

   pd->data.light.light = light;
   eo_ref(light);

   /* Register change notification on the light for this node. */
   evas_3d_light_node_add(light, obj);

   /* Mark changed. */
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_LIGHT, NULL));
}

EOLIAN static Evas_3D_Light *
_evas_3d_node_light_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->data.light.light;
}

EOLIAN static void
_evas_3d_node_mesh_add(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Mesh *mesh)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_3D_NODE_TYPE_MESH)
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
   eo_ref(mesh);

   /* Register change notification. */
   evas_3d_mesh_node_add(mesh, obj);

   /* Mark changed. */
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MESH_GEOMETRY, NULL));
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MESH_MATERIAL, NULL));
}

EOLIAN static void
_evas_3d_node_mesh_del(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Mesh *mesh)
{
   if (pd->type != EVAS_3D_NODE_TYPE_MESH)
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
   evas_3d_mesh_node_del(mesh, obj);
   eo_unref(mesh);

   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MESH_GEOMETRY, NULL));
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MESH_MATERIAL, NULL));
}

EOLIAN static const Eina_List *
_evas_3d_node_mesh_list_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd)
{
   return pd->data.mesh.meshes;
}

EOLIAN static void
_evas_3d_node_mesh_frame_set(Eo *obj, Evas_3D_Node_Data *pd, Evas_3D_Mesh *mesh, int frame)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_3D_NODE_TYPE_MESH)
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
   eo_do(obj, evas_3d_object_change(EVAS_3D_STATE_NODE_MESH_FRAME, NULL));
}

EOLIAN static int
_evas_3d_node_mesh_frame_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd, Evas_3D_Mesh *mesh)
{
   Evas_3D_Node_Mesh *nm = NULL;

   if (pd->type != EVAS_3D_NODE_TYPE_MESH)
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
_evas_3d_node_bounding_box_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *x2, Evas_Real *y2, Evas_Real *z2)
{
   eo_do(obj, evas_3d_object_update());
   if (x) *x = pd->aabb.p0.x;
   if (y) *y = pd->aabb.p0.y;
   if (z) *z = pd->aabb.p0.z;
   if (x2) *x2 = pd->aabb.p1.x;
   if (y2) *y2 = pd->aabb.p1.y;
   if (z2) *z2 = pd->aabb.p1.z;
}

EOLIAN static void
_evas_3d_node_bounding_sphere_get(Eo *obj EINA_UNUSED, Evas_3D_Node_Data *pd, Evas_Real *x, Evas_Real *y, Evas_Real *z, Evas_Real *r)
{
   eo_do(obj, evas_3d_object_update());
   if (x) *x = pd->bsphere.center.x;
   if (y) *y = pd->bsphere.center.y;
   if (z) *z = pd->bsphere.center.z;
   if (r) *r = pd->bsphere.radius;
}

#include "canvas/evas_3d_node.eo.c"
