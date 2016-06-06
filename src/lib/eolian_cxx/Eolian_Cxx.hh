
#ifndef EOLIAN_CXX_LIB_HH
#define EOLIAN_CXX_LIB_HH

extern "C"
{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <Eolian.h>
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

#endif // EOLIAN_CXX_LIB_HH
