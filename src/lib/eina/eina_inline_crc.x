/* EINA - EFL data type library
 * Copyright (C) 20015 Cedric BAIL
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

#ifndef EINA_INLINE_CRC_X_
#define EINA_INLINE_CRC_X_

EINA_API unsigned int _eina_crc(const char *data, int len, unsigned int seed, Eina_Bool start_stream);

static inline unsigned int
eina_crc(const char *key, int len, unsigned int seed, Eina_Bool start_stream)
{
   return _eina_crc(key, len, seed, start_stream);
}

#endif
