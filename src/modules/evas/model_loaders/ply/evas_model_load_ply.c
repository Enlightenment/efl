#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"
#include <Eina.h>

/* set value to position [x][y] to array name which have. */
#define ARRAY_2D(name, x, y, count_y) (*(name + x * count_y + y))

/* Structures for reading data from file. */
typedef struct _PLY_Header    PLY_Header;

struct _PLY_Header
{
   int vertices_count;
   int triangles_count;
   Eina_Bool existence_of_geometries;
   Eina_Bool existence_of_normals;
   Eina_Bool existence_of_texcoords;
   Eina_Bool existence_of_colors;
};

/* create new header */
static inline PLY_Header
_new_ply_header()
{
   PLY_Header header;

   header.vertices_count = 0;
   header.triangles_count = 0;
   header.existence_of_geometries = EINA_FALSE;
   header.existence_of_normals = EINA_FALSE;
   header.existence_of_texcoords = EINA_FALSE;
   header.existence_of_colors = EINA_FALSE;

   return header;
}

static inline char *
_to_next_line(char *current)
{
   while (*current != '\n') current++;
   current++;
   return current;
}

static inline char *
_to_begin_of_line(char *current)
{
   while (*current != '\n') current--;
   current++;
   return current;
}

static inline char *
_to_next_number(char *current, int count)
{
   int i;
   for (i = 0; i < count; i++)
     {
        while (*current != ' ') current++;
        current++;
     }
   return current;
}

static inline char *
_read_data(float *array, int place, int count, char *current, float divider)
{
   int i;
   for (i = 0; i < count; i++)
     {
        sscanf (current,"%f", &ARRAY_2D(array, place, i, count));
        ARRAY_2D(array, place, i, count) /= divider;
        current = _to_next_number(current, 1);
     }
   return current;
}

static inline PLY_Header
_read_header(char *map)//Check properties of mesh in .ply file.
{
   eina_init();

   Eina_Bool reading_vertices = EINA_TRUE, check_next_char = EINA_FALSE;
   int vertex_lines, triangles = 0, vertices_in_current_face = 0;
   char **helping_pointer;
   char *current;
   PLY_Header header;

   header = _new_ply_header();
   helping_pointer = eina_str_split(map, "vertex ", 0);

   if (helping_pointer == NULL)
     {
        ERR("File have not kayword vertex. It is necessary.");
        return header;
     }

   sscanf(helping_pointer[1], "%d", &header.vertices_count);

   free(helping_pointer);
   helping_pointer = eina_str_split(map, "end_header\n", 0);

   if (helping_pointer == NULL)
     {
        ERR("File have not kayword end_header. It is necessary.");
        return header;
     }

   current = helping_pointer[1];

   vertex_lines = header.vertices_count;
   while (*current != '\0')
     {
        if (vertex_lines == 1)
          reading_vertices = EINA_FALSE;
        if (*current == '\n')
          {
             if (reading_vertices)
               vertex_lines--;
             else
               check_next_char = EINA_TRUE;
          }
        if (check_next_char)
          {
             if ((*current <= '9') && (*current >= '0'))
               vertices_in_current_face = (vertices_in_current_face * 10) + (*current - '0');
             else if (*current >= ' ')
               {
                  triangles += (vertices_in_current_face - 2);
                  vertices_in_current_face = 0;
                  check_next_char = EINA_FALSE;
               }
          }
        current++;
     }

   header.triangles_count = triangles;
   free(helping_pointer);

      /* analyse flags used when file was saved in blender */
   helping_pointer = eina_str_split(map, "property float ", 0);

   if ((helping_pointer[1] != NULL) && (*helping_pointer[1] == 'x') &&
       (helping_pointer[2] != NULL) && (*helping_pointer[2] == 'y') &&
       (helping_pointer[3] != NULL) && (*helping_pointer[3] == 'z'))
     header.existence_of_geometries = EINA_TRUE;
   else return header;

   if ((helping_pointer[4] != NULL) && (*helping_pointer[4] == 'n') &&
       (helping_pointer[5] != NULL) && (*helping_pointer[5] == 'n') &&
       (helping_pointer[6] != NULL) && (*helping_pointer[6] == 'n'))
     header.existence_of_normals = EINA_TRUE;

   if ((header.existence_of_normals &&
       ((helping_pointer[7] != NULL) && (*helping_pointer[7] == 's') &&
       (helping_pointer[8] != NULL) && (*helping_pointer[8] == 't'))) ||
       (!header.existence_of_normals &&
       ((helping_pointer[4] != NULL) && (*helping_pointer[4] == 's') &&
       (helping_pointer[5] != NULL) && (*helping_pointer[5] == 't'))))
     header.existence_of_texcoords = EINA_TRUE;

   helping_pointer = eina_str_split(map, "property uchar ", 0);

   if ((helping_pointer[1] != NULL) && (*helping_pointer[1] == 'r') &&
       (helping_pointer[2] != NULL) && (*helping_pointer[2] == 'g') &&
       (helping_pointer[3] != NULL) && (*helping_pointer[3] == 'b'))
     header.existence_of_colors = EINA_TRUE;

   free(helping_pointer);

   return header;
}

void
evas_model_load_file_ply(Evas_3D_Mesh *mesh, Eina_File *file)
{
   Evas_3D_Mesh_Data *pd;
   int i = 0, j = 0, k = 0, count_of_triangles_in_line = 0;
   float *pos, *nor, *tex, *col;
   int stride_pos, stride_nor, stride_tex, stride_col;
   char *current, *map;
   PLY_Header header;
   float *_vertices_ply = NULL, *_normals_ply = NULL;
   float *_tex_coords_ply = NULL, *_colors_ply = NULL;
   char **helping_pointer;

   map = eina_file_map_all(file, EINA_FILE_SEQUENTIAL);

   if (map == NULL)
     {
        ERR("Failed to create map from file %s\n", eina_file_filename_get(file));
        return;
     }

   header = _read_header(map);

   if (!header.existence_of_geometries)
     {
        ERR("File have not x, y, or z field as the first 3 float fields. They are necessary.");
        return;
     }

   helping_pointer = eina_str_split(map, "end_header\n", 0);

   if (helping_pointer == NULL)
     {
        ERR("File have not kayword end_header. It is necessary.");
        return;
     }

   current = helping_pointer[1];

   _vertices_ply = malloc(header.vertices_count * 3 * sizeof(float));
   if (header.existence_of_normals)
     _normals_ply = malloc(header.vertices_count * 3 * sizeof(float));
   if (header.existence_of_texcoords)
     _tex_coords_ply = malloc(header.vertices_count * 2 * sizeof(float));
   if (header.existence_of_colors)
     _colors_ply = malloc(header.vertices_count * 3 * sizeof(float));
   int *_triangles = malloc(header.triangles_count * 3 * sizeof(int));

   if ((header.existence_of_geometries && (_vertices_ply == NULL)) ||
       (header.existence_of_normals && (_normals_ply == NULL)) ||
       (header.existence_of_texcoords && (_tex_coords_ply == NULL)) ||
       (header.existence_of_colors && (_colors_ply == NULL)) ||
       (_triangles == NULL))
     {
        ERR("Allocate memory is failed.");
        free(_vertices_ply);
        free(_normals_ply);
        free(_tex_coords_ply);
        free(_colors_ply);
        free(_triangles);
        return;
     }

   for (i = 0; i < header.vertices_count; i++)
     {
        current = _read_data(_vertices_ply, i, 3, current, 1.0);
        if (header.existence_of_normals)
          current = _read_data(_normals_ply, i, 3, current, 1.0);
        if (header.existence_of_texcoords)
          current = _read_data(_tex_coords_ply, i, 2, current, 1.0);
        if (header.existence_of_colors)
          current = _read_data(_colors_ply, i, 3, current, 255.0);
        current = _to_begin_of_line(current);
     }

   for (i = 0; i < header.triangles_count;)
     {
        sscanf (current,"%d", &count_of_triangles_in_line);
        count_of_triangles_in_line -= 2;
        current = _to_next_number(current, 1);

        sscanf (current,"%i", &ARRAY_2D(_triangles, i, 0, 3));

        for (j = 0; j < count_of_triangles_in_line; j++)
          {
             if (j > 0)
               ARRAY_2D(_triangles, i, 0, 3) = ARRAY_2D(_triangles, (i - 1), 0, 3);
             current = _to_next_number(current, 1);
             sscanf (current,"%i %i",
                     &ARRAY_2D(_triangles, i, 1, 3),
                     &ARRAY_2D(_triangles, i, 2, 3));
             i++;
          }
        current = _to_next_line(current);
     }

   /* prepare of mesh and take pointers to data which must be read */
   eo_do(mesh,
         evas_3d_mesh_vertex_count_set(header.triangles_count * 3),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION, 0, NULL),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,   0, NULL),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_TEXCOORD, 0, NULL),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_COLOR,    0, NULL),

         pos = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_POSITION),
         nor = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_NORMAL),
         tex = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_TEXCOORD),
         col = (float *)evas_3d_mesh_frame_vertex_data_map(0, EVAS_3D_VERTEX_COLOR),

         stride_pos = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_POSITION),
         stride_nor = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_NORMAL),
         stride_tex = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_TEXCOORD),
         stride_col = evas_3d_mesh_frame_vertex_stride_get(0, EVAS_3D_VERTEX_COLOR));

   if (stride_pos == 0) stride_pos = sizeof(float) * 3;
   if (stride_nor == 0) stride_nor = sizeof(float) * 3;
   if (stride_tex == 0) stride_tex = sizeof(float) * 2;
   if (stride_col == 0) stride_col = sizeof(float) * 4;

   for (j = 0; j < header.triangles_count; j++)
     {
        for (k = 0; k < 3; k++)
          {
             float *p, *n, *t, *c;

             p = (float *)((char *)pos + stride_pos * (j * 3 + k));
             n = (float *)((char *)nor + stride_nor * (j * 3 + k));
             t = (float *)((char *)tex + stride_tex * (j * 3 + k));
             c = (float *)((char *)col + stride_col * (j * 3 + k));

             p[0] = ARRAY_2D(_vertices_ply, ARRAY_2D(_triangles, j, k, 3), 0, 3);
             p[1] = ARRAY_2D(_vertices_ply, ARRAY_2D(_triangles, j, k, 3), 1, 3);
             p[2] = ARRAY_2D(_vertices_ply, ARRAY_2D(_triangles, j, k, 3), 2, 3);

             if (header.existence_of_normals)
               {
                  n[0] = ARRAY_2D(_normals_ply, ARRAY_2D(_triangles, j, k, 3), 0, 3);
                  n[1] = ARRAY_2D(_normals_ply, ARRAY_2D(_triangles, j, k, 3), 1, 3);
                  n[2] = ARRAY_2D(_normals_ply, ARRAY_2D(_triangles, j, k, 3), 2, 3);
               }
             else
               {
                  n[0] = 0.0;
                  n[1] = 0.0;
                  n[2] = 0.0;
               }

             if (header.existence_of_texcoords)
               {
                  t[0] = ARRAY_2D(_tex_coords_ply, ARRAY_2D(_triangles, j, k, 3), 0, 2);
                  t[1] = ARRAY_2D(_tex_coords_ply, ARRAY_2D(_triangles, j, k, 3), 1, 2);
               }
             else
               {
                  t[0] = 0.0;
                  t[1] = 0.0;
               }

             if (header.existence_of_colors)
               {
                  c[0] = ARRAY_2D(_colors_ply, ARRAY_2D(_triangles, j, k, 3), 0, 3);
                  c[1] = ARRAY_2D(_colors_ply, ARRAY_2D(_triangles, j, k, 3), 1, 3);
                  c[2] = ARRAY_2D(_colors_ply, ARRAY_2D(_triangles, j, k, 3), 2, 3);
                  c[3] = 1.0;
               }
             else
               {
                  c[0] = 0.0;
                  c[1] = 0.0;
                  c[2] = 0.0;
                  c[3] = 1.0;
               }
          }
     }

   free(helping_pointer);
   free(_vertices_ply);
   if (header.existence_of_normals)
     free(_normals_ply);
   if (header.existence_of_texcoords)
     free(_tex_coords_ply);
   if (header.existence_of_colors)
     free(_colors_ply);
   free(_triangles);

        /* Unmap vertex buffer. */
   eo_do(mesh,
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_POSITION),
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_NORMAL),
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_TEXCOORD),
         evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_COLOR));

   pd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);

   if (!evas_3d_mesh_aabb_add_to_frame(pd, 0, stride_pos))
     {
        ERR("Axis-Aligned Bounding Box wan't added in frame %d ", 0);
     }

   if (map)
     {
        eina_file_map_free(file, map);
        map = NULL;
     }
}

