#include "s3tc.h"

// For INTERP_256 and INTERP_RGB_256
#include "evas_common_private.h"
#include "evas_blend_ops.h"

// From evas_convert_colorspace.c
#define CONVERT_RGB_565_TO_RGB_888(s) \
        (((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
         ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
         ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#define ALPHA4(a) (((a) << 4) | (a))

static void
_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc,
                 unsigned int amask, Eina_Bool dxt1, Eina_Bool alpha)
{
   unsigned short color0, color1;
   unsigned int colors[4];
   unsigned int bits;

   color0 = s3tc[0] | (s3tc[1] << 8);
   color1 = s3tc[2] | (s3tc[3] << 8);

   colors[0] = amask | CONVERT_RGB_565_TO_RGB_888(color0);
   colors[1] = amask | CONVERT_RGB_565_TO_RGB_888(color1);
   if (!dxt1 || (color0 > color1))
     {
        // This is what's not supported by S2TC.
        colors[2] = amask | INTERP_RGB_256((2*256)/3, colors[0], colors[1]);
        colors[3] = amask | INTERP_RGB_256((1*256)/3, colors[0], colors[1]);
     }
   else
     {
        colors[2] = amask | INTERP_RGB_256(128, colors[0], colors[1]);
        colors[3] = (alpha ? 0x00000000 : amask);
     }

   bits = s3tc[4] + ((s3tc[5] + ((s3tc[6] + (s3tc[7] << 8)) << 8)) << 8);
   for (int j = 0; j < 4; j++)
     for (int i = 0; i < 4; i++)
       {
          int idx = bits & 0x3;
          bgra[(j * 4) + i] = colors[idx];
          bits >>= 2;
       }
}

static void
_decode_alpha4(unsigned int *bgra, const unsigned char *s3tc)
{
   for (int k = 0; k < 16; k += 2)
     {
        unsigned int a0 = ALPHA4((*s3tc) & 0x0F);
        unsigned int a1 = ALPHA4(((*s3tc) & 0xF0) >> 4);
        *bgra++ |= (a0 << 24);
        *bgra++ |= (a1 << 24);
        s3tc++;
     }
}

static void
_decode_dxt_alpha(unsigned int *bgra, const unsigned char *s3tc)
{
   unsigned char a0 = s3tc[0];
   unsigned char a1 = s3tc[1];
   unsigned long long bits = 0ull;
   unsigned char alpha[8];

   for (int k = 5; k >= 0; k--)
     bits = (bits << 8) | s3tc[k + 2];

   alpha[0] = a0;
   alpha[1] = a1;

   if (a0 > a1)
     {
        for (int k = 0; k < 6; k++)
          alpha[2 + k] = ((6 - k) * a0 + (k + 1) * a1) / 7;
     }
   else
     {
        for (int k = 0; k < 4; k++)
          alpha[2 + k] = ((4 - k) * a0 + (k + 1) * a1) / 5;
        alpha[6] = 0;
        alpha[7] = 255;
     }

   for (int k = 0; k < 16; k++)
     {
        int index = (int) (bits & 0x7ull);
        *bgra++ |= (alpha[index] << 24);
        bits >>= 3;
     }
}

void s3tc_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc, 0xFF000000, EINA_TRUE, EINA_FALSE);
}

void s3tc_decode_dxt1_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc, 0xFF000000, EINA_TRUE, EINA_TRUE);
}

void s3tc_decode_dxt2_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc + 8, 0x0, EINA_FALSE, EINA_FALSE);
   _decode_alpha4(bgra, s3tc);
}

void s3tc_decode_dxt3_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc + 8, 0x0, EINA_FALSE, EINA_FALSE);
   _decode_alpha4(bgra, s3tc);
}

void s3tc_decode_dxt4_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc + 8, 0x0, EINA_FALSE, EINA_FALSE);
   _decode_dxt_alpha(bgra, s3tc);
}

void s3tc_decode_dxt5_rgba(unsigned int *bgra, const unsigned char *s3tc)
{
   _decode_dxt1_rgb(bgra, s3tc + 8, 0x0, EINA_FALSE, EINA_FALSE);
   _decode_dxt_alpha(bgra, s3tc);
}
