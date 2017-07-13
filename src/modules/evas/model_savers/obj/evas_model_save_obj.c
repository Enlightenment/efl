#include "evas_model_load_save_common.h"

static unsigned short*
_init_obj_indices_data(Evas_Model_Load_Save_Header header)
{
   unsigned short *i_data;
   int i = 0;

   if (header.existence_of_positions) i++;
   if (header.existence_of_normals) i++;
   if (header.existence_of_tex_coords) i++;
   i_data = calloc(header.vertices_count * i, sizeof(unsigned short));

   return i_data;
}

static void
_vertex_data_free_cb(void *data)
{
   eina_stringshare_del(data);
}

static void
_write_point(FILE *obj_file,
             int num,
             int num_of_point,
             Evas_Model_Load_Save_Header header,
             unsigned short *i_data)
{
   if (num_of_point == 0)
     fprintf(obj_file, "f ");

   if (header.existence_of_normals)
     {
        if (header.existence_of_tex_coords)
          fprintf(obj_file, "%hu/%hu/%hu ", i_data[num],
                  i_data[num + header.vertices_count],
                  i_data[num + 2 * header.vertices_count]);
        else
          fprintf(obj_file, "%hu//%hu ", i_data[num],
                  i_data[num + header.vertices_count]);
     }
   else
     {
        if (header.existence_of_tex_coords)
          fprintf(obj_file, "%hu/%hu ", i_data[num],
                  i_data[num + header.vertices_count]);
        else
          fprintf(obj_file, "%hu ", i_data[num]);
     }

   if (num_of_point == 2)
     fprintf(obj_file, "\n");
}

static inline Eina_Bool
_write_obj_header(FILE *file,
                  const char *_mtl_file_name)
{
   time_t current_time;
   char* c_time_string;

   current_time = time(NULL);

   if (current_time == ((time_t)-1))
     {
        ERR("Failure to compute the current time.");
        return EINA_FALSE;
     }

   c_time_string = ctime(&current_time);

   if (c_time_string == NULL)
     {
        ERR("Failure to convert the current time.");
        return EINA_FALSE;
     }

   fprintf(file, "# Evas_Canvas3D saver OBJ v0.03 \n");
   fprintf(file, "# Current time is %s \n", c_time_string);
   fprintf(file, "mtllib %s \n\n", _mtl_file_name);

   return EINA_TRUE;
}

static inline void
_write_obj_vertex_data(FILE *file,
                       Evas_Model_Load_Save_Header header,
                       Evas_Model_Load_Save_Data data,
                       unsigned short *i_data)
{
   int i, j;
   int v = -1;
   Eina_Stringshare *str, *cur_str, *cur_index;
   unsigned short cur_hu;

   Eina_Hash *vb;
#define WRITE_OBJ_VERTEX_DATA(name, num, format)                      \
   if (header.existence_of_##name)                                    \
     {                                                                \
        cur_hu = 0;                                                   \
        v++;                                                          \
        vb = eina_hash_string_superfast_new(_vertex_data_free_cb);    \
        for (i = 0; i < header.vertices_count; i++)                   \
          {                                                           \
             str = eina_stringshare_printf(" ");                      \
             for (j = 0; j < num; j++)                                \
               str = eina_stringshare_printf("%s %f", str,            \
                                             data.name[i * num + j]); \
             cur_index = eina_hash_find(vb, str);                     \
             if (!cur_index)                                          \
               {                                                      \
                  cur_hu++;                                           \
                  cur_str = eina_stringshare_printf("%hu", cur_hu);   \
                  eina_hash_add(vb, str, cur_str);                    \
                  i_data[v * header.vertices_count + i] = cur_hu;     \
                  fprintf(file, "%s%s\n", format, str);               \
               }                                                      \
             else sscanf(cur_index, "%hu",                            \
                         i_data + v * header.vertices_count + i);     \
          }                                                           \
        eina_hash_free(vb);                                           \
     }
   WRITE_OBJ_VERTEX_DATA(positions, 3, "v")
   WRITE_OBJ_VERTEX_DATA(tex_coords, 2, "vt")
   WRITE_OBJ_VERTEX_DATA(normals, 3, "vn")
#undef WRITE_OBJ_VERTEX_DATA
}

static inline void
_write_obj_index_data(FILE *file,
                      Evas_Model_Load_Save_Header header,
                      Evas_Model_Load_Save_Data data,
                      unsigned short *i_data)
{
   int i;
   int ic;
   ic = header.indices_count ? header.indices_count : header.vertices_count;
   for (i = 0; i < ic; i++)
     _write_point(file, data.indices[i], i % 3, header, i_data);
}

static void
_save_mesh(Evas_Canvas3D_Mesh_Data *pd,
           const char *_obj_file_name,
           const char *_mtl_file_name,
           Evas_Canvas3D_Mesh_Frame *f)
{
   Evas_Model_Load_Save_Header header;
   Evas_Model_Load_Save_Data data;
   unsigned short *index_data;

   if (!evas_model_save_header_from_mesh(pd, f, &header)) return;

   index_data = _init_obj_indices_data(header);
   if (index_data == NULL)
     {
        ERR("Allocation of index data is failed.");
        return;
     }

   evas_model_save_data_from_mesh(pd, f, header, &data);

   FILE * _obj_file = fopen(_obj_file_name, "w+");
   if (!_obj_file)
     {
        ERR("File open '%s' for save failed", _obj_file_name);
        free(index_data);
        return;
     }

   if (!_write_obj_header(_obj_file, _mtl_file_name))
     {
        fclose(_obj_file);
        free(index_data);
        return;
     }

   _write_obj_vertex_data(_obj_file, header, data, index_data);
   fprintf(_obj_file,"usemtl Material\n s off\n");
   _write_obj_index_data(_obj_file, header, data, index_data);
   fclose(_obj_file);
   free(index_data);
   free(data.indices);
}

static void
_save_material(Evas_Canvas3D_Mesh_Data *pd EINA_UNUSED,
               const char *_mtl_file_name,
               Evas_Canvas3D_Material_Data *mat)
{
   FILE * _mtl_file = fopen(_mtl_file_name, "w+");
   if (!_mtl_file)
     {
        ERR("File open '%s' for save failed", _mtl_file_name);
        return;
     }
   fprintf(_mtl_file, "# Evas_Canvas3D saver OBJ v0.03 \n");//_mtl_file created in macro
   fprintf(_mtl_file, "# Material Count: 1 \n\n");
   fprintf(_mtl_file, "newmtl Material \n");
   fprintf(_mtl_file, "Ns 1.000000 \n");//exp factor for specular highlight
   fprintf(_mtl_file, "Ka %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.b);
   fprintf(_mtl_file, "Kd %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.b);
   fprintf(_mtl_file, "Ks %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.b);
   fprintf(_mtl_file, "Ni 1.000000 \n");//optical density
   fprintf(_mtl_file, "d 1.000000 \n");//a factor d of 1.0 is fully opaque material
   fprintf(_mtl_file, "illum 2 \n");//illumination properties

   fclose(_mtl_file);
}

void
evas_model_save_file_obj(const Evas_Canvas3D_Mesh *mesh,
                         const char *_obj_file_name,
                         Evas_Canvas3D_Mesh_Frame *f)
{
   int len;
   char *_mtl_file_name, *_without_extention;
   Evas_Canvas3D_Material_Data *mat;

   len = strlen(_obj_file_name);
   _without_extention = alloca(len - 3);
   _mtl_file_name = alloca(len + 1);
   eina_strlcpy(_without_extention, _obj_file_name, len - 3);
   eina_str_join(_mtl_file_name, len + 1, '.', _without_extention, "mtl");

   Evas_Canvas3D_Mesh_Data *pd = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);
   mat = efl_data_scope_get(f->material, EVAS_CANVAS3D_MATERIAL_CLASS);

   if (mat != NULL) _save_material(pd, _mtl_file_name, mat);
   _save_mesh(pd, _obj_file_name, _mtl_file_name, f);
}
