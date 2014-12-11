#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#include <Eo.h>
#include <stdio.h>
#include "evas_common_private.h"
#include "evas_private.h"

extern Evas_3D_File_Eet* eet_file;
extern const char EVAS_3D_FILE_CACHE_FILE_ENTRY[];
extern Eet_Data_Descriptor *_file_descriptor;

void
_set_geometry_to_mesh_from_eet_file(Evas_3D_Mesh *mesh,
                                    Evas_3D_Mesh_Eet *eet_mesh)
{
   float *pos, *nor, *tex;
   int stride_pos, stride_nor, stride_tex, j;

   eo_do(mesh,
         evas_3d_mesh_vertex_count_set(eet_mesh->geometries->vertices_count),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_frame_add(0),

         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION, 0, NULL),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,   0, NULL),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_TEXCOORD, 0, NULL),

         pos = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_POSITION),
         nor = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_NORMAL),
         tex = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_TEXCOORD),

         stride_pos = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_POSITION),
         stride_nor = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_NORMAL),
         stride_tex = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_TEXCOORD));

   if (stride_pos == 0) stride_pos = sizeof(float) * 3;
   if (stride_nor == 0) stride_nor = sizeof(float) * 3;
   if (stride_tex == 0) stride_tex = sizeof(float) * 2;

        /* set data to vertex buffer */
   for (j = 0; j < eet_mesh->geometries->vertices_count; j++)
     {
        float *p, *n, *t;

        p = (float *)((char *)pos + stride_pos * j);
        n = (float *)((char *)nor + stride_nor * j);
        t = (float *)((char *)tex + stride_tex * j);

        p[0] = eet_mesh->geometries->vertices[j].position.x;
        p[1] = eet_mesh->geometries->vertices[j].position.y;
        p[2] = eet_mesh->geometries->vertices[j].position.z;

        n[0] = eet_mesh->geometries->vertices[j].normal.x;
        n[1] = eet_mesh->geometries->vertices[j].normal.y;
        n[2] = eet_mesh->geometries->vertices[j].normal.z;

        t[0] = eet_mesh->geometries->vertices[j].texcoord.x;
        t[1] = eet_mesh->geometries->vertices[j].texcoord.y;
     }

        /* unmap vertex buffer */
   eo_do(mesh,
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_POSITION),
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_NORMAL),
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_TEXCOORD));
}

void
_set_material_to_mesh_from_eet_file(Evas_3D_Mesh *mesh,
                                    Evas_3D_Mesh_Eet *eet_mesh)
{
   Evas_3D_Object_Data *pd = eo_data_scope_get(mesh, EVAS_3D_OBJECT_CLASS);
   Eo *material = NULL;
   material = eo_add(EVAS_3D_MATERIAL_CLASS, pd->evas);

   eo_do(material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT,
                                     !!(eet_mesh->materials->colors[0].a > 0)),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE,
                                     !!(eet_mesh->materials->colors[1].a > 0)),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR,
                                     !!(eet_mesh->materials->colors[2].a > 0)),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    eet_mesh->materials->colors[0].r,
                                    eet_mesh->materials->colors[0].g,
                                    eet_mesh->materials->colors[0].b,
                                    eet_mesh->materials->colors[0].a),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    eet_mesh->materials->colors[1].r,
                                    eet_mesh->materials->colors[1].g,
                                    eet_mesh->materials->colors[1].b,
                                    eet_mesh->materials->colors[1].a),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    eet_mesh->materials->colors[2].r,
                                    eet_mesh->materials->colors[2].g,
                                    eet_mesh->materials->colors[2].b,
                                    eet_mesh->materials->colors[2].a),
         evas_3d_material_shininess_set(eet_mesh->materials->shininess));

   eo_do(mesh, evas_3d_mesh_frame_material_set(0, material));
}

void
evas_model_load_file_eet(Evas_3D_Mesh *mesh, Eina_File *file)
{
   Eet_File *ef;

   _evas_3d_eet_file_init();
   ef = eet_mmap(file);
   eet_file = eet_data_read(ef,
                            _file_descriptor,
                            EVAS_3D_FILE_CACHE_FILE_ENTRY);

   eet_close(ef);

   if ((eet_file->mesh == NULL) || (eet_file->header == NULL))
     {
        ERR("Reading of file is failed.");
        _evas_3d_eet_file_free();
        return;
     }

   _set_geometry_to_mesh_from_eet_file(mesh, eet_file->mesh);
   _set_material_to_mesh_from_eet_file(mesh, eet_file->mesh);

   _evas_3d_eet_file_free();
}


