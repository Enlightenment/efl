#include "evas_common_private.h"
#include "evas_private.h"

#include <strings.h>

#include "evas_vg_private.h"

typedef struct _Efl_VG_Image_Data Efl_VG_Image_Data;
struct _Efl_VG_Image_Data
{
   // FIXME: only manipulate Eina_File internally.
   Eina_File *f;
   Eina_Stringshare *key;

   int x, y;
   unsigned int w, h;
};

static void
_efl_vg_image_position_set(Eo *obj, Efl_VG_Image_Data *pd, int x, int y)
{
   pd->x = x;
   pd->y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_image_position_get(Eo *obj, Efl_VG_Image_Data *pd, int *x, int *y)
{
   if (x) *x = pd->x;
   if (y) *y = pd->y;
}

static void
_efl_vg_image_size_set(Eo *obj, Efl_VG_Image_Data *pd,
                        unsigned int w, unsigned int h)
{
   pd->w = w;
   pd->h = h;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_image_size_get(Eo *obj, Efl_VG_Image_Data *pd,
                        unsigned int *w, unsigned int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

static Eina_Bool
_efl_vg_image_efl_file_mmap_set(Eo *obj EINA_UNUSED, Efl_VG_Image_Data *pd,
                                 const Eina_File *f, const char *key)
{
   Eina_File *tmp = pd->f;

   pd->f = eina_file_dup(f);
   eina_file_close(tmp);
   eina_stringshare_replace(&pd->key, key);

   _efl_vg_base_changed(obj);

   return EINA_TRUE;
}

static void
_efl_vg_image_efl_file_mmap_get(Eo *obj EINA_UNUSED, Efl_VG_Image_Data *pd,
                                 const Eina_File **f, const char **key)
{
   if (f) *f = pd->f;
   if (key) *key = pd->key;
}

static Eina_Bool
_efl_vg_image_efl_file_file_set(Eo *obj, Efl_VG_Image_Data *pd,
                                 const char *file, const char *key)
{
   Eina_File *tmp;
   Eina_Bool r;

   tmp = eina_file_open(file, EINA_FALSE);
   r = _efl_vg_image_efl_file_mmap_set(obj, pd, tmp, key);
   eina_file_close(tmp);

   return r;
}

static void
_efl_vg_image_efl_file_file_get(Eo *obj EINA_UNUSED, Efl_VG_Image_Data *pd,
                                 const char **file, const char **key)
{
   if (file) *file = eina_file_filename_get(pd->f);
   if (key) *key = pd->key;
}

static void
_efl_vg_image_eo_base_constructor(Eo *obj, Efl_VG_Image_Data *pd)
{
   eo_error_set(obj);
}

static void
_efl_vg_image_eo_base_destructor(Eo *obj, Efl_VG_Image_Data *pd)
{
   eo_error_set(obj);
}

#include "efl_vg_image.eo.c"
