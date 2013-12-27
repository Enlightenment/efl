#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

void
evas_3d_scene_data_init(Evas_3D_Scene_Data *data)
{
   data->camera_node = NULL;
   data->light_nodes = NULL;
   data->mesh_nodes = NULL;
}

void
evas_3d_scene_data_fini(Evas_3D_Scene_Data *data)
{
   if (data->light_nodes)
     eina_list_free(data->light_nodes);

   if (data->mesh_nodes)
     eina_list_free(data->mesh_nodes);
}

static void
_scene_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED, Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Scene *scene = (Evas_3D_Scene *)obj;
   Eina_List *l;
   Evas_Object *eo;

   EINA_LIST_FOREACH(scene->images, l, eo)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo, EVAS_OBJ_CLASS);
        evas_object_change(eo, obj);
     }
}

static void
_scene_update(Evas_3D_Object *obj)
{
   Evas_3D_Scene *scene = (Evas_3D_Scene *)obj;

   if (scene->root_node)
     evas_3d_object_update(&scene->root_node->base);

   if (scene->camera_node)
     evas_3d_object_update(&scene->camera_node->base);
}

static void
_scene_free(Evas_3D_Object *obj)
{
   Evas_3D_Scene *scene = (Evas_3D_Scene *)obj;

   if (scene->root_node)
     {
        evas_3d_node_scene_root_del(scene->root_node, scene);
        evas_3d_object_unreference(&scene->root_node->base);
     }

   if (scene->camera_node)
     {
        evas_3d_node_scene_camera_del(scene->camera_node, scene);
        evas_3d_object_unreference(&scene->camera_node->base);
     }

   if (scene->images)
     eina_list_free(scene->images);

   free(scene);
}

static const Evas_3D_Object_Func scene_func =
{
   _scene_free,
   _scene_change,
   _scene_update,
};

Evas_3D_Scene *
evas_3d_scene_new(Evas *e)
{
   Evas_3D_Scene *scene = NULL;

   scene = (Evas_3D_Scene *)calloc(1, sizeof(Evas_3D_Scene));

   if (scene == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&scene->base, e, EVAS_3D_OBJECT_TYPE_SCENE, &scene_func);
   evas_color_set(&scene->bg_color, 0.0, 0.0, 0.0, 0.0);

   return scene;
}

EAPI Evas_3D_Scene *
evas_3d_scene_add(Evas *e)
{
   return evas_3d_scene_new(e);
}

EAPI void
evas_3d_scene_del(Evas_3D_Scene *scene)
{
   evas_3d_object_unreference(&scene->base);
}

EAPI Evas *
evas_3d_scene_evas_get(const Evas_3D_Scene *scene)
{
   return scene->base.evas;
}

EAPI void
evas_3d_scene_root_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node)
{
   if (scene->root_node == node)
     return;

   if (scene->root_node)
     {
        evas_3d_node_scene_root_del(scene->root_node, scene);
        evas_3d_object_unreference(&scene->root_node->base);
     }

   scene->root_node = node;

   if (node)
     {
        evas_3d_object_reference(&node->base);
        evas_3d_node_scene_root_add(node, scene);
     }

   evas_3d_object_change(&scene->base, EVAS_3D_STATE_SCENE_ROOT_NODE, NULL);
}

EAPI Evas_3D_Node *
evas_3d_scene_root_node_get(const Evas_3D_Scene *scene)
{
   return scene->root_node;
}

EAPI void
evas_3d_scene_camera_node_set(Evas_3D_Scene *scene, Evas_3D_Node *node)
{
   if (scene->camera_node == node)
     return;

   if (scene->camera_node)
     {
        evas_3d_node_scene_camera_del(scene->camera_node, scene);
        evas_3d_object_unreference(&scene->camera_node->base);
     }

   scene->camera_node = node;

   if (node)
     {
        evas_3d_object_reference(&node->base);
        evas_3d_node_scene_camera_add(node, scene);
     }

   evas_3d_object_change(&scene->base, EVAS_3D_STATE_SCENE_CAMERA_NODE, NULL);
}

EAPI Evas_3D_Node *
evas_3d_scene_camera_node_get(const Evas_3D_Scene *scene)
{
   return scene->camera_node;
}

EAPI void
evas_3d_scene_size_set(Evas_3D_Scene *scene, int w, int h)
{
   scene->w = w;
   scene->h = h;
   evas_3d_object_change(&scene->base, EVAS_3D_STATE_SCENE_SIZE, NULL);
}

EAPI void
evas_3d_scene_size_get(const Evas_3D_Scene *scene, int *w, int *h)
{
   if (w) *w = scene->w;
   if (h) *h = scene->h;
}

EAPI void
evas_3d_scene_background_color_set(Evas_3D_Scene *scene,
                                   Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   evas_color_set(&scene->bg_color, r, g, b, a);
   evas_3d_object_change(&scene->base, EVAS_3D_STATE_SCENE_BACKGROUND_COLOR, NULL);
}

EAPI void
evas_3d_scene_background_color_get(const Evas_3D_Scene *scene,
                                   Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = scene->bg_color.r;
   if (g) *g = scene->bg_color.g;
   if (b) *b = scene->bg_color.b;
   if (a) *a = scene->bg_color.a;
}

static inline Eina_Bool
_pick_data_triangle_add(Evas_3D_Pick_Data *data, const Evas_Ray3 *ray,
                        const Evas_Triangle3 *tri)
{
   Evas_Vec3   e1, e2, tvec, pvec, qvec;
   Evas_Real   det, inv_det, u, v, t;

   evas_vec3_subtract(&e1, &tri->p1, &tri->p0);
   evas_vec3_subtract(&e2, &tri->p2, &tri->p0);

   evas_vec3_cross_product(&pvec, &ray->dir, &e2);
   det = evas_vec3_dot_product(&e1, &pvec);

   /* If determinant is near zero, ray lies in plane of triangle. */
   if (det > -0.0000001 && det < 0.0000001)
     return EINA_FALSE;

   inv_det = 1.0 / det;

   /* Calculate distance from p0 to ray origin. */
   evas_vec3_subtract(&tvec, &ray->org, &tri->p0);

   /* Calculate U parameter and test bounds. */
   u = evas_vec3_dot_product(&tvec, &pvec) * inv_det;

   if (u < 0.0 || u > 1.0)
     return EINA_FALSE;

   /* Prepare to tst V parameter. */
   evas_vec3_cross_product(&qvec, &tvec, &e1);

   /* Calculate V parameter and test bounds. */
   v = evas_vec3_dot_product(&ray->dir, &qvec) * inv_det;

   if (v < 0.0 || u + v > 1.0)
     return EINA_FALSE;

   /* Calculate T parameter and test bounds. */
   t = evas_vec3_dot_product(&e2, &qvec) * inv_det;

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
_position_get(Evas_Vec3 *out, const Evas_3D_Vertex_Buffer *pos0, const Evas_3D_Vertex_Buffer *pos1,
              Evas_Real weight, int index)
{
   if (pos1->data == NULL)
     {
        float *ptr = (float *)((char *)pos0->data + pos0->stride * index);

        out->x = ptr[0];
        out->y = ptr[1];
        out->z = ptr[2];
     }
   else
     {
        float *ptr0, *ptr1;

        ptr0 = (float *)((char *)pos0->data + pos0->stride * index);
        ptr1 = (float *)((char *)pos1->data + pos1->stride * index);

        out->x = ptr0[0] * weight + ptr1[0] * (1.0 - weight);
        out->y = ptr0[1] * weight + ptr1[1] * (1.0 - weight);
        out->z = ptr0[2] * weight + ptr1[2] * (1.0 - weight);
     }
}

static inline void
_pick_data_texcoord_update(Evas_3D_Pick_Data *data,
                           const Evas_3D_Vertex_Buffer *tex0, const Evas_3D_Vertex_Buffer *tex1,
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


static inline void
_pick_data_mesh_add(Evas_3D_Pick_Data *data, const Evas_Ray3 *ray,
                    Evas_3D_Mesh *mesh, int frame, Evas_3D_Node *node)
{
   Evas_3D_Vertex_Buffer   pos0, pos1, tex0, tex1;
   Evas_Real               pos_weight, tex_weight;
   Evas_Triangle3          tri;
   int                     i;

   memset(&pos0, 0x00, sizeof(Evas_3D_Vertex_Buffer));
   memset(&pos1, 0x00, sizeof(Evas_3D_Vertex_Buffer));
   memset(&tex0, 0x00, sizeof(Evas_3D_Vertex_Buffer));
   memset(&tex1, 0x00, sizeof(Evas_3D_Vertex_Buffer));

   evas_3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_3D_VERTEX_POSITION,
                                              &pos0, &pos1, &pos_weight);

   evas_3d_mesh_interpolate_vertex_buffer_get(mesh, frame, EVAS_3D_VERTEX_TEXCOORD,
                                              &tex0, &tex1, &tex_weight);

   if (mesh->indices)
     {
        unsigned int i0, i1, i2;

        if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES)
          {
             for (i = 0; i < mesh->index_count; i += 3)
               {
                  if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    {
                       i0 = ((unsigned short *)mesh->indices)[i];
                       i1 = ((unsigned short *)mesh->indices)[i + 1];
                       i2 = ((unsigned short *)mesh->indices)[i + 2];
                    }
                  else
                    {
                       i0 = ((unsigned char *)mesh->indices)[i];
                       i1 = ((unsigned char *)mesh->indices)[i + 1];
                       i2 = ((unsigned char *)mesh->indices)[i + 2];
                    }

                  _position_get(&tri.p0, &pos0, &pos1, pos_weight, i0);
                  _position_get(&tri.p1, &pos0, &pos1, pos_weight, i1);
                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP)
          {
             if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
               {
                  i1 = ((unsigned short *)mesh->indices)[0];
                  i2 = ((unsigned short *)mesh->indices)[1];
               }
             else
               {
                  i1 = ((unsigned char *)mesh->indices)[0];
                  i2 = ((unsigned char *)mesh->indices)[1];
               }

             _position_get(&tri.p1, &pos0, &pos1, pos_weight, i1);
             _position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

             for (i = 0; i < mesh->index_count - 2; i++)
               {
                  tri.p0 = tri.p1;
                  tri.p1 = tri.p2;

                  if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    i2 = ((unsigned short *)mesh->indices)[i + 2];
                  else
                    i2 = ((unsigned char *)mesh->indices)[i + 2];

                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
                         {
                            i0 = ((unsigned short *)mesh->indices)[i];
                            i1 = ((unsigned short *)mesh->indices)[i + 1];
                         }
                       else
                         {
                            i0 = ((unsigned char *)mesh->indices)[i];
                            i1 = ((unsigned char *)mesh->indices)[i + 1];
                         }

                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN)
          {
             if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
               {
                  i0 = ((unsigned short *)mesh->indices)[0];
                  i2 = ((unsigned short *)mesh->indices)[1];
               }
             else
               {
                  i0 = ((unsigned char *)mesh->indices)[0];
                  i2 = ((unsigned char *)mesh->indices)[1];
               }

             _position_get(&tri.p0, &pos0, &pos1, pos_weight, i0);
             _position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

             for (i = 1; i < mesh->index_count - 1; i++)
               {
                  tri.p1 = tri.p2;

                  if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
                    i2 = ((unsigned short *)mesh->indices)[i + 1];
                  else
                    i2 = ((unsigned char *)mesh->indices)[i + 1];

                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
                         i1 = ((unsigned short *)mesh->indices)[i];
                       else
                         i1 = ((unsigned char *)mesh->indices)[i];

                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i0, i1, i2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
     }
   else if (mesh->index_format == EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE)
     {
        if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES)
          {
             for (i = 0; i < mesh->index_count; i += 3)
               {
                  _position_get(&tri.p0, &pos0, &pos1, pos_weight, i);
                  _position_get(&tri.p1, &pos0, &pos1, pos_weight, i + 1);
                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_STRIP)
          {
             _position_get(&tri.p1, &pos0, &pos1, pos_weight, 0);
             _position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 0; i < mesh->index_count - 2; i++)
               {
                  tri.p0 = tri.p1;
                  tri.p1 = tri.p2;

                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 2);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, i, i + 1, i + 2);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
        else if (mesh->assembly == EVAS_3D_VERTEX_ASSEMBLY_TRIANGLE_FAN)
          {
             _position_get(&tri.p0, &pos0, &pos1, pos_weight, 0);
             _position_get(&tri.p2, &pos0, &pos1, pos_weight, 1);

             for (i = 1; i < mesh->index_count - 1; i++)
               {
                  tri.p1 = tri.p2;

                  _position_get(&tri.p2, &pos0, &pos1, pos_weight, i + 1);

                  if (_pick_data_triangle_add(data, ray, &tri))
                    {
                       _pick_data_texcoord_update(data, &tex0, &tex1, tex_weight, 0, i, i + 1);
                       data->mesh = mesh;
                       data->node = node;
                    }
               }
          }
     }
}

Eina_Bool
_node_pick(Evas_3D_Node *node, void *data)
{
   Evas_Ray3            ray;
   Evas_3D_Pick_Data   *pick = (Evas_3D_Pick_Data *)data;
   Evas_Mat4            mvp;

   if (! evas_box3_ray3_intersect(&node->aabb, &pick->ray_world))
     {
        /* Skip entire subtree. */
        return EINA_FALSE;
     }

   if (node->type == EVAS_3D_NODE_TYPE_MESH)
     {
        Eina_Iterator *itr;
        void          *ptr;

        /* Transform ray into local coordinate space. */
        evas_mat4_multiply(&mvp, &pick->matrix_vp, &node->data.mesh.matrix_local_to_world);
        evas_ray3_init(&ray, pick->x, pick->y, &mvp);

        itr = eina_hash_iterator_data_new(node->data.mesh.node_meshes);

        while (eina_iterator_next(itr, &ptr))
          {
             Evas_3D_Node_Mesh *nm = (Evas_3D_Node_Mesh *)ptr;
             _pick_data_mesh_add(pick, &ray, nm->mesh, nm->frame, node);
          }
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
evas_3d_scene_pick(const Evas_3D_Scene *scene, Evas_Real x, Evas_Real y,
                   Evas_3D_Node **node, Evas_3D_Mesh **mesh,
                   Evas_Real *s, Evas_Real *t)
{
   /* TODO: Use H/W picking if availabe. */
   Evas_3D_Pick_Data data;

   data.x      = ((x * 2.0) / (Evas_Real)scene->w) - 1.0;
   data.y      = (((scene->h - y - 1) * 2.0) / ((Evas_Real)scene->h)) - 1.0;

   data.picked = EINA_FALSE;
   data.z      = 1.0;
   data.node   = NULL;
   data.mesh   = NULL;
   data.s      = 0.0;
   data.t      = 0.0;

   /* Update the scene graph. */
   evas_3d_object_update((Evas_3D_Object *)&scene->base);

   evas_mat4_multiply(&data.matrix_vp,
                      &scene->camera_node->data.camera.camera->projection,
                      &scene->camera_node->data.camera.matrix_world_to_eye);

   evas_ray3_init(&data.ray_world, data.x, data.y, &data.matrix_vp);


   /* Traverse tree while adding meshes into pick data structure. */
   evas_3d_node_tree_traverse(scene->root_node, EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_TRUE,
                              _node_pick, &data);

   if (!data.picked)
       return EINA_FALSE;

   if (s)     *s     = data.s;
   if (t)     *t     = data.t;
   if (node)  *node  = data.node;
   if (mesh)  *mesh  = data.mesh;

   return EINA_TRUE;
}
