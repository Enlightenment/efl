#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#include <Eet.h>
#include <stdio.h>
#include "evas_common_private.h"
#include "evas_private.h"

#define COMPONENT_OF_DEFAULT_GREY_COLOR 0.3
#define TRANSPARENCY_OF_DEFAULT_GREY_COLOR 0.5

static void
_set_geometry_to_eet_file_from_mesh(Evas_Canvas3D_Mesh_Data *mesh,
                                    Evas_Canvas3D_Mesh_Eet *eet_mesh,
                                    Evas_Canvas3D_Header_Eet *eet_header,
                                    Evas_Canvas3D_Mesh_Frame *f)
{
   Evas_Canvas3D_Vertex_Buffer *vb;
   int i;
   float *src;
   Evas_Canvas3D_Vertex_Eet *vertices =
      calloc(1, sizeof(Evas_Canvas3D_Vertex_Eet) * mesh->vertex_count);
   Evas_Canvas3D_Geometry_Eet *geometry =
      calloc(1, sizeof(Evas_Canvas3D_Geometry_Eet));
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

#define SAVE_GEOMETRICS(a, component, command_for_z_component)\
   vb = &f->vertices[a];\
   if (vb->data == NULL)\
     {\
        ERR("Failed to read geometrics for '%s'.", #component);\
     }\
   else\
     {\
        src = (float *)vb->data;\
        for (i = 0; i < mesh->vertex_count; i++)\
          {\
             vertices[i].component.x = src[0];\
             vertices[i].component.y = src[1];\
             command_for_z_component\
             src += f->vertices[a].element_count;\
          }\
     }
   geometry->vertices = vertices;
   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, position, vertices[i].position.z = src[2];)
   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, normal, vertices[i].normal.z = src[2];)
   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, texcoord,)
#undef SAVE_GEOMETRICS

   eet_mesh->geometries = geometry;
   eet_header->geometries = geometries;
}

static void
_set_material_to_eet_file_from_mesh(Evas_Canvas3D_Mesh_Eet *eet_mesh,
                                    Evas_Canvas3D_Header_Eet *eet_header,
                                    Evas_Canvas3D_Mesh_Frame *f)
{
   int i;
   Evas_Canvas3D_Material_Data *material =
      efl_data_scope_get(f->material, EVAS_CANVAS3D_MATERIAL_CLASS);
   Evas_Canvas3D_Material_Eet *saved_materials =
      malloc(sizeof(Evas_Canvas3D_Material_Eet));
   Evas_Canvas3D_Color_Eet *saved_colors =
      malloc(sizeof(Evas_Canvas3D_Color_Eet) * EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT);
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
        ERR("Material is set to the default values, because no custom material "
             "was saved earlier by evas_3d_mesh_save.");

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

   saved_materials->colors_count = EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT;
   materials[0] = EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT;
   saved_materials->id = 1;
   saved_materials->colors = saved_colors;
   eet_mesh->materials = saved_materials;
   eet_header->materials = materials;
}

static void
_set_frame_to_eet_file_from_mesh(Evas_Canvas3D_Mesh_Eet *eet_mesh)
{
   eet_mesh->frames = malloc(sizeof(Evas_Canvas3D_Frame_Eet));

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
evas_model_save_file_eet(const Evas_Canvas3D_Mesh *mesh,
                         const char *file,
                         Evas_Canvas3D_Mesh_Frame *f)
{
   Evas_Canvas3D_Mesh_Data *pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);
   Eet_File *ef;
   Evas_Canvas3D_Mesh_Eet* eet_mesh = malloc(sizeof(Evas_Canvas3D_Mesh_Eet));
   Evas_Canvas3D_Header_Eet* eet_header = malloc(sizeof(Evas_Canvas3D_Header_Eet));
   Eet_Data_Descriptor *_file_descriptor;
   Evas_Canvas3D_File_Eet *eet_file;

   eet_init();

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (ef == NULL)
     {
        ERR("Opening of file is failed.");
        free(eet_mesh);
        free(eet_header);
        eet_shutdown();
        return;
     }

   _file_descriptor = _evas_canvas3d_eet_file_get();

   eet_file = _evas_canvas3d_eet_file_new();

   if (!eet_file)
     {
        free(eet_mesh);
        free(eet_header);
        eet_shutdown();
        return;
     }

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

   eet_file->mesh = eet_mesh;
   eet_file->header = eet_header;

   eet_data_write(ef,
                  _file_descriptor,
                  EVAS_CANVAS3D_FILE_CACHE_FILE_ENTRY,
                  eet_file,
                  EINA_TRUE);
   eet_close(ef);

   _evas_canvas3d_eet_file_free(eet_file);

   eet_shutdown();
}
