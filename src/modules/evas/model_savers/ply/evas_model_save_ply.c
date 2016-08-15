#include "evas_model_load_save_common.h"

static inline void
_write_ply_header(FILE *file, Evas_Model_Load_Save_Header header)
{
   const char *pf = "property float", *puc = "property uchar";
   fprintf(file, "ply\n" \
                 "format ascii 1.0\n" \
                 "comment Created by EFL evas_canvas3d_mesh_saver_ply.c version 2 " \
                 "(sub 0) - www.enlightenment.org, source file: ''\n");
   fprintf(file, "element vertex %d\n", header.vertices_count);

   if (header.existence_of_positions)
     fprintf(file, "%s x\n%s y\n%s z\n", pf, pf, pf);
   if (header.existence_of_normals)
     fprintf(file, "%s nx\n%s ny\n%s nz\n", pf, pf, pf);
   if (header.existence_of_tex_coords)
     fprintf(file, "%s s\n%s t\n", pf, pf);
   if (header.existence_of_colors)
     fprintf(file, "%s red\n%s green\n%s blue\n", puc, puc, puc);

   fprintf(file, "element face %d\n" \
                 "property list uchar uint vertex_indices\n" \
                 "end_header\n", header.indices_count / 3);
}

static inline void
_write_ply_vertex_data(FILE *file,
                       Evas_Model_Load_Save_Header header,
                       Evas_Model_Load_Save_Data data)
{
   int i, j;
   for (i = 0; i < header.vertices_count; i++)
     {
#define WRITE_PLY_VERTEX_DATA(name, num)\
        if (header.existence_of_##name)\
          for (j = 0; j < num; j++, data.name++)\
            fprintf(file, "%f ", data.name[0]);
        WRITE_PLY_VERTEX_DATA(positions, 3)
        WRITE_PLY_VERTEX_DATA(normals, 3)
        WRITE_PLY_VERTEX_DATA(tex_coords, 2)
#undef WRITE_PLY_VERTEX_DATA

       if (header.existence_of_colors)
         {
            for (j = 0; j < 3; j++, data.colors++)
              fprintf(file, "%.0f ", round(data.colors[0] * 255));
            data.colors++;
         }
        fprintf(file, "\n");
     }
}

static inline void
_write_ply_index_data(FILE *file,
                      Evas_Model_Load_Save_Header header,
                      Evas_Model_Load_Save_Data data)
{
   int i, triangles_count = header.indices_count / 3;
   for (i = 0; i < triangles_count; i++, data.indices += 3)
     fprintf(file, "3 %d %d %d\n", data.indices[0],
                                   data.indices[1],
                                   data.indices[2]);
}

void
evas_model_save_file_ply(const Evas_Canvas3D_Mesh *mesh,
                         const char *file,
                         Evas_Canvas3D_Mesh_Frame *f)
{
   Evas_Model_Load_Save_Header header;
   Evas_Model_Load_Save_Data data;

   Evas_Canvas3D_Mesh_Data *pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);
   if (!evas_model_save_header_from_mesh(pd, f, &header)) return;
   evas_model_save_data_from_mesh(pd, f, header, &data);

   FILE *_ply_file = fopen(file, "w+");
   if (!_ply_file)
     {
        ERR("File open '%s' for save failed", file);
        return;
     }

   _write_ply_header(_ply_file, header);
   _write_ply_vertex_data(_ply_file, header, data);
   _write_ply_index_data(_ply_file, header, data);

   free(data.indices);
   fclose(_ply_file);
}
