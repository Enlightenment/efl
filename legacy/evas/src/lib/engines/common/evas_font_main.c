/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

LK(lock_font_draw); // for freetype2 API calls
LK(lock_fribidi); // for fribidi API calls

EAPI void
evas_common_font_init(void)
{
   int error;

   initialised++;
   if (initialised != 1) return;
   error = FT_Init_FreeType(&evas_ft_lib);
   if (error) return;
   evas_common_font_load_init();
#ifdef EVAS_FRAME_QUEUING
   evas_common_font_draw_init();
#endif
   LKI(lock_font_draw);
   LKI(lock_fribidi);
}

EAPI void
evas_common_font_shutdown(void)
{
   int error;

   if (initialised < 1) return;
   initialised--;
   if (initialised != 0) return;

   LKD(lock_font_draw);
   LKD(lock_fribidi);
   
   evas_common_font_load_shutdown();
   evas_common_font_cache_set(0);
   evas_common_font_flush();

   error = FT_Done_FreeType(evas_ft_lib);
#ifdef EVAS_FRAME_QUEUING
   evas_common_font_draw_finish();
#endif
   evas_ft_lib = 0;
}

EAPI void
evas_common_font_font_all_unload(void)
{
   evas_common_font_all_clear();
}

EAPI int
evas_common_font_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
#if 0
     {
        Eina_List *l;
        
        EINA_LIST_FOREACH(fn->fonts, l, fi)
          {
             if (!fi->src->ft.face) continue;
             if (fi->src->current_size != fi->size)
               {
                  FT_Activate_Size(fi->ft.size);
                  fi->src->current_size = fi->size;
               }
             val = (int)fi->src->ft.face->size->metrics.ascender;
             if (fi->src->ft.face->units_per_EM == 0)
               return val;
             dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
             ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
             printf(" ==== %p: %i\n", fi, ret);
          }
     }
#endif
   fi = fn->fonts->data;
   if (fi->src->current_size != fi->size)
     {
        FT_Activate_Size(fi->ft.size);
        fi->src->current_size = fi->size;
     }
   if (!FT_IS_SCALABLE(fi->src->ft.face))
     {
        printf("NOT SCALABLE!\n");
     }
   val = (int)fi->src->ft.face->size->metrics.ascender;
   return val >> 6;
//   printf("%i | %i\n", val, val >> 6);
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   if (fi->src->current_size != fi->size)
     {
        FT_Activate_Size(fi->ft.size);
        fi->src->current_size = fi->size;
     }
   val = -(int)fi->src->ft.face->size->metrics.descender;
   return val >> 6;
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   if (fi->src->current_size != fi->size)
     {
        FT_Activate_Size(fi->ft.size);
        fi->src->current_size = fi->size;
     }
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

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   if (fi->src->current_size != fi->size)
     {
        FT_Activate_Size(fi->ft.size);
        fi->src->current_size = fi->size;
     }
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

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   if (fi->src->current_size != fi->size)
     {
        FT_Activate_Size(fi->ft.size);
        fi->src->current_size = fi->size;
     }
   val = (int)fi->src->ft.face->size->metrics.height;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   return val >> 6;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_utf8_get_next(const unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
   int index = *iindex, len, r;
   unsigned char d, d2, d3, d4;

   /* if this char is the null terminator, exit */
   if (!buf[index])
     return 0;
     
   d = buf[index++];

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
evas_common_font_utf8_get_prev(const unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the prev code point after this.
    *
    * Returns 0 to indicate there is no prev char
    */

   int r;
   int index = *iindex;
   /* although when index == 0 there's no previous char, we still want to get
    * the current char */
   if (index < 0) 
     return 0;

   /* First obtain the codepoint at iindex */
   r = evas_common_font_utf8_get_next(buf, &index);

   /* Next advance iindex to previous codepoint */
   index = *iindex;
   index--;
   while ((index > 0) && ((buf[index] & 0xc0) == 0x80))
     index--;

   *iindex = index;
   return r;
}

EAPI int
evas_common_font_utf8_get_last(const unsigned char *buf, int buflen)
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

EAPI int
evas_common_font_utf8_get_len(const unsigned char *buf)
{
   /* returns the number of utf8 characters (not bytes) in the string */
   int index = 0, len = 0;

   while (buf[index])
     {
	if ((buf[index] & 0xc0) != 0x80)
	  len++;
	index++;
     }
   return len;
}
