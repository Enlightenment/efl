
#include <iostream>
#include <iomanip>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.hh"
#include "Eina.hh"

extern "C"
{
#include "colourablesquare_stub.h"
#include "colourable.eo.h"
#include "colourablesquare.eo.h"
}

#define MY_CLASS COLOURABLESQUARE_CLASS

static efl::eina::log_domain domain("colourablesquare");

void
_colourablesquare_size_constructor(Eo *obj, ColourableSquare_Data *self, int size)
{
   self->size = size;
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << size << " ]" << std::endl;
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

int
_colourablesquare_size_get(Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << self->size << " ]" << std::endl;
   return self->size;
}

void
_colourablesquare_size_print(Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << self->size << " ]" << std::endl;
}

void
_colourablesquare_size_set(Eo *obj EINA_UNUSED, ColourableSquare_Data *self EINA_UNUSED, int size)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << self->size << " ]" << std::endl;
}

