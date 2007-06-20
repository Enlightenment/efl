/** NOTE: This file is meant to be included by users **/

/*****************************************************************************
 * Scanline processing
 *
 *    _soft16_scanline_<description>_<src>_<dst>[_<modifier>]()
 *
 ****************************************************************************/

/***********************************************************************
 * Regular blend operations
 */
static inline void
_soft16_scanline_blend_transp_solid(DATA16 *src, DATA8 *alpha, DATA16 *dst, int size)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(alpha, 0);
   pld(src, 0);

#define BLEND(dst, src, alpha)                                          \
   if (UNLIKELY(alpha == 31))                                           \
     (dst) = (src);                                                     \
   else if (alpha != 0)                                                 \
     {                                                                  \
        DATA32 a, b;                                                    \
        a = RGB_565_UNPACK(src);                                        \
        b = RGB_565_UNPACK(dst);                                        \
        b = RGB_565_UNPACKED_BLEND(a, b, alpha);                        \
        dst = RGB_565_PACK(b);                                          \
     }

   /* work on 8 pixels per time, do data preload */
   while (start < end)
     {
	DATA8 alpha1, alpha2;

	alpha1 = alpha[0];
	alpha += 8;

	/* empirical tests show these give the best performance */
	pld(alpha, 8);
	pld(src, 32);

	src += 8;
	start += 8;

	alpha2 = alpha[-7];
	BLEND(start[-8], src[-8], alpha1);

	alpha1 = alpha[-6];
	BLEND(start[-7], src[-7], alpha2);

	alpha2 = alpha[-5];
	BLEND(start[-6], src[-6], alpha1);

	alpha1 = alpha[-4];
	BLEND(start[-5], src[-5], alpha2);

	alpha2 = alpha[-3];
	BLEND(start[-4], src[-4], alpha1);

	alpha1 = alpha[-2];
	BLEND(start[-3], src[-3], alpha2);

	alpha2 = alpha[-1];
	BLEND(start[-2], src[-2], alpha1);

	BLEND(start[-1], src[-1], alpha2);
     }

   /* remaining pixels (up to 7) */
   end = start + (size & 7);
   for (; start < end; start++, src++, alpha++)
      BLEND(*start, *src, *alpha);
#undef BLEND
}

static inline void
_soft16_scanline_blend_solid_solid(DATA16 *src, DATA16 *dst, int size)
{
   memcpy(dst, src, size * sizeof(DATA16));
}

/***********************************************************************
 * Blend operations taking an extra alpha (fade in, out)
 */
static inline void _soft16_scanline_blend_transp_solid_mul_alpha(DATA16 *src, DATA8 *alpha, DATA16 *dst, int size, char rel_alpha)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(alpha, 0);
   pld(src, 0);

#define BLEND(dst, src, alpha)                                          \
   if (alpha > rel_alpha)                                               \
     {                                                                  \
        DATA32 a, b;                                                    \
        a = RGB_565_UNPACK(src);                                        \
        b = RGB_565_UNPACK(dst);                                        \
        b = RGB_565_UNPACKED_BLEND(a, b, alpha - rel_alpha);            \
        dst = RGB_565_PACK(b);                                          \
     }

   while (start < end)
     {
	DATA8 alpha1, alpha2;

	alpha1 = alpha[0];
	alpha += 8;

	pld(alpha, 8);
	pld(src, 32);

	src += 8;
	start += 8;

	alpha2 = alpha[-7];
	BLEND(start[-8], src[-8], alpha1);

	alpha1 = alpha[-6];
	BLEND(start[-7], src[-7], alpha2);

	alpha2 = alpha[-5];
	BLEND(start[-6], src[-6], alpha1);

	alpha1 = alpha[-4];
	BLEND(start[-5], src[-5], alpha2);

	alpha2 = alpha[-3];
	BLEND(start[-4], src[-4], alpha1);

	alpha1 = alpha[-2];
	BLEND(start[-3], src[-3], alpha2);

	alpha2 = alpha[-1];
	BLEND(start[-2], src[-2], alpha1);

	BLEND(start[-1], src[-1], alpha2);
     }

   end = start + (size & 7);
   for (; start < end; start++, src++, alpha++)
      BLEND(*start, *src, *alpha);
#undef BLEND
}

static inline void
_soft16_scanline_blend_solid_solid_mul_alpha(DATA16 *src, DATA16 *dst, int size, char rel_alpha)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(src, 0);

#define BLEND(dst, src)                                                 \
   {                                                                    \
   DATA32 a, b;                                                         \
   a = RGB_565_UNPACK(src);                                             \
   b = RGB_565_UNPACK(dst);                                             \
   b = RGB_565_UNPACKED_BLEND(a, b, rel_alpha);                         \
   dst = RGB_565_PACK(b);                                               \
   }

   while (start < end)
     {
	pld(src, 32);
        UNROLL8({
           BLEND(*start, *src);
           start++;
           src++;
        });
     }

   end = start + (size & 7);
   for (; start < end; start++, src++)
      BLEND(*start, *src);
#undef BLEND
}

/***********************************************************************
 * Blend operations with extra alpha and multiply color
 */
static inline void _soft16_scanline_blend_transp_solid_mul_color_transp(DATA16 *src, DATA8 *alpha, DATA16 *dst, int size, char rel_alpha, short r, short g, short b)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(alpha, 0);
   pld(src, 0);

   /* rel_alpha is always > 0, so (alpha - rel_alpha) is always < 31 */
#define BLEND(dst, src, alpha)                                          \
   if ((alpha) > rel_alpha)                                             \
     {                                                                  \
        short r1, g1, b1;                                               \
        int rgb, d;                                                     \
        r1 = ((((src) >> 11) & 0x1f) * r) >> 8;                         \
        g1 = ((((src) >> 5) & 0x3f) * g) >> 8;                          \
        b1 = (((src) & 0x1f) * b) >> 8;                                 \
        rgb = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;   \
        d = RGB_565_UNPACK(dst);                                        \
        d = RGB_565_UNPACKED_BLEND(rgb, d, alpha - rel_alpha);          \
        dst = RGB_565_PACK(d);                                          \
     }

   while (start < end)
     {
	DATA8 alpha1, alpha2;

	alpha1 = alpha[0];
	alpha += 8;

        pld(src, 32);
        pld(start, 32);

	src += 8;
	start += 8;

	alpha2 = alpha[-7];
	BLEND(start[-8], src[-8], alpha1);

	alpha1 = alpha[-6];
	BLEND(start[-7], src[-7], alpha2);

	alpha2 = alpha[-5];
	BLEND(start[-6], src[-6], alpha1);

	alpha1 = alpha[-4];
	BLEND(start[-5], src[-5], alpha2);

	alpha2 = alpha[-3];
	BLEND(start[-4], src[-4], alpha1);

	alpha1 = alpha[-2];
	BLEND(start[-3], src[-3], alpha2);

	alpha2 = alpha[-1];
	BLEND(start[-2], src[-2], alpha1);

	BLEND(start[-1], src[-1], alpha2);
     }

   end = start + (size & 7);
   for (; start < end; start++, src++, alpha++)
      BLEND(*start, *src, *alpha);
#undef BLEND
}

static inline void
_soft16_scanline_blend_solid_solid_mul_color_transp(DATA16 *src, DATA16 *dst, int size, char rel_alpha, short r, short g, short b)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(src, 0);

#define BLEND(dst, src)                                                 \
     {                                                                  \
        short r1, g1, b1;                                               \
        int rgb, d;                                                     \
        r1 = ((((src) >> 11) & 0x1f) * r) >> 8;                         \
        g1 = ((((src) >> 5) & 0x3f) * g) >> 8;                          \
        b1 = (((src) & 0x1f) * b) >> 8;                                 \
        rgb = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;   \
        d = RGB_565_UNPACK(dst);                                        \
        d = RGB_565_UNPACKED_BLEND(rgb, d, rel_alpha);                  \
        dst = RGB_565_PACK(d);                                          \
     }

   while (start < end)
     {
	pld(src, 32);
        UNROLL8({
           BLEND(*start, *src);
           start++;
           src++;
        });
     }

   end = start + (size & 7);
   for (; start < end; start++, src++)
      BLEND(*start, *src);
#undef BLEND
}

/***********************************************************************
 * Blend operations with extra multiply color
 */
static inline void _soft16_scanline_blend_transp_solid_mul_color_solid(DATA16 *src, DATA8 *alpha, DATA16 *dst, int size, short r, short g, short b)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(alpha, 0);
   pld(src, 0);

#define BLEND(dst, src, alpha)                                          \
   if (UNLIKELY(alpha == 31))                                           \
     {                                                                  \
        short r1, g1, b1;                                               \
        r1 = (((((src) >> 11) & 0x1f) * r) >> 8) & 0x1f;                \
        g1 = (((((src) >> 5) & 0x3f) * g) >> 8) & 0x3f;                 \
        b1 = ((((src) & 0x1f) * b) >> 8) & 0x1f;                        \
        dst = ((r1 << 11) | (g1 << 5) | b1);                            \
     }                                                                  \
   else if (alpha != 0)                                                 \
     {                                                                  \
        short r1, g1, b1;                                               \
        int rgb, d;                                                     \
        r1 = ((((src) >> 11) & 0x1f) * r) >> 8;                         \
        g1 = ((((src) >> 5) & 0x3f) * g) >> 8;                          \
        b1 = (((src) & 0x1f) * b) >> 8;                                 \
        rgb = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;   \
        d = RGB_565_UNPACK(dst);                                        \
        d = RGB_565_UNPACKED_BLEND(rgb, d, alpha);                      \
        dst = RGB_565_PACK(d);                                          \
     }

   while (start < end)
     {
	DATA8 alpha1, alpha2;

	alpha1 = alpha[0];
	alpha += 8;

	pld(alpha, 8);
	pld(src, 32);

	src += 8;
	start += 8;

	alpha2 = alpha[-7];
	BLEND(start[-8], src[-8], alpha1);

	alpha1 = alpha[-6];
	BLEND(start[-7], src[-7], alpha2);

	alpha2 = alpha[-5];
	BLEND(start[-6], src[-6], alpha1);

	alpha1 = alpha[-4];
	BLEND(start[-5], src[-5], alpha2);

	alpha2 = alpha[-3];
	BLEND(start[-4], src[-4], alpha1);

	alpha1 = alpha[-2];
	BLEND(start[-3], src[-3], alpha2);

	alpha2 = alpha[-1];
	BLEND(start[-2], src[-2], alpha1);

	BLEND(start[-1], src[-1], alpha2);
     }

   end = start + (size & 7);
   for (; start < end; start++, src++, alpha++)
      BLEND(*start, *src, *alpha);
#undef BLEND
}

static inline void
_soft16_scanline_blend_solid_solid_mul_color_solid(DATA16 *src, DATA16 *dst, int size, short r, short g, short b)
{
   DATA16 *start, *end;

   start = dst;
   end = start + (size & ~7);

   pld(src, 0);

#define BLEND(dst, src)                                                 \
  {                                                                     \
     short r1, g1, b1;                                                  \
     r1 = (((((src) >> 11) & 0x1f) * r) >> 8) & 0x1f;                   \
     g1 = (((((src) >> 5) & 0x3f) * g) >> 8) & 0x3f;                    \
     b1 = ((((src) & 0x1f) * b) >> 8) & 0x1f;                           \
     dst = ((r1 << 11) | (g1 << 5) | b1);                               \
  }

   while (start < end)
     {
	pld(src, 32);
        UNROLL8({
           BLEND(*start, *src);
           start++;
           src++;
        });
     }

   end = start + (size & 7);
   for (; start < end; start++, src++)
      BLEND(*start, *src);
#undef BLEND
}
