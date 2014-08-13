#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include "stdio.h"
#include "evas_common_private.h"
#include "evas_private.h"

#define OPEN_FILE(extension)\
   int length=strlen(file);\
   char * extension = "."#extension;\
   char * _##extension##_file_name = (char *)malloc(length+4);\
   strcpy(_##extension##_file_name,file);\
   strcpy(_##extension##_file_name+length,extension);\
   FILE * _##extension##_file = fopen(_##extension##_file_name, "w+");\
   free(_##extension##_file_name);

#define SAVE_GEOMETRICS(a, format)\
   vb = &f->vertices[a];\
   fprintf(_obj_file, "o %s\n",file);\
   if (vb->data == NULL)\
     {\
        ERR("Reading of geometrics is failed.");\
        fclose(_obj_file);\
        return;\
     }\
   src = (float *)vb->data;\
   for (i = 0; i < pd->vertex_count; i++)\
     {\
        fprintf(_obj_file, format, src[0], src[1], src[2]);\
        src += f->vertices[a].element_count;\
     }

static void
_save_mesh(Evas_3D_Mesh_Data *pd, const char *file, Evas_3D_Mesh_Frame *f)
{
   Evas_3D_Vertex_Buffer *vb;
   time_t current_time;
   char* c_time_string;
   int    i;
   float *src;

   OPEN_FILE(obj)
   if (!_obj_file)
     {
        ERR("File open '%s' for save failed", file);
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
   fprintf(_obj_file,"mtllib %s.mtl \n\n", file);

   /* Adding geometrics to file. */
   if (f == NULL)
     {
        ERR("Not existing mesh frame.");
        fclose(_obj_file);
        return;
     }

   SAVE_GEOMETRICS(EVAS_3D_VERTEX_POSITION, "v %.4f %.4f %.4f \n")
   SAVE_GEOMETRICS(EVAS_3D_VERTEX_NORMAL, "vn %.4f %.4f %.4f \n")
   SAVE_GEOMETRICS(EVAS_3D_VERTEX_TEXCOORD, "vt %.4f %.4f %.4f \n")
   fprintf(_obj_file,"usemtl Material\n s off\n");
   for (i = 1; i <= pd->vertex_count; i++)//numeration of faces in .obj started from 1
     {
        fprintf(_obj_file,"f %i/%i/%i ", i, i, i);
        i++;
        fprintf(_obj_file,"%i/%i/%i ", i, i, i);
        i++;
        fprintf(_obj_file,"%i/%i/%i \n", i, i, i);
     }
   fclose(_obj_file);
}

static void
_save_material(Evas_3D_Mesh_Data *pd EINA_UNUSED, const char *file, Evas_3D_Mesh_Frame *f)
{
   Evas_3D_Material_Data *mat = eo_data_scope_get(f->material, EVAS_3D_MATERIAL_CLASS);

   OPEN_FILE(mtl)

   fprintf(_mtl_file, "# Evas_3D saver OBJ v0.03 \n");//_mtl_file created in macro
   fprintf(_mtl_file, "# Material Count: 1 \n\n");
   fprintf(_mtl_file, "newmtl Material \n");
   fprintf(_mtl_file, "Ns 1.000000 \n");//exp factor for specular highlight
   fprintf(_mtl_file, "Ka %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_3D_MATERIAL_AMBIENT].color.r,
           (float)mat->attribs[EVAS_3D_MATERIAL_AMBIENT].color.g,
           (float)mat->attribs[EVAS_3D_MATERIAL_AMBIENT].color.b);
   fprintf(_mtl_file, "Kd %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_3D_MATERIAL_DIFFUSE].color.r,
           (float)mat->attribs[EVAS_3D_MATERIAL_DIFFUSE].color.g,
           (float)mat->attribs[EVAS_3D_MATERIAL_DIFFUSE].color.b);
   fprintf(_mtl_file, "Ks %.6f %.6f %.6f \n",
           (float)mat->attribs[EVAS_3D_MATERIAL_SPECULAR].color.r,
           (float)mat->attribs[EVAS_3D_MATERIAL_SPECULAR].color.g,
           (float)mat->attribs[EVAS_3D_MATERIAL_SPECULAR].color.b);
   fprintf(_mtl_file, "Ni 1.000000 \n");//optical density
   fprintf(_mtl_file, "d 1.000000 \n");//a factor d of 1.0 is fully opaque material
   fprintf(_mtl_file, "illum 2 \n");//illumination properties

   fclose(_mtl_file);
}

void
evas_3d_mesh_save_obj(Evas_3D_Mesh *mesh, const char *file, Evas_3D_Mesh_Frame *f)
{
   Evas_3D_Mesh_Data *pd = eo_data_scope_get(mesh, EVAS_3D_MESH_CLASS);
   _save_mesh(pd, file, f);
   _save_material(pd, file, f);
}
