#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define TESTS_IMG_DIR TESTS_SRC_DIR"/images"

#if 0
static const char *
_test_image_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_IMG_DIR"/%s", name);

   return filename;
}
#endif

START_TEST(evas_object_image_loader)
{
   Evas *e = _setup_evas();
   Evas_Object *o;
   Eina_Iterator *it;
   const Eina_File_Direct_Info *file;

   o = evas_object_image_add(e);

   it = eina_file_direct_ls(TESTS_IMG_DIR);
   EINA_ITERATOR_FOREACH(it, file)
     {
        int w, h;

        evas_object_image_file_set(o, file->path, NULL);
        fail_if(evas_object_image_load_error_get(o) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(o, &w, &h);
        fail_if(w == 0 || h == 0);
     }
   eina_iterator_free(it);

   evas_object_del(o);

   evas_free(e);
   evas_shutdown();
}
END_TEST

typedef struct _orientation_Test_Res {
   const char *img;
   const char *desc;
   int (*compare_func)(const uint32_t *d1, const uint32_t *d2, int w2, int h2);
} Orientation_Test_Res;

static int _compare_img(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   return memcmp(d1, d2, w2 * h2 * 4);
}

static int _compare_img_90(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (x = w2 - 1; x >= 0; x--)
     {
        for (y = 0; y < h2; y++)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_180(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (y = h2 - 1; y >= 0; y--)
     {
        for (x = w2 - 1; x >= 0; x--)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_270(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (x = 0; x < w2; x++)
     {
        for (y = h2 - 1; y >= 0; y--)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_flip_h(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (y = 0; y < h2; y++)
     {
        for (x = w2 - 1; x >= 0; x--)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_flip_v(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (y = h2 - 1; y >= 0; y--)
     {
        for (x = 0; x < w2; x++)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_transpose(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (x = 0; x < w2; x++)
     {
        for (y = 0; y < h2; y++)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

static int _compare_img_transverse(const uint32_t *d1, const uint32_t *d2, int w2, int h2)
{
   int x, y;
   int r;

   for (x = w2 - 1; x >= 0; x--)
     {
        for (y = h2 - 1; y >= 0; y--)
          {
             r = *d1 - *(d2 + x + y * w2);
             if (r != 0) return r;
             d1++;
          }
     }

   return 0;
}

START_TEST(evas_object_image_loader_orientation)
{
   Evas *e = _setup_evas();
   Evas_Object *orig, *rot;
   Orientation_Test_Res res[] = {
       {TESTS_IMG_DIR"/Light_exif.jpg", "Original", _compare_img},
       {TESTS_IMG_DIR"/Light_exif_flip_h.jpg", "Flip horizontally", _compare_img_flip_h},
       {TESTS_IMG_DIR"/Light_exif_180.jpg", "Rotate 180° CW", _compare_img_180},
       {TESTS_IMG_DIR"/Light_exif_flip_v.jpg", "Flip vertically", _compare_img_flip_v},
       {TESTS_IMG_DIR"/Light_exif_transpose.jpg", "Transpose", _compare_img_transpose},
       {TESTS_IMG_DIR"/Light_exif_90.jpg", "Rotate 90° CW", _compare_img_90},
       {TESTS_IMG_DIR"/Light_exif_transverse.jpg", "Transverse", _compare_img_transverse},
       {TESTS_IMG_DIR"/Light_exif_270.jpg", "Rotate 90° CCW", _compare_img_270},
       {NULL, NULL, NULL}
   };
   int w, h, r_w, r_h;
   const uint32_t *d, *r_d;
   int i;

   orig = evas_object_image_add(e);
   evas_object_image_file_set(orig, TESTS_IMG_DIR"/Light.jpg", NULL);
   fail_if(evas_object_image_load_error_get(orig) != EVAS_LOAD_ERROR_NONE);
   evas_object_image_size_get(orig, &w, &h);
   fail_if(w == 0 || h == 0);
   d = evas_object_image_data_get(orig, EINA_FALSE);

   rot = evas_object_image_add(e);
   evas_object_image_load_orientation_set(rot, EINA_TRUE);

   for (i = 0; res[i].img; i++)
     {
        evas_object_image_file_set(rot, res[i].img, NULL);
        fail_if(evas_object_image_load_error_get(rot) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(rot, &r_w, &r_h);
        fail_if(w * h != r_w * r_h);

        r_d = evas_object_image_data_get(rot, EINA_FALSE);

        fail_if(res[i].compare_func(d, r_d, r_w, r_h),
                "Image orientation test failed: exif orientation flag: %s\n", res[i].desc);
     }

   evas_object_del(orig);
   evas_object_del(rot);

   evas_free(e);
   evas_shutdown();
}
END_TEST

void evas_test_image_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_image_loader);
   tcase_add_test(tc, evas_object_image_loader_orientation);
}
