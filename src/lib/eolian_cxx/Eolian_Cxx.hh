
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

struct eolian_state
{
   Eolian_State *value;
   eolian_state()
   {
       value = ::eolian_state_new();
   }
   ~eolian_state()
   {
     ::eolian_state_free(value);
   }

   inline Eolian_Unit const* as_unit() const
   {
       return (Eolian_Unit const*)value;
   }
};

} }

#endif // EOLIAN_CXX_LIB_HH
