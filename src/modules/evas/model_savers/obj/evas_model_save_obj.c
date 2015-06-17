#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"

#define OPEN_FILE(extension)\
   FILE * _##extension##_file = fopen(_##extension##_file_name, "w+");

#define SAVE_GEOMETRICS(a, format)                                 \
   vb = &f->vertices[a];                                           \
   if (vb->data != NULL)                                           \
     {                                                             \
        fprintf(_obj_file, "o %s\n",_obj_file_name);               \
        src = (float *)vb->data;                                   \
        for (i = 0; i < pd->vertex_count; i++)                     \
          {                                                        \
             fprintf(_obj_file, format, src[0], src[1]);           \
             if (a != EVAS_CANVAS3D_VERTEX_TEXCOORD)                     \
               fprintf(_obj_file, " %.4f", src[2]);                \
             fprintf(_obj_file, "\n");                             \
             src += f->vertices[a].element_count;                  \
          }                                                        \
     }

static void
_write_point(FILE * obj_file,
             int num,
             int num_of_point,
             Eina_Bool existence_of_normal,
             Eina_Bool existence_of_tex_point)
{
   if (num_of_point == 1)
     fprintf(obj_file, "f ");

   if (existence_of_normal)
     {
        if (existence_of_tex_point)
          fprintf(obj_file, "%i/%i/%i ", num, num, num);
        else
          fprintf(obj_file, "%i//%i ", num, num);
     }
   else
     {
        if (existence_of_tex_point)
          fprintf(obj_file, "%i/%i ", num, num);
        else
          fprintf(obj_file, "%i ", num);
     }

   if (num_of_point == 3)
     fprintf(obj_file, "\n");
}

static void
_save_mesh(Evas_Canvas3D_Mesh_Data *pd, const char *_obj_file_name, Evas_Canvas3D_Mesh_Frame *f)
{
   Evas_Canvas3D_Vertex_Buffer *vb;
   time_t current_time;
   char* c_time_string;
   int    i;
   float *src;
   Eina_Bool existence_of_normal, existence_of_tex_point;

   OPEN_FILE(obj)
   if (!_obj_file)
     {
        ERR("File open '%s' for save failed", _obj_file_name);
        return;
     }
   fprintf(_obj_file, "# Evas_3D saver OBJ v0.03 \n");//_obj_file created in macro
   /* Adding time comment to .obj file. */
   current_time = time(NULL);

   if (current_time == ((time_t)-1))
     {
        ERR("Failure to compute the current time.");
        fclose(_obj_file);
        return;
     }

   c_time_string = ctime(&current_time);

   if (c_time_string == NULL)
     {
        ERR("Failure to convert the current time.");
        fclose(_obj_file);
        return;
     }

   fprintf(_obj_file,"# Current time is %s \n", c_time_string);
   fprintf(_obj_file,"mtllib %s.mtl \n\n", _obj_file_name);

   /* Adding geometrics to file. */
   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        fclose(_obj_file);
        return;
     }

   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_POSITION, "v %.4f %.4f")
   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_NORMAL, "vn %.4f %.4f")
   SAVE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_TEXCOORD, "vt %.4f %.4f")

   existence_of_normal = (f->vertices[EVAS_CANVAS3D_VERTEX_NORMAL].data != NULL);
   existence_of_tex_point = (f->vertices[EVAS_CANVAS3D_VERTEX_TEXCOORD].data != NULL);

   fprintf(_obj_file,"usemtl Material\n s off\n");
   for (i = 1; i <= pd->vertex_count; i++)//numeration of faces in .obj started from 1
     {
        _write_point(_obj_file, i, 1, existence_of_normal, existence_of_tex_point);
        i++;
        _write_point(_obj_file, i, 2, existence_of_normal, existence_of_tex_point);
        i++;
        _write_point(_obj_file, i, 3, existence_of_normal, existence_of_tex_point);
     }
   fclose(_obj_file);
}

static void
_save_material(Evas_Canvas3D_Mesh_Data *pd EINA_UNUSED,
               const char *_mtl_file_name,
               Evas_Canvas3D_Material_Data *mat)
{
   OPEN_FILE(mtl)
   if (!_mtl_file)
     {
        ERR("File open '%s' for save failed", _mtl_file_name);
        return;
     }
   fprintf(_mtl_file, "# Evas_3D saver OBJ v0.03 \n");//_mtl_file created in macro
   fprintf(_mtl_file, "# Material Count: 1 \n\n");
   fprintf(_mtl_file, "newmtl Material \n");
   fprintf(_mtl_file, "Ns 1.000000 \n");//exp factor for specular highlight
   fprintf(_mtl_file, "Ka %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_AMBIENT].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_AMBIENT].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_AMBIENT].color.b);
   fprintf(_mtl_file, "Kd %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_DIFFUSE].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_DIFFUSE].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_DIFFUSE].color.b);
   fprintf(_mtl_file, "Ks %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_SPECULAR].color.r,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_SPECULAR].color.g,
           (float)mat->attribs[EVAS_CANVAS3D_MATERIAL_SPECULAR].color.b);
   fprintf(_mtl_file, "Ni 1.000000 \n");//optical density
   fprintf(_mtl_file, "d 1.000000 \n");//a factor d of 1.0 is fully opaque material
   fprintf(_mtl_file, "illum 2 \n");//illumination properties

   fclose(_mtl_file);
}

void
evas_model_save_file_obj(Evas_Canvas3D_Mesh *mesh, const char *_obj_file_name, Evas_Canvas3D_Mesh_Frame *f)
{
   int len;
   char *_mtl_file_name, *_without_extention;
   Evas_Canvas3D_Material_Data *mat;

   len = strlen(_obj_file_name);
   _without_extention = (char *)malloc(len - 3);
   _mtl_file_name = (char *)malloc(len + 1);

   Evas_Canvas3D_Mesh_Data *pd = eo_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);
   _save_mesh(pd, _obj_file_name, f);

   mat = eo_data_scope_get(f->material, EVAS_CANVAS3D_MATERIAL_CLASS);
   if (mat != NULL)
     {
        eina_strlcpy(_without_extention, _obj_file_name, len - 3);
        eina_str_join(_mtl_file_name, len + 1, '.', _without_extention, "mtl");
        _save_material(pd, _mtl_file_name, mat);
     }

   free(_without_extention);
   free(_mtl_file_name);
}
