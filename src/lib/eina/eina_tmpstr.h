/* EINA - EFL data type library
 * Copyright (C) 2002-2012 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 *  all copies of the Software and its Copyright notices. In addition, publicly
 *  documented acknowledgement must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgements in either Copyright notices, Manuals, Publicity, and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.
 *
 *  Please see OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS, OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef EINA_TMPSTR_H_
#define EINA_TMPSTR_H_

#include "eina_types.h"

/**
 * @page eina_tmpstr_page
 *
 * Eina tmpstr is intended for being able to conveniently pass strings back
 * to a calling parent without having to use single static buffers (which
 * don't work with multiple threads or when returning multiple times as
 * parameters to a single function.
 *
 * The traditional way to "return" a string in C is either to provide a buffer
 * as a parameter to return it in, return a pointer to a single static buffer,
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
 * type marker letting you know that the function cleans up those
 * strings after use, and it is totally interchangeable with const char.
 */

/**
 * @defgroup Eina_Tmpstr_Group Tmpstr
 * @ingroup Eina_Data_Types_Group
 *
 * @brief Eina tmpstr is intended for being able to conveniently pass strings back
 *        to a calling parent without having to use single static buffers (which
 *        don't work with multiple threads or when returning multiple times as
 *        parameters to a single function.
 *
 * @ref eina_tmpstr_page
 *
 * @{
 */

/**
 * @typedef Eina_Tmpstr
 *
 * @brief Interchangeable with "const char *", but still a good visual hint for the
 *        purpose. This indicates that the string is temporary and should be freed after
 *        use.
 *
 * @since 1.8.0
 */

typedef const char Eina_Tmpstr;

/**
 * @brief Adds a new temporary string based on the input string.
 *
 * @since 1.8.0
 *
 * @since_tizen 2.3
 *
 * @remarks When you add a temporary string (tmpstr) it is expected to have a very
 *          short lifespan, and at any one time only a few of these are intended to
 *          exist. This is not intended for long term storage of strings. The
 *          intended use is the ability to safely pass strings as return values from
 *          functions directly into parameters of new functions and then have the
 *          string cleaned up automatically by the caller.
 *
 * @remarks If @a str is @c NULL, or no memory space exists to store the tmpstr, then
 *          @c NULL is returned, otherwise a valid string pointer is returned
 *          that you can treat as any other C string (eg: strdup(tmpstr) or
 *          printf("%s\n", tmpstr) etc.). This string should be considered read-only
 *          and immutable, and when you are done with the string you should delete it
 *          using eina_tmpstr_del().
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
 * @param[in] str The input string that is copied into the temp string
 * @return A pointer to the tmp string that is a standard C string
 *
 * @see eina_tmpstr_del()
 * @see eina_tmpstr_add_length()
 */
EAPI Eina_Tmpstr *eina_tmpstr_add(const char *str) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Adds a new temporary string based on the input string and length.
 *
 * @since 1.8.0
 *
 * @since_tizen 2.3
 *
 * @remarks When you add a temporary string (tmpstr) it is expected to have a very
 *          short lifespan, and at any one time only a few of these are intended to
 *          exist. This is not intended for long term storage of strings. The
 *          intended use is the ability to safely pass strings as return values from
 *          functions directly into parameters of new functions and then have the
 *          string cleaned up automatically by the caller.
 *
 * @remarks If @a str is @c NULL, or no memory space exists to store the tmpstr, then
 *          @c NULL is returned, otherwise a valid string pointer is returned
 *          that you can treat as any other C string (eg strdup(tmpstr) or
 *          printf("%s\n", tmpstr) etc.). This string should be considered read-only
 *          and immutable, and when you are done with the string you should delete it
 *          using eina_tmpstr_del().
 *
 * @note If the length is greater than the actual string, but still '\0'
 *       terminateed. Their won't be any crash and the string will be correct,
 *       but eina_tmpstr_strlen will return an erroneous length. So if you
 *       want to have the correct length always call eina_tmpstr_add_length
 *       with length == strlen(str).
 *
 * @param[in] str The input string that is copied into the temp string
 * @param[in] length The maximum length and the allocated length of the temp string
 * @return A pointer to the tmp string that is a standard C string
 *
 * @see eina_tmpstr_del()
 * @see eina_tmpstr_add()
 */
EAPI Eina_Tmpstr *eina_tmpstr_add_length(const char *str, size_t length);

/**
 * @brief Returns the length of a temporary string including the '\0'.
 *
 * @since 1.8.0
 *
 * @since_tizen 2.3
 *
 * @param[in] tmpstr A C string pointer, but if it is a tmp string
 *               it returns the length faster
 * @return The length of the string including the '\0'
 */
EAPI size_t eina_tmpstr_strlen(Eina_Tmpstr *tmpstr);

/**
 * @brief Deletes the temporary string if it is one, or ignores it if it is not.
 *
 * @details This deletes the given temporary string @a tmpstr if it is a valid
 *          temporary string, otherwise it ignores it and does nothing, so this
 *          can be used safely with non-temporary strings.
 *
 * @since 1.8.0
 *
 * @since_tizen 2.3
 *
 * @param[in] tmpstr A C string pointer, but if it is a tmp string
 *               it is freed
 *
 * @see eina_tmpstr_add()
 */
EAPI void eina_tmpstr_del(Eina_Tmpstr *tmpstr) EINA_ARG_NONNULL(1);

/**
 * @}
 */

#endif
