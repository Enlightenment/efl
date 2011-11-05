/** NOTE: This file is meant to be included by users **/

/*****************************************************************************
 * Point processing
 *
 *    _soft8_pt_<description>_<src>_<dst>[_<modifier>]()
 *
 * Scanline processing
 *
 *    _soft8_scanline_<description>_<src>_<dst>[_<modifier>]()
 *
 ****************************************************************************/
EFL_ALWAYS_INLINE void
_soft8_pt_fill_solid_solid(DATA8 * dst, DATA8 gry8)
{
   *dst = gry8;
}

EFL_ALWAYS_INLINE void
_soft8_scanline_fill_solid_solid(DATA8 * dst, int size, DATA8 gry8)
{
   memset(dst, gry8, size);
}

EFL_ALWAYS_INLINE void
_soft8_pt_fill_transp_solid(DATA8 * dst, DATA8 gry8, DATA8 alpha)
{
   *dst = GRY_8_BLEND(gry8, *dst, alpha);
}

static void
_soft8_scanline_fill_transp_solid(DATA8 * dst, int size, DATA8 gry8,
                                  DATA8 alpha)
{
   DATA8 *start, *end;

   start = dst;
   pld(start, 0);
   end = start + (size & ~7);

   while (start < end)
     {
        pld(start, 32);
        UNROLL8(
                  {
                  _soft8_pt_fill_transp_solid(start, gry8, alpha); start++;}
        );
     }

   end = start + (size & 7);
   for (; start < end; start++)
      _soft8_pt_fill_transp_solid(start, gry8, alpha);
}
