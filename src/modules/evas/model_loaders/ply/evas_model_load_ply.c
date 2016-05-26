#include "evas_model_load_save_common.h"

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

static inline Eina_Bool
_read_ply_header(char *map, Evas_Model_Load_Save_Header *header)
{
   eina_init();

   Eina_Bool reading_vertices = EINA_TRUE, check_next_char = EINA_FALSE;
   int vertex_lines, triangles = 0, vertices_in_current_face = 0;
   char **helping_pointer;
   char *current;

   helping_pointer = eina_str_split(map, "vertex ", 0);

   if (helping_pointer == NULL)
     {
        ERR("File have not kayword vertex. It is necessary.");
        return EINA_FALSE;
     }

   sscanf(helping_pointer[1], "%d", &header->vertices_count);

   free(helping_pointer);
   helping_pointer = eina_str_split(map, "end_header\n", 0);

   if (helping_pointer == NULL)
     {
        ERR("File have not kayword end_header. It is necessary.");
        return EINA_FALSE;
     }

   current = helping_pointer[1];

   vertex_lines = header->vertices_count;
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

   header->indices_count = 3 * triangles;

      /* analyse flags used when file was saved in blender */
   helping_pointer = eina_str_split(map, "property float ", 0);

   if ((helping_pointer[1] != NULL) && (*helping_pointer[1] == 'x') &&
       (helping_pointer[2] != NULL) && (*helping_pointer[2] == 'y') &&
       (helping_pointer[3] != NULL) && (*helping_pointer[3] == 'z'))
     header->existence_of_positions = EINA_TRUE;
   else return EINA_FALSE;

   if ((helping_pointer[4] != NULL) && (*helping_pointer[4] == 'n') &&
       (helping_pointer[5] != NULL) && (*helping_pointer[5] == 'n') &&
       (helping_pointer[6] != NULL) && (*helping_pointer[6] == 'n'))
     header->existence_of_normals = EINA_TRUE;

   if ((header->existence_of_normals &&
       ((helping_pointer[7] != NULL) && (*helping_pointer[7] == 's') &&
       (helping_pointer[8] != NULL) && (*helping_pointer[8] == 't'))) ||
       (!header->existence_of_normals &&
       ((helping_pointer[4] != NULL) && (*helping_pointer[4] == 's') &&
       (helping_pointer[5] != NULL) && (*helping_pointer[5] == 't'))))
     header->existence_of_tex_coords = EINA_TRUE;

   helping_pointer = eina_str_split(map, "property uchar ", 0);

   if ((helping_pointer[1] != NULL) && (*helping_pointer[1] == 'r') &&
       (helping_pointer[2] != NULL) && (*helping_pointer[2] == 'g') &&
       (helping_pointer[3] != NULL) && (*helping_pointer[3] == 'b'))
     header->existence_of_colors = EINA_TRUE;

   if (!header->existence_of_positions)
     {
        ERR("File have not x, y, or z field as the first 3 float fields. They are necessary.");
        return EINA_FALSE;
     }

   free(helping_pointer);
   return EINA_TRUE;
}

static inline void
_read_ply_vertex_data(Evas_Model_Load_Save_Header header,
                      char **current,
                      Evas_Model_Load_Save_Data data)
{
   int i;
   for (i = 0; i < header.vertices_count; i++)
     {
        *current = _read_data(data.positions, i, 3, *current, 1.0);
        if (header.existence_of_normals)
          *current = _read_data(data.normals, i, 3, *current, 1.0);
        if (header.existence_of_tex_coords)
          *current = _read_data(data.tex_coords, i, 2, *current, 1.0);
        if (header.existence_of_colors)
          *current = _read_data(data.colors, i, 3, *current, 255.0);
        *current = _to_begin_of_line(*current);
     }
}

static inline void
_read_ply_indices_data(Evas_Model_Load_Save_Header header,
                       char **current,
                       Evas_Model_Load_Save_Data data)
{
   int i, j, count_of_triangles_in_line = 0;

   for (i = 0; i < header.indices_count;)
     {
        sscanf (*current,"%d", &count_of_triangles_in_line);
        count_of_triangles_in_line -= 2;
        *current = _to_next_number(*current, 1);

        sscanf (*current,"%hu", data.indices + i);

        for (j = 0; j < count_of_triangles_in_line; j++)
          {
             if (j > 0)
               data.indices[i] = data.indices[i - 3];
             *current = _to_next_number(*current, 1);
             sscanf (*current,"%hu %hu",
                     data.indices + i + 1,
                     data.indices + i + 2);
             i+=3;
          }
        *current = _to_next_line(*current);
     }
}

void
evas_model_load_file_ply(Evas_Canvas3D_Mesh *mesh, Eina_File *file)
{
   char *current = NULL, *map = NULL;
   Evas_Model_Load_Save_Header header = { 0, 0, 0, 0, 0, 0 };
   Evas_Model_Load_Save_Data data = { NULL, NULL, NULL, NULL, NULL };
   Evas_Model_Load_Save_Stride stride = { 0, 0, 0, 0 };

   map = eina_file_map_all(file, EINA_FILE_SEQUENTIAL);
   if (map == NULL)
     {
        ERR("Failed to create map from file %s\n", eina_file_filename_get(file));
        return;
     }

   header = evas_model_load_save_header_new();
   if(!_read_ply_header(map, &header)) return;

   if (!evas_model_load_allocate_data_due_to_header(header, &data))
     {
        ERR("Memory allocation is failed.");
        return;
     }

   current = eina_str_split(map, "end_header\n", 0)[1];
   _read_ply_vertex_data(header, &current, data);
   _read_ply_indices_data(header, &current, data);
   evas_model_load_vertex_data_to_mesh(mesh, header, data, &stride);
   evas_model_load_indices_data_to_mesh(mesh, header, data);
   evas_model_load_vertex_data_unmap(mesh, 0, header);
   evas_model_load_aabb_add_to_frame(mesh, 0, stride);

   if (map)
     {
        eina_file_map_free(file, map);
        map = NULL;
     }

   evas_model_load_save_data_free(header, &data);
}
