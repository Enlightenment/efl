/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_STRINGSHARE_H_
#define EINA_STRINGSHARE_H_

#include <stdarg.h>

#include "eina_types.h"

/**
 * @page eina_stringshare_example_01_page
 * @dontinclude eina_stringshare_01.c
 *
 * Like all examples we start by including Eina:
 * @skip #include
 * @line #include
 *
 * Here we declare some variables and initialize eina:
 * @until eina_init
 *
 * We start the substantive part of the example by showing how to make a part
 * of a string shared and how to get the length of a shared string:
 * @until stringshare_strlen
 * As we add shared strings we also need to delete them when done using them:
 * @line del
 *
 * There are many ways of creating shared strings including an equivalent to
 * sprintf:
 * @until del
 *
 * An equivalent to snprintf:
 * @until printf
 *
 * But the simplest way of creating a shared string is through
 * eina_stringshare_add():
 * @until printf
 *
 * Sometimes you already have a pointer to a shared string and want to use it,
 * so to make sure the provider of the pointer won't free it while you're using
 * it you can increase the shared string's ref count:
 * @until printf
 *
 * Whenever you have a pointer to a shared string and would like to change it's
 * value you should use eina_stringshare_replace():
 * @until printf
 * @warning @b Don't use eina_stringshare_del() followed by
 * eina_share_common_add(), under some circunstances you might end up deleting
 * a shared string some other piece of code is using.
 *
 * We created str but haven't deleted it yet, and while we called
 * eina_stringshare_del() on str2, we created it and then increased the ref
 * count so it's still alive:
 * @until str2
 *
 * You can see the full source code @ref eina_stringshare_example_01 "here".
 */
/**
 * @page eina_stringshare_example_01
 * @include eina_stringshare_01.c
 * @example eina_stringshare_01.c
 */
/**
 * @addtogroup Eina_Stringshare_Group Stringshare
 *
 * These functions allow you to store a single copy of a string, and use in
 * multiple places throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash lookup.
 *
 * It sounds like micro-optimizing, but profiling has shown this can have
 * a significant impact as you scale the number of copies up. It improves
 * string creation/destruction speed, reduces memory use and decreases
 * memory fragmentation, so a win all-around.
 *
 * Using eina stringshares usually boils down to:
 * @code
 * const char *str = eina_stringshare_add("My string");
 * ...
 * //Use str
 * ...
 * eina_stringshare_del(str);
 * @endcode
 * @note It's very important to note that string shares are @b @c const,
 * changing them will result in undefined behavior.
 * @note eina_stringshare_del() @b doesn't guarantee the string share will be
 * freed, it releases a reference to it, but if other references to it still
 * exist the string share will live until those are released.
 *
 * The following diagram gives an idea of what happens as you create strings
 * with eina_stringshare_add():
 *
 * @image html eina_stringshare.png
 * @image latex eina_stringshare.eps height=\textheight
 *
 * For more information, see @ref eina_stringshare_example_01_page
 * "this example".
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
 * @typedef Eina_Stringshare
 *
 * "Eina_Stringshare *" is interchangeable with "const char *" but still a good
 * visual hint for the purpose. Maybe in the far far future we'll even add
 * strict type checking.
 *
 * @since 1.2.0
 */
typedef const char Eina_Stringshare;

/**
 * @brief Retrieve an instance of a string with a specific size for use in a
 * program.
 *
 * @param   str The string to retrieve an instance of.
 * @param   slen The string size (<= strlen(str)).
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * a duplicated string of @p str is returned.
 *
 * This function does not check string size, but uses the
 * exact given size. This can be used to share_common part of a larger
 * buffer or substring.
 *
 * @see eina_share_common_add()
 */
EAPI Eina_Stringshare  *eina_stringshare_add_length(const char *str, unsigned int slen) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve an instance of a string for use in a program.
 *
 * @param   str The NULL-terminated string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * a duplicated string of @p str is returned.
 *
 * The string @p str must be NULL terminated ('@\0') and its full
 * length will be used. To use part of the string or non-null
 * terminated, use eina_stringshare_add_length() instead.
 *
 * @see eina_stringshare_add_length()
 */
EAPI Eina_Stringshare  *eina_stringshare_add(const char *str) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string.
 *
 * @param   fmt The NULL-terminated format string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt. If @p fmt is
 * @c NULL, then @c NULL is returned. If @p fmt is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * a duplicated string is returned.
 *
 * The format string @p fmt must be NULL-terminated ('@\0') and its full
 * length will be used. To use part of the format string or non-null
 * terminated, use eina_stringshare_nprintf() instead.
 *
 * @see eina_stringshare_nprintf()
 */
EAPI Eina_Stringshare  *eina_stringshare_printf(const char *fmt, ...) EINA_WARN_UNUSED_RESULT EINA_PRINTF(1, 2);

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string.
 *
 * @param   fmt The NULL-terminated format string to retrieve an instance of.
 * @param   args The va_args for @p fmt
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt with @p args. If @p fmt is
 * @c NULL, then @c NULL is returned. If @p fmt with @p args is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * a duplicated string is returned.
 *
 * The format string @p fmt must be NULL-terminated ('@\0') and its full
 * length will be used. To use part of the format string or non-null
 * terminated, use eina_stringshare_nprintf() instead.
 *
 * @see eina_stringshare_nprintf()
 */
EAPI Eina_Stringshare  *eina_stringshare_vprintf(const char *fmt, va_list args) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string with size limitation.
 * @param   len The length of the format string to use
 * @param   fmt The format string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt limited by @p len. If @p fmt is
 * @c NULL or @p len is < 1, then @c NULL is returned. If the resulting string
 * is already stored, it is returned and its reference counter is increased.
 * Otherwise a duplicated string is returned.
 *
 * @p len length of the format string will be used. To use the
 * entire format string, use eina_stringshare_printf() instead.
 *
 * @see eina_stringshare_printf()
 */
EAPI Eina_Stringshare  *eina_stringshare_nprintf(unsigned int len, const char *fmt, ...) EINA_WARN_UNUSED_RESULT EINA_PRINTF(2, 3);

/**
 * Increment references of the given shared string.
 *
 * @param str The shared string.
 * @return    A pointer to an instance of the string on success.
 *            @c NULL on failure.
 *
 * This is similar to eina_share_common_add(), but it's faster since it will
 * avoid lookups if possible, but on the down side it requires the parameter
 * to be shared string. In other words, it must be the return of a previous
 * call to one of the stringshare functions.
 *
 * There is no unref since this is the work of eina_share_common_del().
 */
EAPI Eina_Stringshare  *eina_stringshare_ref(Eina_Stringshare *str);

/**
 * @brief Note that the given string has lost an instance.
 *
 * @param str string The given string.
 *
 * This function decreases the reference counter associated to @p str
 * if it exists. If that counter reaches 0, the memory associated to
 * @p str is freed. If @p str is @c NULL, the function returns
 * immediately.
 *
 * @note If the given pointer is not shared, bad things will happen, likely a
 * segmentation fault.
 */
EAPI void               eina_stringshare_del(Eina_Stringshare *str);

/**
 * @brief Note that the given string @b must be shared.
 *
 * @param str the shared string to know the length. It is safe to
 *        give @c NULL, in that case @c 0 is returned.
 * @return The length of a shared string.
 *
 * This function is a cheap way to known the length of a shared
 * string.
 *
 * @note If the given pointer is not shared, bad things will happen, likely a
 * segmentation fault. If in doubt, try strlen().
 */
EAPI int                eina_stringshare_strlen(Eina_Stringshare *str) EINA_PURE EINA_WARN_UNUSED_RESULT;

/**
 * @brief Dump the contents of the share_common.
 *
 * This function dumps all strings in the share_common to stdout with a
 * DDD: prefix per line and a memory usage summary.
 */
EAPI void               eina_stringshare_dump(void);

static inline Eina_Bool eina_stringshare_replace(Eina_Stringshare **p_str, const char *news) EINA_ARG_NONNULL(1);
static inline Eina_Bool eina_stringshare_replace_length(Eina_Stringshare **p_str, const char *news, unsigned int slen) EINA_ARG_NONNULL(1);

#include "eina_inline_stringshare.x"

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STRINGSHARE_H_ */
