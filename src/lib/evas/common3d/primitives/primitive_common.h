#ifdef HAVE_CONFIG_H
# include "config.h"
#endif //HAVE_CONFIG_H

#ifndef PRIMITIVE_COMMON
#define PRIMITIVE_COMMON
#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"

#define ALLOCATE_VERTEX_DATA                                  \
   Eina_Vector3 *vertices = malloc(sizeof(Eina_Vector3) * vcount);  \
   Eina_Vector3 *normals = malloc(sizeof(Eina_Vector3) * vcount);   \
   Eina_Vector2 *tex_coord = malloc(sizeof(Eina_Vector2) * vcount); \
   Eina_Vector3 *tangents = malloc(sizeof(Eina_Vector3) * vcount);  \
   unsigned short *indices = malloc(sizeof(short) * icount);

#define SET_VERTEX_DATA(frame)                                                     \
   Eina_Bool frame_exist; \
   frame_exist = evas_canvas3d_mesh_frame_exist(mesh, frame); \
   if (!frame_exist) \
   evas_canvas3d_mesh_frame_add(mesh, frame); \
   evas_canvas3d_mesh_vertex_count_set(mesh, vcount); \
   evas_canvas3d_mesh_index_data_copy_set(mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, \
                                          icount, &indices[0]);                   \
   _set_vec3_vertex_data(mesh, frame, vcount, vertices, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);  \
   _set_vec3_vertex_data(mesh, frame, vcount, normals, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);     \
   _set_vec2_vertex_data(mesh, frame, vcount, tex_coord, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD); \
   _set_vec3_vertex_data(mesh, frame, vcount, tangents, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT);   \
   free(indices);

#define SET_VERTEX_DATA_FROM_ARRAY(mesh, frame, varray, vcount, indices, icount) \
   Eina_Bool frame_exist; \
   frame_exist = evas_canvas3d_mesh_frame_exist(mesh, frame); \
   if (!frame_exist) \
   evas_canvas3d_mesh_frame_add(mesh, frame); \
   evas_canvas3d_mesh_vertex_count_set(mesh, vcount); \
   evas_canvas3d_mesh_index_data_copy_set(mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, \
                                          icount, &indices[0]); \
   _set_vertex_data_from_array(mesh, frame, varray, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, \
                               0, 3, 15, vcount); \
   _set_vertex_data_from_array(mesh, frame, varray, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, \
                               3, 3, 15, vcount); \
   _set_vertex_data_from_array(mesh, frame, varray, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, \
                               6, 4, 15, vcount); \
   _set_vertex_data_from_array(mesh, frame, varray, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, \
                               10, 2, 15, vcount); \
   _set_vertex_data_from_array(mesh, frame, varray, EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT, \
                               12, 3, 15, vcount);

void _generate_indices(unsigned short *indices, int count, int width);
void _set_vec3_vertex_data(Evas_Canvas3D_Mesh *mesh, int frame, int vcount, Eina_Vector3 *data, Evas_Canvas3D_Vertex_Attrib attr);
void _set_vec2_vertex_data(Evas_Canvas3D_Mesh *mesh, int frame, int vcount, Eina_Vector2 *data, Evas_Canvas3D_Vertex_Attrib attr);
void _set_vertex_data_from_array(Evas_Canvas3D_Mesh *mesh, int frame, const float *data, Evas_Canvas3D_Vertex_Attrib attr, int start, int attr_count, int line, int vcount);

void _primitives_vec3_copy(Eina_Vector3 *dst, const Eina_Vector3 *src);
void _primitives_vec3_subtract(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b);
void _primitives_vec3_cross_product(Eina_Vector3 *out, const Eina_Vector3 *a, const Eina_Vector3 *b);
void _primitives_vec3_normalize(Eina_Vector3 *out);

#endif //PRIMITIVE_COMMON
