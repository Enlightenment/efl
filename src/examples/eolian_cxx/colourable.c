#include <stdio.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "Eina.h"
#include "colourable.eo.h"

#define MY_CLASS COLOURABLE_CLASS

static int _colourable_impl_logdomain;

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_colourable_impl_logdomain, __VA_ARGS__)

#ifdef INFO
#undef INFO
#endif
#define INFO(...) EINA_LOG_DOM_INFO(_colourable_impl_logdomain, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_colourable_impl_logdomain, __VA_ARGS__)

struct _Colourable_Data
{
   int r;
   int g;
   int b;
};

typedef struct _Colourable_Data  Colourable_Data;

#define COLOURABLE_DATA_GET(o, wd)                             \
  Colourable_Data *wd = eo_data_scope_get(o, MY_CLASS)

void
_colourable_constructor(Eo *obj, Colourable_Data *self EINA_UNUSED)
{
   if(!_colourable_impl_logdomain)
     {
        _colourable_impl_logdomain
          = eina_log_domain_register("colourable", EINA_COLOR_BLUE);
     }
   DBG("_colourable_constructor(%p, %p)\n", obj, MY_CLASS);
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

void
_colourable_destructor(Eo *obj, Colourable_Data *self EINA_UNUSED)
{
   if(_colourable_impl_logdomain)
     {
        eina_log_domain_unregister(_colourable_impl_logdomain);
        _colourable_impl_logdomain = 0;
     }
   DBG("_colourable_destructor()\n");
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

void
_colourable_rgb_composite_constructor(Eo *obj, Colourable_Data *self, int r, int g, int b)
{
   if(!_colourable_impl_logdomain)
     {
        _colourable_impl_logdomain
          = eina_log_domain_register("colourable", EINA_COLOR_BLUE);
     }
   self->r = r;
   self->g = g;
   self->b = b;
   DBG("_colourable_rgb_composite_constructor(0x%2.x, %0x2.x, %0x2.x)\n",
       (unsigned int)self->r,
       (unsigned int)self->g,
       (unsigned int)self->b);
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

void
_colourable_rgb_24bits_constructor(Eo *obj, Colourable_Data *self, int rgb)
{
   if(!_colourable_impl_logdomain)
     {
        _colourable_impl_logdomain
          = eina_log_domain_register("colourable", EINA_COLOR_BLUE);
     }

   self->r = (rgb & 0x00ff0000) >> 16;
   self->g = (rgb & 0x0000ff00) >> 8;
   self->b = rgb & 0x000000ff;
   DBG("_colourable_rgb_24bits_constructor(0x%.6x)\n", (int)rgb);
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

void
_colourable_print_colour(Eo *obj EINA_UNUSED, Colourable_Data *self EINA_UNUSED)
{
   DBG("_colourable_print_colour() ==> 0x%2.x 0x%2.x 0x%2.x\n", self->r, self->g, self->b);
}

int
_colourable_colour_mask(Eo *obj EINA_UNUSED, Colourable_Data *self, int mask)
{
   int masked_rgb =
     (((self->r << 16)& 0x00ff0000) |
      ((self->g << 8) & 0x0000ff00) |
      (self->b & 0x000000ff)) & mask;
   DBG("_colourable_colour_mask() ==> 0x%2.x\n", (unsigned int)masked_rgb);
   return masked_rgb;
}

void
_colourable_composite_colour_get(Eo *obj EINA_UNUSED, Colourable_Data *self, int* r, int* g, int* b)
{
   *r = self->r;
   *g = self->g;
   *b = self->b;
   DBG("_colourable_composite_colour_get() ==> 0x%2.x 0x%2.x 0x%2.x\n", *r, *g, *b);
   return;
}

void
_colourable_composite_colour_set(Eo *obj EINA_UNUSED, Colourable_Data *self, int r, int g, int b)
{
   self->r = r;
   self->g = g;
   self->b = b;
   DBG("_colourable_composite_colour_set(0x%2.x, 0x%2.x, 0x%2.x)\n",
       (int)self->r, (int)self->g, (int)self->b);
   return;
}

int
_colourable_colour_get(Eo *obj EINA_UNUSED, Colourable_Data *self)
{
   int rgb =
     ((self->r << 16)& 0x00ff0000) |
     ((self->g << 8) & 0x0000ff00) |
     (self->b & 0x000000ff);
   DBG("_colourable_colour_get() ==> 0x%.6x\n", (unsigned int)rgb);
   return rgb;
}

void
_colourable_colour_set(Eo *obj EINA_UNUSED, Colourable_Data *self, int rgb)
{
   self->r = (rgb & 0x00ff0000) >> 16;
   self->g = (rgb & 0x0000ff00) >> 8;
   self->b = rgb & 0x000000ff;
   DBG("_colourable_colour_set(0x%.6x)\n", (unsigned int)rgb);
   return;
}

#include "colourable.eo.c"
