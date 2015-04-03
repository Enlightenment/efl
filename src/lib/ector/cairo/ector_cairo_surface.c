#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

static unsigned int _cairo_count = 0;
static Eina_Module *_cairo_so = NULL;

static void *
_ector_cairo_surface_symbol_get(Eo *obj EINA_UNUSED,
                                Ector_Cairo_Surface_Data *pd EINA_UNUSED,
                                const char *name)
{
   if (!_cairo_so)
     {
#define LOAD(x)                                 \
        if (!_cairo_so)                         \
          {                                     \
             _cairo_so = eina_module_new(x);    \
             if (_cairo_so &&                   \
                 !eina_module_load(_cairo_so))  \
               {                                \
                  eina_module_free(_cairo_so);  \
                  _cairo_so = NULL;             \
               }                                \
          }
#if defined(_WIN32) || defined(__CYGWIN__)
        LOAD("libcairo.dll");
#elif defined(__APPLE__) && defined(__MACH__)
        LOAD("libcairo.dylib");
        LOAD("libcairo.so");
#else
        LOAD("libcairo.so");
#endif

#undef LOAD
     }

   return eina_module_symbol_get(_cairo_so, name);
}


static Ector_Renderer *
_ector_cairo_surface_ector_generic_surface_renderer_factory_new(Eo *obj,
                                                                Ector_Cairo_Surface_Data *pd EINA_UNUSED,
                                                                const Eo_Class *type)
{
   if (eo_isa(type, ECTOR_RENDERER_CAIRO_SHAPE_CLASS))
     return eo_add(ECTOR_RENDERER_CAIRO_SHAPE_CLASS, obj);
   else if (eo_isa(type, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS))
     return eo_add(ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, obj);
   else if (eo_isa(type, ECTOR_RENDERER_CAIRO_GRADIENT_RADIAL_CLASS))
     return eo_add(ECTOR_RENDERER_CAIRO_GRADIENT_RADIAL_CLASS, obj);
   return NULL;
}

static void
_ector_cairo_surface_context_set(Eo *obj EINA_UNUSED,
                                 Ector_Cairo_Surface_Data *pd,
                                 cairo_t *ctx)
{
   pd->cairo = ctx;
}

static cairo_t *
_ector_cairo_surface_context_get(Eo *obj EINA_UNUSED,
                                 Ector_Cairo_Surface_Data *pd)
{
   return pd->cairo;
}

static void
_ector_cairo_surface_eo_base_constructor(Eo *obj EINA_UNUSED,
                                         Ector_Cairo_Surface_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ECTOR_CAIRO_SURFACE_CLASS, eo_constructor());
   _cairo_count++;
}

static void
_ector_cairo_surface_eo_base_destructor(Eo *obj EINA_UNUSED,
                                        Ector_Cairo_Surface_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ECTOR_CAIRO_SURFACE_CLASS, eo_destructor());

   if (--_cairo_count) return ;
   if (_cairo_so) eina_module_free(_cairo_so);
   _cairo_so = NULL;
}

#include "ector_cairo_surface.eo.c"
#include "ector_renderer_cairo_base.eo.c"
