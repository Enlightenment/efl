#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

#define IM0 0x00112233
#define IM1 0x44556677
#define IM2 0x8899aabb
#define IM3 0xccddeeff

typedef struct _Eet_Test_Image Eet_Test_Image;
struct _Eet_Test_Image
{
   unsigned int w;
   unsigned int h;
   int          alpha;
   unsigned int color[64];
};

static const Eet_Test_Image test_noalpha = {
   8, 8, 0,
   {
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x000000AA,
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x000000AA
   }
};

static const Eet_Test_Image test_alpha = {
   8, 8, 1,
   {
      0x0FAA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x0F110000,
      0x0000AA00, 0x0F0000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x0F110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x0FAA0000, 0x0000AA00, 0x000000AA, 0x0F110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x0F0000AA, 0x0F110000, 0x00AA0000,
      0x0000AA00, 0x000000AA,
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x0F110000, 0x0FAA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x0F110000, 0x00AA0000, 0x0000AA00, 0x0F0000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x0F110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x0FAA0000, 0x0000AA00,
      0x0F110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x0F0000AA
   }
};

EFL_START_TEST(eet_test_image_normal)
{
   Eet_File *ef;
   char *file;
   unsigned int *data;
   int compress;
   int quality;
   int result;
   Eet_Image_Encoding lossy;
   int alpha;
   unsigned int w;
   unsigned int h;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "0",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "1",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 5,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "2",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 9,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "3",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 100,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "4",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 60,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "5",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 10,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "6",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 0,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "7", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 9, 100, 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "8", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 0, 80, 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "9", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 0, 100, 1);
   fail_if(result == 0);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "2",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "2",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);

   eet_close(ef);

   /* Test read of image */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "0",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(lossy != 0);

   data = malloc(w * h * 4);
   fail_if(data == NULL);
   result = eet_data_image_read_to_surface(ef,
                                           EET_TEST_FILE_IMAGE "0",
                                           4,
                                           4,
                                           data,
                                           2,
                                           2,
                                           w * 4,
                                           &alpha,
                                           &compress,
                                           &quality,
                                           &lossy);
   fail_if(result != 1);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[4 + 4 * w]);
   free(data);

   data = malloc(w * h * 4);
   fail_if(data == NULL);
   result = eet_data_image_read_to_surface(ef,
                                           EET_TEST_FILE_IMAGE "0",
                                           0,
                                           0,
                                           data,
                                           w,
                                           h,
                                           w * 4,
                                           &alpha,
                                           &compress,
                                           &quality,
                                           &lossy);
   fail_if(result != 1);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "1",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 5);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "2",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "3",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "5",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "6",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "7",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "7",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_alpha.color[0]);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "9",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(lossy != 1);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "9",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(lossy != 1);
   free(data);

   eet_close(ef);

   fail_if(unlink(file) != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_image_small)
{
   char *file;
   unsigned int image[4];
   unsigned int *data;
   Eet_File *ef;
   unsigned int w;
   unsigned int h;
   int alpha;
   int compression;
   int quality;
   Eet_Image_Encoding lossy;
   int result;
   int tmpfd;

   file = strdup("/tmp/eet_suite_testXXXXXX");

   image[0] = IM0;
   image[1] = IM1;
   image[2] = IM2;
   image[3] = IM3;

   fail_if(-1 == (tmpfd = mkstemp(file)));
   fail_if(!!close(tmpfd));

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   result = eet_data_image_write(ef, "/images/test", image, 2, 2, 1, 9, 100, 0);
   fail_if(result == 0);

   eet_close(ef);

   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   data = (unsigned int *)eet_data_image_read(ef,
                                              "/images/test",
                                              &w,
                                              &h,
                                              &alpha,
                                              &compression,
                                              &quality,
                                              &lossy);
   fail_if(data == NULL);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   fail_if(data[0] != IM0);
   fail_if(data[1] != IM1);
   fail_if(data[2] != IM2);
   fail_if(data[3] != IM3);

   free(data);

}
EFL_END_TEST

void eet_test_image(TCase *tc)
{
   tcase_add_test(tc, eet_test_image_normal);
   tcase_add_test(tc, eet_test_image_small);
}
