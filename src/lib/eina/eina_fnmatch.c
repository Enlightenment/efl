/* EINA - EFL data type library
 * Copyright (C) 2012 Rich Felker
 * Copyright (C) 2021 Vincent Torri
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

#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_fnmatch.h"
#include "../../static_libs/fnmatch/fnmatch.h"
#include "../../static_libs/fnmatch/fnmatch.c"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EINA_API Eina_Bool
eina_fnmatch(const char *glob, const char *string, Eina_Fnmatch_Flags flags)
{
   return (__fnmatch(glob, string, flags) == 0);
}
