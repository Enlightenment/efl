#include <stdio.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

#include "ns_colourable.eo.h"
#include "ns_colourablesquare.eo.h"

#define MY_CLASS NS_COLOURABLESQUARE_CLASS

static int _ns_colourablesquare_impl_logdomain;

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ns_colourablesquare_impl_logdomain, __VA_ARGS__)

struct _ColourableSquare_Data
{
   int size;
};
typedef struct _ColourableSquare_Data  ColourableSquare_Data;

static void
_ns_colourablesquare_size_constructor(Eo *obj EINA_UNUSED, ColourableSquare_Data *self, int size)
{
   if(!_ns_colourablesquare_impl_logdomain)
     {
        _ns_colourablesquare_impl_logdomain
          = eina_log_domain_register
          ("colourablesquare", EINA_COLOR_LIGHTBLUE);
     }
   self->size = size;
   DBG("_ns_colourablesquare_constructor(%d)\n", size);
}

static int
_ns_colourablesquare_size_get(const Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   DBG("_ns_colourablesquare_size_get() => %d\n", self->size);
   return self->size;
}

static void
_ns_colourablesquare_size_print(Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   DBG("_ns_colourablesquare_size_print() ==> %d\n", self->size);
}

static void
_ns_colourablesquare_size_set(Eo *obj EINA_UNUSED, ColourableSquare_Data *self EINA_UNUSED, int size)
{
   DBG("_ns_colourablesquare_size_set(%d)\n", size);
}

#include "ns_colourablesquare.eo.c"
