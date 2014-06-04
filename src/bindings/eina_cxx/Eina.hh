#ifndef EINA_HH_
#define EINA_HH_

/**
 * @file
 * @brief Eina C++
 */

#include <eina_iterator.hh>
#include <eina_ptrarray.hh>
#include <eina_ptrlist.hh>
#include <eina_inarray.hh>
#include <eina_inlist.hh>
#include <eina_stringshare.hh>
#include <eina_error.hh>
#include <eina_accessor.hh>
#include <eina_thread.hh>
#include <eina_value.hh>
#include <eina_ref.hh>
#include <eina_log.hh>
#include <eina_optional.hh>
#include <eina_integer_sequence.hh>

/**
 * @defgroup Eina_Cxx Eina C++
 *
 * @defgroup Eina_Cxx_Data_Types_Group Data Types
 * @ingroup Eina_Cxx
 *
 * @defgroup Eina_Cxx_Content_Access_Group Content Access
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * @defgroup Eina_Cxx_Containers_Group Containers
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * @defgroup Eina_Cxx_Tools_Group Tools
 * @ingroup Eina_Cxx
 *
 */

namespace efl { namespace eina {

/**
 * @addtogroup Eina_Cxx
 *
 * @{
 */

/**
 * @brief Initialize the Eina library.
 *
 * Initialize all the Eina modules upon construction and finalize them
 * upon destruction, using the RAII programming idiom.
 */
struct eina_init
{
  eina_init()
  {
    ::eina_init();
  }
  ~eina_init()
  {
    ::eina_shutdown();
  }
};

/**
 * @brief Initialize the mutexes of the Eina library.
 *
 * Set up all the mutexes in all Eina modules upon construction and
 * shut them down upon destruction, using the RAII programming idiom.
 */
struct eina_threads_init
{
  eina_threads_init()
  {
    ::eina_threads_init();
  }
  ~eina_threads_init()
  {
    ::eina_threads_shutdown();
  }
};

/**
 * @}
 */

} }

#endif
