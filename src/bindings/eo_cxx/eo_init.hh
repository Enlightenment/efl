
///
/// @file eo_init.hh
///

#ifndef EFL_CXX_EO_INIT_HH
#define EFL_CXX_EO_INIT_HH

#include "eo_ops.hh"

namespace efl { namespace eo {

/// @addtogroup Efl_Cxx_API
/// @{

/// @brief A simple RAII implementation to initialize and terminate
/// the <em>EO Subsystem</em>.
///
struct eo_init
{
   /// @brief Default constructor.
   ///
   /// Invokes @c ::eo_init().
   ///
   eo_init() { detail::init(); }

   /// @brief Class destructor.
   ///
   /// Invokes @c ::eo_shutdown().
   ///
   ~eo_init() { detail::shutdown(); }
};

/// @}

} }

#endif // EFL_CXX_EO_INIT_HH
