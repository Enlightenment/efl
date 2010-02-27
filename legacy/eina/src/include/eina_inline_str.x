/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Gustavo Sverzut Barbieri
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

#ifndef EINA_STR_INLINE_H_
#define EINA_STR_INLINE_H_

/**
 * @addtogroup Eina_Str_Group Str
 *
 * @{
 */

/**
 * strlen() that will count up to maxlen bytes.
 *
 * If one wants to know the size of @a str, but it should not be
 * greater than @a maxlen, then use this function and avoid needless
 * iterations after that size.
 *
 * @param str the string pointer, must be valid and not @c NULL.
 * @param maxlen the maximum length to allow.
 * @return the string size or (size_t)-1 if greater than @a maxlen.
 */
static inline size_t
eina_strlen_bounded(const char *str, size_t maxlen)
{
   const char *itr, *str_maxend = str + maxlen;
   for (itr = str; *itr != '\0'; itr++)
     if (itr == str_maxend) return (size_t)-1;
   return itr - str;
}

/**
 * @}
 */

#endif /* EINA_STR_INLINE_H_ */
