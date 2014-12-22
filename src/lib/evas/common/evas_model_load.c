#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"

void
_evas_common_load_model(Evas_3D_Mesh *model,
                        Eina_File *file,
                        const char *file_path)
{
   char *p;
   char *loader_type = NULL;

   p = strrchr(file_path, '.');
   if (p)
     {
        p++;
#define CHECK_EXTENTION_BY_FILE_NAME(extention)                \
        if (!strcasecmp(p, #extention))                        \
          {                                                    \
             evas_model_load_file_##extention(model, file);    \
             loader_type = #extention;                         \
          }
        CHECK_EXTENTION_BY_FILE_NAME(eet)
        CHECK_EXTENTION_BY_FILE_NAME(md2)
        CHECK_EXTENTION_BY_FILE_NAME(obj)
        CHECK_EXTENTION_BY_FILE_NAME(ply)
#undef CHECK_EXTENTION_BY_FILE_NAME
     }
   if (!loader_type) ERR("Invalid mesh file type.");
}

void
evas_common_load_model_from_file(Evas_3D_Mesh *model, const char *file)
{
   Eina_File *tmp_file = eina_file_open(file, 0);

   if (tmp_file == NULL)
     {
        ERR("Failed to open file %s\n", file);
        ERR("Failed to initialize loader.");
        return;
     }

   Eina_File *e_file = eina_file_dup(tmp_file);

   if (e_file == NULL)
     {
        eina_file_close(tmp_file);
        file = NULL;
        ERR("Failed to initialize loader.");
        return;
     }

   eina_file_close(tmp_file);

   _evas_common_load_model(model, e_file, file);

   eina_file_close(e_file);
   e_file = NULL;
}

void
evas_common_load_model_from_eina_file(Evas_3D_Mesh *model, Eina_File *file)
{
   Eina_File *e_file = eina_file_dup(file);

   if (e_file == NULL)
     {
        ERR("Failed to initialize loader.");
        return;
     }

   _evas_common_load_model(model, e_file, eina_file_filename_get(e_file));

   eina_file_close(e_file);
   e_file = NULL;
}
