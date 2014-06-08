#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"

/* set value to position [x][y] to array name which have. */
#define ARRAY_2D(name, x, y, count_y) (*(name + x * count_y + y))

/* read 3 float values in string and put it in array */
#define PUT_DATA_TO_ARRAY(array_name, name)                         \
        sscanf (current,"%f %f %f",                         \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 0, 3),   \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 1, 3),   \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 2, 3));  \
     counts.current_##name##_counter++;

/* Structures for reading data from file. */
typedef struct _OBJ_Counts    OBJ_Counts;

struct _OBJ_Counts
{
   int _vertex_counter;
   int _normal_counter;
   int _texture_point_counter;
   int _triangles_counter;

   int current_vertex_counter;
   int current_normal_counter;
   int current_texture_point_counter;
   int current_triangles_counter;

   Eina_Bool existence_of_normal;
   Eina_Bool existence_of_tex_point;
};

static inline char*
_file_to_buf(const char *file, long *length)//prepare text file for reading
{
   FILE *file_for_print;
   char *buf;
   int unused __attribute__((unused));//this variable fixes warning "ignoring return value of fread"

   *length = 0;
   file_for_print = fopen(file, "rb");
   if (!file_for_print) return NULL;
   fseek(file_for_print, 0, SEEK_END);//set file_for_print to the end of file
   *length = ftell(file_for_print);//set current position of file_for_print
   buf = malloc(*length + 1);
   fseek(file_for_print, 0, SEEK_SET);//set file_for_print to the begining of file
   unused = fread(buf, *length, 1, file_for_print);
   fclose(file_for_print);
   buf[*length] = '\0';
   return buf;
}

/* create new counter */
static inline OBJ_Counts
_new_count_elements()
{
   OBJ_Counts counts;

   counts._vertex_counter = 0;
   counts._normal_counter = 0;
   counts._texture_point_counter = 0;
   counts._triangles_counter = 0;

   counts.current_vertex_counter = 0;
   counts.current_normal_counter = 0;
   counts.current_texture_point_counter = 0;
   counts.current_triangles_counter = 0;

   counts.existence_of_normal = EINA_FALSE;
   counts.existence_of_tex_point = EINA_FALSE;
   return counts;
}

/* count triangles in face */
static void
_analyze_face_line(char * face_analyzer,
                   int * count_of_triangles_in_line)
{
   int polygon_checker = -2;
   Eina_Bool previous_is_space = EINA_TRUE;
   while ((*face_analyzer != '\n') && (*face_analyzer != '#'))
     {
        if (*face_analyzer == ' ')
          {
             previous_is_space = EINA_TRUE;
          }
        else if ((previous_is_space) && (*face_analyzer  >= '0') && (*face_analyzer  <= '9'))
          {
             polygon_checker++;
             previous_is_space = EINA_FALSE;
          }
        face_analyzer++;
     }
   *count_of_triangles_in_line = polygon_checker;
}

static inline OBJ_Counts
_count_elements(char *start, long length)//count elements of mesh in .obj
{
   OBJ_Counts counts = _new_count_elements();

   char * current = start;
   int polygon_checker = -2;//polygons with n vertices can be represented as n-2 triangles
   Eina_Bool will_check_next_char = EINA_FALSE;
   Eina_Bool first_char_is_v = EINA_FALSE;
   Eina_Bool first_char_is_f = EINA_FALSE;
   Eina_Bool previous_is_space = EINA_FALSE;

   long i = 0;
   /* count elements of mesh in .obj */
   for (; length > i; i++)
     {
        if (will_check_next_char)
          {
             if (first_char_is_v)
               {
                  switch (*current)
                    {
                     case ' ':
                       counts._vertex_counter++;
                       break;
                     case 't':
                       counts._texture_point_counter++;
                       counts.existence_of_tex_point = EINA_TRUE;
                       break;
                     case 'n':
                       counts._normal_counter++;
                       counts.existence_of_normal = EINA_TRUE;
                       break;
                     default:
                       break;
                    }
                  first_char_is_v = EINA_FALSE;
                  will_check_next_char = EINA_FALSE;
               }
             else if (first_char_is_f)
               {
                  switch (*current)
                    {
                     case '\n':
                       first_char_is_f = EINA_FALSE;
                       counts._triangles_counter += polygon_checker;
                       polygon_checker = -2;
                       previous_is_space = EINA_FALSE;
                       break;
                     case '#':
                       first_char_is_f = EINA_FALSE;
                       counts._triangles_counter += polygon_checker;
                       polygon_checker = -2;
                       previous_is_space = EINA_FALSE;
                       break;
                     case ' ':
                       previous_is_space = EINA_TRUE;
                       break;
                     default:
                       if ((previous_is_space) && (*current  >= '0') && (*current  <= '9'))
                         {
                            polygon_checker++;
                            previous_is_space = EINA_FALSE;
                         }
                       break;
                    }
               }
             else
               {
                  switch (*current)
                    {
                     case 'v':
                       first_char_is_v = EINA_TRUE;
                       break;
                     case 'f':
                       first_char_is_f = EINA_TRUE;
                       break;
                     case 'm':
                       will_check_next_char = EINA_FALSE;
                       break;
                     default:
                       will_check_next_char = EINA_FALSE;
                       break;
                    }
               }
          }
        else if (*current == '\n')
          {
             will_check_next_char = EINA_TRUE;
          }
        current++;
     }
   return counts;
}

void
evas_3d_mesh_file_obj_set(Evas_3D_Mesh *mesh, const char *file)
{
   long length, i;
   char * start = _file_to_buf(file, &length);
   OBJ_Counts counts = _count_elements(start, length);//count elements of mesh in .obj
   Eina_Bool will_check_next_char = EINA_FALSE;
   Eina_Bool first_char_is_v = EINA_FALSE;
   Eina_Bool first_char_is_f = EINA_FALSE;
   float *pos, *nor, *tex;
   int stride_pos, stride_nor, stride_tex;
   int j, k;
   char * current;

   float *_vertices_obj = malloc(counts._vertex_counter * 3 * sizeof(float));
   float *_normales_obj = malloc(counts._normal_counter * 3 * sizeof(float));
   float *_tex_coords_obj = malloc(counts._texture_point_counter * 3 * sizeof(float));
   /* triangle has 3 points, every point has 3(vertix, texture and normal) coord */
   int *_triangles = malloc(counts._triangles_counter * 9 * sizeof(int));

   if ((start == NULL) || (_vertices_obj == NULL) ||
        (_normales_obj == NULL) || (_tex_coords_obj == NULL) || (_triangles == NULL))
     {
        ERR("Allocate memory is failed.");
        free(start);
        free(_vertices_obj);
        free(_normales_obj);
        free(_tex_coords_obj);
        free(_triangles);
        return;
     }

   current = start;
   i = 0;

   /* put data to arrays */
   for (; length > i; i++)
     {
        if (will_check_next_char)
          {
             if (first_char_is_v)
               {
                  switch (*current)
                    {
                     case ' ':
                       PUT_DATA_TO_ARRAY(vertices, vertex)
                       i--;
                       break;
                     case 't':
                       current++;
                       PUT_DATA_TO_ARRAY(tex_coords, texture_point)
                       break;
                     case 'n':
                       current++;
                       PUT_DATA_TO_ARRAY(normales, normal)
                       break;
                     default:
                       break;
                    }
                  first_char_is_v = EINA_FALSE;
                  will_check_next_char = EINA_FALSE;
               }
             else if (first_char_is_f)
               {
                  char * auxiliary_pointer = current;
                  int count_of_triangles_in_line;

                  _analyze_face_line(auxiliary_pointer,
                                   &count_of_triangles_in_line);
                  current++;
                  i++;
                  int first_pos, first_tex, first_norm;
                  sscanf (current,"%i/%i/%i",
                            &first_pos,
                            &first_tex,
                            &first_norm);

                  do
                    {
                       current++;
                       i++;
                    }
                  while (*current != ' ');

                  current++;
                  i++;

                  for (j = 0; j < count_of_triangles_in_line; j++)
                    {
                       auxiliary_pointer = current;
                       ARRAY_2D(_triangles, counts.current_triangles_counter, 0, 9) = first_pos;
                       ARRAY_2D(_triangles, counts.current_triangles_counter, 1, 9) = first_tex;
                       ARRAY_2D(_triangles, counts.current_triangles_counter, 2, 9) = first_norm;
                       sscanf (auxiliary_pointer,"%i/%i/%i %i/%i/%i",
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 3, 9),
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 4, 9),
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 5, 9),
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 6, 9),
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 7, 9),
                         &ARRAY_2D(_triangles, counts.current_triangles_counter, 8, 9));

                       while (*current != ' ')
                         {
                            current++;
                            i++;
                         }

                       counts.current_triangles_counter++;
                    }
                  first_char_is_f = EINA_FALSE;
               }
             else
               {
                  switch (*current)
                    {
                     case 'v':
                       first_char_is_v = EINA_TRUE;
                       break;
                     case 'f':
                       first_char_is_f = EINA_TRUE;
                       break;
                     case 'm':
                       will_check_next_char = EINA_FALSE;
                       break;
                     default:
                       will_check_next_char = EINA_FALSE;
                       break;
                    }
               }
          }
        else if (*current == '\n')
          {
             will_check_next_char = EINA_TRUE;
          }
        current++;
     }
   free(start);

   /* prepare of mesh and take pointers to data which must be read */
   eo_do(mesh,
         evas_3d_mesh_vertex_count_set(counts._triangles_counter * 3),
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

   for (j = 0; j < counts._triangles_counter; j++)
     {
        for (k = 0; k < 3; k++)
          {
             float *p, *n, *t;

             p = (float *)((char *)pos + stride_pos * (j * 3 + k));
             n = (float *)((char *)nor + stride_nor * (j * 3 + k));
             t = (float *)((char *)tex + stride_tex * (j * 3 + k));

             p[0] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 0, 3);
             p[1] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 1, 3);
             p[2] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 2, 3);

             n[0] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 0, 3);
             n[1] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 1, 3);
             n[2] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 2, 3);

             t[0] = ARRAY_2D(_tex_coords_obj, (ARRAY_2D(_triangles, j, (3 * k + 1), 9) - 1), 0, 3);
             t[1] = ARRAY_2D(_tex_coords_obj, (ARRAY_2D(_triangles, j, (3 * k + 1), 9) - 1), 1, 3);
          }
     }
   free(_vertices_obj);
   free(_normales_obj);
   free(_tex_coords_obj);
   free(_triangles);
        /* Unmap vertex buffer. */
   eo_do(mesh,
     evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_POSITION),
     evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_NORMAL),
     evas_3d_mesh_frame_vertex_data_unmap(0, EVAS_3D_VERTEX_TEXCOORD));

   Evas_3D_Mesh_Data *pd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);

   if (!evas_3d_mesh_aabb_add_to_frame(pd, 0, stride_pos))
     {
        ERR("Axis-Aligned Bounding Box wan't added in frame %d ", 0);
     }
}
