/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"

FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

EAPI void
evas_common_font_init(void)
{
   int error;

   initialised++;
   if (initialised != 1) return;
   error = FT_Init_FreeType(&evas_ft_lib);
   if (error)
     {
	initialised--;
	return;
     }
}

EAPI void
evas_common_font_shutdown(void)
{
   int error;

   initialised--;
   if (initialised != 0) return;

   evas_common_font_cache_set(0);
   evas_common_font_flush();

   error = FT_Done_FreeType(evas_ft_lib);
   evas_ft_lib = 0;
}

EAPI int
evas_common_font_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->size->metrics.ascender;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;  
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = -(int)fi->src->ft.face->size->metrics.descender;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;  
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->bbox.yMax;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;  
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_max_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = -(int)fi->src->ft.face->bbox.yMin;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;  
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_get_line_advance(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->size->metrics.height;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;  
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_utf8_get_next(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
   int index = *iindex, len, r;
   unsigned char d, d2, d3, d4;

   d = buf[index++];
   if (!d)
     return 0;
   
   while (buf[index] && ((buf[index] & 0xc0) == 0x80))
     index++;
   len = index - *iindex;
   
   if (len == 1)
      r = d;
   else if (len == 2)
     {
	/* 2 bytes */
        d2 = buf[*iindex + 1];
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if (len == 3)
     {
	/* 3 bytes */
        d2 = buf[*iindex + 1];
        d3 = buf[*iindex + 2];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     {
	/* 4 bytes */
        d2 = buf[*iindex + 1];
        d3 = buf[*iindex + 2];
        d4 = buf[*iindex + 3];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
	r <<= 6;
	r |= (d4 & 0x3f);
     }
   
   *iindex = index;
   return r;
}

EAPI int
evas_common_font_utf8_get_prev(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the prev code point after this.
    *
    * Returns 0 to indicate there is no prev char
    */
   int index = *iindex, len, r;
   unsigned char d, d2, d3, d4;

   if (index <= 0)
     return 0;
   d = buf[index--];
   
   while ((index > 0) && ((buf[index] & 0xc0) == 0x80))
     index--;
   len = *iindex - index;
   
   if (len == 1)
      r = d;
   else if (len == 2)
     {
	/* 2 bytes */
        d2 = buf[index + 1];
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if (len == 3)
     {
	/* 3 bytes */
        d2 = buf[index + 1];
        d3 = buf[index + 2];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     {
	/* 4 bytes */
        d2 = buf[index + 1];
        d3 = buf[index + 2];
        d4 = buf[index + 3];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
	r <<= 6;
	r |= (d4 & 0x3f);
     }
   
   *iindex = index;
   return r;
}

EAPI int
evas_common_font_utf8_get_last(unsigned char *buf, int buflen)
{
   /* jumps to the nul byte at the buffer end and decodes backwards and
    * returns the offset index byte in the buffer where the last character
    * in the buffer begins.
    *
    * Returns -1 to indicate an error
    */
   int index;
   unsigned char d;

   if (buflen < 1) return 0;
   index = buflen - 1;
   d = buf[index];
   if (!(d & 0x80))
     return index;
   else
     {
	while (index > 0)
	  {
	     index--;
	     d = buf[index];
	     if ((d & 0xc0) != 0x80)
	       return index;
	  }
     }
   return 0;
}
