#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Esvg.h>

#include "evas_common.h"
#include "evas_private.h"

static inline Eina_Bool evas_image_load_file_is_svg(const char *file) EINA_ARG_NONNULL(1) EINA_PURE;
static Eina_Bool evas_image_load_file_head_svg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_svg(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

Evas_Image_Load_Func evas_image_load_svg_func =
{
  EINA_FALSE,
  evas_image_load_file_head_svg,
  evas_image_load_file_data_svg,
  NULL,
  EINA_FALSE
};

static Eina_Bool esvg_initialized = EINA_FALSE;


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
   Ender_Element *e;
   int            w, h;
   double         sw, sh;

   /* ignore all files not called .svg or .svg.gz - because rsvg has a leak
    * where closing the handle doesn't free mem */
   if (!evas_image_load_file_is_svg(file))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   e = esvg_parser_load(file, NULL, NULL);
   if (!e)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   esvg_renderable_x_dpi_set(e, 92.0);
   esvg_renderable_y_dpi_set(e, 92.0);
   esvg_svg_actual_width_get(e, &sw);
   esvg_svg_actual_height_get(e, &sh);
   w = (int)ceil(sw);
   h = (int)ceil(sh);
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
        ender_element_unref(e);
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
        unsigned int w2, h2;

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

   ender_element_unref(e);

   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

/** FIXME: All evas loaders need to be tightened up **/
static Eina_Bool
evas_image_load_file_data_svg(Image_Entry *ie, const char *file, const char *key __UNUSED__, int *error)
{
   DATA32          *pixels;
   Ender_Element   *e;
   Enesim_Error    *err = NULL;
   Enesim_Surface  *s;
   void            *data;
   size_t           stride;
   int              w, h;
   double           sw, sh;

   if (!evas_image_load_file_is_svg(file))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        return EINA_FALSE;
     }

   e = esvg_parser_load(file, NULL, NULL);
   if (!e)
     {
        *error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
        return EINA_FALSE;
     }

   esvg_renderable_x_dpi_set(e, 75.0);
   esvg_renderable_y_dpi_set(e, 75.0);
   esvg_svg_actual_width_get(e, &sw);
   esvg_svg_actual_height_get(e, &sh);
   w = (int)ceil(sw);
   h = (int)ceil(sh);
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE))
     {
        ender_element_unref(e);
        if (IMG_TOO_BIG(w, h))
          *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        else
          *error = EVAS_LOAD_ERROR_GENERIC;
        goto unref_renderer;
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
        unsigned int w2, h2;

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
   if ((w != (int)ie->w) || (h != (int)ie->h))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto unref_renderer;
     }
   ie->flags.alpha = 1;
   evas_cache_image_surface_alloc(ie, w, h);
   pixels = evas_cache_image_pixels(ie);
   if (!pixels)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto unref_renderer;
     }

   memset(pixels, 0, w * h * sizeof(DATA32));

   s = enesim_surface_new(ENESIM_FORMAT_ARGB8888, w, h);
   if (!s)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto unref_renderer;
     }

   esvg_element_setup(e, NULL);

   if (!esvg_renderable_draw(e, s, NULL, 0, 0, &err))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        enesim_error_dump(err);
        goto unref_surface;
     }

   if (!enesim_surface_data_get(s, &data, &stride))
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        goto unref_surface;
     }

/*    printf("test : %d %d\n", w * h * sizeof(int), h * stride); */
/*    if ((w * h * sizeof(int)) != (h * stride)) */
/*      { */
/*         *error = EVAS_LOAD_ERROR_GENERIC; */
/*         goto unref_surface; */
/*      } */

   /* FIXME: scale to (double)ie->w / dim.em, (double)ie->h / dim.ex */

   memcpy (pixels, data, h * stride);

   enesim_surface_unref(s);
   ender_element_unref(e);

   evas_common_image_set_alpha_sparse(ie);

   return EINA_TRUE;

 unref_surface:
   enesim_surface_unref(s);
 unref_renderer:
   ender_element_unref(e);

   return EINA_FALSE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_svg_func);
   if (!esvg_initialized) esvg_init();
   esvg_initialized = EINA_TRUE;
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
   if (!esvg_initialized) return;
   esvg_shutdown();
   esvg_initialized = EINA_FALSE;
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
