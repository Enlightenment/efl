#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

static Evas_3D_Mesh_Frame *
evas_3d_mesh_frame_new(Evas_3D_Mesh *mesh)
{
   Evas_3D_Mesh_Frame *frame = NULL;

   frame = (Evas_3D_Mesh_Frame *)calloc(1, sizeof(Evas_3D_Mesh_Frame));

   if (frame == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   frame->mesh = mesh;
   evas_box3_empty_set(&frame->aabb);

   return frame;
}

static void
evas_3d_mesh_frame_free(Evas_3D_Mesh_Frame *frame)
{
   int i;

   if (frame->material)
     {
        evas_3d_material_mesh_del(frame->material, frame->mesh);
        evas_3d_object_unreference(&frame->material->base);
     }

   for (i = 0; i < EVAS_3D_VERTEX_ATTRIB_COUNT; i++)
     {
        if (frame->vertices[i].owns_data)
          free(frame->vertices[i].data);
     }

   free(frame);
}

static Evas_3D_Mesh_Frame *
evas_3d_mesh_frame_find(Evas_3D_Mesh *mesh, int frame)
{
   Eina_List *l;
   Evas_3D_Mesh_Frame *f;

   EINA_LIST_FOREACH(mesh->frames, l, f)
     {
        if (f->frame == frame)
          return f;
     }

   return NULL;
}

static inline void
_mesh_init(Evas_3D_Mesh *mesh)
{
   mesh->vertex_count = 0;
   mesh->frame_count = 0;
   mesh->frames = NULL;

   mesh->index_format = EVAS_3D_INDEX_FORMAT_NONE;
   mesh->index_count = 0;
   mesh->indices = NULL;
   mesh->owns_indices = EINA_FALSE;
   mesh->assembly = EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES;

   mesh->nodes = NULL;
}

static inline void
_mesh_fini(Evas_3D_Mesh *mesh)
{
   Eina_List           *l;
   Evas_3D_Mesh_Frame  *f;

   if (mesh->frames)
     {
        EINA_LIST_FOREACH(mesh->frames, l, f)
          {
             evas_3d_mesh_frame_free(f);
          }

        eina_list_free(mesh->frames);
     }

   if (mesh->indices && mesh->owns_indices)
     free(mesh->indices);

   if (mesh->nodes)
     eina_hash_free(mesh->nodes);
}

static void
_mesh_free(Evas_3D_Object *obj)
{
   Evas_3D_Mesh *mesh = (Evas_3D_Mesh *)obj;
   _mesh_fini(mesh);
   free(mesh);
}

static Eina_Bool
_mesh_node_geometry_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                  void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   evas_3d_object_change(&n->base, EVAS_3D_STATE_NODE_MESH_GEOMETRY, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static Eina_Bool
_mesh_node_material_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                  void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   evas_3d_object_change(&n->base, EVAS_3D_STATE_NODE_MESH_MATERIAL, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_mesh_change(Evas_3D_Object *obj, Evas_3D_State state, Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Mesh *mesh = (Evas_3D_Mesh *)obj;

   if (state == EVAS_3D_STATE_MESH_MATERIAL)
     {
        if (mesh->nodes)
          eina_hash_foreach(mesh->nodes, _mesh_node_material_change_notify, obj);
     }
   else
     {
        if (mesh->nodes)
          eina_hash_foreach(mesh->nodes, _mesh_node_geometry_change_notify, obj);
     }
}

static void
_mesh_update(Evas_3D_Object *obj)
{
   Eina_List *l;
   Evas_3D_Mesh_Frame *f;
   Evas_3D_Mesh *mesh = (Evas_3D_Mesh *)obj;

   EINA_LIST_FOREACH(mesh->frames, l, f)
     {
        if (f->material)
          evas_3d_object_update(&f->material->base);
     }
}

static const Evas_3D_Object_Func mesh_func =
{
   _mesh_free,
   _mesh_change,
   _mesh_update,
};

void
evas_3d_mesh_node_add(Evas_3D_Mesh *mesh, Evas_3D_Node *node)
{
   int count = 0;

   if (mesh->nodes == NULL)
     {
        mesh->nodes = eina_hash_pointer_new(NULL);

        if (mesh->nodes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(mesh->nodes, &node);

   eina_hash_set(mesh->nodes, &node, (const void *)(count + 1));
}

void
evas_3d_mesh_node_del(Evas_3D_Mesh *mesh, Evas_3D_Node *node)
{
   int count = 0;

   if (mesh->nodes == NULL)
     {
        ERR("No node to delete.");
        return;
     }

   count = (int)eina_hash_find(mesh->nodes, &node);

   if (count == 1)
     eina_hash_del(mesh->nodes, &node, NULL);
   else
     eina_hash_set(mesh->nodes, &node, (const void *)(count - 1));
}

Evas_3D_Mesh *
evas_3d_mesh_new(Evas *e)
{
   Evas_3D_Mesh *mesh = NULL;

   mesh = (Evas_3D_Mesh *)malloc(sizeof(Evas_3D_Mesh));

   if (mesh == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&mesh->base, e, EVAS_3D_OBJECT_TYPE_MESH, &mesh_func);
   _mesh_init(mesh);
   return mesh;
}

EAPI Evas_3D_Mesh *
evas_3d_mesh_add(Evas *e)
{
   return evas_3d_mesh_new(e);
}

EAPI void
evas_3d_mesh_del(Evas_3D_Mesh *mesh)
{
   evas_3d_object_unreference(&mesh->base);
}

EAPI Evas *
evas_3d_mesh_evas_get(const Evas_3D_Mesh *mesh)
{
   return mesh->base.evas;
}

EAPI void
evas_3d_mesh_shade_mode_set(Evas_3D_Mesh *mesh, Evas_3D_Shade_Mode mode)
{
   if (mesh->shade_mode != mode)
     {
        mesh->shade_mode = mode;
        evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_SHADE_MODE, NULL);
     }
}

EAPI Evas_3D_Shade_Mode
evas_3d_mesh_shade_mode_get(const Evas_3D_Mesh *mesh)
{
   return mesh->shade_mode;
}

EAPI void
evas_3d_mesh_vertex_count_set(Evas_3D_Mesh *mesh, unsigned int count)
{
   mesh->vertex_count = count;
   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_VERTEX_COUNT, NULL);
}

EAPI int
evas_3d_mesh_vertex_count_get(const Evas_3D_Mesh *mesh)
{
   return mesh->vertex_count;
}

EAPI void
evas_3d_mesh_frame_add(Evas_3D_Mesh *mesh, int frame)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);

   if (f != NULL)
     {
        ERR("Already existing frame.");
        return;
     }

   f = evas_3d_mesh_frame_new(mesh);

   if (f == NULL)
     return;

   f->frame = frame;
   mesh->frames = eina_list_append(mesh->frames, f);
   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_FRAME, NULL);
}

EAPI void
evas_3d_mesh_frame_del(Evas_3D_Mesh *mesh, int frame)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return;
     }

   mesh->frames = eina_list_remove(mesh->frames, f);
   evas_3d_mesh_frame_free(f);
   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_FRAME, NULL);
}

EAPI void
evas_3d_mesh_frame_material_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Material *material)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return;
     }

   if (f->material == material)
     return;

   if (f->material)
     {
        evas_3d_material_mesh_del(f->material, mesh);
        evas_3d_object_unreference(&f->material->base);
     }

   f->material = material;
   evas_3d_object_reference(&material->base);
   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_MATERIAL, NULL);
   evas_3d_material_mesh_add(material, mesh);
}

EAPI Evas_3D_Material *
evas_3d_mesh_frame_material_get(const Evas_3D_Mesh *mesh, int frame)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find((Evas_3D_Mesh *)mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return NULL;
     }

   return f->material;
}

EAPI void
evas_3d_mesh_frame_vertex_data_set(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib,
                                   int stride, const void *data)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);
   int element_count;

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return;
     }

   if (attrib == EVAS_3D_VERTEX_POSITION)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_NORMAL)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_TANGENT)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_COLOR)
     {
        element_count = 4;
     }
   else if (attrib == EVAS_3D_VERTEX_TEXCOORD)
     {
        element_count = 2;
     }
   else
     {
        ERR("Invalid vertex attrib.");
        return;
     }

   if (f->vertices[attrib].owns_data && f->vertices[attrib].data)
     free(f->vertices[attrib].data);

   f->vertices[attrib].size = 0;
   f->vertices[attrib].stride = stride;
   f->vertices[attrib].data = (void *)data;
   f->vertices[attrib].owns_data = EINA_FALSE;
   f->vertices[attrib].element_count = element_count;

   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_VERTEX_DATA, NULL);
}

EAPI void
evas_3d_mesh_frame_vertex_data_copy_set(Evas_3D_Mesh *mesh, int frame,
                                        Evas_3D_Vertex_Attrib attrib,
                                        int stride, const void *data)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);
   Evas_3D_Vertex_Buffer *vb;
   int size, element_count;

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return;
     }

   if (attrib == EVAS_3D_VERTEX_POSITION)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_NORMAL)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_TANGENT)
     {
        element_count = 3;
     }
   else if (attrib == EVAS_3D_VERTEX_COLOR)
     {
        element_count = 4;
     }
   else if (attrib == EVAS_3D_VERTEX_TEXCOORD)
     {
        element_count = 2;
     }
   else
     {
        ERR("Invalid vertex attrib.");
        return;
     }

   vb = &f->vertices[attrib];
   size = element_count * sizeof(float) * mesh->vertex_count;

   if (!vb->owns_data || vb->size < size)
     {
        if (vb->owns_data && vb->data)
          free(vb->data);

        vb->data = malloc(size);

        if (vb->data == NULL)
          {
             vb->element_count = 0;
             vb->size = 0;
             vb->stride = 0;
             vb->owns_data = EINA_FALSE;

             ERR("Failed to allocate memory.");
             return;
          }

        vb->size = size;
        vb->owns_data = EINA_TRUE;
     }

   vb->element_count = element_count;
   vb->stride = 0;

   if (data == NULL)
     return;

   if (stride == 0 || stride == (int)(element_count * sizeof(float)))
     {
        memcpy(vb->data, data, size);
     }
   else
     {
        int    i;
        float *dst = (float *)vb->data;
        float *src = (float *)data;

        if (element_count == 1)
          {
             for (i = 0; i <mesh->vertex_count; i++)
               {
                  *dst++ = src[0];

                  src = (float *)((char *)src + stride);
               }
          }
        else if (element_count == 2)
          {
             for (i = 0; i <mesh->vertex_count; i++)
               {
                  *dst++ = src[0];
                  *dst++ = src[1];

                  src = (float *)((char *)src + stride);
               }
          }
        else if (element_count == 3)
          {
             for (i = 0; i <mesh->vertex_count; i++)
               {
                  *dst++ = src[0];
                  *dst++ = src[1];
                  *dst++ = src[2];

                  src = (float *)((char *)src + stride);
               }
          }
        else if (element_count == 4)
          {
             for (i = 0; i <mesh->vertex_count; i++)
               {
                  *dst++ = src[0];
                  *dst++ = src[1];
                  *dst++ = src[2];
                  *dst++ = src[3];

                  src = (float *)((char *)src + stride);
               }
          }
     }

   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_VERTEX_DATA, NULL);
}

EAPI void *
evas_3d_mesh_frame_vertex_data_map(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return NULL;
     }

   if (f->vertices[attrib].mapped)
     {
        ERR("Try to map alreadly mapped data.");
        return NULL;
     }

   f->vertices[attrib].mapped = EINA_TRUE;
   return f->vertices[attrib].data;
}

EAPI void
evas_3d_mesh_frame_vertex_data_unmap(Evas_3D_Mesh *mesh, int frame, Evas_3D_Vertex_Attrib attrib)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find(mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return;
     }

   if (!f->vertices[attrib].mapped)
     {
        ERR("Try to unmap data which is not mapped yet.");
        return;
     }

   f->vertices[attrib].mapped = EINA_FALSE;
}

EAPI int
evas_3d_mesh_frame_vertex_stride_get(const Evas_3D_Mesh *mesh, int frame,
                                     Evas_3D_Vertex_Attrib attrib)
{
   Evas_3D_Mesh_Frame *f = evas_3d_mesh_frame_find((Evas_3D_Mesh *)mesh, frame);

   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        return 0;
     }

   return f->vertices[attrib].stride;
}

EAPI void
evas_3d_mesh_index_data_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count,
                            const void *indices)
{
   if (mesh->owns_indices && mesh->indices)
     free(mesh->indices);

   mesh->index_format = format;
   mesh->index_count = count;
   mesh->index_size = 0;
   mesh->indices = (void *)indices;
   mesh->owns_indices = EINA_FALSE;

   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_INDEX_DATA, NULL);
}

EAPI void
evas_3d_mesh_index_data_copy_set(Evas_3D_Mesh *mesh, Evas_3D_Index_Format format, int count, const void *indices)
{
   int size;

   if (format == EVAS_3D_INDEX_FORMAT_UNSIGNED_BYTE)
     {
        size = count * sizeof(unsigned char);
     }
   else if (format == EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT)
     {
        size = count * sizeof(unsigned short);
     }
   else
     {
        ERR("Invalid index format.");
        return;
     }

   if (!mesh->owns_indices || mesh->index_size < size)
     {
        if (mesh->owns_indices && mesh->indices)
          free(mesh->indices);

        mesh->indices = malloc(size);

        if (mesh->indices == NULL)
          {
             ERR("Failed to allocate memory.");
             return;
          }

        mesh->index_size = size;
        mesh->owns_indices = EINA_TRUE;
     }

   mesh->index_format = format;
   mesh->index_count = count;

   if (indices)
     memcpy(mesh->indices, indices, size);
}

EAPI Evas_3D_Index_Format
evas_3d_mesh_index_format_get(const Evas_3D_Mesh *mesh)
{
   return mesh->index_format;
}

EAPI int
evas_3d_mesh_index_count_get(const Evas_3D_Mesh *mesh)
{
   return mesh->index_count;
}

EAPI void *
evas_3d_mesh_index_data_map(Evas_3D_Mesh *mesh)
{
   if (mesh->index_mapped)
     {
        ERR("Try to map alreadly mapped data.");
        return NULL;
     }

   mesh->index_mapped = EINA_TRUE;
   return mesh->indices;
}

EAPI void
evas_3d_mesh_index_data_unmap(Evas_3D_Mesh *mesh)
{
   if (!mesh->index_mapped)
     {
        ERR("Try to unmap data which is not mapped yet.");
        return;
     }

   mesh->index_mapped = EINA_FALSE;
}

EAPI void
evas_3d_mesh_vertex_assembly_set(Evas_3D_Mesh *mesh, Evas_3D_Vertex_Assembly assembly)
{
   mesh->assembly = assembly;
   evas_3d_object_change(&mesh->base, EVAS_3D_STATE_MESH_VERTEX_ASSEMBLY, NULL);
}

EAPI Evas_3D_Vertex_Assembly
evas_3d_mesh_vertex_assembly_get(const Evas_3D_Mesh *mesh)
{
   return mesh->assembly;
}

EAPI void
evas_3d_mesh_file_set(Evas_3D_Mesh *mesh, Evas_3D_Mesh_File_Type type,
                      const char *file, const char *key EINA_UNUSED)
{
   _mesh_fini(mesh);
   _mesh_init(mesh);

   if (file == NULL)
     return;

   switch (type)
     {
      case EVAS_3D_MESH_FILE_TYPE_MD2:
         evas_3d_mesh_file_md2_set(mesh, file);
         break;
      default:
         ERR("Invalid mesh file type.");
         break;
     }
}

static inline void
_mesh_frame_find(Evas_3D_Mesh *mesh, int frame,
                 Eina_List **l, Eina_List **r)
{
   Eina_List *left, *right;
   Evas_3D_Mesh_Frame *f0, *f1;

   left = mesh->frames;
   right = eina_list_next(left);

   while (right)
     {
        f0 = (Evas_3D_Mesh_Frame *)eina_list_data_get(left);
        f1 = (Evas_3D_Mesh_Frame *)eina_list_data_get(right);

        if (frame >= f0->frame && frame <= f1->frame)
          break;

        left = right;
        right = eina_list_next(left);
     }

   if (right == NULL)
     {
        if (frame <= f0->frame)
          {
             *l = NULL;
             *r = left;
          }
        else
          {
             *l = left;
             *r = NULL;
          }
     }

   *l = left;
   *r = right;
}

void
evas_3d_mesh_interpolate_vertex_buffer_get(Evas_3D_Mesh *mesh, int frame,
                                           Evas_3D_Vertex_Attrib  attrib,
                                           Evas_3D_Vertex_Buffer *buf0,
                                           Evas_3D_Vertex_Buffer *buf1,
                                           Evas_Real             *weight)
{
   Eina_List *l, *r;
   const Evas_3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   _mesh_frame_find(mesh, frame, &l, &r);

   while (l)
     {
        f0 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->vertices[attrib].data != NULL)
          break;

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->vertices[attrib].data != NULL)
          break;

        r = eina_list_next(r);
        f1 = NULL;
     }

   if (f0 == NULL && f1 == NULL)
     return;

   if (f0 == NULL)
     {
        f0 = f1;
     }
   else if (f1 != NULL)
     {
        if (frame == f0->frame)
          {
             f1 = NULL;
          }
        else if (frame == f1->frame)
          {
             f0 = f1;
             f1 = NULL;
          }
     }

   buf0->data = f0->vertices[attrib].data;
   buf0->stride = f0->vertices[attrib].stride;
   buf0->size = f0->vertices[attrib].size;

   if (f1)
     {
        buf1->data = f1->vertices[attrib].data;
        buf1->stride = f1->vertices[attrib].stride;
        buf1->size = f1->vertices[attrib].size;

        *weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);
     }
   else
     {
        buf1->data = NULL;
        buf1->stride = 0;
        buf1->size = 0;

        *weight = 1.0;
     }
}
