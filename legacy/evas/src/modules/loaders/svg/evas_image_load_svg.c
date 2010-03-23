#include "evas_common.h"
#include "evas_private.h"

#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

static inline Eina_Bool evas_image_load_file_is_svg(const char *file) EINA_ARG_NONNULL(1) EINA_PURE;
static Eina_Bool evas_image_load_file_head_svg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_svg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

Evas_Image_Load_Func evas_image_load_svg_func =
{
  EINA_FALSE,
  evas_image_load_file_head_svg,
  evas_image_load_file_data_svg
};

static int  rsvg_initialized = 0;


static inline Eina_Bool evas_image_load_file_is_svg(const char *file)
{
   int i, len = strlen(file);
   Eina_Bool is_gz = EINA_FALSE;

   for (i = len - 1; i > 0; i--)
     {
	if (file[i] == '.')
	  {
	     if (is_gz)
	       break;
	     else if (strcasecmp(file + i + 1, "gz") == 0)
	       is_gz = EINA_TRUE;
	     else
	       break;
	  }
     }

   if (i < 1) return EINA_FALSE;
   i++;
   if (i >= len) return EINA_FALSE;
   if (strncasecmp(file + i, "svg", 3) != 0) return EINA_FALSE;
   i += 3;
   if (is_gz)
     {
	if (file[i] == '.') return EINA_TRUE;
	else return EINA_FALSE;
     }
   else
     {
	if (file[i] == '\0') return EINA_TRUE;
	else if (((file[i] == 'z') || (file[i] == 'Z')) && (!file[i + 1])) return EINA_TRUE;
	else return EINA_FALSE;
     }
}

static Eina_Bool
evas_image_load_file_head_svg(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   RsvgHandle         *rsvg;
   RsvgDimensionData   dim;
   int                 w, h;

   /* ignore all files not called .svg or .svg.gz - because rsvg has a leak
    * where closing the handle doesn't free mem */
   if (!evas_image_load_file_is_svg(file))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

   rsvg = rsvg_handle_new_from_file(file, NULL);
   if (!rsvg)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   rsvg_handle_set_dpi(rsvg, 75.0);
   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
	if (IMG_TOO_BIG(w, h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if (ie->load_opts.scale_down_by > 1)
     {
	w /= ie->load_opts.scale_down_by;
	h /= ie->load_opts.scale_down_by;
     }
   else if (ie->load_opts.dpi > 0.0)
     {
	w = (w * ie->load_opts.dpi) / 75.0;
	h = (h * ie->load_opts.dpi) / 75.0;
     }
   else if ((ie->load_opts.w > 0) &&
	    (ie->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = ie->load_opts.w;
	h2 = (ie->load_opts.w * h) / w;
	if (h2 > ie->load_opts.h)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ie->w = w;
   ie->h = h;
   ie->flags.alpha = 1;
   rsvg_handle_close(rsvg, NULL);
   g_object_unref(rsvg);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

/** FIXME: All evas loaders need to be tightened up **/
static Eina_Bool
evas_image_load_file_data_svg(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   DATA32             *pixels;
   RsvgHandle         *rsvg;
   RsvgDimensionData   dim;
   int                 w, h;
   cairo_surface_t    *surface;
   cairo_t            *cr;

   /* ignore all files not called .svg or .svg.gz - because rsvg has a leak
    * where closing the handle doesn't free mem */
   if (!evas_image_load_file_is_svg(file))
     {
	*error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
	return EINA_FALSE;
     }

   rsvg = rsvg_handle_new_from_file(file, NULL);
   if (!rsvg)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   rsvg_handle_set_dpi(rsvg, 75.0);
   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE))
     {
	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
	if (IMG_TOO_BIG(w, h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if (ie->load_opts.scale_down_by > 1)
     {
	w /= ie->load_opts.scale_down_by;
	h /= ie->load_opts.scale_down_by;
     }
   else if (ie->load_opts.dpi > 0.0)
     {
	w = (w * ie->load_opts.dpi) / 75.0;
	h = (h * ie->load_opts.dpi) / 75.0;
     }
   else if ((ie->load_opts.w > 0) &&
	    (ie->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = ie->load_opts.w;
	h2 = (ie->load_opts.w * h) / w;
	if (h2 > ie->load_opts.h)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ie->flags.alpha = 1;
   evas_cache_image_surface_alloc(ie, w, h);
   pixels = evas_cache_image_pixels(ie);
   if (!pixels)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto error;
     }

   memset(pixels, 0, w * h * sizeof(DATA32));
   surface = cairo_image_surface_create_for_data((unsigned char *)pixels, CAIRO_FORMAT_ARGB32,
						 w, h, w * sizeof(DATA32));
   if (!surface)
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto error;
     }
   cr = cairo_create(surface);
   if (!cr)
     {
	cairo_surface_destroy(surface);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto error;
     }

   cairo_scale(cr,
	       (double)ie->w / dim.em,
	       (double)ie->h / dim.ex);
   rsvg_handle_render_cairo(rsvg, cr);
   cairo_surface_destroy(surface);
   /* need to check if this is required... */
   cairo_destroy(cr);
   rsvg_handle_close(rsvg, NULL);
   g_object_unref(rsvg);
   evas_common_image_set_alpha_sparse(ie);
   return EINA_TRUE;

 error:
   rsvg_handle_close(rsvg, NULL);
   g_object_unref(rsvg);
   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_svg_func);
   if (!rsvg_initialized) rsvg_init();
   rsvg_initialized = 1;
   return 1;
}

static void
module_close(Evas_Module *em)
{
   if (!rsvg_initialized) return;
   //rsvg_term();
   //rsvg_initialized = 0;
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "svg",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, svg);

#ifndef EVAS_STATIC_BUILD_SVG
EVAS_EINA_MODULE_DEFINE(image_loader, svg);
#endif
