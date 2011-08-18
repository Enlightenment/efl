#include "evas_common.h"/* Also includes international specific stuff */
#include "evas_common_soft16.h"

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */
int _evas_soft16_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR( _evas_soft16_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef CRIT
#undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_soft16_log_dom, __VA_ARGS__)

#define NOT_IMPLEMENTED()                                               \
  WRN("NOT_IMPLEMENTED: %s() at %s:%d",				\
           __FUNCTION__, __FILE__, __LINE__)

static void
eng_rectangle_draw(void *data __UNUSED__, void *context, void *surface, int x, int y, int w, int h)
{
   evas_common_soft16_rectangle_draw(surface, context, x, y, w, h);
}

static void
eng_line_draw(void *data __UNUSED__, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   evas_common_soft16_line_draw(surface, context, x1, y1, x2, y2);
}

static void
eng_polygon_draw(void *data __UNUSED__, void *context, void *surface, void *polygon, int x, int y)
{
   evas_common_soft16_polygon_draw(surface, context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data __UNUSED__, void *image)
{
   Soft16_Image *im;

   if (!image) return 0;
   im = image;
   return im->cache_entry.flags.alpha;
}

static void *
eng_image_alpha_set(void *data __UNUSED__, void *image, int have_alpha)
{
   if (!image) return NULL;
   have_alpha = !!have_alpha;
   image = evas_common_soft16_image_alpha_set(image, have_alpha);
   return image;
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image __UNUSED__, char *key __UNUSED__)
{
   return NULL;
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return EVAS_COLORSPACE_RGB565_A5P;
}


static void
eng_image_colorspace_set(void *data __UNUSED__, void *image __UNUSED__, int cspace __UNUSED__)
{
   NOT_IMPLEMENTED();
}

static void *
eng_image_native_set(void *data __UNUSED__, void *image __UNUSED__, void *native __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void *
eng_image_load(void *data __UNUSED__, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   return evas_cache_image_request(evas_common_soft16_image_cache_get(), file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	WRN("Unsupported colorspace %d in %s() (%s:%d)",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void *
eng_image_new_from_copied_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	WRN("Unsupported colorspace %d in %s() (%s:%d)",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_copied_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   Soft16_Image *im;

   if (w) *w = 0;
   if (h) *h = 0;
   if (!image) return;
   im = image;
   if (w) *w = im->cache_entry.w;
   if (h) *h = im->cache_entry.h;
}

static void *
eng_image_size_set(void *data __UNUSED__, void *image, int w, int h)
{
   if (!image) return NULL;
   if ((w <= 0) || (h <= 0))
     {
        evas_cache_image_drop((Image_Entry *) image);
	return NULL;
     }
   return evas_cache_image_size_set((Image_Entry *) image, w, h);
}

static void
eng_image_stride_get(void *data __UNUSED__, void *image, int *stride)
{
   Soft16_Image *im;

   if (stride) *stride = 0;
   if (!image) return;
   im = image;
   if (stride) *stride = im->stride;
}

static void *
eng_image_dirty_region(void *data __UNUSED__, void *image, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   /* FIXME: is this required? */
   //NOT_IMPLEMENTED();
   return image;
}

static void *
eng_image_data_get(void *data __UNUSED__, void *image, int to_write, DATA32 **image_data, int *err)
{
   Soft16_Image *im;
   int error;

   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }

   im = image;
   error = evas_cache_image_load_data(&im->cache_entry);

   if (to_write)
     im = (Soft16_Image *) evas_cache_image_alone(&im->cache_entry);

   if (image_data) *image_data = (DATA32 *) im->pixels;

   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data __UNUSED__, void *image, DATA32 *image_data)
{
   Soft16_Image *old_im, *new_im;

   if (!image) return NULL;

   old_im = image;
   if ((DATA16 *)image_data == old_im->pixels) return old_im;

   new_im = (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), old_im->cache_entry.w, old_im->cache_entry.h, image_data, old_im->cache_entry.flags.alpha, EVAS_COLORSPACE_RGB565_A5P);
   evas_cache_image_drop(&old_im->cache_entry);
   return new_im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   Soft16_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   Soft16_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data __UNUSED__, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Soft16_Image *im;

   im = (Soft16_Image *) image;

   evas_cache_image_load_data(&im->cache_entry);
   evas_common_soft16_image_draw(im, surface, context,
		     src_x, src_y, src_w, src_h,
		     dst_x, dst_y, dst_w, dst_h,
		     smooth);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image __UNUSED__, int hint __UNUSED__)
{
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return EVAS_IMAGE_SCALE_HINT_NONE;
}

static void
eng_image_cache_flush(void *data __UNUSED__)
{
   evas_cache_image_flush(evas_common_soft16_image_cache_get());
}

static void
eng_image_cache_set(void *data __UNUSED__, int bytes)
{
   evas_cache_image_set(evas_common_soft16_image_cache_get(), bytes);
}

static int
eng_image_cache_get(void *data __UNUSED__)
{
   return evas_cache_image_get(evas_common_soft16_image_cache_get());
}

static void
eng_font_draw(void *data __UNUSED__, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const Evas_Text_Props *text_props)
{
   static RGBA_Image    *im = NULL;
   Soft16_Image         *dst = surface;

   if (!im)
     im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
   evas_cache_image_surface_alloc(&im->cache_entry, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_font_ext_set(context,
					 surface,
					 evas_common_soft16_font_glyph_new,
					 evas_common_soft16_font_glyph_free,
					 evas_common_soft16_font_glyph_draw);
   evas_common_font_draw(im, context, (RGBA_Font *) font, x, y, text_props);
   evas_common_draw_context_font_ext_set(context,
					 NULL,
					 NULL,
					 NULL,
					 NULL);
}

/*
 *****
 **
 ** MODULE ACCESSIBLE API API
 **
 *****
 */

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   _evas_soft16_log_dom = eina_log_domain_register
     ("evas-software_16", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_soft16_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   EVAS_API_RESET(info, &func);
   EVAS_API_RESET(info_free, &func);
   EVAS_API_RESET(setup, &func);
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_draw);
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_stride_get);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(font_draw);
   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);

   em->functions = (void *)(&func);
   
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
   eina_log_domain_unregister(_evas_soft16_log_dom);
}

static Evas_Module_Api evas_modapi =
{
  EVAS_MODULE_API_VERSION,
  "software_16",
  "none",
  {
    module_open,
    module_close
  }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_16);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_16
EVAS_EINA_MODULE_DEFINE(engine, software_16);
#endif
