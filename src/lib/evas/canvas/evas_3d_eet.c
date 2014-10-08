#include <stdio.h>
#include "evas_common_private.h"
#include "evas_private.h"

Evas_3D_File_Eet* eet_file;
const char EVAS_3D_FILE_CACHE_FILE_ENTRY[] = "evas_3d file";
Eet_Data_Descriptor *_vec3_descriptor;
Eet_Data_Descriptor *_vertex_descriptor;
Eet_Data_Descriptor *_geometry_descriptor;
Eet_Data_Descriptor *_color_descriptor;
Eet_Data_Descriptor *_material_descriptor;
Eet_Data_Descriptor *_frame_descriptor;
Eet_Data_Descriptor *_mesh_descriptor;
Eet_Data_Descriptor *_header_descriptor;
Eet_Data_Descriptor *_file_descriptor;

Evas_3D_File_Eet *
_evas_3d_eet_file_new(void)
{
   Evas_3D_File_Eet *creating_file = calloc(1, sizeof(Evas_3D_File_Eet));

   if (!creating_file)
     {
        ERR("Could not calloc Evas_3D_File_Eet");
        return NULL;
     }

   return creating_file;
}

void
_evas_3d_eet_file_init(void)
{
   eina_init();
   eet_init();

/* initialization of bonding between structure units in eet file */
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Vec3_Eet);
   _vec3_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Vertex_Eet);
   _vertex_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Geometry_Eet);
   _geometry_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Color_Eet);
   _color_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Frame_Eet);
   _frame_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Material_Eet);
   _material_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Mesh_Eet);
   _mesh_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_Header_Eet);
   _header_descriptor = eet_data_descriptor_file_new(&eddc);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Evas_3D_File_Eet);
   _file_descriptor = eet_data_descriptor_file_new(&eddc);

/* Vec_3 */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_vec3_descriptor, Evas_3D_Vec3_Eet, # member, member, eet_type);
   ADD_BASIC(x, EET_T_FLOAT);
   ADD_BASIC(y, EET_T_FLOAT);
   ADD_BASIC(z, EET_T_FLOAT);
#undef ADD_BASIC

/* Vertex */
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet,
                                      "position", position, _vec3_descriptor);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet,
                                      "normal", normal, _vec3_descriptor);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_vertex_descriptor, Evas_3D_Vertex_Eet,
                                      "texcoord", texcoord, _vec3_descriptor);

/* Geometry */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_geometry_descriptor, Evas_3D_Geometry_Eet,
                                     "vertices", vertices, _vertex_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_geometry_descriptor, Evas_3D_Geometry_Eet, # member, member, eet_type);
   ADD_BASIC(vertices_count, EET_T_UINT);
   ADD_BASIC(id, EET_T_UINT);
#undef ADD_BASIC

/* Color */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_color_descriptor, Evas_3D_Color_Eet, # member, member, eet_type);
   ADD_BASIC(r, EET_T_FLOAT);
   ADD_BASIC(g, EET_T_FLOAT);
   ADD_BASIC(b, EET_T_FLOAT);
   ADD_BASIC(a, EET_T_FLOAT);
#undef ADD_BASIC

/* Material */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_material_descriptor, Evas_3D_Material_Eet,
                                     "colors", colors, _color_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_material_descriptor, Evas_3D_Material_Eet, # member, member, eet_type);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(colors_count, EET_T_UINT);
   ADD_BASIC(shininess, EET_T_FLOAT);
#undef ADD_BASIC

/* Frame */
#define ADD_BASIC(member, eet_type) EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_frame_descriptor, Evas_3D_Frame_Eet, # member, member, eet_type);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(geometry_id, EET_T_UINT);
   ADD_BASIC(material_id, EET_T_UINT);
#undef ADD_BASIC

/* Mesh */
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_mesh_descriptor, Evas_3D_Mesh_Eet,
                                     "frames", frames, _frame_descriptor);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_mesh_descriptor, Evas_3D_Mesh_Eet,
                                     "geometries", geometries, _geometry_descriptor);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_mesh_descriptor, Evas_3D_Mesh_Eet,
                                     "materials", materials, _material_descriptor);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_mesh_descriptor, Evas_3D_Mesh_Eet, # member, member, eet_type);
   ADD_BASIC(materials_count, EET_T_UINT);
   ADD_BASIC(frames_count, EET_T_UINT);
   ADD_BASIC(geometries_count, EET_T_UINT);
#undef ADD_BASIC

/* Header */
   EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(_header_descriptor, Evas_3D_Header_Eet,
                                           "materials", materials, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(_header_descriptor, Evas_3D_Header_Eet,
                                           "geometries", geometries, EET_T_UINT);
#define ADD_BASIC(member, eet_type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC \
   (_header_descriptor, Evas_3D_Header_Eet, # member, member, eet_type);
   ADD_BASIC(version, EET_T_UINT);
   ADD_BASIC(materials_count, EET_T_UINT);
   ADD_BASIC(frames, EET_T_UINT);
   ADD_BASIC(geometries_count, EET_T_UINT);
#undef ADD_BASIC

/* File */
   EET_DATA_DESCRIPTOR_ADD_SUB(_file_descriptor, Evas_3D_File_Eet,
                               "mesh", mesh, _mesh_descriptor);
   EET_DATA_DESCRIPTOR_ADD_SUB(_file_descriptor, Evas_3D_File_Eet,
                               "header", header, _header_descriptor);

}

void
_evas_3d_eet_descriptor_shutdown(void)
{
   eet_data_descriptor_free(_geometry_descriptor);
   eet_data_descriptor_free(_vertex_descriptor);
   eet_data_descriptor_free(_vec3_descriptor);
   eet_data_descriptor_free(_color_descriptor);
   eet_data_descriptor_free(_material_descriptor);
   eet_data_descriptor_free(_frame_descriptor);
   eet_data_descriptor_free(_mesh_descriptor);
   eet_data_descriptor_free(_header_descriptor);
   eet_data_descriptor_free(_file_descriptor);
}

void
_evas_3d_eet_file_free(void)
{
   free(eet_file->mesh->geometries[0].vertices);
   free(eet_file->mesh->geometries);
   free(eet_file->mesh->frames);
   free(eet_file->mesh->materials[0].colors);
   free(eet_file->mesh->materials);
   free(eet_file->mesh);
   free(eet_file->header->materials);
   free(eet_file->header->geometries);
   free(eet_file->header);
   free(eet_file);

   _evas_3d_eet_descriptor_shutdown();
   eet_shutdown();
   eina_shutdown();
}
