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
        LOAD("libcairo-2.dll");
        LOAD("libcairo.dll");
#elif defined(__APPLE__) && defined(__MACH__)
        LOAD("libcairo.dylib");
        LOAD("libcairo.so");
#else
        LOAD("libcairo.so");
#endif

#undef LOAD
     }

   if (!_cairo_so)
     {
        ERR("Couldn't find cairo library. Please make sure that your system can locate it.");
        return NULL;
     }

   return eina_module_symbol_get(_cairo_so, name);
}

#undef USE
#define USE(Obj, Sym, Error)                                            \
  if (!Sym) Sym = _ector_cairo_surface_symbol_get(Obj, NULL, #Sym);     \
  if (!Sym) return Error;

static Ector_Renderer *
_ector_cairo_surface_ector_surface_renderer_factory_new(Eo *obj,
                                                                Ector_Cairo_Surface_Data *pd EINA_UNUSED,
                                                                const Efl_Class *type)
{
   if (type == ECTOR_RENDERER_SHAPE_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_CAIRO_SHAPE_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN)
     return efl_add_ref(ECTOR_RENDERER_CAIRO_GRADIENT_RADIAL_CLASS, NULL, ector_renderer_surface_set(efl_added, obj));

   ERR("Couldn't find class for type: %s\n", efl_class_name_get(type));
   return NULL;
}

typedef struct _cairo_surface_t cairo_surface_t;

static void (*cairo_destroy)(cairo_t *cr) = NULL;
static cairo_surface_t *(*cairo_image_surface_create)(int format,
                                                      int width,
                                                      int height) = NULL;
static cairo_t *(*cairo_create)(cairo_surface_t *target) = NULL;

static cairo_surface_t *internal = NULL;

static void
_ector_cairo_surface_context_set(Eo *obj EINA_UNUSED,
                                 Ector_Cairo_Surface_Data *pd,
                                 cairo_t *ctx)
{
   if (pd->cairo) cairo_destroy(pd->cairo);
   if (!ctx)
     {
        if (!internal) internal = cairo_image_surface_create(0, 1, 1);
        ctx = cairo_create(internal);
     }
   pd->current.x = pd->current.y = 0;
   pd->cairo = ctx;
}

static cairo_t *
_ector_cairo_surface_context_get(const Eo *obj EINA_UNUSED,
                                 Ector_Cairo_Surface_Data *pd)
{
   return pd->cairo;
}

static void
_ector_cairo_surface_ector_surface_reference_point_set(Eo *obj EINA_UNUSED,
                                                               Ector_Cairo_Surface_Data *pd,
                                                               int x, int y)
{
   pd->current.x = x;
   pd->current.y = y;
}

static Eo *
_ector_cairo_surface_efl_object_constructor(Eo *obj,
                                         Ector_Cairo_Surface_Data *pd)
{
   USE(obj, cairo_destroy, NULL);
   USE(obj, cairo_image_surface_create, NULL);
   USE(obj, cairo_create, NULL);

   obj = efl_constructor(efl_super(obj, ECTOR_CAIRO_SURFACE_CLASS));
   if (!obj) return NULL;

   _cairo_count++;

   _ector_cairo_surface_context_set(obj, pd, NULL);

   return obj;
}

static void
_ector_cairo_surface_efl_object_destructor(Eo *obj EINA_UNUSED,
                                        Ector_Cairo_Surface_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, ECTOR_CAIRO_SURFACE_CLASS));

   if (--_cairo_count) return ;
   if (_cairo_so) eina_module_free(_cairo_so);
   _cairo_so = NULL;
}

#include "ector_cairo_surface.eo.c"
