/* EINA - EFL data type library
 * Copyright (C) 2008-2013 Enlightenment Developers:
 *           Albin "Lutin" Tonnerre <albin.tonnerre@gmail.com>
 *           Alexandre "diaxen" Becoulet <diaxen@free.fr>
 *           Andre Dieb <andre.dieb@gmail.com>
 *           Arnaud de Turckheim "quarium" <quarium@gmail.com>
 *           Carsten Haitzler <raster@rasterman.com>
 *           Cedric Bail <cedric.bail@free.fr>
 *           Corey "atmos" Donohoe <atmos@atmos.org>
 *           Fabiano Fidêncio <fidencio@profusion.mobi>
 *           Gustavo Chaves <glima@profusion.mobi>
 *           Gustavo Sverzut Barbieri <barbieri@gmail.com>
 *           Jorge Luis "turran" Zapata <jorgeluis.zapata@gmail.com>
 *           Peter "pfritz" Wehrfritz <peter.wehrfritz@web.de>
 *           Raphael Kubo da Costa <kubo@profusion.mobi>
 *           Tilman Sauerbeck <tilman@code-monkey.de>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Tom Hacohen <tom@stosb.com>
 *           Jonas M. Gastal <jgastal@profusion.mobi>
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
 * @file
 * @brief Eina Utility library
 *
 * These routines are used for Eina.
 */

/**
 * @page eina_main Eina
 *
 * @date 2008 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eina_main_intro
 * @li @ref eina_main_compiling
 * @li @ref eina_main_next_steps
 * @li @ref eina_main_intro_example
 *
 * @section eina_main_intro Introduction
 *
 * The Eina library is a library that implements an API for data types
 * in an efficient way. It also provides some useful tools like
 * opening shared libraries, errors management, type conversion,
 * time accounting and memory pool.
 *
 * This library is cross-platform and can be compiled and used on
 * Linux, BSD, Opensolaris and Windows (XP and CE).
 *
 * The data types that are available are (see @ref Eina_Data_Types_Group):
 * @li @ref Eina_Inline_Array_Group standard array of inlined members.
 * @li @ref Eina_Array_Group standard array of @c void* data.
 * @li @ref Eina_Hash_Group standard hash of @c void* data.
 * @li @ref Eina_Inline_List_Group list with nodes inlined into user type.
 * @li @ref Eina_CList_Group Compact List.
 * @li @ref Eina_List_Group standard list of @c void* data.
 * @li @ref Eina_Iterator_Group Iterator functions.
 * @li @ref Eina_Matrixsparse_Group sparse matrix of @c void* data.
 * @li @ref Eina_Rbtree_Group red-black tree with nodes inlined into user type.
 * @li @ref Eina_String_Buffer_Group mutable string to prepend, insert or append strings to a buffer.
 * @li @ref Eina_Stringshare_Group saves memory by sharing read-only string references.
 * @li @ref Eina_Tiler_Group split, merge and navigates into 2D tiled regions.
 * @li @ref Eina_Trash_Group container of unused but allocated data.
 * @li @ref Eina_Value_Group container for generic value storage and access.
 * @li @ref Eina_Model_Group container for data with user defined hierarchy/structure.
 *
 * The tools that are available are (see @ref Eina_Tools_Group):
 * @li @ref Eina_Benchmark_Group helper to write benchmarks.
 * @li @ref Eina_Convert_Group faster conversion from strings to integers, double, etc.
 * @li @ref Eina_Counter_Group measures number of calls and their time.
 * @li @ref Eina_Error_Group error identifiers.
 * @li @ref Eina_File_Group simple file list and path split.
 * @li @ref Eina_Lalloc_Group simple lazy allocator.
 * @li @ref Eina_Log_Group full-featured logging system.
 * @li @ref Eina_Magic_Group provides runtime type checking.
 * @li @ref Eina_Memory_Pool_Group abstraction for various memory allocators.
 * @li @ref Eina_Module_Group lists, loads and share modules using Eina_Module standard.
 * @li @ref Eina_Rectangle_Group rectangle structure and standard manipulation methods.
 * @li @ref Eina_Safety_Checks_Group extra checks that will report unexpected conditions and can be disabled at compile time.
 * @li @ref Eina_String_Group a set of functions that manages C strings.
 *
 * @section eina_main_compiling How to compile
 *
 * Eina is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eina`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eina`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eina_main_next_steps Next Steps
 *
 * After you understood what Eina is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Eina_Data_Types_Group to find about implemented types and
 *     how to use them.
 * @li @ref Eina_Tools_Group to find about helper tools provided by eina.
 *
 * @section eina_main_intro_example Introductory Example
 *
 * @include eina_list_01.c
 *
 * More examples can be found at @ref eina_examples.
 *
 *
 * @defgroup Eina_Data_Types_Group Data Types
 * @ingroup Eina
 *
 * Eina provides an easy to use and optimized data types and structures.
 *
 * @defgroup Eina_Content_Access_Group Content Access
 * @ingroup Eina_Data_Types_Group
 *
 * Eina provides an uniform way to access elements of a container,
 * either sequential or indexed.
 *
 * @defgroup Eina_Containers_Group Containers
 * @ingroup Eina_Data_Types_Group
 *
 * Data types that contains other types. Examples: list, array and hash.
 *
 * @section eina_main_Intro Introduction
 * Containers are data types that hold data and allow iteration over
 * their elements with an @ref Eina_Iterator_Group, or eventually an
 * @ref Eina_Accessor_Group.
 *
 * The containers in eina are designed with performance in mind, one consequence
 * of this is that they @b don't check the validity of data structures given to
 * them(@ref Eina_Magic_Group).
 *
 * @section eina_main_Choice Choosing container type
 *
 * The choice of which container to use in each situation is very important in
 * achieving good performance and readable code. The most common container types
 * to be used are:
 * @li List
 * @li Inline list
 * @li Array
 * @li Inline array
 * @li Hash
 *
 * All types have virtues and vices. The following considerations are good
 * starting point in deciding which container to use:
 * @li Hashes are appropriate for datasets which will be searched often;
 * @li arrays are good when accessing members by position;
 * @li lists provide good versatility for adding elements in any position with
 * minimal overhead;
 * @li inline arrays use very little memory and don't cause fragmentation and
 * therefore are a good option in memory constrained systems;
 * @li inline lists are the appropriate type to use when the flexibility of a
 * list is required but the overhead of pointer indirection is not acceptable.
 * @warning These are general considerations, every situation is different,
 * don't follow these recommendations blindly.
 *
 * @section eina_main_Creation Creating custom container types
 *
 * @note Before creating a custom container check if one of the existing ones
 * doesn't suit your needs. For example, while there is no stack type @ref
 * Eina_Array_Group is a very good substitute, similarly there is no queue type
 * however an @ref Eina_List_Group works very well as a queue.
 *
 * If creating a custom container type consider allowing access to the data in
 * your container through @ref Eina_Iterator_Group "Iterators" and @ref
 * Eina_Accessor_Group "Accessors". To do so your container should have an
 * iterator creation function and an accessor creation function, these functions
 * should return properly populated @ref _Eina_Iterator and @ref _Eina_Accessor.
 *
 * @defgroup Eina_Tools_Group Tools
 * @ingroup Eina
 *
 * Eina tools aims to help application development, providing ways to
 * make it safer, log errors, manage memory more efficiently and more.
 * 
 */

#ifdef _WIN32
# include <Evil.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "eina_config.h"
#include "eina_types.h"
#include "eina_alloca.h"
#include "eina_main.h"
#include "eina_fp.h"
#include "eina_rectangle.h"
#include "eina_clist.h"
#include "eina_inlist.h"
#include "eina_file.h"
#include "eina_list.h"
#include "eina_hash.h"
#include "eina_trash.h"
#include "eina_lalloc.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_inarray.h"
#include "eina_array.h"
#include "eina_binshare.h"
#include "eina_stringshare.h"
#include "eina_ustringshare.h"
#include "eina_magic.h"
#include "eina_counter.h"
#include "eina_rbtree.h"
#include "eina_accessor.h"
#include "eina_iterator.h"
#include "eina_benchmark.h"
#include "eina_convert.h"
#include "eina_cpu.h"
#include "eina_sched.h"
#include "eina_tiler.h"
#include "eina_thread.h"
#include "eina_hamster.h"
#include "eina_matrixsparse.h"
#include "eina_str.h"
#include "eina_strbuf.h"
#include "eina_binbuf.h"
#include "eina_ustrbuf.h"
#include "eina_unicode.h"
#include "eina_quadtree.h"
#include "eina_simple_xml_parser.h"
#include "eina_lock.h"
#include "eina_prefix.h"
#include "eina_refcount.h"
#include "eina_mmap.h"
#include "eina_xattr.h"
#include "eina_value.h"
#include "eina_cow.h"

#ifdef __cplusplus
}
#endif

#endif /* EINA_H */
