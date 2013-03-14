/* EINA - EFL data type library
 * Copyright (C) 2002-2012 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2008 Peter Wehrfritz
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies of the Software and its Copyright notices. In addition publicly
 *  documented acknowledgment must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.
 *
 *  Please see the OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef EINA_TMPSTR_H_
#define EINA_TMPSTR_H_

#include "eina_types.h"

/**
 * @page eina_tmpstr_ppage
 * 
 * Eina tmpstr is intended for being able to conveniently pass strings back
 * to a calling parent without having to use single static buffers (which
 * don't work with multiple threads or when returning multilpe times as
 * parameters to a single function.
 * 
 * The traditional way to "return" a string in C is either to provide a buffer
 * as a paramater to return it in, return a pointer to a single static buffer,
 * which has issues, or return a duplicated string. All cases are inconvenient
 * and return special handling. This is intended to make this easier. Now you
 * can do something like this:
 * 
 * @code
 * Eina_Tmpstr *my_homedir(void) {
 *   return eina_tmpstr_add(getenv("HOME"));
 * }
 * 
 * Eina_Tmpstr *my_tmpdir(void) {
 *   return eina_tmpstr_add(getenv("TMP"));
 * }
 * 
 * void my_movefile(Eina_Tmpstr *src, Eina_Tmpstr *dst) {
 *   rename(src, dst);
 *   eina_tmpstr_del(src);
 *   eina_tmpstr_del(dst);
 * }
 * 
 * char buf[500];
 * my_movefile(my_homedir(), my_tmpdir());
 * my_movefile("/tmp/file", "/tmp/newname");
 * my_movefile(my_homedir(), "/var/tmp");
 * snprintf(buf, sizeof(buf), "/tmp/%i.file", rand());
 * my_movefile("/tmp.file", buf);
 * @endcode
 * 
 * Notice that you can interchange standard C strings (static ones or even
 * generated buffers) with tmpstrings. The Eina_Tmpstr type is merely a
 * type marker letting you know that the function will clean up those
 * strings after use, and it is totally interchangeable with const char.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Stringshare_Group Stringshare
 *
 * @{
 */

/**
 * @typedef Eina_Tmpstr
 *
 * Interchangeable with "const char *" but still a good visual hint for the
 * purpose. This indicates the string is temporary and should be freed after
 * use.
 *
 * @since 1.8.0
 */

typedef const char Eina_Tmpstr;

/**
 * @brief Add a new temporary string based on the input string.
 * 
 * @param str This is the input stringthat is copied into the temp string.
 * @return A pointer to the tmp string that is a standard C string.
 * 
 * When you add a temporary string (tmpstr) it is expected to have a very
 * short lifespan, and at any one time only a few of these are intended to
 * exist. This is not intended for longer term storage of strings. The
 * intended use is the ability to safely pass strings as return values from
 * functions directly into parameters of new functions and then have the
 * string be cleaned up automatically by the caller.
 * 
 * If @p str is NULL, or no memory space exists to store the tmpstr, then
 * NULL will be returned, otherwise a valid string pointer will be returned
 * that you can treat as any other C string (eg strdup(tmpstr) or
 * printf("%s\n", tmpstr) etc.). This string should be considered read-only
 * and immutable, and when youa re done with the string yo should delete it
 * with eina_tmpstr_del().
 * 
 * Example usage:
 * 
 * @code
 * Eina_Tmpstr *my_homedir(void) {
 *   return eina_tmpstr_add(getenv("HOME"));
 * }
 * 
 * void my_rmfile(Eina_Tmpstr *str) {
 *   if (!str) return;
 *   unlink(str);
 *   eina_tmpstr_del(str);
 * }
 * 
 * my_rmfile(my_homedir());
 * my_rmfile("/tmp/file");
 * @endcode
 * 
 * @see eina_tmpstr_del()
 * @see eina_tmpstr_add_length()
 * 
 * @since 1.8.0
 */
EAPI Eina_Tmpstr *eina_tmpstr_add(const char *str) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Add a new temporary string based on the input string and length.
 * 
 * @param str This is the input stringthat is copied into the temp string.
 * @param length This is the maximum length and the allocated length of the temp string.
 * @return A pointer to the tmp string that is a standard C string.
 * 
 * When you add a temporary string (tmpstr) it is expected to have a very
 * short lifespan, and at any one time only a few of these are intended to
 * exist. This is not intended for longer term storage of strings. The
 * intended use is the ability to safely pass strings as return values from
 * functions directly into parameters of new functions and then have the
 * string be cleaned up automatically by the caller.
 * 
 * If @p str is NULL, or no memory space exists to store the tmpstr, then
 * NULL will be returned, otherwise a valid string pointer will be returned
 * that you can treat as any other C string (eg strdup(tmpstr) or
 * printf("%s\n", tmpstr) etc.). This string should be considered read-only
 * and immutable, and when youa re done with the string yo should delete it
 * with eina_tmpstr_del().
 * 
 * @see eina_tmpstr_del()
 * @see eina_tmpstr_add()
 * 
 * @since 1.8.0
 */
EAPI Eina_Tmpstr *eina_tmpstr_add_length(const char *str, size_t length);

/**
 * @brief Return the length of a temporary string including the '\0'.
 *
 * @param tmpstr This is any C string pointer, but if it is a tmp string
 * it will return the length faster.
 * @return The length of the string including the '\0';
 *
 * @since 1.8.0
 */
EAPI size_t eina_tmpstr_strlen(Eina_Tmpstr *tmpstr);

/**
 * @brief Delete the temporary string if it is one, or ignore it if it is not.
 * 
 * @param tmpstr This is any C string pointer, but if it is a tmp string
 * it is freed.
 * 
 * This will delete the given temporary string @p tmpstr if it is a valid
 * temporary string, or otherwise it will ignore it and do nothing so this
 * can be used safely with non-temporary strings.
 * 
 * @see eina_tmpstr_add()
 * 
 * @since 1.8.0
 */
EAPI void eina_tmpstr_del(Eina_Tmpstr *tmpstr) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */

#endif
