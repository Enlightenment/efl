/* @file evas_image_load_dds.c
 * @author Jean-Philippe ANDRE <jpeg@videolan.org>
 *
 * Load Microsoft DirectDraw Surface files.
 * Decode S3TC image format.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Evas_Loader.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;
};

static void *
evas_image_load_file_open_dds(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   // TODO: Check file size
   if (eina_file_size_get(f) <= 16)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }


   loader->f = eina_file_dup(f);
   if (!loader->f)
     {
        free(loader);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   return loader;
}

static void
evas_image_load_file_close_dds(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eina_file_close(loader->f);
   free(loader);
}

static Eina_Bool
evas_image_load_file_head_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   const char *m;

   m = eina_file_map_all(loader->f, EINA_FILE_SEQUENTIAL);
   if (!m)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   if (strncmp(m, "DDS ", 4) != 0)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   // TODO

   return EINA_TRUE;
}

Eina_Bool
evas_image_load_file_data_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   const char *m;

   Eina_Bool r = EINA_FALSE;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   m = eina_file_map_all(loader->f, EINA_FILE_WILLNEED);
   if (!m) return EINA_FALSE;

   // TODO

   r = EINA_TRUE;

on_error:
   eina_file_map_free(loader->f, m);
   return r;
}

Evas_Image_Load_Func evas_image_load_dds_func =
{
  evas_image_load_file_open_dds,
  evas_image_load_file_close_dds,
  evas_image_load_file_head_dds,
  evas_image_load_file_data_dds,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_dds_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "dds",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, dds);

#ifndef EVAS_STATIC_BUILD_DDS
EVAS_EINA_MODULE_DEFINE(image_loader, dds);
#endif
