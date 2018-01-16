#ifndef EFL_CXX_ELEMENTARY_HH
#define EFL_CXX_ELEMENTARY_HH
#if defined(ELEMENTARY_H) || defined(ELM_WIDGET_H)
#error "Do not include Elm C API headers before including Elementary.hh"
#endif

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif
#ifndef ELM_INTERNAL_API_ARGESFSDFEFC
#define ELM_INTERNAL_API_ARGESFSDFEFC
#endif

#include <Efl.hh>

extern "C" {
#include "Elementary.h"
}

#include <Edje.hh>
#include "Elementary.eo.hh"

#endif
