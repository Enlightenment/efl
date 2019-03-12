#ifndef EVAS_HH
#define EVAS_HH

#if defined(_EVAS_H)
#error "Do not include Evas C API headers before including Evas.hh"
#endif

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Efl.hh>

extern "C" {
#include "Evas.h"
}

#include "Evas.eo.hh"

#endif
