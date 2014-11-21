/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_USTRINGSHARE_H_
#define EINA_USTRINGSHARE_H_

#include "eina_types.h"
#include "eina_unicode.h"

/**
 * @defgroup Eina_UStringshare_Group Unicode Stringshare
 * @ingroup Eina_Data_Types_Group Data
 *
 * @brief This group discusses functions that allow you to store one copy of a string, and use it
 *        throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in the
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash lookup.
 *
 * It sounds like micro-optimizing, but profiling has shown that this can have
 * a significant impact as you scale the number of copies up. It improves the
 * string creation/destruction speed, reduces memory use, and decreases
 * memory fragmentation, so a win all-around.
 *
 * @{
 */

/**
 * @brief Retrieves an instance of a string for use in a program.
 *
 * @details This function retrieves an instance of @a str. If @a str is
 *          @c NULL, then @c NULL is returned. If @a str is already stored, it
 *          is just returned and its reference counter is increased. Otherwise
 *          it is added to the strings to be searched and a duplicated string
 *          of @a str is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks This function does not check the string size, but uses the
 *          exact given size. This can be used to share_common part of a larger
 *          buffer or substring.
 *
 * @param  [in] str The string to retrieve an instance of
 * @param  [in] slen The string size (<= strlen(str))
 * @return  A pointer to an instance of the string on success,
 *          otherwise @c NULL on failure
 *
 * @see eina_ustringshare_add()
 */
EAPI const Eina_Unicode *eina_ustringshare_add_length(const Eina_Unicode *str, unsigned int slen) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieves an instance of a string for use in a program.
 *
 * @details This function retrieves an instance of @a str. If @a str is
 *          @c NULL, then @c NULL is returned. If @a str is already stored, it
 *          is just returned and its reference counter is increased. Otherwise
 *          it is added to the strings to be searched and a duplicated string
 *          of @a str is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The string @a str must be NULL-terminated ('@\0') and its full
 *          length should be used. To use part of the string or non-null
 *          terminated, use eina_stringshare_add_length() instead.
 *
 * @param  [in] str The NULL-terminated string to retrieve an instance of
 * @return  A pointer to an instance of the string on success,
 *          otherwise @c NULL on failure
 *
 * @see eina_ustringshare_add_length()
 */
EAPI const Eina_Unicode *eina_ustringshare_add(const Eina_Unicode *str) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Increments references of the given shared string.
 *
 * @since_tizen 2.3
 *
 * @remarks This is similar to eina_share_common_add(), but it's faster since it 
 *          avoids lookups if possible, but on the down side it requires the parameter
 *          to be shared before, in other words, it must be the return of a previous
 *          eina_ustringshare_add().
 *
 * @remarks There is no unref since this is the work of eina_ustringshare_del().
 *
 * @param[in] str The shared string
 * @return    A pointer to an instance of the string on success,
 *            otherwise @c NULL on failure
 */
EAPI const Eina_Unicode *eina_ustringshare_ref(const Eina_Unicode *str);

/**
 * @brief Notes that the given string has lost an instance.
 *
 * @details This function decreases the reference counter associated to @a str
 *          if it exists. If that counter reaches @c 0, the memory associated to
 *          @a str is freed. If @a str is @c NULL, the function returns
 *          immediately.
 *
 * @since_tizen 2.3
 *
 * @remarks If the given pointer is not shared, bad things happen, mostly a
 *          segmentation fault.
 *
 * @param[in] str string The given string
 */
EAPI void                eina_ustringshare_del(const Eina_Unicode *str);

/**
 * @brief Notes that the given string @b must be shared.
 *
 * @since_tizen 2.3
 *
 * @remarks This function is a cheap way to know the length of a shared
 *          string.
 *
 * @remarks If the given pointer is not shared, bad things happen, mostly a
 *          segmentation fault. If in doubt, try strlen().
 *
 * @param[in] str The shared string to know the length \n
 *            It is safe to give @c NULL, in which case @c -1 is returned.
 * @return The string length
 */
EAPI int                 eina_ustringshare_strlen(const Eina_Unicode *str) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Dumps the contents of share_common.
 *
 * @details This function dumps all the strings from share_common to stdout with a
 *          DDD: prefix per line and a memory usage summary.
 *
 * @since_tizen 2.3
 */
EAPI void                eina_ustringshare_dump(void);

/**
 * @brief Replace the previously stringshared pointer with new content.
 *
 * @details The string pointed by @a p_str should be previously stringshared or
 *          @c NULL and it will be eina_ustringshare_del(). The new string will
 *          be passed to eina_ustringshare_add() and then assigned to @c *p_str.
 *
 * @since_tizen 2.3
 *
 * @param[in] p_str pointer to the stringhare to be replaced. Must not be
 *        @c NULL, but @c *p_str may be @c NULL as it is a valid
 *        stringshare handle.
 * @param[in] news new string to be stringshared, may be @c NULL.
 *
 * @return @c EINA_TRUE if the strings were different and thus replaced, 
 *         @c EINA_FALSE if the strings were the same after shared.
 */
static inline Eina_Bool  eina_ustringshare_replace(const Eina_Unicode **p_str, const Eina_Unicode *news) EINA_ARG_NONNULL(1);

/**
 * @brief Replace the previously stringshared pointer with a new content.
 *
 * @details The string pointed by @a p_str should be previously stringshared or
 *          @c NULL and it will be eina_ustringshare_del(). The new string will
 *          be passed to eina_ustringshare_add_length() and then assigned to @c *p_str.
 *
 * @since_tizen 2.3
 *
 * @param[in] p_str pointer to the stringhare to be replaced. Must not be
 *        @c NULL, but @c *p_str may be @c NULL as it is a valid
 *        stringshare handle.
 * @param[in] news new string to be stringshared, may be @c NULL.
 * @param[in] slen The string size (<= strlen(str)).
 *
 * @return @c EINA_TRUE if the strings were different and thus replaced,
 *         @c EINA_FALSE if the strings were the same after shared.
 */
static inline Eina_Bool  eina_ustringshare_replace_length(const Eina_Unicode **p_str, const Eina_Unicode *news, unsigned int slen) EINA_ARG_NONNULL(1);

#include "eina_inline_ustringshare.x"

/**
 * @}
 */

#endif /* EINA_STRINGSHARE_H_ */
