/** NOTE: This file is meant to be included by users **/

/** NOTE2: r, g, b parameters are 16bits, so you can pass 0 to 256 inclusive.
 **        this is due our division by 256 when multiplying the color.
 **/

/*****************************************************************************
 * Scanline processing
 *
 *    _soft16_pt_<description>_<src>_<dst>[_<modifier>]()
 *
 ****************************************************************************/

/***********************************************************************
 * Regular blend operations
 */

static always_inline void
_soft16_pt_blend_transp_solid(DATA16 *p_dst, DATA16 src, DATA8 alpha)
{
   if (alpha == 31) *p_dst = src;
   else if (alpha != 0)
     {
        DATA32 a, b;

        a = RGB_565_UNPACK(src);
        b = RGB_565_UNPACK(*p_dst);
        b = RGB_565_UNPACKED_BLEND(a, b, alpha);
        *p_dst = RGB_565_PACK(b);
     }
}

static always_inline void
_soft16_pt_blend_solid_solid(DATA16 *p_dst, DATA16 src)
{
   *p_dst = src;
}

/***********************************************************************
 * Blend operations taking an extra alpha (fade in, out)
 */

static always_inline void
_soft16_pt_blend_transp_solid_mul_alpha(DATA16 *p_dst, DATA16 src, DATA8 alpha, DATA8 rel_alpha)
{
   DATA32 a, b;

   alpha = (alpha * rel_alpha) >> 5;
   if (alpha == 0)
     return;

   alpha++;

   a = ((RGB_565_UNPACK(src) * rel_alpha) >> 5) & RGB_565_UNPACKED_MASK;
   b = RGB_565_UNPACK(*p_dst);
   b = RGB_565_UNPACKED_BLEND(a, b, alpha);
   *p_dst = RGB_565_PACK(b);
}

static always_inline void
_soft16_pt_blend_solid_solid_mul_alpha(DATA16 *p_dst, DATA16 src, DATA8 rel_alpha)
{
   DATA32 a, b;
   a = RGB_565_UNPACK(src);
   b = RGB_565_UNPACK(*p_dst);
   b = RGB_565_UNPACKED_BLEND_UNMUL(a, b, rel_alpha);
   *p_dst = RGB_565_PACK(b);
}

/***********************************************************************
 * Blend operations with extra alpha and multiply color
 */

static always_inline void
_soft16_pt_blend_transp_solid_mul_color_transp(DATA16 *p_dst, DATA16 src, DATA8 alpha, DATA8 rel_alpha, DATA16 r, DATA16 g, DATA16 b)
{
   DATA32 rgb, d;
   int r1, g1, b1;

   alpha = (alpha * rel_alpha) >> 5;
   if (alpha == 0)
     return;

   alpha++;

   r1 = ((((src) >> 11) & 0x1f) * r) >> 5;
   g1 = ((((src) >> 5) & 0x3f) * g) >> 6;
   b1 = (((src) & 0x1f) * b) >> 5;
   rgb = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;
   d = RGB_565_UNPACK(*p_dst);
   d = RGB_565_UNPACKED_BLEND(rgb, d, alpha);

   *p_dst = RGB_565_PACK(d);
}

static always_inline void
_soft16_pt_blend_solid_solid_mul_color_transp(DATA16 *p_dst, DATA16 src, DATA8 rel_alpha, DATA16 r, DATA16 g, DATA16 b)
{
   int r1, g1, b1;
   DATA32 rgb, d;

   r1 = ((((src) >> 11) & 0x1f) * r) >> 5;
   g1 = ((((src) >> 5) & 0x3f) * g) >> 6;
   b1 = (((src) & 0x1f) * b) >> 5;

   rgb = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;
   d = RGB_565_UNPACK(*p_dst);
   d = RGB_565_UNPACKED_BLEND(rgb, d, rel_alpha);
   *p_dst = RGB_565_PACK(d);
}

/***********************************************************************
 * Blend operations with extra multiply color
 */

static always_inline void
_soft16_pt_blend_transp_solid_mul_color_solid(DATA16 *p_dst, DATA16 src, DATA8 alpha, DATA8 r, DATA8 g, DATA8 b)
{
   int r1, g1, b1;

   if (alpha == 0) return;

   r1 = ((((src >> 11) & 0x1f) * r) >> 5) & 0x1f;
   g1 = ((((src >> 5) & 0x3f) * g) >> 6) & 0x3f;
   b1 = (((src & 0x1f) * b) >> 5) & 0x1f;

   if (alpha == 31) *p_dst = (r1 << 11) | (g1 << 5) | b1;
   else
     {
        DATA32 rgb_unpack, d;

        rgb_unpack = ((r1 << 11) | (g1 << 21) | b1) & RGB_565_UNPACKED_MASK;
        d = RGB_565_UNPACK(*p_dst);
        d = RGB_565_UNPACKED_BLEND(rgb_unpack, d, alpha);
        *p_dst = RGB_565_PACK(d);
     }
}

static always_inline void
_soft16_pt_blend_solid_solid_mul_color_solid(DATA16 *p_dst, DATA16 src, DATA16 r, DATA16 g, DATA16 b)
{
   int r1, g1, b1;

   r1 = ((((src >> 11) & 0x1f) * r) >> 5) & 0x1f;
   g1 = ((((src >> 5) & 0x3f) * g) >> 6) & 0x3f;
   b1 = (((src & 0x1f) * b) >> 5) & 0x1f;

   *p_dst = (r1 << 11) | (g1 << 5) | b1;
}
