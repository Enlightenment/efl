#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"

//TODO Increase of stability (reading .obj file saved with any flags).
/* set value to position [x][y] to array name which have. */
#define ARRAY_2D(name, x, y, count_y) (*(name + x * count_y + y))

/* read 3 float values in string and put it in array */
#define PUT_DATA_TO_ARRAY(array_name, name)                                      \
        sscanf (current,"%f %f %f",                                              \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 0, 3),   \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 1, 3),   \
        &ARRAY_2D(_##array_name##_obj, counts.current_##name##_counter, 2, 3));  \
     counts.current_##name##_counter++;

#define AFTER_NEXT_SPACE(pointer)\
        do                       \
          {                      \
             pointer++;          \
             i++;                \
          }                      \
        while (*pointer != ' ');

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
_count_elements(char *map)//count elements of mesh in .obj
{
   OBJ_Counts counts = _new_count_elements();

   char *current = map;
   int polygon_checker = -2;//polygons with n vertices can be represented as n-2 triangles
   Eina_Bool will_check_next_char = EINA_FALSE;
   Eina_Bool first_char_is_v = EINA_FALSE;
   Eina_Bool first_char_is_f = EINA_FALSE;
   Eina_Bool previous_is_space = EINA_FALSE;

   long i = 0;
   /* count elements of mesh in .obj */
   for (; *current != '\00'; i++)
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
        if (*current == '\n')
          {
             will_check_next_char = EINA_TRUE;
          }
        current++;
     }
   return counts;
}

static void
_read_point(int *triangles,
            int num,
            OBJ_Counts counts,
            int num_cur,
            char *pointer)
{
   if (counts.existence_of_normal)
     {
        if (counts.existence_of_tex_point)
          sscanf(pointer, "%i/%i/%i",
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3, 9),
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3 + 1, 9),
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3 + 2, 9));
        else
          sscanf(pointer, "%i//%i",
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3, 9),
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3 + 2, 9));
     }
   else
     {
        if (counts.existence_of_tex_point)
          sscanf(pointer, "%i/%i",
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3, 9),
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3 + 1, 9));
        else
          sscanf(pointer, "%i",
                 &ARRAY_2D(triangles, num_cur, (num - 1) * 3, 9));
     }
}

void
evas_model_load_file_obj(Evas_Canvas3D_Mesh *mesh, Eina_File *file)
{
   long i;
   OBJ_Counts counts;//count elements of mesh in .obj
   Eina_Bool will_check_next_char = EINA_FALSE;
   Eina_Bool first_char_is_v = EINA_FALSE;
   Eina_Bool first_char_is_f = EINA_FALSE;
   float *pos = NULL, *nor = NULL, *tex = NULL;
   int stride_pos = 0, stride_nor = 0, stride_tex = 0;
   int j, k, data_for_one_point;
   char *current, *map;
   float *_vertices_obj = NULL, *_normales_obj = NULL, *_tex_coords_obj = NULL;
   int *_triangles;

   map = eina_file_map_all(file, EINA_FILE_SEQUENTIAL);

   if (map == NULL)
     {
        ERR("Failed to create map from file %s\n", eina_file_filename_get(file));
        return;
     }

   counts = _count_elements(map);
   _vertices_obj = malloc(counts._vertex_counter * 3 * sizeof(float));
   data_for_one_point = 1;
   if (counts.existence_of_normal)
     {
        data_for_one_point++;
        _normales_obj = malloc(counts._normal_counter * 3 * sizeof(float));
     }
   if (counts.existence_of_tex_point)
     {
        data_for_one_point++;
        _tex_coords_obj = malloc(counts._texture_point_counter * 3 * sizeof(float));
     }
   _triangles = malloc(counts._triangles_counter * 9 * sizeof(int));

   if ((map == NULL) || (_vertices_obj == NULL) || (_triangles == NULL) ||
       ((counts.existence_of_normal) && (_normales_obj == NULL)) ||
       ((counts.existence_of_tex_point) && (_tex_coords_obj == NULL)))
     {
        ERR("Allocate memory is failed.");
        free(_vertices_obj);
        free(_triangles);
        if (counts.existence_of_normal)
          free(_normales_obj);
        if (counts.existence_of_tex_point)
          free(_tex_coords_obj);
        return;
     }

   current = map;
   i = 0;
   /* put data to arrays */
   for (; *current != '\00'; i++)
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
                       if (counts.existence_of_tex_point)
                         {
                            PUT_DATA_TO_ARRAY(tex_coords, texture_point)
                         }
                       break;
                     case 'n':
                       current++;
                       if (counts.existence_of_normal)
                         {
                            PUT_DATA_TO_ARRAY(normales, normal)
                         }
                       break;
                     default:
                       break;
                    }
                  first_char_is_v = EINA_FALSE;
                  will_check_next_char = EINA_FALSE;
               }
             else if (first_char_is_f)
               {
                  char *auxiliary_pointer = current;
                  int count_of_triangles_in_line;
                  int the_first_point = counts.current_triangles_counter;

                  _analyze_face_line(auxiliary_pointer,
                                   &count_of_triangles_in_line);
                  current++;
                  i++;
                  _read_point(_triangles, 1, counts,
                              the_first_point,
                              current);

                  AFTER_NEXT_SPACE(current)

                  for (j = 0; j < count_of_triangles_in_line; j++)
                    {
                       auxiliary_pointer = current;
                       if (counts.current_triangles_counter != the_first_point)
                         {
                            ARRAY_2D(_triangles, counts.current_triangles_counter, 0, 9) = \
                            ARRAY_2D(_triangles, the_first_point, 0, 9);
                            ARRAY_2D(_triangles, counts.current_triangles_counter, 1, 9) = \
                            ARRAY_2D(_triangles, the_first_point, 1, 9);
                            ARRAY_2D(_triangles, counts.current_triangles_counter, 2, 9) = \
                            ARRAY_2D(_triangles, the_first_point, 2, 9);
                         }

                       _read_point(_triangles, 2, counts,
                                   counts.current_triangles_counter,
                                   auxiliary_pointer);
                       AFTER_NEXT_SPACE(auxiliary_pointer);
                       _read_point(_triangles, 3, counts,
                                   counts.current_triangles_counter,
                                   auxiliary_pointer);
                       AFTER_NEXT_SPACE(current);

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
        if (*current == '\n')
          {
             will_check_next_char = EINA_TRUE;
          }
        current++;
     }

   /* prepare of mesh and take pointers to data which must be read */
   evas_canvas3d_mesh_vertex_count_set(mesh, counts._triangles_counter * 3);
   evas_canvas3d_mesh_vertex_assembly_set(mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_frame_add(mesh, 0);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 0, NULL);
   pos = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);
   stride_pos = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);

   if (counts.existence_of_normal)
     {
        evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, 0, NULL);
        nor = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
        stride_nor = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
     }

   if (counts.existence_of_tex_point)
     {
        evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, 0, NULL);
        tex = (float *)evas_canvas3d_mesh_frame_vertex_data_map(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);
        stride_tex = evas_canvas3d_mesh_frame_vertex_stride_get(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);
     }

   if (stride_pos == 0) stride_pos = sizeof(float) * 3;
   if ((counts.existence_of_normal) && (stride_nor == 0))
     stride_nor = sizeof(float) * 3;
   if ((counts.existence_of_tex_point) && (stride_tex == 0))
     stride_tex = sizeof(float) * 2;

   for (j = 0; j < counts._triangles_counter; j++)
     {
        for (k = 0; k < 3; k++)
          {
             float *p, *n, *t;

             p = (float *)((char *)pos + stride_pos * (j * 3 + k));
             if (counts.existence_of_normal)
               n = (float *)((char *)nor + stride_nor * (j * 3 + k));
             if (counts.existence_of_tex_point)
               t = (float *)((char *)tex + stride_tex * (j * 3 + k));

             p[0] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 0, 3);
             p[1] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 1, 3);
             p[2] = ARRAY_2D(_vertices_obj, (ARRAY_2D(_triangles, j, (3 * k), 9) - 1), 2, 3);

             if (counts.existence_of_normal)
               {
                  n[0] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 0, 3);
                  n[1] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 1, 3);
                  n[2] = ARRAY_2D(_normales_obj, (ARRAY_2D(_triangles, j, (3 * k + 2), 9) - 1), 2, 3);
               }

             if (counts.existence_of_tex_point)
               {
                  t[0] = ARRAY_2D(_tex_coords_obj, (ARRAY_2D(_triangles, j, (3 * k + 1), 9) - 1), 0, 3);
                  t[1] = ARRAY_2D(_tex_coords_obj, (ARRAY_2D(_triangles, j, (3 * k + 1), 9) - 1), 1, 3);
               }
          }
     }

   free(_vertices_obj);
   if (counts.existence_of_normal)
      free(_normales_obj);
   if (counts.existence_of_tex_point)
      free(_tex_coords_obj);
   free(_triangles);

        /* Unmap vertex buffer. */
   evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION);
   if (counts.existence_of_normal)
     evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL);
   if (counts.existence_of_tex_point)
     evas_canvas3d_mesh_frame_vertex_data_unmap(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD);

   Evas_Canvas3D_Mesh_Data *pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);

   if (!evas_canvas3d_mesh_aabb_add_to_frame(pd, 0, stride_pos))
     {
        ERR("Axis-Aligned Bounding Box wan't added in frame %d ", 0);
     }

   if (map)
     {
        eina_file_map_free(file, map);
        map = NULL;
     }
}
