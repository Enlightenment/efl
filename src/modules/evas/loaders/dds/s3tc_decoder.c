#include "s3tc.h"

// For INTERP_256 and INTERP_RGB_256
#include "evas_common_private.h"
#include "evas_blend_ops.h"

// From evas_convert_colorspace.c
#define CONVERT_RGB_565_TO_RGB_888(s) \
        (((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
         ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
         ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

static inline unsigned int
_rgb565_to_rgba8888(unsigned short s)
{
   return 0xFF000000 | CONVERT_RGB_565_TO_RGB_888(s);
}

static void
_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc,
                 Eina_Bool alpha)
{
   unsigned short color0, color1;
   unsigned int colors[4];
   unsigned int bits;

   color0 = s3tc[0] | (s3tc[1] << 8);
   color1 = s3tc[2] | (s3tc[3] << 8);

   colors[0] = _rgb565_to_rgba8888(color0);
   colors[1] = _rgb565_to_rgba8888(color1);
   if (color0 > color1)
     {
        // This is what's not supported by S2TC.
        colors[2] = 0xFF000000 | INTERP_RGB_256((2*256)/3, colors[0], colors[1]);
        colors[3] = 0xFF000000 | INTERP_RGB_256((1*256)/3, colors[0], colors[1]);
     }
   else
     {
        colors[2] = 0xFF000000 | INTERP_RGB_256(128, colors[0], colors[1]);
        colors[3] = (alpha ? 0x00000000 : 0xFF000000);
     }

   bits = s3tc[4] + ((s3tc[5] + ((s3tc[6] + (s3tc[7] << 8)) << 8)) << 8);
   for (int j = 0; j < 4; j++)
     for (int i = 0; i < 4; i++)
       {
          int idx = bits & 0x3;
          bits >>= 2;

          bgra[(j * 4) + i] = colors[idx];
       }
}

void s3tc_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc, EINA_FALSE);
}

void s3tc_decode_dxt1_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc, EINA_TRUE);
}
