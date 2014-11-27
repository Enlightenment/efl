#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include "stdio.h"
#include "math.h"
#include "evas_common_private.h"
#include "evas_private.h"

void
evas_model_save_file_ply(Evas_3D_Mesh *mesh, const char *file, Evas_3D_Mesh_Frame *f)
{
   float *src_pos, *src_nor, *src_tex, *src_col;
   int    i;

   Evas_3D_Mesh_Data *pd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);
   FILE *_ply_file = fopen(file, "w+");
   if (!_ply_file)
     {
        ERR("File open '%s' for save failed", file);
        return;
     }

   fprintf(_ply_file,"ply\nformat ascii 1.0\ncomment Created by EFL evas_3d_mesh_saver_ply.c" \
                      "version 1 (sub 0) - www.enlightenment.org, source file: ''\n");
   fprintf(_ply_file,"element vertex %d\n", pd->vertex_count);
   fprintf(_ply_file,"property float x\nproperty float y\nproperty float z\n" \
                     "property float nx\nproperty float ny\nproperty float nz\n" \
                     "property float s\nproperty float t\n" \
                     "property uchar red\nproperty uchar green\nproperty uchar blue\n");
   fprintf(_ply_file,"element face %d\nproperty list uchar uint vertex_indices\nend_header\n",
           pd->vertex_count / 3);

   src_pos = (float*)(&f->vertices[EVAS_3D_VERTEX_POSITION])->data;
   src_nor = (float*)(&f->vertices[EVAS_3D_VERTEX_NORMAL])->data;
   src_tex = (float*)(&f->vertices[EVAS_3D_VERTEX_TEXCOORD])->data;
   src_col = (float*)(&f->vertices[EVAS_3D_VERTEX_COLOR])->data;

   for (i = 0; i < pd->vertex_count; i++)
     {
        fprintf(_ply_file,"%f %f %f %f %f %f %f %f %.0f %.0f %.0f\n",
                src_pos[0], src_pos[1], src_pos[2],
                src_nor[0], src_nor[1], src_nor[2],
                src_tex[0], src_tex[1],
                round(src_col[0] * 255), round(src_col[1] * 255), round(src_col[2] * 255));
        src_pos += 3;
        src_nor += 3;
        src_tex += 2;
        src_col += 4;
     }

   for (i = 0; i < pd->vertex_count;)
     {
        fprintf(_ply_file,"3 %d %d %d\n", i, i + 1, i + 2);
        i += 3;
     }

   fclose(_ply_file);
}
