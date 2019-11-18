/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
#include <eina_array.hh>
#include <eina_list.hh>
#include <eina_stringshare.hh>
#include <eina_string_view.hh>
#include <eina_strbuf.hh>
#include <eina_error.hh>
#include <eina_accessor.hh>
#include <eina_thread.hh>
#include <eina_value.hh>
#include <eina_ref.hh>
#include <eina_log.hh>
#include <eina_optional.hh>
#include <eina_integer_sequence.hh>
#include <eina_pp.hh>
#include <eina_workarounds.hh>
#include <eina_future.hh>
#include <eina_deleter.hh>
#include <eina_copy_traits.hh>
#include <eina_variant.hh>

/**
 * @page eina_cxx_main Eina C++ (BETA)
 *
 * @date 2014 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eina_cxx_main_intro
 * @li @ref eina_cxx_main_compiling
 * @li @ref eina_cxx_main_next_steps
 *
 * @section eina_cxx_main_intro Introduction
 *
 * Eina C++ bindings

 * @section eina_cxx_main_compiling How to compile
 *
 * Eina CXX is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eina-cxx`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eina-cxx`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eina_cxx_main_next_steps Next Steps
 *
 * After you understood what Eina CXX is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Eina_Cxx
 *
 * @addtogroup Eina_Cxx
 * @{
 */



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

/**
 * @}
 */

#endif
