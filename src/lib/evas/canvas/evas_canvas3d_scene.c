#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_SCENE_CLASS

void
evas_canvas3d_scene_data_init(Evas_Canvas3D_Scene_Public_Data *data)
{
   data->camera_node = NULL;
   data->light_nodes = NULL;
   data->mesh_nodes = NULL;
   data->node_mesh_colors = NULL;
   data->colors_node_mesh = NULL;
   data->render_to_texture = EINA_FALSE;
   data->lod_distance = 0;
   data->post_processing = EINA_FALSE;
   data->post_processing_type = EVAS_CANVAS3D_SHADER_MODE_POST_PROCESSING_FXAA;
}

void
evas_canvas3d_scene_data_fini(Evas_Canvas3D_Scene_Public_Data *data)
{
   if (data->light_nodes)
     eina_list_free(data->light_nodes);

   if (data->mesh_nodes)
     eina_list_free(data->mesh_nodes);
}

EOLIAN static void
_evas_canvas3d_scene_evas_canvas3d_object_change_notify(Eo *eo_obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd,
                                            Evas_Canvas3D_State state EINA_UNUSED,
                                            Evas_Canvas3D_Object *ref EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *eo;

   EINA_LIST_FOREACH(pd->images, l, eo)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);
        evas_object_change(eo, obj);
     }
}

EOLIAN static void
_evas_canvas3d_scene_evas_canvas3d_object_update_notify(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd)
{
   if (pd->root_node)
     {
        evas_canvas3d_object_update(pd->root_node);
     }

   if (pd->camera_node)
     {
        evas_canvas3d_object_update(pd->camera_node);
     }

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_UPDATED, NULL);
}

EAPI Evas_Canvas3D_Scene *
evas_canvas3d_scene_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return efl_add(MY_CLASS, e);
}

EOLIAN static Eo *
_evas_canvas3d_scene_efl_object_constructor(Eo *obj, Evas_Canvas3D_Scene_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_canvas3d_object_type_set(obj, EVAS_CANVAS3D_OBJECT_TYPE_SCENE);
   evas_color_set(&pd->bg_color, 0.0, 0.0, 0.0, 0.0);
   pd->shadows_enabled = EINA_FALSE;
   pd->color_pick_enabled = EINA_FALSE;
   pd->node_mesh_colors = NULL;
   pd->colors_node_mesh = NULL;
   pd->depth_offset = 4.0;
   pd->depth_constant = 100.0;
   return obj;
}

EOLIAN static void
_evas_canvas3d_scene_root_node_set(Eo *obj, Evas_Canvas3D_Scene_Data *pd, Evas_Canvas3D_Node *node)
{
   if (pd->root_node == node)
     return;

   if (pd->root_node)
     {
        evas_canvas3d_node_scene_root_del(pd->root_node, obj);
        efl_unref(pd->root_node);
     }

   pd->root_node = node;

   if (node)
     {
        efl_ref(node);
        evas_canvas3d_node_scene_root_add(node, obj);
     }

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_ROOT_NODE, NULL);
}

EOLIAN static Evas_Canvas3D_Node *
_evas_canvas3d_scene_root_node_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd)
{
   return pd->root_node;
}

EOLIAN static void
_evas_canvas3d_scene_camera_node_set(Eo *obj, Evas_Canvas3D_Scene_Data *pd, Evas_Canvas3D_Node *node)
{
   if (pd->camera_node == node)
     return;

   if (pd->camera_node)
     {
        evas_canvas3d_node_scene_camera_del(pd->camera_node, obj);
     }

   pd->camera_node = node;

   if (node)
     {
        evas_canvas3d_node_scene_camera_add(node, obj);
     }

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_CAMERA_NODE, NULL);
}

EOLIAN static Evas_Canvas3D_Node *
_evas_canvas3d_scene_camera_node_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd)
{
   return pd->camera_node;
}

EOLIAN static void
_evas_canvas3d_scene_size_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd, int w, int h)
{
   pd->w = w;
   pd->h = h;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_SIZE, NULL);
}

EOLIAN static void
_evas_canvas3d_scene_size_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd, int *w, int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

EOLIAN static void
_evas_canvas3d_scene_background_color_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd,
                                   Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   evas_color_set(&pd->bg_color, r, g, b, a);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_BACKGROUND_COLOR, NULL);
}

EOLIAN static void
_evas_canvas3d_scene_background_color_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd,
                                   Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = pd->bg_color.r;
   if (g) *g = pd->bg_color.g;
   if (b) *b = pd->bg_color.b;
   if (a) *a = pd->bg_color.a;
}

static inline Eina_Bool
_pick_data_triangle_add(Evas_Canvas3D_Pick_Data *data, const Evas_Ray3 *ray,
                        const Evas_Triangle3 *tri)
{
   Eina_Vector3   e1, e2, tvec, pvec, qvec;
   Evas_Real   det, inv_det, u, v, t;

   eina_vector3_subtract(&e1, &tri->p1, &tri->p0);
   eina_vector3_subtract(&e2, &tri->p2, &tri->p0);

   eina_vector3_cross_product(&pvec, &ray->dir, &e2);
   det = eina_vector3_dot_product(&e1, &pvec);

   /* If determinant is near zero, ray lies in plane of triangle. */
   if (det > -0.0000001 && det < 0.0000001)
     return EINA_FALSE;

   inv_det = 1.0 / det;

   /* Calculate distance from p0 to ray origin. */
   eina_vector3_subtract(&tvec, &ray->org, &tri->p0);

   /* Calculate U parameter and test bounds. */
   u = eina_vector3_dot_product(&tvec, &pvec) * inv_det;

   if (u < 0.0 || u > 1.0)
     return EINA_FALSE;

   /* Prepare to tst V parameter. */
   eina_vector3_cross_product(&qvec, &tvec, &e1);

   /* Calculate V parameter and test bounds. */
   v = eina_vector3_dot_product(&ray->dir, &qvec) * inv_det;

   if (v < 0.0 || u + v > 1.0)
     return EINA_FALSE;

   /* Calculate T parameter and test bounds. */
   t = eina_vector3_dot_product(&e2, &qvec) * inv_det;

   if (t >= 0.0 && t <= 1.0)
     {
        if (!data->picked || t < data->z)
          {
             data->picked = EINA_TRUE;
             data->z = t;
             data->u = u;
             data->v = v;
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

static inline void
_pick_data_texcoord_update(Evas_Canvas3D_Pick_Data *data,
                           const Evas_Canvas3D_Vertex_Buffer *tex0, const Evas_Canvas3D_Vertex_Buffer *tex1,
                           Evas_Real weight, unsigned int i0, unsigned int i1, unsigned int i2)
{
   Evas_Real s0, s1, s2;
   Evas_Real t0, t1, t2;

   if (tex1->data == NULL)
     {
        float *ptr;

        ptr = (float *)((char *)tex0->data + tex0->stride * i0);

        s0 = ptr[0];
        t0 = ptr[1];

        ptr = (float *)((char *)tex0->data + tex0->stride * i1);

        s1 = ptr[0];
        t1 = ptr[1];

        ptr = (float *)((char *)tex0->data + tex0->stride * i2);

        s2 = ptr[0];
        t2 = ptr[1];
     }
   else
     {
        float *ptr0, *ptr1;

        ptr0 = (float *)((char *)tex0->data + tex0->stride * i0);
        ptr1 = (float *)((char *)tex1->data + tex1->stride * i0);

        s0 = ptr0[0] * weight + ptr1[0] * (1.0 - weight);
        t0 = ptr0[1] * weight + ptr1[1] * (1.0 - weight);

        ptr0 = (float *)((char *)tex0->data + tex0->stride * i1);
        ptr1 = (float *)((char *)tex1->data + tex1->stride * i1);

        s1 = ptr0[0] * weight + ptr1[0] * (1.0 - weight);
        t1 = ptr0[1] * weight + ptr1[1] * (1.0 - weight);

        ptr0 = (float *)((char *)tex0->data + tex0->stride * i2);
        ptr1 = (float *)((char *)tex1->data + tex1->stride * i2);

        s2 = ptr0[0] * weight + ptr1[0] * (1.0 - weight);
        t2 = ptr0[1] * weight + ptr1[1] * (1.0 - weight);
     }

   data->s = s0 * (1 - data->u - data->v) + s1 * data->u + s2 * data->v;
   data->t = t0 * (1 - data->u - data->v) + t1 * data->u + t2 * data->v;
}


static inline Eina_Bool
_pick_data_mesh_add(Evas_Canvas3D_Pick_Data *data, const Evas_Ray3 *ray,
                    Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Node *node)
{
   Evas_Canvas3D_Vertex_Buffer   pos0, pos1, tex0, tex1;
   Evas_Real               pos_weight, tex_weight;
   Evas_Triangle3          tri;
   int                     i;

   memset(&pos0, 0x00, sizeof(Evas_Canvas3D_Vertex_Buffer));
   memset(&pos1, 0x00, sizeof(Evas_Canvas3D_Vertex_Buffer));
   memset(&tex0, 0x00, sizeof(Evas_Canvas3D_Vertex_Buffer));
   memset(&tex1, 0x00, sizeof(Evas_Canvas3D_Vertex_Buffer));

   evas_canvas3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION,
                                              &pos0, &pos1, &pos_weight);

   evas_canvas3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD,
                                              &tex0, &tex1, &tex_weight);
   Evas_Canvas3D_Mesh_Data *pdmesh = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);
   if (pdmesh->indices)
     {
        unsigned int i0, i1, i2;

        if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES)
          {
             for (i = 0; i < pdmesh->index_count; i += 3)
               {
                  if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    {
                       i0 = ((unsigned short *)pdmesh->indices)[i];
                       i1 = ((unsigned short *)pdmesh->indices)[i + 1];
                       i2 = ((unsigned short *)pdmesh->indices)[i + 2];
                    }
                  else
                    {
                       i0 = ((unsigned char *)pdmesh->indices)[i];
                       i1 = ((unsigned char *)pdmesh->indices)[i + 1];
                       i2 = ((unsigned char *)pdmesh->indices)[i + 2];
                    }

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, i0);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, i1);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                       return EINA_TRUE;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP)
          {
             if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
               {
                  i1 = ((unsigned short *)pdmesh->indices)[0];
                  i2 = ((unsigned short *)pdmesh->indices)[1];
               }
             else
               {
                  i1 = ((unsigned char *)pdmesh->indices)[0];
                  i2 = ((unsigned char *)pdmesh->indices)[1];
               }

             evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, i1);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

             for (i = 0; i < pdmesh->index_count - 2; i++)
               {
                  tri.p0 = tri.p1;
                  tri.p1 = tri.p2;

                  if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    i2 = ((unsigned short *)pdmesh->indices)[i + 2];
                  else
                    i2 = ((unsigned char *)pdmesh->indices)[i + 2];

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
                         {
                            i0 = ((unsigned short *)pdmesh->indices)[i];
                            i1 = ((unsigned short *)pdmesh->indices)[i + 1];
                         }
                       else
                         {
                            i0 = ((unsigned char *)pdmesh->indices)[i];
                            i1 = ((unsigned char *)pdmesh->indices)[i + 1];
                         }

                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                       return EINA_TRUE;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_FAN)
          {
             if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
               {
                  i0 = ((unsigned short *)pdmesh->indices)[0];
                  i2 = ((unsigned short *)pdmesh->indices)[1];
               }
             else
               {
                  i0 = ((unsigned char *)pdmesh->indices)[0];
                  i2 = ((unsigned char *)pdmesh->indices)[1];
               }

             evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, i0);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

             for (i = 1; i < pdmesh->index_count - 1; i++)
               {
                  tri.p1 = tri.p2;

                  if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    i2 = ((unsigned short *)pdmesh->indices)[i + 1];
                  else
                    i2 = ((unsigned char *)pdmesh->indices)[i + 1];

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT)
                         i1 = ((unsigned short *)pdmesh->indices)[i];
                       else
                         i1 = ((unsigned char *)pdmesh->indices)[i];

                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
     }
   else if (pdmesh->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_BYTE)
     {
        if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES)
          {
             for (i = 0; i < pdmesh->index_count; i += 3)
               {
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, i);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, i + 1);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP)
          {
             evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, 0);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 0; i < pdmesh->index_count - 2; i++)
               {
                  tri.p0 = tri.p1;
                  tri.p1 = tri.p2;

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_FAN)
          {
             evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, 0);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 1; i < pdmesh->index_count - 1; i++)
               {
                  tri.p1 = tri.p2;

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 1);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                      if (tex0.data)
                        _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, 0, i, i + 1);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
     }
   else if ((EINA_DBL_EQ(pdmesh->index_count, 0.0)) &&
            pdmesh->vertex_count != 0)
     {
        if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES)
          {
             for (i = 0; i < pdmesh->vertex_count; i += 3)
               {
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, i);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, i + 1);
                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP)
          {
             evas_canvas3d_mesh_interpolate_position_get(&tri.p1, &pos0, &pos1, pos_weight, 0);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 0; i < pdmesh->vertex_count - 2; i++)
               {
                  tri.p0 = tri.p1;
                  tri.p1 = tri.p2;

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                      if (tex0.data)
                        _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                       return EINA_TRUE;
                    }
               }
          }
        else if (pdmesh->assembly == EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLE_FAN)
          {
             evas_canvas3d_mesh_interpolate_position_get(&tri.p0, &pos0, &pos1, pos_weight, 0);
             evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 1; i < pdmesh->vertex_count - 1; i++)
               {
                  tri.p1 = tri.p2;

                  evas_canvas3d_mesh_interpolate_position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 1);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (tex0.data)
                         _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, 0, i, i + 1);
                       data->mesh = mesh;
                       data->node = node;
                       return EINA_TRUE;
                    }
               }
          }
     }
   return EINA_FALSE;
}

Eina_Bool
_node_pick(Evas_Canvas3D_Node *node, void *data)
{
   Evas_Ray3            ray;
   Evas_Canvas3D_Pick_Data   *pick = (Evas_Canvas3D_Pick_Data *)data;
   Eina_Matrix4            mvp;
   Evas_Canvas3D_Node_Data *pd_node = efl_data_scope_get(node, EVAS_CANVAS3D_NODE_CLASS);

   evas_canvas3d_node_tree_traverse(node, EVAS_CANVAS3D_TREE_TRAVERSE_POST_ORDER, EINA_FALSE,
                                 node_aabb_update, NULL);

   if (! evas_box3_ray3_intersect(&pd_node->aabb, &pick->ray_world))
     {
        /* Skip entire subtree. */
        return EINA_FALSE;
     }

   if (pd_node->type == EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        Eina_Iterator *itr;
        void          *ptr;

        /* Transform ray into local coordinate space. */
        eina_matrix4_multiply(&mvp, &pick->matrix_vp, &pd_node->data.mesh.matrix_local_to_world);
        evas_ray3_init(&ray, pick->x, pick->y, &mvp);

        itr = eina_hash_iterator_data_new(pd_node->data.mesh.node_meshes);

        while (eina_iterator_next(itr, &ptr))
          {
             Evas_Canvas3D_Node_Mesh *nm = (Evas_Canvas3D_Node_Mesh *)ptr;
             if(_pick_data_mesh_add(pick, &ray, nm->mesh, nm->frame, node))
               break;
          }
     }

   return EINA_TRUE;
}

static void _node_mesh_colors_free_cb(void *data)
{
   if (data) free(data);
}

static inline void
_pick_data_init(Evas_Canvas3D_Pick_Data *data, Evas_Public_Data *e, Evas_Real x, Evas_Real y)
{
   data->x      = ((x * 2.0) / ((Evas_Real)e->viewport.w)) - 1.0;
   data->y      = ((((Evas_Real)e->viewport.h - y - 1) * 2.0) / ((Evas_Real)e->viewport.h)) - 1.0;
   data->picked = EINA_FALSE;
   data->z      = 1.0;
   data->node   = NULL;
   data->mesh   = NULL;
   data->s      = 0.0;
   data->t      = 0.0;
}

EOLIAN static Eina_Bool
_evas_canvas3d_scene_pick(const Eo *obj, Evas_Canvas3D_Scene_Data *pd, Evas_Real x, Evas_Real y,
                    Evas_Canvas3D_Node **node, Evas_Canvas3D_Mesh **mesh,
                    Evas_Real *s, Evas_Real *t)
{
   Evas_Canvas3D_Pick_Data data;
   Evas_Canvas3D_Node_Data *pd_camera_node;
   Evas_Canvas3D_Camera_Data *pd_camera;
   Evas_Canvas3D_Object_Data *pd_parent;
   Evas_Public_Data *e;
   int tex = 0, px, py;;
   Evas_Color color = {0.0, 0.0, 0.0, 0.0};
   Eina_Stringshare *tmp;
   Eina_Array *arr = NULL;
   Eina_Bool update_scene = EINA_FALSE;
   Evas_Canvas3D_Node *picked_node = NULL;
   const Efl_Event_Description *eo_desc = NULL;

   pd_parent = efl_data_scope_get(obj, EVAS_CANVAS3D_OBJECT_CLASS);
   e = efl_data_scope_get(pd_parent->evas, EVAS_CANVAS_CLASS);

   _pick_data_init(&data, e, x, y);
   px = round(x * pd->w / e->viewport.w);
   py = round((pd->h - (y * pd->h / e->viewport.h) - 1));

   /*Use color pick mechanism finding node and mesh*/
   if (pd->color_pick_enabled)
     {
        Evas_Canvas3D_Scene_Public_Data scene_data;

        scene_data.bg_color = pd->bg_color;
        scene_data.shadows_enabled = pd->shadows_enabled;
        scene_data.camera_node = pd->camera_node;
        scene_data.color_pick_enabled = pd->color_pick_enabled;
        update_scene = evas_canvas3d_object_dirty_get(obj, EVAS_CANVAS3D_STATE_SCENE_UPDATED);
        scene_data.post_processing = EINA_FALSE;
        if (update_scene)
          {
             if (pd->node_mesh_colors)
               {
                  eina_hash_free(pd->node_mesh_colors);
                  eina_hash_free(pd->colors_node_mesh);
                  pd->node_mesh_colors = NULL;
                  pd->colors_node_mesh = NULL;
               }
             pd->node_mesh_colors = eina_hash_stringshared_new(_node_mesh_colors_free_cb);
             pd->colors_node_mesh = eina_hash_stringshared_new(_node_mesh_colors_free_cb);
          }
        scene_data.node_mesh_colors = pd->node_mesh_colors;
        scene_data.colors_node_mesh = pd->colors_node_mesh;
        evas_canvas3d_node_tree_traverse(pd->root_node,
                                   EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_TRUE,
                                   evas_canvas3d_node_color_node_mesh_collect, &scene_data);

        if (e->engine.func->drawable_scene_render_to_texture)
          {
             if (e->engine.func->drawable_scene_render_to_texture(_evas_engine_context(e),
                                                         pd->surface, &scene_data))
               {
                  if (e->engine.func->drawable_texture_color_pick_id_get)
                    tex = e->engine.func->drawable_texture_color_pick_id_get(pd->surface);
                  if (e->engine.func->drawable_texture_pixel_color_get)
                    {
                       e->engine.func->drawable_texture_pixel_color_get(tex, px, py, &color, pd->surface);
                       tmp = eina_stringshare_printf("%f %f %f", color.r, color.g, color.b);
                       arr = (Eina_Array *)eina_hash_find(pd->colors_node_mesh, tmp);
                       if (arr)
                         {
                            picked_node = (Evas_Canvas3D_Node *)eina_array_data_get(arr, 0);
                            if (mesh) *mesh = (Evas_Canvas3D_Mesh *)eina_array_data_get(arr, 1);
                            if (node) *node = picked_node;
                            eina_stringshare_del(tmp);

                            /*Calling callback clicked*/
                            eo_desc = efl_object_legacy_only_event_description_get("clicked,private");
                            efl_event_callback_legacy_call(picked_node, eo_desc, picked_node);

                            return EINA_TRUE;
                         }
                       else
                         {
                            eina_stringshare_del(tmp);
                            if (mesh) *mesh = NULL;
                            if (node) *node = NULL;
                         }
                    }
               }
          }
        return EINA_FALSE;
     }
   /* Update the scene graph. */
   evas_canvas3d_object_update((Eo *) obj);
   pd_camera_node = efl_data_scope_get(pd->camera_node, EVAS_CANVAS3D_NODE_CLASS);
   pd_camera = efl_data_scope_get(pd_camera_node->data.camera.camera, EVAS_CANVAS3D_CAMERA_CLASS);
   eina_matrix4_multiply(&data.matrix_vp,
                      &pd_camera->projection,
                      &pd_camera_node->data.camera.matrix_world_to_eye);

   evas_ray3_init(&data.ray_world, data.x, data.y, &data.matrix_vp);

   /* Traverse tree while adding meshes into pick data structure. */
   evas_canvas3d_node_tree_traverse(pd->root_node, EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_TRUE,
                              _node_pick, &data);

   if (!data.picked)
       return EINA_FALSE;

   if (s)     *s     = data.s;
   if (t)     *t     = data.t;
   if (node)  *node  = data.node;
   if (mesh)  *mesh  = data.mesh;

   /*Calling callback clicked*/
   eo_desc = efl_object_legacy_only_event_description_get("clicked,private");
   efl_event_callback_legacy_call(data.node, eo_desc, data.node);

   return EINA_TRUE;
}

EOLIAN static Evas_Canvas3D_Node *
_evas_canvas3d_scene_exist(const Eo *obj, Evas_Canvas3D_Scene_Data *pd, Evas_Real x, Evas_Real y, Evas_Canvas3D_Node *node)
{
   Evas_Canvas3D_Pick_Data data;
   Evas_Canvas3D_Node_Data *pd_camera_node;
   Evas_Canvas3D_Camera_Data *pd_camera;
   Evas_Canvas3D_Object_Data *pd_parent;
   Evas_Public_Data *e;

   pd_parent = efl_data_scope_get(obj, EVAS_CANVAS3D_OBJECT_CLASS);
   e = efl_data_scope_get(pd_parent->evas, EVAS_CANVAS_CLASS);

   _pick_data_init(&data, e, x, y);

   /* Update the scene graph. */
   evas_canvas3d_object_update((Eo *) obj);
   pd_camera_node = efl_data_scope_get(pd->camera_node, EVAS_CANVAS3D_NODE_CLASS);
   pd_camera = efl_data_scope_get(pd_camera_node->data.camera.camera, EVAS_CANVAS3D_CAMERA_CLASS);
   eina_matrix4_multiply(&data.matrix_vp,
                      &pd_camera->projection,
                      &pd_camera_node->data.camera.matrix_world_to_eye);

   evas_ray3_init(&data.ray_world, data.x, data.y, &data.matrix_vp);

   /* Check pick for given node. */
   _node_pick(node, &data);

   if (!data.picked || data.node != node) node = NULL;

   return node;
}

EOLIAN static Eina_List *
_evas_canvas3d_scene_pick_member_list_get(const Eo *obj, Evas_Canvas3D_Scene_Data *pd, Evas_Real x, Evas_Real y)
{
   const Eina_List *list = NULL, *l = NULL;
   Eina_List *picked_nodes = NULL;
   void *node;
   Eina_Bool pick = EINA_FALSE;

   /* Check pick for given scene. */
   pick = evas_canvas3d_scene_pick(obj, x, y, NULL, NULL, NULL, NULL);

   if (!pick)
     return NULL;

   /* Get all members from root node. */
   list = evas_canvas3d_node_member_list_get(pd->root_node);

   EINA_LIST_FOREACH(list, l, node)
     {
        if (evas_canvas3d_scene_exist(obj, x, y, node))
          picked_nodes = eina_list_append(picked_nodes, l);
     }

   return picked_nodes;
}

EOLIAN static Eina_Bool
_evas_canvas3d_scene_shadows_enable_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd)
{
   return pd->shadows_enabled;
}

EOLIAN static void
_evas_canvas3d_scene_shadows_enable_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd, Eina_Bool _shadows_enabled)
{
   pd->shadows_enabled = _shadows_enabled;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_SHADOWS_ENABLED, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas3d_scene_color_pick_enable_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd)
{
   return pd->color_pick_enabled;
}

EOLIAN static Eina_Bool
_evas_canvas3d_scene_color_pick_enable_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd, Eina_Bool _enabled)
{
   if (pd->color_pick_enabled != _enabled)
     pd->color_pick_enabled = _enabled;

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_UPDATED, NULL);
   return EINA_TRUE;
}

EOLIAN static void
_evas_canvas3d_scene_shadows_depth_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd,
                                       Evas_Real depth_offset, Evas_Real depth_constant)
{
   pd->depth_offset = depth_offset;
   pd->depth_constant = depth_constant;
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_SCENE_SHADOWS_DEPTH, NULL);
}

EOLIAN static void
_evas_canvas3d_scene_shadows_depth_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Scene_Data *pd,
                                       Evas_Real *depth_offset, Evas_Real *depth_constant)
{
   if (depth_offset) *depth_offset = pd->depth_offset;
   if (depth_constant) *depth_constant = pd->depth_constant;
}

#include "canvas/evas_canvas3d_scene.eo.c"
