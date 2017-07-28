#include "draw.h"
#include "draw_private.h"
#include "../rg_etc/rg_etc1.h"

#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

#if DIV_USING_BITSHIFT
# define DEFINE_DIVIDER(div) const int pow2 = _pow2_geq((div) << 10); const int numerator = (1 << pow2) / (div);
# define DIVIDE(val) (((val) * numerator) >> pow2)
#else
# define DEFINE_DIVIDER(div) const int divider = (div);
# define DIVIDE(val) ((val) / divider)
#endif

#define CONVERT_RGB_565_TO_RGB_888(s) \
        (((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
         ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
         ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#define CONVERT_A5P_TO_A8(s) \
        ((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7))

#define CONVERT_ARGB_8888_TO_A_8(s)	((s) >> 24)

// finds smallest power of 2 above val
static int
_pow2_geq(int val)
{
   for (int n = 0; n < 32; n++)
     if (val <= (1 << n))
       return n;

   return 32; // impossible
}

static Eina_Bool
_convert_gry8_to_argb8888(void *dst, const void *src, int w, int h,
                          int src_stride, int dst_stride, Eina_Bool has_alpha,
                          Efl_Gfx_Colorspace srccs EINA_UNUSED,
                          Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint8_t *in = src;
   uint32_t *out = dst;
   int in_step, out_step, x, y;

   if (!src_stride) src_stride = w;
   if (!dst_stride) dst_stride = w * 4;
   in_step = src_stride;
   out_step = dst_stride / 4;

   if (has_alpha)
     {
        // transparent white
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = in[x];
                  out[x] = DRAW_ARGB_JOIN(c, c, c, c);
               }
             in += in_step;
             out += out_step;
          }
     }
   else
     {
        // opaque grayscale
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = in[x];
                  out[x] = DRAW_ARGB_JOIN(0xFF, c, c, c);
               }
             in += in_step;
             out += out_step;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_agry88_to_argb8888(void *dst, const void *src, int w, int h,
                            int src_stride, int dst_stride, Eina_Bool has_alpha,
                            Efl_Gfx_Colorspace srccs EINA_UNUSED,
                            Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint16_t *in = src;
   uint32_t *out = dst;
   int in_step, out_step, x, y;

   if (!src_stride) src_stride = w * 2;
   if (!dst_stride) dst_stride = w * 4;
   in_step = src_stride / 2;
   out_step = dst_stride / 4;

   if (has_alpha)
     {
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = in[x] & 0xFF;
                  int a = in[x] >> 8;
                  out[x] = DRAW_ARGB_JOIN(a, c, c, c);
               }
             in += in_step;
             out += out_step;
          }
     }
   else
     {
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = in[x] & 0xFF;
                  out[x] = DRAW_ARGB_JOIN(0xFF, c, c, c);
               }
             in += in_step;
             out += out_step;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_argb8888_to_gry8(void *dst, const void *src, int w, int h,
                          int src_stride, int dst_stride, Eina_Bool has_alpha,
                          Efl_Gfx_Colorspace srccs EINA_UNUSED,
                          Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint32_t *in = src;
   uint8_t *out = dst;
   int in_step, out_step, x, y;
#if RGBA2LUM_WEIGHTED
   const int WR = 299;
   const int WG = 587;
   const int WB = 114;
#else
   const int WR = 1;
   const int WG = 1;
   const int WB = 1;
#endif
   DEFINE_DIVIDER(WR + WG + WB);

   if (!src_stride) src_stride = w * 4;
   if (!dst_stride) dst_stride = w;
   in_step = src_stride / 4;
   out_step = dst_stride;

   if (has_alpha)
     {
        // copy only alpha
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               out[x] = A_VAL(in + x);
             in += in_step;
             out += out_step;
          }
     }
   else
     {
        // copy only color to grayscale
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               out[x] = DIVIDE((R_VAL(in + x) * WR) + (G_VAL(in + x) * WG) + (B_VAL(in + x) * WB));
             in += in_step;
             out += out_step;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_argb8888_to_agry88(void *dst, const void *src, int w, int h,
                            int src_stride, int dst_stride, Eina_Bool has_alpha,
                            Efl_Gfx_Colorspace srccs EINA_UNUSED,
                            Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint32_t *in = src;
   uint16_t *out = dst;
   int in_step, out_step, x, y;
#if RGBA2LUM_WEIGHTED
   const int WR = 299;
   const int WG = 587;
   const int WB = 114;
#else
   const int WR = 1;
   const int WG = 1;
   const int WB = 1;
#endif
   DEFINE_DIVIDER(WR + WG + WB);

   if (!src_stride) src_stride = w * 4;
   if (!dst_stride) dst_stride = w * 2;
   in_step = src_stride / 4;
   out_step = dst_stride / 2;

   if (has_alpha)
     {
        // copy only alpha
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int a = A_VAL(in + x);
                  int c = DIVIDE((R_VAL(in + x) * WR) + (G_VAL(in + x) * WG) + (B_VAL(in + x) * WB));
                  out[x] = (a << 8) | c;
               }
             in += in_step;
             out += out_step;
          }
     }
   else
     {
        // copy only color to grayscale
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = DIVIDE((R_VAL(in + x) * WR) + (G_VAL(in + x) * WG) + (B_VAL(in + x) * WB));
                  out[x] = 0xFF00 | c;
               }
             in += in_step;
             out += out_step;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_rgb565_a5p_to_argb8888(void *dst, const void *src, int w, int h,
                                int src_stride, int dst_stride, Eina_Bool has_alpha,
                                Efl_Gfx_Colorspace srccs EINA_UNUSED,
                                Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint16_t *in = src;
   const uint8_t *in_alpha;
   uint32_t *out = dst;
   int in_step, out_step, a_step, x, y;

   if (!src_stride) src_stride = w * 2;
   if (!dst_stride) dst_stride = w * 4;
   in_step = src_stride / 2;
   a_step = src_stride;
   out_step = dst_stride / 4;

   // no region support (2 planes): basic safety check (can't verify h)
   EINA_SAFETY_ON_FALSE_RETURN_VAL((src_stride == (w * 2)) && (dst_stride == (w * 4)), EINA_FALSE);

   if (has_alpha)
     {
        in_alpha = ((uint8_t *) in) + (src_stride * h);
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int a = CONVERT_A5P_TO_A8(in_alpha[x]);
                  int c = CONVERT_RGB_565_TO_RGB_888(in[x]);
                  out[x] = (a << 24) | c;
               }
             in_alpha += a_step;
             in += in_step;
             out += out_step;
          }
     }
   else
     {
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  int c = CONVERT_RGB_565_TO_RGB_888(in[x]);
                  out[x] = 0xFF000000 | c;
               }
             in += in_step;
             out += out_step;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_etc2_rgb8_to_argb8888(void *dst, const void *src, int w, int h,
                               int src_stride, int dst_stride, Eina_Bool has_alpha,
                               Efl_Gfx_Colorspace srccs EINA_UNUSED,
                               Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint8_t *in = src;
   uint32_t *out = dst;
   int out_step, x, y, k;
   unsigned int bgra[16];

   EINA_SAFETY_ON_FALSE_RETURN_VAL(!(w & 3) && !(h & 3), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!has_alpha, EINA_FALSE);

   // jumps lines 4 by 4
   if (!src_stride) src_stride = w * 8 / 4;
   if (!dst_stride) dst_stride = w * 4;
   out_step = dst_stride / 4;

   for (y = 0; y < h; y += 4)
     {
        for (x = 0; x < w; x += 4, in += 8)
          {
             rg_etc2_rgb8_decode_block(in, bgra);
             for (k = 0; k < 4; k++)
               memcpy(out + x + k * out_step, bgra + (k * 4), 16);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_etc2_rgba8_to_argb8888(void *dst, const void *src, int w, int h,
                                int src_stride, int dst_stride, Eina_Bool has_alpha,
                                Efl_Gfx_Colorspace srccs EINA_UNUSED,
                                Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint8_t *in = src;
   uint32_t *out = dst;
   int out_step, x, y, k;
   unsigned int bgra[16];

   EINA_SAFETY_ON_FALSE_RETURN_VAL(!(w & 3) && !(h & 3), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!has_alpha, EINA_FALSE);

   // jumps lines 4 by 4
   if (!src_stride) src_stride = w * 16 / 4;
   if (!dst_stride) dst_stride = w * 4;
   out_step = dst_stride / 4;

   for (y = 0; y < h; y += 4)
     {
        for (x = 0; x < w; x += 4, in += 16)
          {
             rg_etc2_rgba8_decode_block(in, bgra);
             for (k = 0; k < 4; k++)
               memcpy(out + x + k * out_step, bgra + (k * 4), 16);
          }
        out += out_step * 4;
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_etc1_alpha_to_argb8888(void *dst, const void *src, int w, int h,
                                int src_stride, int dst_stride, Eina_Bool has_alpha,
                                Efl_Gfx_Colorspace srccs EINA_UNUSED,
                                Efl_Gfx_Colorspace dstcs EINA_UNUSED)
{
   const uint8_t *in = src, *in_alpha;
   uint32_t *out = dst;
   int out_step, x, y, j, k;
   unsigned int bgra[16], alpha[16];

   EINA_SAFETY_ON_FALSE_RETURN_VAL(!(w & 3) && !(h & 3), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!has_alpha, EINA_FALSE);

   // jumps lines 4 by 4
   if (!src_stride) src_stride = w * 8 / 4;
   if (!dst_stride) dst_stride = w * 4;
   out_step = dst_stride / 4;

   in_alpha = in + src_stride * h;

   for (y = 0; y < h; y += 4)
     {
        for (x = 0; x < w; x += 4, in += 8, in_alpha += 8)
          {
             rg_etc2_rgba8_decode_block(in, bgra);
             rg_etc2_rgba8_decode_block(in_alpha, alpha);
             for (k = 0; k < 4; k++)
               for (j = 0; j < 4; j++)
                 {
                    int a = (alpha[(k * 4) + j] & 0x00FF00) >> 8;
                    int c = (bgra[(k * 4) + j] & 0x00FFFFFF);
                    out[(k * out_step) + j] = (a << 24) | c;
                 }
          }
        out += out_step;
     }

   return EINA_TRUE;
}

static Eina_Bool
_convert_generic_two_pass(void *dst, const void *src, int w, int h,
                          int src_stride, int dst_stride, Eina_Bool has_alpha,
                          Efl_Gfx_Colorspace srccs, Efl_Gfx_Colorspace dstcs)
{
   Cspace_Convert_Func to_argb = efl_draw_convert_func_get(srccs, EFL_GFX_COLORSPACE_ARGB8888, NULL);
   Cspace_Convert_Func from_argb = efl_draw_convert_func_get(EFL_GFX_COLORSPACE_ARGB8888, dstcs, NULL);
   uint32_t *argb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(to_argb, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(from_argb, EINA_FALSE);

   argb = malloc(w * h * sizeof(uint32_t));
   if (!argb) return EINA_FALSE;

   if (!to_argb(argb, src, w, h, src_stride, 0, has_alpha, srccs, EFL_GFX_COLORSPACE_ARGB8888))
     goto fail;

   if (!from_argb(dst, argb, w, h, 0, dst_stride, has_alpha, EFL_GFX_COLORSPACE_ARGB8888, dstcs))
     goto fail;

   free(argb);
   return EINA_TRUE;

fail:
   free(argb);
   return EINA_FALSE;
}

Cspace_Convert_Func
efl_draw_convert_func_get(Efl_Gfx_Colorspace srccs, Efl_Gfx_Colorspace dstcs,
                          Eina_Bool *region_can)
{
   Eina_Bool reg1 = EINA_FALSE, reg2 = EINA_FALSE;
   Cspace_Convert_Func to_argb = NULL;
   Cspace_Convert_Func from_argb = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(srccs != dstcs, NULL);

   if ((dstcs != EFL_GFX_COLORSPACE_ARGB8888) && (srccs != EFL_GFX_COLORSPACE_ARGB8888))
     {
        to_argb = efl_draw_convert_func_get(srccs, EFL_GFX_COLORSPACE_ARGB8888, &reg1);
        from_argb = efl_draw_convert_func_get(EFL_GFX_COLORSPACE_ARGB8888, dstcs, &reg2);
     }

   if (region_can) *region_can = EINA_TRUE;

   switch (srccs)
     {
      case EFL_GFX_COLORSPACE_ARGB8888:
        if (dstcs == EFL_GFX_COLORSPACE_GRY8)
          return _convert_argb8888_to_gry8;
        if (dstcs == EFL_GFX_COLORSPACE_AGRY88)
          return _convert_argb8888_to_agry88;
        break;

      case EFL_GFX_COLORSPACE_GRY8:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          return _convert_gry8_to_argb8888;
        break;

      case EFL_GFX_COLORSPACE_AGRY88:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          return _convert_agry88_to_argb8888;
        break;

      case EFL_GFX_COLORSPACE_RGB565_A5P:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          {
             // we could do regions, but we would need a pointer to the alpha plane.
             if (region_can) *region_can = EINA_FALSE;
             return _convert_rgb565_a5p_to_argb8888;
          }
        break;

      case EFL_GFX_COLORSPACE_YCBCR422P601_PL:
      case EFL_GFX_COLORSPACE_YCBCR422601_PL:
      case EFL_GFX_COLORSPACE_YCBCR420NV12601_PL:
      case EFL_GFX_COLORSPACE_YCBCR420TM12601_PL:
      case EFL_GFX_COLORSPACE_YCBCR422P709_PL:
         /* FIXME */

      case EFL_GFX_COLORSPACE_ETC1:
      case EFL_GFX_COLORSPACE_RGB8_ETC2:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          {
             // we could do regions, but only if they are 4x4 aligned.
             if (region_can) *region_can = EINA_FALSE;
             return _convert_etc2_rgb8_to_argb8888;
          }
        break;

      case EFL_GFX_COLORSPACE_RGBA8_ETC2_EAC:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          {
             // we could do regions, but only if they are 4x4 aligned.
             if (region_can) *region_can = EINA_FALSE;
             return _convert_etc2_rgba8_to_argb8888;
          }
        break;

      case EFL_GFX_COLORSPACE_ETC1_ALPHA:
        if (dstcs == EFL_GFX_COLORSPACE_ARGB8888)
          {
             // we could do regions, but only if they are 4x4 aligned.
             if (region_can) *region_can = EINA_FALSE;
             return _convert_etc1_alpha_to_argb8888;
          }
        break;

      case EFL_GFX_COLORSPACE_RGB_S3TC_DXT1:
      case EFL_GFX_COLORSPACE_RGBA_S3TC_DXT1:
      case EFL_GFX_COLORSPACE_RGBA_S3TC_DXT2:
      case EFL_GFX_COLORSPACE_RGBA_S3TC_DXT3:
      case EFL_GFX_COLORSPACE_RGBA_S3TC_DXT4:
      case EFL_GFX_COLORSPACE_RGBA_S3TC_DXT5:
         /* FIXME: can convert to ARGB */

      default:
        break;
     }


   // fallback to two-pass
   if (to_argb && from_argb)
     {
        if (region_can) *region_can = (reg1 && reg2);
        return _convert_generic_two_pass;
     }

   ERR("unsupported colorspace conversion from %d to %d", srccs, dstcs);
   if (region_can) *region_can = EINA_FALSE;
   return NULL;
}

int
efl_draw_argb_premul(uint32_t *data, unsigned int len)
{
   uint32_t *de = data + len;
   int nas = 0;

#ifdef BUILD_NEON
   if (eina_cpu_features_get() & EINA_CPU_NEON)
     {
        uint8x8_t mask_0x00 = vdup_n_u8(0);
        uint8x8_t mask_0x01 = vdup_n_u8(1);
        uint8x8_t mask_0xff = vdup_n_u8(255);
        uint8x8_t cmp;
        uint64x1_t tmp;

        while (data <= de - 8)
          {
             uint8x8x4_t rgba = vld4_u8((uint8_t *) data);

             cmp = vand_u8(vorr_u8(vceq_u8(rgba.val[3], mask_0xff),
                                   vceq_u8(rgba.val[3], mask_0x00)),
                           mask_0x01);
             tmp = vpaddl_u32(vpaddl_u16(vpaddl_u8(cmp)));
             nas += vget_lane_u32(vreinterpret_u32_u64(tmp), 0);

             uint16x8x4_t lrgba;
             lrgba.val[0] = vmovl_u8(rgba.val[0]);
             lrgba.val[1] = vmovl_u8(rgba.val[1]);
             lrgba.val[2] = vmovl_u8(rgba.val[2]);
             lrgba.val[3] = vaddl_u8(rgba.val[3], mask_0x01);

             rgba.val[0] = vshrn_n_u16(vmlaq_u16(lrgba.val[0], lrgba.val[0],
                                                 lrgba.val[3]), 8);
             rgba.val[1] = vshrn_n_u16(vmlaq_u16(lrgba.val[1], lrgba.val[1],
                                                 lrgba.val[3]), 8);
             rgba.val[2] = vshrn_n_u16(vmlaq_u16(lrgba.val[2], lrgba.val[2],
                                                 lrgba.val[3]), 8);
             vst4_u8((uint8_t *) data, rgba);
             data += 8;
          }
     }
#endif

   while (data < de)
     {
        uint32_t  a = 1 + (*data >> 24);

        *data = (*data & 0xff000000) +
          (((((*data) >> 8) & 0xff) * a) & 0xff00) +
          (((((*data) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
        data++;

        if ((a == 1) || (a == 256))
          nas++;
     }

   return nas;
}

void
efl_draw_argb_unpremul(uint32_t *data, unsigned int len)
{
   uint32_t *de = data + len;
   uint32_t p_val = 0x00000000, p_res = 0x00000000;

   while (data < de)
     {
        uint32_t a = (*data >> 24);

        if (p_val == *data) *data = p_res;
        else
          {
             p_val = *data;
             if ((a > 0) && (a < 255))
               *data = DRAW_ARGB_JOIN(a,
                                      (R_VAL(data) * 255) / a,
                                      (G_VAL(data) * 255) / a,
                                      (B_VAL(data) * 255) / a);
             else if (a == 0)
               *data = 0x00000000;
             p_res = *data;
          }
        data++;
     }
}
