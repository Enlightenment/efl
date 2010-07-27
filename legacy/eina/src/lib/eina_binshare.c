/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Tom Hacohen
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
 /**
 * @page tutorial_binshare_page Binary Share Tutorial
 *
 * Should call eina_binshare_init() before usage and eina_binshare_shutdown() after.
 * to be written...
 *
 */

#include "eina_share_common.h"
#include "eina_unicode.h"
#include "eina_private.h"

/* The actual share */
static Eina_Share *binshare_share;
static const char EINA_MAGIC_BINSHARE_NODE_STR[] = "Eina Binshare Node";

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @internal
 * @brief Initialize the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the share_common module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
EAPI Eina_Bool
eina_binshare_init(void)
{
   return eina_share_common_init(&binshare_share, EINA_MAGIC_BINSHARE_NODE, EINA_MAGIC_BINSHARE_NODE_STR);
}

/**
 * @internal
 * @brief Shut down the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the share_common module set up by
 * eina_share_common_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
EAPI Eina_Bool
eina_binshare_shutdown(void)
{
   Eina_Bool ret;
   ret = eina_share_common_shutdown(&binshare_share);
   return ret;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * @addtogroup Eina_Binshare_Group Binary Share
 *
 * These functions allow you to store one copy of a string, and use it
 * throughout your program.
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
 * For more information, you can look at the @ref tutorial_binshare_page.
 *
 * @{
 */

/**
 * @brief Note that the given string has lost an instance.
 *
 * @param str string The given string.
 *
 * This function decreases the reference counter associated to @p str
 * if it exists. If that counter reaches 0, the memory associated to
 * @p str is freed. If @p str is NULL, the function returns
 * immediatly.
 *
 * Note that if the given pointer is not shared or NULL, bad things
 * will happen, likely a segmentation fault.
 */
EAPI void
eina_binshare_del(const char *str)
{
   if (!str)
      return;
   eina_share_common_del(binshare_share,(const char *) str);
}

/**
 * @brief Retrieve an instance of a string for use in a program.
 *
 * @param   str The binary string to retrieve an instance of.
 * @param   slen The byte size
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * of @p str is returned.
 *
 * This function does not check string size, but uses the
 * exact given size. This can be used to share_common part of a larger
 * buffer or substring.
 *
 * @see eina_binshare_add()
 */
EAPI const char *
eina_binshare_add_length(const char *str, unsigned int slen)
{
   return (const char *) eina_share_common_add_length(binshare_share,(const char *) str, (slen) * sizeof(char), 0);
}

/**
 * Increment references of the given shared string.
 *
 * @param str The shared string.
 * @return    A pointer to an instance of the string on success.
 *            @c NULL on failure.
 *
 * This is similar to eina_share_common_add(), but it's faster since it will
 * avoid lookups if possible, but on the down side it requires the parameter
 * to be shared before, in other words, it must be the return of a previous
 * eina_binshare_add().
 *
 * There is no unref since this is the work of eina_binshare_del().
 */
EAPI const char *
eina_binshare_ref(const char *str)
{
   return (const char *) eina_share_common_ref(binshare_share, (const char *) str);
}

/**
 * @brief Note that the given string @b must be shared.
 *
 * @param str the shared string to know the length. It is safe to
 *        give NULL, in that case -1 is returned.
 *
 * This function is a cheap way to known the length of a shared
 * string. Note that if the given pointer is not shared, bad
 * things will happen, likely a segmentation fault. If in doubt, try
 * strlen().
 */
EAPI int
eina_binshare_length(const char *str)
{
   return eina_share_common_length(binshare_share, (const char *) str);
}

/**
 * @brief Dump the contents of the share_common.
 *
 * This function dumps all strings in the share_common to stdout with a
 * DDD: prefix per line and a memory usage summary.
 */
EAPI void
eina_binshare_dump(void)
{
   eina_share_common_dump(binshare_share, NULL, 0);
}

/**
 * @}
 */

