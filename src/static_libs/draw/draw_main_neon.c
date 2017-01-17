#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "draw_private.h"

#undef BUILD_NEON
#ifdef BUILD_NEON
#include <arm_neon.h>

static void
comp_func_solid_source_over_neon(uint32_t * __restrict dest, int length, uint32_t color, uint32_t const_alpha)
{
   uint16x8_t temp00_16x8;
   uint16x8_t temp01_16x8;
   uint16x8_t temp10_16x8;
   uint16x8_t temp11_16x8;
   uint32x4_t temp0_32x4;
   uint32x4_t temp1_32x4;
   uint32x4_t c_32x4;
   uint32x4_t d0_32x4;
   uint32x4_t d1_32x4;
   uint8x16_t d0_8x16;
   uint8x16_t d1_8x16;
   uint8x16_t temp0_8x16;
   uint8x16_t temp1_8x16;
   uint8x8_t alpha_8x8;
   uint8x8_t d00_8x8;
   uint8x8_t d01_8x8;
   uint8x8_t d10_8x8;
   uint8x8_t d11_8x8;
   uint8x8_t temp00_8x8;
   uint8x8_t temp01_8x8;
   uint8x8_t temp10_8x8;
   uint8x8_t temp11_8x8;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);

   // alpha can only be 0 if color is 0x0. In that case we can just return.
   // Otherwise we can assume alpha != 0. This allows more optimization in
   // NEON code.
   if (!color)
     return;

   DATA32 *start = dest;
   int size = length;
   DATA32 *end = start + (size & ~7);

   unsigned char alpha;
   alpha = ~(color >> 24) + 1;
   alpha_8x8 = vdup_n_u8(alpha);

   c_32x4 = vdupq_n_u32(color);

   while (start < end)
     {
        d0_32x4 = vld1q_u32(start);
        d1_32x4 = vld1q_u32(start+4);
        d0_8x16 = vreinterpretq_u8_u32(d0_32x4);
        d1_8x16 = vreinterpretq_u8_u32(d1_32x4);

        d00_8x8 = vget_low_u8(d0_8x16);
        d01_8x8 = vget_high_u8(d0_8x16);
        d10_8x8 = vget_low_u8(d1_8x16);
        d11_8x8 = vget_high_u8(d1_8x16);

        temp00_16x8 = vmull_u8(alpha_8x8, d00_8x8);
        temp01_16x8 = vmull_u8(alpha_8x8, d01_8x8);
        temp10_16x8 = vmull_u8(alpha_8x8, d10_8x8);
        temp11_16x8 = vmull_u8(alpha_8x8, d11_8x8);

        temp00_8x8 = vshrn_n_u16(temp00_16x8,8);
        temp01_8x8 = vshrn_n_u16(temp01_16x8,8);
        temp10_8x8 = vshrn_n_u16(temp10_16x8,8);
        temp11_8x8 = vshrn_n_u16(temp11_16x8,8);

        temp0_8x16 = vcombine_u8(temp00_8x8, temp01_8x8);
        temp1_8x16 = vcombine_u8(temp10_8x8, temp11_8x8);

        temp0_32x4 = vreinterpretq_u32_u8(temp0_8x16);
        temp1_32x4 = vreinterpretq_u32_u8(temp1_8x16);

        d0_32x4 = vaddq_u32(c_32x4, temp0_32x4);
        d1_32x4 = vaddq_u32(c_32x4, temp1_32x4);

        vst1q_u32(start, d0_32x4);
        vst1q_u32(start+4, d1_32x4);
        start+=8;
     }

   end += (size & 7);
   while (start <  end)
     {
        *start = color + MUL_256(alpha, *start);
        start++;
     }
}

/* Note: Optimisation is based on keeping _dest_ aligned: else it's a pair of
 * reads, then two writes, a miss on read is 'just' two reads */
static void
comp_func_source_over_sse2(uint32_t * __restrict dest, const uint32_t * __restrict src, int length, uint32_t color, uint32_t const_alpha)
{
   uint16x8_t ad0_16x8;
   uint16x8_t ad1_16x8;
   uint16x8_t sc0_16x8;
   uint16x8_t sc1_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t c_32x2;
   uint32x4_t ad_32x4;
   uint32x4_t alpha_32x4;
   uint32x4_t cond_32x4;
   uint32x4_t d_32x4;
   uint32x4_t s_32x4;
   uint32x4_t sc_32x4;
   uint32x4_t x0_32x4;
   uint32x4_t x1_32x4;
   uint8x16_t ad_8x16;
   uint8x16_t alpha_8x16;
   uint8x16_t d_8x16;
   uint8x16_t s_8x16;
   uint8x16_t sc_8x16;
   uint8x16_t x0_8x16;
   uint8x16_t x1_8x16;
   uint8x8_t ad0_8x8;
   uint8x8_t ad1_8x8;
   uint8x8_t alpha0_8x8;
   uint8x8_t alpha1_8x8;
   uint8x8_t c_8x8;
   uint8x8_t d0_8x8;
   uint8x8_t d1_8x8;
   uint8x8_t s0_8x8;
   uint8x8_t s1_8x8;
   uint8x8_t sc0_8x8;
   uint8x8_t sc1_8x8;
   int size;
   DATA32 *start;
   DATA32 *end;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);

   c_32x2 = vdup_n_u32(color);
   c_8x8 = vreinterpret_u8_u32(c_32x2);
   x255_16x8 = vdupq_n_u16(0xff);
   x0_8x16 = vdupq_n_u8(0x0);
   x0_32x4 = vreinterpretq_u32_u8(x0_8x16);
   x1_8x16 = vdupq_n_u8(0x1);
   x1_32x4 = vreinterpretq_u32_u8(x1_8x16);
   start = dest;
   size = l;
   end = start + (size & ~3);

   while (start < end)
     {
        s_32x4 = vld1q_u32(src);
        s_8x16 = vreinterpretq_u8_u32(s_32x4);

        d_32x4 = vld1q_u32(start);
        d_8x16 = vreinterpretq_u8_u32(d_32x4);
        d0_8x8 = vget_low_u8(d_8x16);
        d1_8x8 = vget_high_u8(d_8x16);

        s0_8x8 = vget_low_u8(s_8x16);
        s1_8x8 = vget_high_u8(s_8x16);

        sc0_16x8 = vmull_u8(s0_8x8, c_8x8);
        sc1_16x8 = vmull_u8(s1_8x8, c_8x8);
        sc0_16x8 = vaddq_u16(sc0_16x8, x255_16x8);
        sc1_16x8 = vaddq_u16(sc1_16x8, x255_16x8);
        sc0_8x8 = vshrn_n_u16(sc0_16x8, 8);
        sc1_8x8 = vshrn_n_u16(sc1_16x8, 8);
        sc_8x16 = vcombine_u8(sc0_8x8, sc1_8x8);

        alpha_32x4 = vreinterpretq_u32_u8(sc_8x16);
        alpha_32x4 = vshrq_n_u32(alpha_32x4, 24);
        alpha_32x4 = vmulq_u32(x1_32x4, alpha_32x4);
        alpha_8x16 = vreinterpretq_u8_u32(alpha_32x4);
        alpha_8x16 = vsubq_u8(x0_8x16, alpha_8x16);
        alpha0_8x8 = vget_low_u8(alpha_8x16);
        alpha1_8x8 = vget_high_u8(alpha_8x16);

        ad0_16x8 = vmull_u8(alpha0_8x8, d0_8x8);
        ad1_16x8 = vmull_u8(alpha1_8x8, d1_8x8);
        ad0_8x8 = vshrn_n_u16(ad0_16x8,8);
        ad1_8x8 = vshrn_n_u16(ad1_16x8,8);
        ad_8x16 = vcombine_u8(ad0_8x8, ad1_8x8);
        ad_32x4 = vreinterpretq_u32_u8(ad_8x16);

        alpha_32x4 = vreinterpretq_u32_u8(alpha_8x16);
        cond_32x4 = vceqq_u32(alpha_32x4, x0_32x4);
        ad_32x4 = vbslq_u32(cond_32x4, d_32x4 , ad_32x4);

        sc_32x4 = vreinterpretq_u32_u8(sc_8x16);
        d_32x4 = vaddq_u32(sc_32x4, ad_32x4);

        vst1q_u32(start, d_32x4);

        src+=4;
        start+=4;
     }

   end += (size & 3);
   while (start <  end)
     {
        DATA32 sc = MUL4_SYM(color, *s);
        DATA32 alpha = 256 - (sc >> 24);
        *start = sc + MUL_256(alpha, *start);
        start++;
        src++;
     }
}
#endif

void
efl_draw_neon_init(void)
{
#ifdef BUILD_NEON
   if (getenv("EVAS_CPU_NO_NEON")) return; // compat with evas env vars
   if (eina_cpu_features_get() & EINA_CPU_NEON)
     {
        // update the comp_function table for solid color
        //func_for_mode_solid[EFL_GFX_RENDER_OP_COPY] = comp_func_solid_source_sse2;
        func_for_mode_solid[EFL_GFX_RENDER_OP_BLEND] = comp_func_solid_source_over_neon;

        // update the comp_function table for source data
        //func_for_mode[EFL_GFX_RENDER_OP_COPY] = comp_func_source_sse2;
        func_for_mode[EFL_GFX_RENDER_OP_BLEND] = comp_func_source_over_neon;
      }
#endif
}
