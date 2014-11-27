#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#include <stdio.h>
#include "evas_common_private.h"
#include "evas_private.h"

#define COMPONENT_OF_DEFAULT_GREY_COLOR 0.3
#define TRANSPARENCY_OF_DEFAULT_GREY_COLOR 0.5

extern Evas_3D_File_Eet* eet_file;
extern const char EVAS_3D_FILE_CACHE_FILE_ENTRY[];
extern Eet_Data_Descriptor *_file_descriptor;

void
_set_geometry_to_eet_file_from_mesh(Evas_3D_Mesh_Data *mesh,
                                    Evas_3D_Mesh_Eet *eet_mesh,
                                    Evas_3D_Header_Eet *eet_header,
                                    Evas_3D_Mesh_Frame *f)
{
   Evas_3D_Vertex_Buffer *vb;
   int i;
   float *src;
   Evas_3D_Vertex_Eet *vertices =
      malloc(sizeof(Evas_3D_Vertex_Eet) * mesh->vertex_count);
   Evas_3D_Geometry_Eet *geometry =
      malloc(sizeof(Evas_3D_Geometry_Eet));
   int *geometries = malloc(sizeof(int));

   if ((vertices == NULL) || (geometry == NULL))
     {
        ERR("Allocating of memory is failed.");
        free(vertices);
        free(geometry);
        free(geometries);
        return;
     }

   geometry->vertices_count = mesh->vertex_count;
   geometries[0] = mesh->vertex_count;

#define SAVE_GEOMETRICS(a, component)\
   vb = &f->vertices[a];\
   if (vb->data == NULL)\
     {\
        ERR("Reading of geometrics is failed.");\
        free(vertices);\
        free(geometry);\
        free(geometries);\
        return;\
     }\
   src = (float *)vb->data;\
   for (i = 0; i < mesh->vertex_count; i++)\
     {\
        vertices[i].component.x = src[0];\
        vertices[i].component.y = src[1];\
        vertices[i].component.z = src[2];\
        src += f->vertices[a].element_count;\
     }

   geometry->vertices = vertices;
   SAVE_GEOMETRICS(EVAS_3D_VERTEX_POSITION, position)
   SAVE_GEOMETRICS(EVAS_3D_VERTEX_NORMAL, normal)
   SAVE_GEOMETRICS(EVAS_3D_VERTEX_TEXCOORD, texcoord)

#undef SAVE_GEOMETRICS

   eet_mesh->geometries = geometry;
   eet_header->geometries = geometries;
}

void
_set_material_to_eet_file_from_mesh(Evas_3D_Mesh_Eet *eet_mesh,
                                    Evas_3D_Header_Eet *eet_header,
                                    Evas_3D_Mesh_Frame *f)
{
   int i;
   Evas_3D_Material_Data *material =
      eo_data_scope_get(f->material, EVAS_3D_MATERIAL_CLASS);
   Evas_3D_Material_Eet *saved_materials =
      malloc(sizeof(Evas_3D_Material_Eet));
   Evas_3D_Color_Eet *saved_colors =
      malloc(sizeof(Evas_3D_Color_Eet) * EVAS_3D_MATERIAL_ATTRIB_COUNT);
   int *materials = malloc(sizeof(int));

   if ((saved_materials == NULL) || (saved_colors == NULL))
     {
        ERR("Allocating of memory is failed.");
        free(material);
        free(saved_materials);
        free(saved_colors);
        free(materials);
        return;
     }

   if (material == NULL)
     {
        ERR("Default material is set to saved file, because custom material \
             was not saved before using of function evas_3d_mesh_save.");

        for (i = 0; i < 5; i++)
          {
             saved_colors[i].r = COMPONENT_OF_DEFAULT_GREY_COLOR;
             saved_colors[i].g = COMPONENT_OF_DEFAULT_GREY_COLOR;
             saved_colors[i].b = COMPONENT_OF_DEFAULT_GREY_COLOR;
             saved_colors[i].a = TRANSPARENCY_OF_DEFAULT_GREY_COLOR;
          }

        saved_materials->shininess = 50;
     }
   else
     {

        for (i = 0; i < 5; i++)
          {
             saved_colors[i].r = material->attribs[i].color.r;
             saved_colors[i].g = material->attribs[i].color.g;
             saved_colors[i].b = material->attribs[i].color.b;
             saved_colors[i].a = material->attribs[i].color.a;
          }

        saved_materials->shininess = material->shininess;
     }

   saved_materials->colors_count = EVAS_3D_MATERIAL_ATTRIB_COUNT;
   materials[0] = EVAS_3D_MATERIAL_ATTRIB_COUNT;
   saved_materials->id = 1;
   saved_materials->colors = saved_colors;
   eet_mesh->materials = saved_materials;
   eet_header->materials = materials;
}

void
_set_frame_to_eet_file_from_mesh(Evas_3D_Mesh_Eet *eet_mesh)
{
   eet_mesh->frames = malloc(sizeof(Evas_3D_Frame_Eet));

   if (eet_mesh->frames == NULL)
     {
        ERR("Allocating of memory is failed.");
        return;
     }

   eet_mesh->frames->id = 1;
   eet_mesh->frames->geometry_id = 1;
   eet_mesh->frames->material_id = 1;
}

void
evas_model_save_file_eet(Evas_3D_Mesh *mesh,
                         const char *file,
                         Evas_3D_Mesh_Frame *f)
{
   Evas_3D_Mesh_Data *pd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);
   Eet_File *ef = eet_open(file, EET_FILE_MODE_WRITE);
   Evas_3D_Mesh_Eet* eet_mesh = malloc(sizeof(Evas_3D_Mesh_Eet));
   Evas_3D_Header_Eet* eet_header = malloc(sizeof(Evas_3D_Header_Eet));

   _evas_3d_eet_file_init();

   eet_file = _evas_3d_eet_file_new();

   eet_mesh->materials_count = 1;
   eet_header->materials_count = 1;
   eet_mesh->frames_count = 1;
   eet_header->frames = 1;
   eet_mesh->geometries_count = 1;
   eet_header->geometries_count = 1;
   eet_header->version = 1;

   _set_geometry_to_eet_file_from_mesh(pd, eet_mesh, eet_header, f);
   _set_material_to_eet_file_from_mesh(eet_mesh, eet_header, f);
   _set_frame_to_eet_file_from_mesh(eet_mesh);

   if (ef == NULL)
     {
        ERR("Opening of file is failed.");
        free(eet_mesh);
        free(eet_header);
        _evas_3d_eet_file_free();
        return;
     }

   eet_file->mesh = eet_mesh;
   eet_file->header = eet_header;

   eet_data_write(ef,
                  _file_descriptor,
                  EVAS_3D_FILE_CACHE_FILE_ENTRY,
                  eet_file,
                  EINA_TRUE);
   eet_close(ef);

   _evas_3d_eet_file_free();
}

