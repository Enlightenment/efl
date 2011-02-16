#include "evas_common.h"
#include "evas_private.h"

#include "evas_font_private.h"

FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

LK(lock_font_draw); // for freetype2 API calls
LK(lock_bidi); // for evas bidi internal usage.
LK(lock_ot); // for evas bidi internal usage.

EAPI void
evas_common_font_init(void)
{
   int error;

   initialised++;
   if (initialised != 1) return;
   error = FT_Init_FreeType(&evas_ft_lib);
   if (error) return;
   evas_common_font_load_init();
   evas_common_font_draw_init();
   LKI(lock_font_draw);
   LKI(lock_bidi);
   LKI(lock_ot);
}

EAPI void
evas_common_font_shutdown(void)
{
   int error;

   if (initialised < 1) return;
   initialised--;
   if (initialised != 0) return;

   LKD(lock_font_draw);
   LKD(lock_bidi);
   LKD(lock_ot);

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
   int val;
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
		  FTLOCK();
                  FT_Activate_Size(fi->ft.size);
		  FTUNLOCK();
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
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   if (!FT_IS_SCALABLE(fi->src->ft.face))
     {
        WRN("NOT SCALABLE!");
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
   int val;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
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
   evas_common_font_int_reload(fi); 
  if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
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
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
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
   int val;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
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

