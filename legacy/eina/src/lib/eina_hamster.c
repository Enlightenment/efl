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

#include <stdio.h>

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
#include "eina_rectangle.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int _eina_hamsters = 7;

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Main_Group Hamster
 *
 * @brief These functions provide hamster calls.
 *
 * @{
 */

/**
 * @brief Get the hamster count.
 *
 * @return The number of available hamsters.
 *
 * This function returns how many hamsters you have.
 */
EAPI int
eina_hamster_count(void)
{
   return _eina_hamsters;
}

/**
 * @}
 */
