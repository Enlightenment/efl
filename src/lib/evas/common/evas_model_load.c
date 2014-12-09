#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"

static inline void
_model_common_loader_fini(Model_Common_Loader *loader)
{
   if (loader->map)
     {
        eina_file_map_free(loader->file, loader->map);
        loader->map = NULL;
     }

   if (loader->file)
     {
        eina_file_close(loader->file);
        loader->file = NULL;
     }
}

static inline Eina_Bool
_model_common_loader_init(Model_Common_Loader *loader, const char *file)
{
   Eina_File *tmp_file = eina_file_open(file, 0);
   memset(loader, 0x00, sizeof(Model_Common_Loader));

   /* Open given file. */
   loader->file = eina_file_dup(tmp_file);

   if (loader->file == NULL)
     {
        ERR("Failed to open file %s\n", file);
        goto error;
     }

   /* Map the file. */
   loader->map = eina_file_map_all(loader->file, EINA_FILE_SEQUENTIAL);

   if (loader->map == NULL)
     {
        ERR("Failed to create map from file %s\n", file);
        goto error;
     }

   eina_file_close(tmp_file);
   return EINA_TRUE;

error:
   eina_file_close(tmp_file);
   _model_common_loader_fini(loader);
   return EINA_FALSE;
}

void
evas_common_load_model_to_file(Evas_3D_Mesh *model, const char *file)
{
   char *p;
   char *loader_type = NULL;
   Model_Common_Loader *loader = malloc(sizeof(Model_Common_Loader));

   if (!_model_common_loader_init(loader, file))
     {
        ERR("Failed to initialize loader.");
        return;
     }

   p = strrchr(file, '.');
   if (p)
     {
        p++;
#define CHECK_EXTENTION_BY_FILE_NAME(extention)                \
        if (!strcasecmp(p, #extention))                        \
          {                                                    \
             evas_model_load_file_##extention(model, loader);  \
             loader_type = #extention;                         \
          }
        CHECK_EXTENTION_BY_FILE_NAME(eet)
        CHECK_EXTENTION_BY_FILE_NAME(md2)
        CHECK_EXTENTION_BY_FILE_NAME(obj)
        CHECK_EXTENTION_BY_FILE_NAME(ply)
#undef CHECK_EXTENTION_BY_FILE_NAME
     }
   if (!loader_type) ERR("Invalid mesh file type.");

   _model_common_loader_fini(loader);
   free(loader);
}

