#include "evas_common.h"

FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

void
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

void
evas_common_font_shutdown(void)
{
   int error;

   initialised--;
   if (initialised != 0) return;
   error = FT_Done_FreeType(evas_ft_lib);
}

int
evas_common_font_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;
   
   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->size->metrics.ascender;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM; 
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

int
evas_common_font_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;
   
   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = -(int)fi->src->ft.face->size->metrics.descender;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM; 
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;
   
   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->bbox.yMax;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM; 
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

int
evas_common_font_max_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;
   
   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = -(int)fi->src->ft.face->bbox.yMin;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;   
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

int
evas_common_font_get_line_advance(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;
   
   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   val = (int)fi->src->ft.face->size->metrics.height;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;   
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

int
evas_common_font_utf8_get_next(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iidnex 
    * to the next code point after this.
    * 
    * Returns 0 to indicate an error (e.g. invalid UTF8)
    */   
   int index = *iindex, r;
   unsigned char d, d2, d3, d4;
   
   d = buf[index++];
   if (!d)
     return 0;
   if (d < 0x80) 
     {
	*iindex = index;
	return d;      
     }
   if ((d & 0xe0) == 0xc0) 
     { 
	/* 2 byte */
	if (((d2 = buf[index++]) & 0xc0) != 0x80)
	  return 0;
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if ((d & 0xf0) == 0xe0) 
     { 
	/* 3 byte */
	if (((d2 = buf[index++]) & 0xc0) != 0x80 ||
	   ((d3 = buf[index++]) & 0xc0) != 0x80)
	  return 0;
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     { 
	/* 4 byte */
	if (((d2 = buf[index++]) & 0xc0) != 0x80 ||
	    ((d3 = buf[index++]) & 0xc0) != 0x80 ||
	    ((d4 = buf[index++]) & 0xc0) != 0x80)
	  return 0;
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

int
evas_common_font_utf8_get_prev(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iidnex 
    * to the next code point after this.
    * 
    * Returns 0 to indicate an error (e.g. invalid UTF8)
    */   
   int index = *iindex, r, istart = *iindex;
   unsigned char d, d2, d3, d4;

   d = buf[index++];
   if (d < 0x80) 
     {
	r = d;
     }
   else if ((d & 0xe0) == 0xc0) 
     { 
	/* 2 byte */
	d2 = buf[index++];
	if ((d2 & 0xc0) != 0x80)
	  return 0;
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if ((d & 0xf0) == 0xe0) 
     { 
	/* 3 byte */
	d2 = buf[index++];
	d3 = buf[index++];
	if ((d2 & 0xc0) != 0x80 ||
	    (d3 & 0xc0) != 0x80)
	  return 0;
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     { 
	/* 4 byte */
	d2 = buf[index++];
	d3 = buf[index++];
	d4 = buf[index++];
	if ((d2 & 0xc0) != 0x80 ||
	    (d3 & 0xc0) != 0x80 ||
	    (d4 & 0xc0) != 0x80)
	  return 0;
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
	r <<= 6;
	r |= (d4 & 0x3f);	
     }
   if (istart > 0)
     {
	index = istart - 1;
	d = buf[index];
	if (!(d & 0x80))
	  *iindex = index;
	else
	  {
	     while (index > 0)
	       {
		  index--;
		  d = buf[index];
		  if ((d & 0xc0) != 0x80)
		    {
		       *iindex = index;
		       return r;
		    }
	       }
	  }
     }
   else
     {
	*iindex = -1;
     }
   return r;
}

int
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
