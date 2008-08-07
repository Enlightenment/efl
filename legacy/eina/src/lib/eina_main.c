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

#include "Eina.h"

EAPI int
eina_init(void)
{
   int r;

   r = eina_error_init();
   r += eina_hash_init();
   r += eina_stringshare_init();
   r += eina_list_init();
   r += eina_array_init();

   return r;
}

EAPI int
eina_shutdown(void)
{
   int r;

   eina_array_shutdown();
   eina_list_shutdown();
   r = eina_stringshare_shutdown();
   r += eina_hash_shutdown();
   r += eina_error_shutdown();

   return r;
}

