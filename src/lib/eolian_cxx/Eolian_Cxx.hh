
#ifndef EOLIAN_CXX_LIB_HH
#define EOLIAN_CXX_LIB_HH

extern "C"
{
#include <Eolian.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
}

namespace efl { namespace eolian {

struct eolian_init
{
   eolian_init()
   {
      ::eolian_init();
   }
   ~eolian_init()
   {
      ::eolian_shutdown();
   }
};

} }

#ifdef EFL_BETA_API_SUPPORT

#include "eo_types.hh"
#include "eo_validate.hh"
#include "eo_generate.hh"

#endif

#endif // EOLIAN_CXX_LIB_HH
