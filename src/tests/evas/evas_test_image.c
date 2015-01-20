#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define TESTS_IMG_DIR TESTS_SRC_DIR"/images"

static const char *exts[] = {
  "png"
#ifdef BUILD_LOADER_TGA
  ,"tga"
#endif
#ifdef BUILD_LOADER_WBMP
  ,"wbmp"
#endif
  // FIXME: Seems like XPM support is currently broken
#if 0
#ifdef BUILD_LOADER_XPM
  ,"xpm"
#endif
#endif
#ifdef BUILD_LOADER_BMP
  ,"bmp"
#endif
#ifdef BUILD_LOADER_GIF
  ,"gif"
#endif
#ifdef BUILD_LOADER_PSD
  ,"psd"
#endif
#ifdef BUILD_LOADER_WEBP
  ,"webp"
#endif
#ifdef BUILD_LOADER_JPEG
  ,"jpeg"
  ,"jpg"
#endif
#ifdef BUILD_LOADER_TGV
  ,"tgv"
#endif
};

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
        Eina_Bool found = EINA_FALSE;
        unsigned int i;
        int w, h;

        for (i = 0; i < (sizeof (exts) / sizeof (exts[0])); i++)
          if (!strcasecmp(file->path + file->path_length - strlen(exts[i]),
                          exts[i]))
            {
               found = EINA_TRUE;
               break;
            }

        if (!found) continue;

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

START_TEST(evas_object_image_tgv_loader_data)
{
   Evas *e = _setup_evas();
   Evas_Object *obj, *ref;
   Eina_Strbuf *str;

   const char *files[] = {
     "Light-50",
     "Pic1-10",
     "Pic1-100",
     "Pic1-50",
     "Pic4-10",
     "Pic4-100",
     "Pic4-50",
     "Train-10"
   };
   unsigned int i;

   obj = evas_object_image_add(e);
   ref = evas_object_image_add(e);
   str = eina_strbuf_new();

   for (i = 0; i < sizeof (files) / sizeof (files[0]); i++)
     {
        int w, h, r_w, r_h;
        const uint32_t *d, *r_d;

        eina_strbuf_append_printf(str, "%s/%s.tgv", TESTS_IMG_DIR, files[i]);
        evas_object_image_file_set(obj, eina_strbuf_string_get(str), NULL);
        fail_if(evas_object_image_load_error_get(obj) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(obj, &w, &h);
        d = evas_object_image_data_get(obj, EINA_FALSE);

        eina_strbuf_reset(str);

        eina_strbuf_append_printf(str, "%s/%s.png", TESTS_IMG_DIR, files[i]);
        evas_object_image_file_set(ref, eina_strbuf_string_get(str), NULL);
        fail_if(evas_object_image_load_error_get(ref) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(ref, &r_w, &r_h);
        r_d = evas_object_image_data_get(ref, EINA_FALSE);

        eina_strbuf_reset(str);

        fail_if(w != r_w || h != r_h);
        fail_if(memcmp(d, r_d, w * h * 4));
     }

   evas_object_del(obj);
   evas_object_del(ref);

   evas_free(e);
   evas_shutdown();
}
END_TEST

START_TEST(evas_object_image_all_loader_data)
{
   Evas *e = _setup_evas();
   Evas_Object *obj, *ref;
   Eina_Strbuf *str;
   unsigned int i;

   obj = evas_object_image_add(e);
   ref = evas_object_image_add(e);
   str = eina_strbuf_new();

   for (i = 0; i < sizeof (exts) / sizeof (exts[0]); i++)
     {
        struct stat st;
        int w, h, s, r_w, r_h, r_s;
        const uint32_t *d, *r_d;
        Evas_Colorspace c, r_c;

        eina_strbuf_reset(str);

        eina_strbuf_append_printf(str, "%s/Pic4-%s.png", TESTS_IMG_DIR, exts[i]);

        if (stat(eina_strbuf_string_get(str), &st) != 0) continue;

        evas_object_image_file_set(obj, eina_strbuf_string_get(str), NULL);
        fail_if(evas_object_image_load_error_get(obj) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(obj, &w, &h);
        s = evas_object_image_stride_get(obj);
        c = evas_object_image_colorspace_get(obj);
        d = evas_object_image_data_get(obj, EINA_FALSE);

        eina_strbuf_reset(str);

        eina_strbuf_append_printf(str, "%s/Pic4.%s", TESTS_IMG_DIR, exts[i]);
        evas_object_image_file_set(ref, eina_strbuf_string_get(str), NULL);
        fail_if(evas_object_image_load_error_get(ref) != EVAS_LOAD_ERROR_NONE);
        evas_object_image_size_get(ref, &r_w, &r_h);
        r_s = evas_object_image_stride_get(ref);
        r_c = evas_object_image_colorspace_get(ref);
        r_d = evas_object_image_data_get(ref, EINA_FALSE);

        fail_if(w != r_w || h != r_h);
        fail_if(s != r_s);
        fail_if(c != r_c);
        fail_if(w*4 != s);
        if (strcmp(exts[i], "jpeg") == 0 || strcmp(exts[i], "jpg") == 0)
          {
             //jpeg norm allows a variation of 1 bit per component
             for (int j = 0; j < s * h; j++)
               {
                  fail_if(abs(((char*)d)[j] - ((char*)r_d)[j]) > 1);
               }
          }
        else
          {
             fail_if(memcmp(d, r_d, w * h * 4));
          }
     }

   evas_object_del(obj);
   evas_object_del(ref);

   eina_strbuf_free(str);

   evas_free(e);
   evas_shutdown();
}
END_TEST

const char *buggy[] = {
  "BMP301K"
};

START_TEST(evas_object_image_buggy)
{
   Evas *e = _setup_evas();
   Evas_Object *obj, *ref;
   Eina_Strbuf *str;
   unsigned int i, j;

   obj = evas_object_image_add(e);
   ref = evas_object_image_add(e);
   str = eina_strbuf_new();

   for (i = 0; i < sizeof (buggy) / sizeof (buggy[0]); i++)
     {
        for (j = 0; j < sizeof (exts) / sizeof (exts[0]); j++)
          {
             struct stat st;
             int w, h, r_w, r_h;
             const uint32_t *d, *r_d;

             eina_strbuf_reset(str);

             if (!strcmp(exts[j], "png")) continue ;

             eina_strbuf_append_printf(str, "%s/%s.%s", TESTS_IMG_DIR,
                                       buggy[i], exts[j]);

             if (stat(eina_strbuf_string_get(str), &st) != 0) continue;

             evas_object_image_file_set(obj, eina_strbuf_string_get(str), NULL);
             fail_if(evas_object_image_load_error_get(obj) != EVAS_LOAD_ERROR_NONE);
             evas_object_image_size_get(obj, &w, &h);
             d = evas_object_image_data_get(obj, EINA_FALSE);

             eina_strbuf_reset(str);

             eina_strbuf_append_printf(str, "%s/%s.png", TESTS_IMG_DIR,
                                       buggy[i]);
             evas_object_image_file_set(ref, eina_strbuf_string_get(str), NULL);
             fail_if(evas_object_image_load_error_get(ref) != EVAS_LOAD_ERROR_NONE);
             evas_object_image_size_get(ref, &r_w, &r_h);
             r_d = evas_object_image_data_get(ref, EINA_FALSE);

             fail_if(w != r_w || h != r_h);
             fail_if(memcmp(d, r_d, w * h * 4));
          }
     }

   evas_object_del(obj);
   evas_object_del(ref);

   eina_strbuf_free(str);

   evas_free(e);
   evas_shutdown();
}
END_TEST

void evas_test_image_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_image_loader);
   tcase_add_test(tc, evas_object_image_loader_orientation);
#if BUILD_LOADER_TGV && BUILD_LOADER_PNG
   tcase_add_test(tc, evas_object_image_tgv_loader_data);
#endif
#if BUILD_LOADER_PNG
   tcase_add_test(tc, evas_object_image_all_loader_data);
   tcase_add_test(tc, evas_object_image_buggy);
#endif
}
