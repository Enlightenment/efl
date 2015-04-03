#include "evas_common_private.h"
#include "evas_private.h"

#include <strings.h>

typedef struct _Evas_VG_Image_Data Evas_VG_Image_Data;
struct _Evas_VG_Image_Data
{
   // FIXME: only manipulate Eina_File internally.
   Eina_Stringshare *file, *key;

   int x, y;
   unsigned int w, h;
};

void
_evas_vg_image_position_set(Eo *obj, Evas_VG_Image_Data *pd, int x, int y)
{
   pd->x = x;
   pd->y = y;
}

void
_evas_vg_image_position_get(Eo *obj, Evas_VG_Image_Data *pd, int *x, int *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

void
_evas_vg_image_size_set(Eo *obj, Evas_VG_Image_Data *pd,
                        unsigned int w, unsigned int h)
{
   pd->w = w;
   pd->h = h;
}

void
_evas_vg_image_size_get(Eo *obj, Evas_VG_Image_Data *pd,
                        unsigned int *w, unsigned int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

Eina_Bool
_evas_vg_image_efl_file_file_set(Eo *obj, Evas_VG_Image_Data *pd,
                                 const char *file, const char *key)
{
   eina_stringshare_replace(&pd->file, file);
   eina_stringshare_replace(&pd->key, key);
}

void
_evas_vg_image_efl_file_file_get(Eo *obj, Evas_VG_Image_Data *pd,
                                 const char **file, const char **key)
{
   if (file) *file = pd->file;
   if (key) *key = pd->key;
}


#include "evas_vg_image.eo.c"
