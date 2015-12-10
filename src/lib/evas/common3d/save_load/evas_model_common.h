#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef MODEL_LOAD_COMMON
#define MODEL_LOAD_COMMON

#include <stdlib.h>
#include <time.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"
#include <Eina.h>

/* set value to position [x][y] to array name which have. */
#define ARRAY_2D(name, x, y, count_y) (*(name + x * count_y + y))

/* Structures for reading data from file. */
typedef struct _Evas_Model_Load_Save_Header    Evas_Model_Load_Save_Header;
typedef struct _Evas_Model_Load_Save_Data      Evas_Model_Load_Save_Data;
typedef struct _Evas_Model_Load_Save_Stride    Evas_Model_Load_Save_Stride;

struct _Evas_Model_Load_Save_Header
{
   int vertices_count;
   int indices_count;
   Eina_Bool existence_of_positions;
   Eina_Bool existence_of_normals;
   Eina_Bool existence_of_tex_coords;
   Eina_Bool existence_of_colors;
};

struct _Evas_Model_Load_Save_Data
{
   float *positions;
   float *normals;
   float *tex_coords;
   float *colors;
   unsigned short *indices;
};

struct _Evas_Model_Load_Save_Stride
{
   int positions;
   int normals;
   int tex_coords;
   int colors;
};

/* create new header */
Evas_Model_Load_Save_Header evas_model_load_save_header_new(void);
void evas_model_load_vertex_data_unmap(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Model_Load_Save_Header header);
void evas_model_load_save_data_free(Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data *data);
void evas_model_load_vertex_data_to_mesh(Evas_Canvas3D_Mesh *mesh, Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data data, Evas_Model_Load_Save_Stride *stride);
void evas_model_load_indices_data_to_mesh(Evas_Canvas3D_Mesh *mesh, Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data data);
Eina_Bool evas_model_load_allocate_data_due_to_header(Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data *data);
void evas_model_load_aabb_add_to_frame(Evas_Canvas3D_Mesh *mesh, int frame, Evas_Model_Load_Save_Stride stride);
Eina_Bool evas_model_save_header_from_mesh(Evas_Canvas3D_Mesh_Data *pd, Evas_Canvas3D_Mesh_Frame *f, Evas_Model_Load_Save_Header *header);
void evas_model_save_data_from_mesh(Evas_Canvas3D_Mesh_Data *pd, Evas_Canvas3D_Mesh_Frame *f, Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data *data);
void evas_model_save_data_copy_from_mesh(Evas_Canvas3D_Mesh_Data *pd, Evas_Canvas3D_Mesh_Frame *f, Evas_Model_Load_Save_Header header, Evas_Model_Load_Save_Data *data);

#endif //MODEL_LOAD_COMMON
