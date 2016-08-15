#include "evas_model_load_save_common.h"

# define SAVE_MESH_INDICES_COPY                                           \
   if (header.indices_count)                                              \
     {                                                                    \
        data->indices = malloc(header.indices_count                       \
                               * sizeof(unsigned short));                 \
        if (pd->index_format == EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_BYTE) \
          {                                                               \
             for (i = 0; i < header.indices_count; i++)                   \
               data->indices[i] = ((unsigned char*)(pd->indices))[i];     \
          }                                                               \
        else                                                              \
          {                                                               \
             for (i = 0; i < header.indices_count; i++)                   \
               data->indices[i] = ((unsigned short*)(pd->indices))[i];    \
          }                                                               \
     }                                                                    \
   else                                                                   \
     {                                                                    \
        data->indices = malloc(header.vertices_count * 3                  \
                               * sizeof(unsigned short));                 \
        for (i = 0; i < header.vertices_count * 3; i++)                   \
          data->indices[i] = (unsigned short)i;                           \
     }

/* create new header */
Evas_Model_Load_Save_Header
evas_model_load_save_header_new()
{
   Evas_Model_Load_Save_Header header;

   header.vertices_count = 0;
   header.indices_count = 0;
   header.existence_of_positions = EINA_FALSE;
   header.existence_of_normals = EINA_FALSE;
   header.existence_of_tex_coords = EINA_FALSE;
   header.existence_of_colors = EINA_FALSE;

   return header;
}

void
evas_model_load_vertex_data_unmap(Evas_Canvas3D_Mesh *mesh,
                                  int frame,
                                  Evas_Model_Load_Save_Header header)
{
#define UNMAP_IF_EXIST(existence, vertex_data_type)                           \
   if (existence)                                                             \
     {                                                                        \
        evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, frame, \
                                                         vertex_data_type);  \
     }
   UNMAP_IF_EXIST(header.existence_of_positions, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION)
   UNMAP_IF_EXIST(header.existence_of_normals, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL)
   UNMAP_IF_EXIST(header.existence_of_tex_coords, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD)
   UNMAP_IF_EXIST(header.existence_of_colors, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR)
#undef UNMAP_IF_EXIST
}

void
evas_model_load_save_data_free(Evas_Model_Load_Save_Header header,
                               Evas_Model_Load_Save_Data *data)
{
   if (header.existence_of_positions) free(data->positions);
   if (header.existence_of_normals) free(data->normals);
   if (header.existence_of_tex_coords) free(data->tex_coords);
   if (header.existence_of_colors) free(data->colors);
   free(data->indices);
}

void
evas_model_load_vertex_data_to_mesh(Evas_Canvas3D_Mesh *mesh,
                                     Evas_Model_Load_Save_Header header,
                                     Evas_Model_Load_Save_Data data,
                                     Evas_Model_Load_Save_Stride *stride)
{
   Evas_Model_Load_Save_Data map;
   int i, j;

   evas_canvas3d_mesh_vertex_count_set(mesh, header.vertices_count);
   evas_canvas3d_mesh_vertex_assembly_set(mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_frame_add(mesh, 0);

#define VERTEX_DATA_MAP(name, vertex_data_type, default_size)                                    \
   if (header.existence_of_##name)                                                               \
     {                                                                                           \
        evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, vertex_data_type, 0, NULL); \
        map.name = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, vertex_data_type); \
        stride->name = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, vertex_data_type);   \
        if (stride->name == 0) stride->name = sizeof(float) * default_size;                      \
     }
   VERTEX_DATA_MAP(positions, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 3)
   VERTEX_DATA_MAP(normals, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 3)
   VERTEX_DATA_MAP(tex_coords, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 2)
   VERTEX_DATA_MAP(colors, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, 4)
#undef VERTEX_DATA_MAP

   for (i = 0; i < header.vertices_count; i++)
     {
        Evas_Model_Load_Save_Data current_pointer;

#define FILL_VERTEX_DATA(name, size)                                                \
        if (header.existence_of_##name)                                             \
          {                                                                         \
             current_pointer.name = (float *)((char *)map.name + stride->name * i); \
             for (j = 0; j < size; j++)                                             \
               current_pointer.name[j] = ARRAY_2D(data.name, i, j, size);           \
          }
        FILL_VERTEX_DATA(positions, 3)
        FILL_VERTEX_DATA(normals, 3)
        FILL_VERTEX_DATA(tex_coords, 2)
        FILL_VERTEX_DATA(colors, 3)
#undef FILL_VERTEX_DATA

        if (header.existence_of_colors) current_pointer.colors[3] = 1.0;
     }
}

void
evas_model_load_indices_data_to_mesh(Evas_Canvas3D_Mesh *mesh,
                                      Evas_Model_Load_Save_Header header,
                                      Evas_Model_Load_Save_Data data)
{
   evas_canvas3d_mesh_index_data_copy_set(mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, header.indices_count, data.indices);
}

Eina_Bool
evas_model_load_allocate_data_due_to_header(Evas_Model_Load_Save_Header header,
                                            Evas_Model_Load_Save_Data *data)
{
   if (header.existence_of_positions)
     data->positions = malloc(header.vertices_count * 3 * sizeof(float));
   if (header.existence_of_normals)
     data->normals = malloc(header.vertices_count * 3 * sizeof(float));
   if (header.existence_of_tex_coords)
     data->tex_coords = malloc(header.vertices_count * 2 * sizeof(float));
   if (header.existence_of_colors)
     data->colors = malloc(header.vertices_count * 3 * sizeof(float));
   data->indices = malloc(header.indices_count * sizeof(unsigned short));

   if ((header.existence_of_positions && (data->positions == NULL)) ||
       (header.existence_of_normals && (data->normals == NULL)) ||
       (header.existence_of_tex_coords && (data->tex_coords == NULL)) ||
       (header.existence_of_colors && (data->colors == NULL)) ||
       (data->indices == NULL))
     {
        free(data->positions);
        free(data->normals);
        free(data->tex_coords);
        free(data->colors);
        free(data->indices);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
evas_model_load_aabb_add_to_frame(Evas_Canvas3D_Mesh *mesh,
                                   int frame,
                                   Evas_Model_Load_Save_Stride stride)
{
   Evas_Canvas3D_Mesh_Data *pd;
   pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);

   if (!evas_canvas3d_mesh_aabb_add_to_frame(pd, frame, stride.positions))
     {
        ERR("Axis-Aligned Bounding Box wan't added in frame %d ", 0);
     }
}

Eina_Bool
evas_model_save_header_from_mesh(Evas_Canvas3D_Mesh_Data *pd,
                                 Evas_Canvas3D_Mesh_Frame *f,
                                 Evas_Model_Load_Save_Header *header)
{
   header->indices_count = pd->index_count;
   if (header->indices_count % 3 != 0)
     {
        printf("Index count is %d. It cannot be divided to triangles correctly.\n",
               header->indices_count);
        return EINA_FALSE;
     }

   header->existence_of_positions = (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].data != NULL);
   header->existence_of_normals = (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].data != NULL);
   header->existence_of_tex_coords = (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].data != NULL);
   header->existence_of_colors = (f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR].data != NULL);
   header->vertices_count = pd->vertex_count;

   return EINA_TRUE;
}

void
evas_model_save_data_from_mesh(Evas_Canvas3D_Mesh_Data *pd,
                               Evas_Canvas3D_Mesh_Frame *f,
                               Evas_Model_Load_Save_Header header,
                               Evas_Model_Load_Save_Data *data)
{
   int i;
   if (header.existence_of_positions)
     data->positions = (float*)(&f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION])->data;
   if (header.existence_of_normals)
     data->normals = (float*)(&f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL])->data;
   if (header.existence_of_tex_coords)
     data->tex_coords = (float*)(&f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD])->data;
   if (header.existence_of_colors)
     data->colors = (float*)(&f->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR])->data;
   SAVE_MESH_INDICES_COPY
}

void
evas_model_save_data_copy_from_mesh(Evas_Canvas3D_Mesh_Data *pd,
                                    Evas_Canvas3D_Mesh_Frame *f,
                                    Evas_Model_Load_Save_Header header,
                                    Evas_Model_Load_Save_Data *data)
{
   int i;
#define SAVE_MESH_VERTICES_COPY(name, vertex_data_type) \
   if (header.existence_of_##name)                      \
     memcpy(data->name,                                 \
            (&f->vertices[vertex_data_type])->data,     \
            header.vertices_count);
   SAVE_MESH_VERTICES_COPY(positions, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION)
   SAVE_MESH_VERTICES_COPY(normals, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL)
   SAVE_MESH_VERTICES_COPY(tex_coords, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD)
   SAVE_MESH_VERTICES_COPY(colors, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR)
#undef SAVE_MESH_VERTICES_COPY
   SAVE_MESH_INDICES_COPY
}
