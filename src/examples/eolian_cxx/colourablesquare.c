#include <stdio.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

#include "colourable.eo.h"
#include "colourablesquare.eo.h"

#define MY_CLASS COLOURABLESQUARE_CLASS

static int _colourablesquare_impl_logdomain;

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_colourablesquare_impl_logdomain, __VA_ARGS__)

#ifdef INFO
#undef INFO
#endif
#define INFO(...) EINA_LOG_DOM_INFO(_colourablesquare_impl_logdomain, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_colourablesquare_impl_logdomain, __VA_ARGS__)

struct _ColourableSquare_Data
{
   int size;
};

typedef struct _ColourableSquare_Data  ColourableSquare_Data;

#define COLOURABLESQUARE_DATA_GET(o, wd)                        \
  ColourableSquare_Data *wd = eo_data_scope_get(o, MY_CLASS)

static void
_colourablesquare_size_constructor(Eo *obj, ColourableSquare_Data *self, int size)
{
   if(!_colourablesquare_impl_logdomain)
     {
        _colourablesquare_impl_logdomain
          = eina_log_domain_register
          ("colourablesquare", EINA_COLOR_LIGHTBLUE);
     }
   self->size = size;
   DBG("_colourablesquare_constructor(%d)\n", size);
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

static void
_colourablesquare_destructor(Eo *obj, ColourableSquare_Data *self)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
   if(_colourablesquare_impl_logdomain)
     {
        eina_log_domain_unregister(_colourablesquare_impl_logdomain);
        _colourablesquare_impl_logdomain = 0;
     }
}

static int
_colourablesquare_size_get(Eo *obj, ColourableSquare_Data *self)
{
   DBG("_colourablesquare_size_get() => %d\n", self->size);
   return self->size;
}

static void
_colourablesquare_size_print(Eo *obj, ColourableSquare_Data *self)
{
   DBG("_colourablesquare_size_print() ==> %d\n", self->size);
}

static void
_colourablesquare_size_set(Eo *obj, ColourableSquare_Data *self, int size)
{
   DBG("_colourablesquare_size_set(%d)\n", size);
}

static void
_user_colourablesquare_class_constructor(Eo_Class *klass)
{
   DBG("_colourablesquare_class_constructor()\n");
}

#include "colourablesquare.eo.c"
