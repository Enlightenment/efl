/* EINA - EFL data type library
 * Copyright (C) 2008 Jorge Luis Zapata Muga, Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_H_
#define EINA_H_

/**
 * @mainpage Eina
 * @author Jorge Luis Zapata Muga
 * @date 2008
 *
 * @section eina_toc_sec Table of contents
 *
 * <ul>
 *   <li> @ref eina_intro_sec
 *   <li> @ref eina_data_types_sec
 *   <ul>
 *     <li> @ref eina_container_subsec
 *     <ul>
 *       <li> @ref eina_array_subsubsec
 *       <li> @ref eina_hash_subsubsec
 *       <li> @ref eina_inlist_subsubsec
 *       <li> @ref eina_rbtree_subsubsec
 *       <li> @ref eina_list_subsubsec
 *     </ul>
 *     <li> @ref eina_stringshare_subsec
 *   </ul>
 *   <li> @ref eina_access_contents_sec
 *   <ul>
 *     <li> @ref eina_iterators_subsec
 *     <li> @ref eina_accessors_subsec
 *   </ul>
 *   <li> @ref eina_tools_sec
 *   <ul>
 *     <li> @ref eina_convert_subsec
 *     <li> @ref eina_counter_subsec
 *     <li> @ref eina_error_subsec
 *   </ul>
 * </ul>
 *
 * @section eina_intro_sec Introduction
 *
 * The Eina library is a library that implemente an API for data types
 * in an efficient way. It also provides some useful tools like
 * openin shared libraries, errors management, type conversion,
 * time accounting and memory pool.
 *
 * This library is cross-platform and can be compiled and used on
 * Linux and Windows (XP and CE).
 *
 * The data types that are available are
 * @li array
 * @li hash table
 * @li inlined list
 * @li inlined red black tree
 * @li list
 * @li shared string
 *
 * This document describes each data type by providing the algorithms
 * used in the implementation, the performance, a short tutorial and
 * the API.
 *
 * @section eina_data_types_sec Eina Data Types
 *
 * @subsection eina_container_subsec Containers
 *
 * Containers are data types that hold data and allow iteration over
 * their elements with an @ref eina_iterators_subsec, or eventually an
 * @ref eina_accessors_subsec. The only data type that is not a container (in
 * that sense) is the @ref eina_stringshare_subsec.
 *
 * @subsubsection eina_array_subsubsec Array
 *
 * @subsubsection eina_hash_subsubsec Hash Table
 *
 * @subsubsection eina_inlist_subsubsec Inlined List
 *
 * @subsubsection eina_rbtree_subsubsec Inlined Red Black Tree
 *
 * @subsubsection eina_list_subsubsec List
 *
 * @subsection eina_stringshare_subsec Shared String
 *
 * @section eina_access_contents_sec Accessing Data Struct Contents
 *
 * For the container data types, you can access to the elements
 * sequentially with iterators, or randomly with accessors. They are
 * created from the data types themselves and allow a generic way to
 * traverse these data types.
 *
 * @subsection eina_iterators_subsec Iterator
 *
 * Iterators allow a sequential access of the data of a
 * container. They can only access the next element. To look at the
 * API, go to @ref Eina_Iterator_Group.
 *
 * @subsection eina_accessors_subsec Accessor
 *
 * Accessors allow a random access of the data of a container. They
 * can access an element at any position. To look at the API, go to
 * @ref Eina_Accessor_Group.
 *
 * @section eina_tools_sec Eina Tools
 *
 * @subsection eina_convert_subsec Convert Functions
 *
 * @subsection eina_counter_subsec Timing Functions
 *
 * @subsection eina_error_subsec Error Functions
 *
 * Take a look at the API of @ref Eina_Error_Group.
 *
 * Take a look at the @ref tutorial_error_page.
 *
 * @todo add debug function
 * @todo add magic function
 * @todo add other todo items :)
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "eina_config.h"
#include "eina_types.h"
#include "eina_main.h"
#include "eina_f16p16.h"
#include "eina_rectangle.h"
#include "eina_inlist.h"
#include "eina_file.h"
#include "eina_list.h"
#include "eina_hash.h"
#include "eina_lalloc.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_error.h"
#include "eina_array.h"
#include "eina_stringshare.h"
#include "eina_magic.h"
#include "eina_counter.h"
#include "eina_rbtree.h"
#include "eina_accessor.h"
#include "eina_iterator.h"
#include "eina_benchmark.h"
#include "eina_convert.h"
#include "eina_cpu.h"
#include "eina_tiler.h"

#ifdef __cplusplus
}
#endif

#endif /* EINA_H */
