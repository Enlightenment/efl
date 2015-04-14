#ifdef HAVE_CONFIG_H
#include "config.h"  /* so that EAPI in Evas.h is correctly defined */
#endif

#include <Ector.h>

#include "cairo/Ector_Cairo.h"
#include "software/Ector_Software.h"

#include "ector_cairo_software_surface.eo.h"

#define USE(Obj, Sym, Error)                            \
  if (!Sym) Sym = _ector_cairo_symbol_get(Obj, #Sym);   \
  if (!Sym) return Error;

static inline void *
_ector_cairo_symbol_get(Eo *ector_surface, const char *name)
{
   void *sym;

   eo_do(ector_surface,
         sym = ector_cairo_surface_symbol_get(name));
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
   cairo_t *ctx;

   void *pixels;

   unsigned int width;
   unsigned int height;
};

static void
_ector_cairo_software_surface_surface_set(Eo *obj, Ector_Cairo_Software_Surface_Data *pd, void *pixels, unsigned int width, unsigned int height)
{
   USE(obj, cairo_image_surface_create_for_data, );
   USE(obj, cairo_surface_destroy, );
   USE(obj, cairo_create, );
   USE(obj, cairo_destroy, );

   if (pd->surface) cairo_surface_destroy(pd->surface); pd->surface = NULL;
   if (pd->ctx) cairo_destroy(pd->ctx); pd->ctx = NULL;

   pd->pixels = NULL;
   pd->width = 0;
   pd->height = 0;

   if (pixels)
     {
        pd->surface = cairo_image_surface_create_for_data(pixels,
                                                          CAIRO_FORMAT_ARGB32,
                                                          width, height, width);
        if (!pd->surface) goto end;

        pd->ctx = cairo_create(pd->surface);
        if (!pd->ctx) goto end;
     }
   pd->pixels = pixels;
   pd->width = width;
   pd->height = height;

 end:
   eo_do(obj,
         ector_cairo_surface_context_set(pd->ctx),
         ector_surface_size_set(pd->width, pd->height));
}

static void
_ector_cairo_software_surface_surface_get(Eo *obj EINA_UNUSED, Ector_Cairo_Software_Surface_Data *pd, void **pixels, unsigned int *width, unsigned int *height)
{
   if (pixels) *pixels = pd->pixels;
   if (width) *width = pd->width;
   if (height) *height = pd->height;
}

#include "ector_cairo_software_surface.eo.c"
