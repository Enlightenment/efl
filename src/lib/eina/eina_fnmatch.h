/* EINA - EFL data type library
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

#ifndef	EINA_FNMATCH_H
#define	EINA_FNMATCH_H

typedef enum
{
   EINA_FNMATCH_PATHNAME    = (1 << 0),
   EINA_FNMATCH_NOESCAPE    = (1 << 1),
   EINA_FNMATCH_PERIOD      = (1 << 2),
   EINA_FNMATCH_LEADING_DIR = (1 << 3),
   EINA_FNMATCH_CASEFOLD    = (1 << 4),
   EINA_FNMATCH_FILE_NAME   = EINA_FNMATCH_PATHNAME,
} Eina_Fnmatch_Flags;

EINA_API Eina_Bool eina_fnmatch(const char *glob, const char *string, Eina_Fnmatch_Flags flags);

#endif
