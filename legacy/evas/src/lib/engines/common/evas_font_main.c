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
   int val;
   int ret;
   
   val = (int)fn->src->ft.face->ascender;
   fn->src->ft.face->units_per_EM = 2048; /* nasy hack - need to have correct val */
   ret = (val * fn->src->ft.face->size->metrics.y_scale) / (fn->src->ft.face->units_per_EM * fn->src->ft.face->units_per_EM);
   return ret;
}

int
evas_common_font_descent_get(RGBA_Font *fn)
{
   int val;
   int ret;
   
   evas_common_font_size_use(fn);
   val = -(int)fn->src->ft.face->descender;
   fn->src->ft.face->units_per_EM = 2048; /* nasy hack - need to have correct val */
   ret = (val * fn->src->ft.face->size->metrics.y_scale) / (fn->src->ft.face->units_per_EM * fn->src->ft.face->units_per_EM);
   return ret;
}

int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
   int val;
   int ret;
   
   evas_common_font_size_use(fn);
   val = (int)fn->src->ft.face->bbox.yMax;
   fn->src->ft.face->units_per_EM = 2048; /* nasy hack - need to have correct val */
   ret = (val * fn->src->ft.face->size->metrics.y_scale) / (fn->src->ft.face->units_per_EM * fn->src->ft.face->units_per_EM);
   return ret;
}

int
evas_common_font_max_descent_get(RGBA_Font *fn)
{
   int val;
   int ret;
   
   evas_common_font_size_use(fn);
   val = -(int)fn->src->ft.face->bbox.yMin;
   fn->src->ft.face->units_per_EM = 2048; /* nasy hack - need to have correct val */
   ret = (val * fn->src->ft.face->size->metrics.y_scale) / (fn->src->ft.face->units_per_EM * fn->src->ft.face->units_per_EM);
   return ret;
}

int
evas_common_font_get_line_advance(RGBA_Font *fn)
{
   int val;
   int ret;
   
   evas_common_font_size_use(fn);
   val = (int)fn->src->ft.face->height;
   fn->src->ft.face->units_per_EM = 2048; /* nasy hack - need to have correct val */
   ret = (val * fn->src->ft.face->size->metrics.y_scale) / (fn->src->ft.face->units_per_EM * fn->src->ft.face->units_per_EM);
   return ret;
}

int
evas_common_font_utf8_get_next(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the code point of the next valid code point. @index is
    * updated ready for the next call.
    * 
    * Returns 0 to indicate an error (e.g. invalid UTF8)
    */   
   int index = *iindex, r;
   unsigned char d = buf[index++], d2, d3, d4;
   
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
   *iindex = index;
   return r;
}

int
evas_common_font_utf8_get_prev(unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the code point of the previous valid code point. @index is
    * updated ready for the next call.
    * 
    * Returns 0 to indicate an error (e.g. invalid UTF8)
    */   
   int index = *iindex, r, istart = *iindex;
   unsigned char d = buf[index++], d2, d3, d4;
   
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
   return r;
}
