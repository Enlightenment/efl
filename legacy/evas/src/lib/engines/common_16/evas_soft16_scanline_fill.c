/** NOTE: This file is meant to be included by users **/

/*****************************************************************************
 * Point processing
 *
 *    _soft16_pt_<description>_<src>_<dst>[_<modifier>]()
 *
 * Scanline processing
 *
 *    _soft16_scanline_<description>_<src>_<dst>[_<modifier>]()
 *
 ****************************************************************************/
static always_inline void
_soft16_pt_fill_solid_solid(DATA16 *dst, DATA16 rgb565)
{
   *dst = rgb565;
}

static void
_soft16_scanline_fill_solid_solid(DATA16 *dst, int size, DATA16 rgb565)
{
   DATA16 *start, *end;
   DATA32 rgb565_double;

   start = dst;

   if ((long)start & 0x2)
     {
	*start = rgb565;
	start++;
	size--;
     }

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

static always_inline void
_soft16_pt_fill_transp_solid(DATA16 *dst, DATA32 rgb565_unpack, DATA8 alpha)
{
   DATA32 d;

   d = RGB_565_UNPACK(*dst);
   d = RGB_565_UNPACKED_BLEND(rgb565_unpack, d, alpha);
   *dst = RGB_565_PACK(d);
}

static void
_soft16_scanline_fill_transp_solid(DATA16 *dst, int size, DATA32 rgb565_unpack, DATA8 alpha)
{
   DATA16 *start, *end;

   start = dst;
   pld(start, 0);
   end = start + (size & ~7);

   while (start < end)
     {
        pld(start, 32);
        UNROLL8({
           _soft16_pt_fill_transp_solid(start, rgb565_unpack, alpha);
           start++;
        });
     }

   end = start + (size & 7);
   for (; start < end; start++)
     _soft16_pt_fill_transp_solid(start, rgb565_unpack, alpha);
}
