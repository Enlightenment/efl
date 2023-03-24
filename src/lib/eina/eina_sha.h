/* EINA - EFL data type library
 * Copyright (C) 2023 Carsten Haitzler
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

#ifndef EINA_SHA_H_
#define EINA_SHA_H_

/**
 * @brief Calculate a SHA1 checksum of data
 *
 * This function calculates a SHA1 checksum of the binary data pointed to by
 * @p data of size @p size in bytes and will fill 20 bytes pointed to by
 * @p dst with the resulting 20 byte binary SHA1 sum.
 *
 * The input data should be a valid pointer to at least @p size bytes of data
 * that will be summed. @p dst should also point to a buffer of at least 20
 * bytes in size to store the checksum.
 *
 * @since 1.27
 */
EINA_API void eina_sha1(const unsigned char *data, int size, unsigned char dst[20]) EINA_ARG_NONNULL(1);


#endif
