#ifdef HAVE_CONFIG_H
#include "config.h"  /* so that EAPI in Evas.h is correctly defined */
#endif

#include "Ector_Cairo.h"
#include "ector_private.h"
#include <software/Ector_Software.h>
#include "ector_cairo_software_surface.eo.h"

#define MY_CLASS ECTOR_CAIRO_SOFTWARE_SURFACE_CLASS

#define USE(Obj, Sym, Error)                            \
  if (!Sym) Sym = _ector_cairo_symbol_get(Obj, #Sym);   \
  if (!Sym) return Error;

static inline void *
_ector_cairo_symbol_get(Eo *ector_surface, const char *name)
{
   void *sym;

   sym = ector_cairo_surface_symbol_get(ector_surface, name);
   return sym;
}

typedef struct _cairo_surface_t cairo_surface_t;
typedef enum {
  CAIRO_FORMAT_INVALID   = -1,
  CAIRO_FORMAT_ARGB32    = 0,
  CAIRO_FORMAT_RGB24     = 1,
  CAIRO_FORMAT_A8        = 2,
  CAIRO_FORMAT_A1        = 3,
  CAIRO_FORMAT_RGB16_565 = 4,
  CAIRO_FORMAT_RGB30     = 5
} cairo_format_t;

static cairo_surface_t *(*cairo_image_surface_create_for_data)(unsigned char *data,
                                                               cairo_format_t format,
                                                               int width,
                                                               int height,
                                                               int stride) = NULL;
static void (*cairo_surface_destroy)(cairo_surface_t *surface) = NULL;
static cairo_t *(*cairo_create)(cairo_surface_t *target) = NULL;
static void (*cairo_destroy)(cairo_t *cr) = NULL;

typedef struct _Ector_Cairo_Software_Surface_Data Ector_Cairo_Software_Surface_Data;
struct _Ector_Cairo_Software_Surface_Data
{
   cairo_surface_t *surface;
   Ector_Software_Buffer_Base_Data *base;
};

EOLIAN static Eina_Bool
_ector_cairo_software_surface_ector_buffer_pixels_set(Eo *obj, Ector_Cairo_Software_Surface_Data *pd,
                                                      void *pixels, int width, int height, int stride,
                                                      Efl_Gfx_Colorspace cspace, Eina_Bool writable)
{
   cairo_t *ctx = NULL;
   Eina_Bool ok = EINA_FALSE;

   if ((cspace != EFL_GFX_COLORSPACE_ARGB8888) || !writable)
     {
        ERR("Unsupported surface type!");
        return EINA_FALSE;
     }

   USE(obj, cairo_image_surface_create_for_data, EINA_FALSE);
   USE(obj, cairo_surface_destroy, EINA_FALSE);
   USE(obj, cairo_create, EINA_FALSE);
   USE(obj, cairo_destroy, EINA_FALSE);

   if (pd->surface)
     cairo_surface_destroy(pd->surface);
   pd->surface = NULL;

   ok = ector_buffer_pixels_set(efl_super(obj, MY_CLASS), pixels, width, height, stride, cspace, writable);

   if (ok && pixels)
     {
        pd->surface = cairo_image_surface_create_for_data(pixels,
                                                          CAIRO_FORMAT_ARGB32,
                                                          width, height, pd->base->stride);
        if (!pd->surface) goto end;

        ctx = cairo_create(pd->surface);
     }

 end:
   /* evas_common_cpu_end_opt(); // do we need this? */
   ector_cairo_surface_context_set(obj, ctx);
   return ok;
}

void
_ector_cairo_software_surface_surface_get(Eo *obj EINA_UNUSED, Ector_Cairo_Software_Surface_Data *pd, void **pixels, unsigned int *width, unsigned int *height)
{
   if (pixels) *pixels = pd->base->pixels.u8;
   if (width) *width = pd->base->generic->w;
   if (height) *height = pd->base->generic->h;
}

static Efl_Object *
_ector_cairo_software_surface_efl_object_constructor(Eo *obj, Ector_Cairo_Software_Surface_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (!obj) return NULL;
   pd->base = efl_data_ref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   pd->base->generic = efl_data_ref(obj, ECTOR_BUFFER_MIXIN);
   pd->base->generic->eo = obj;
   return obj;
}

EOLIAN static void
_ector_cairo_software_surface_efl_object_destructor(Eo *obj, Ector_Cairo_Software_Surface_Data *pd)
{
   efl_data_unref(obj, pd->base);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "ector_cairo_software_surface.eo.c"
