/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_types.h"
#include "eina_main.h"
#include "eina_error.h"
#include "eina_hash.h"
#include "eina_stringshare.h"
#include "eina_list.h"
#include "eina_array.h"
#include "eina_counter.h"
#include "eina_benchmark.h"
#include "eina_magic.h"

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

static int _eina_main_count = 0;

EAPI int
eina_init(void)
{
   if (_eina_main_count) goto finish_init;

   eina_error_init();
   eina_hash_init();
   eina_stringshare_init();
   eina_list_init();
   eina_array_init();
   eina_counter_init();
   eina_benchmark_init();
   eina_magic_string_init();

 finish_init:
   return ++_eina_main_count;
}

EAPI int
eina_shutdown(void)
{
   if (_eina_main_count != 1) goto finish_shutdown;

   eina_magic_string_shutdown();
   eina_benchmark_shutdown();
   eina_counter_shutdown();
   eina_array_shutdown();
   eina_list_shutdown();
   eina_stringshare_shutdown();
   eina_hash_shutdown();
   eina_error_shutdown();

 finish_shutdown:
   return --_eina_main_count;
}

