#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#include <Eo.h>
#include <Eet.h>
#include <stdio.h>
#include "evas_common_private.h"
#include "evas_private.h"

static void
_set_geometry_to_mesh_from_eet_file(Evas_Canvas3D_Mesh *mesh,
                                    Evas_Canvas3D_Mesh_Eet *eet_mesh)
{
   float *pos, *nor, *tex;
   int stride_pos, stride_nor, stride_tex, j;

   evas_canvas3d_mesh_vertex_count_set(mesh, eet_mesh->geometries->vertices_count);
   evas_canvas3d_mesh_vertex_assembly_set(mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_frame_add(mesh, 0);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 0, NULL);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 0, NULL);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 0, NULL);
   pos = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);
   nor = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
   tex = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);
   stride_pos = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);
   stride_nor = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
   stride_tex = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);

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
   evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);
   evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
   evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);
}

static void
_set_material_to_mesh_from_eet_file(Evas_Canvas3D_Mesh *mesh,
                                    Evas_Canvas3D_Mesh_Eet *eet_mesh)
{
   Evas_Canvas3D_Object_Data *pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_OBJECT_CLASS);
   Eo *material = NULL;
   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, pd->evas);

   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, !!(eet_mesh->materials->colors[0].a > 0));
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, !!(eet_mesh->materials->colors[1].a > 0));
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, !!(eet_mesh->materials->colors[2].a > 0));
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, eet_mesh->materials->colors[0].r, eet_mesh->materials->colors[0].g, eet_mesh->materials->colors[0].b, eet_mesh->materials->colors[0].a);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, eet_mesh->materials->colors[1].r, eet_mesh->materials->colors[1].g, eet_mesh->materials->colors[1].b, eet_mesh->materials->colors[1].a);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, eet_mesh->materials->colors[2].r, eet_mesh->materials->colors[2].g, eet_mesh->materials->colors[2].b, eet_mesh->materials->colors[2].a);
   evas_canvas3d_material_shininess_set(material, eet_mesh->materials->shininess);

   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);
}

void
evas_model_load_file_eet(Evas_Canvas3D_Mesh *mesh, Eina_File *file)
{
   Eet_File *ef;
   Evas_Canvas3D_File_Eet* eet_file;
   Eet_Data_Descriptor *_file_descriptor;

   eet_init();

   _file_descriptor = _evas_canvas3d_eet_file_get();
   ef = eet_mmap(file);
   eet_file = eet_data_read(ef,
                            _file_descriptor,
                            EVAS_CANVAS3D_FILE_CACHE_FILE_ENTRY);

   eet_close(ef);

   if (!eet_file || !eet_file->mesh || !eet_file->header)
     {
        ERR("Failed to read model file");
        goto on_error;
     }

   _set_geometry_to_mesh_from_eet_file(mesh, eet_file->mesh);
   _set_material_to_mesh_from_eet_file(mesh, eet_file->mesh);

 on_error:
   _evas_canvas3d_eet_file_free(eet_file);
   eet_shutdown();
}
