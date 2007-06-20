/** NOTE: This file is meant to be included by users **/

/*****************************************************************************
 * Scanline processing
 *
 *    _soft16_scanline_<description>_<src>_<dst>[_<modifier>]()
 *
 ****************************************************************************/
static inline void
_soft16_scanline_fill_solid_solid(DATA16 *dst, int size, DATA16 rgb565)
{
   DATA16 *start, *end;
   DATA32 rgb565_double;

   start = dst;
   end = start + (size & ~7);

   rgb565_double = (rgb565 << 16) | rgb565;

   while (start < end)
     {
        DATA32 *p = (DATA32 *)start;

        p[0] = rgb565_double;
        p[1] = rgb565_double;
        p[2] = rgb565_double;
        p[3] = rgb565_double;

        start += 8;
     }

   end = start + (size & 7);
   for (; start < end; start++)
      *start = rgb565;
}

static inline void
_soft16_scanline_fill_transp_solid(DATA16 *dst, int size, DATA32 rgb565_unpack, char alpha)
{
   DATA16 *start, *end;
   DATA32 a;

   start = dst;
   pld(start, 0);
   end = start + (size & ~7);

#define BLEND(dst)                                                      \
    { DATA32 b;                                                         \
      b = RGB_565_UNPACK(dst);                                          \
      b = RGB_565_UNPACKED_BLEND(rgb565_unpack, b, alpha);              \
      dst = RGB_565_PACK(b); }

   while (start < end)
      {
	 pld(start, 32);
	 UNROLL8({
	    BLEND(*start);
	    start++;
	 });
      }

   end = start + (size & 7);
   for (; start < end; start++)
      BLEND(*start);
#undef BLEND
}
