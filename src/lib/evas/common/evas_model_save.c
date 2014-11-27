#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"

void
evas_common_save_model_to_file(Evas_3D_Mesh *model,
                               const char *file,
                               Evas_3D_Mesh_Frame *f)
{
   char *p;
   char *saver = NULL;

   p = strrchr(file, '.');
   if (p)
     {
        p++;
#define CHECK_EXTENTION_BY_FILE_NAME(extention)                \
        if (!strcasecmp(p, #extention))                        \
          {                                                    \
             evas_model_save_file_##extention(model, file, f); \
             saver = #extention;                               \
          }
        CHECK_EXTENTION_BY_FILE_NAME(eet)
        CHECK_EXTENTION_BY_FILE_NAME(obj)
        CHECK_EXTENTION_BY_FILE_NAME(ply)
#undef CHECK_EXTENTION_BY_FILE_NAME
     }
   if (!saver) ERR("Invalid mesh file type.");
}
