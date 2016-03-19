/* EINA - EFL data type library
 * Copyright (C) 2015 Vincent Torri
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

#ifndef EINA_FILE_INLINE_H_
#define EINA_FILE_INLINE_H_

static inline size_t
eina_file_path_join_len(char *dst,
                        size_t size,
                        const char *a,
                        size_t a_len,
                        const char *b,
                        size_t b_len)
{
   return eina_str_join_len(dst, size, EINA_PATH_SEP_C, a, a_len, b, b_len);
}

static inline size_t
eina_file_path_join(char *dst, size_t size, const char *a, const char *b)
{
   return eina_file_path_join_len(dst, size, a, strlen(a), b, strlen(b));
}

/**
 * @}
 */

#endif /* EINA_FILE_INLINE_H_ */
