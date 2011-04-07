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
 * @page tutorial_ustringshare_page UStringshare Tutorial
 *
 * to be written...
 *
 */

#include "eina_share_common.h"
#include "eina_unicode.h"
#include "eina_private.h"
#include "eina_ustringshare.h"

/* The actual share */
static Eina_Share *ustringshare_share;
static const char EINA_MAGIC_USTRINGSHARE_NODE_STR[] = "Eina UStringshare Node";

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
Eina_Bool
eina_ustringshare_init(void)
{
   return eina_share_common_init(&ustringshare_share,
                                 EINA_MAGIC_USTRINGSHARE_NODE,
                                 EINA_MAGIC_USTRINGSHARE_NODE_STR);
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
Eina_Bool
eina_ustringshare_shutdown(void)
{
   Eina_Bool ret;
   ret = eina_share_common_shutdown(&ustringshare_share);
   return ret;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI void
eina_ustringshare_del(const Eina_Unicode *str)
{
   if (!str)
      return;

   eina_share_common_del(ustringshare_share,(const char *)str);
}

EAPI const Eina_Unicode *
eina_ustringshare_add_length(const Eina_Unicode *str, unsigned int slen)
{
   return (const Eina_Unicode *)eina_share_common_add_length(ustringshare_share,
                                                             (const char *)str,
                                                             slen *
                                                             sizeof(
                                                                Eina_Unicode),
                                                             sizeof(
                                                                Eina_Unicode));
}

EAPI const Eina_Unicode *
eina_ustringshare_add(const Eina_Unicode *str)
{
   int slen = (str) ? (int)eina_unicode_strlen(str) : -1;
   return eina_ustringshare_add_length(str, slen);
}

EAPI const Eina_Unicode *
eina_ustringshare_ref(const Eina_Unicode *str)
{
   return (const Eina_Unicode *)eina_share_common_ref(ustringshare_share,
                                                      (const char *)str);
}

EAPI int
eina_ustringshare_strlen(const Eina_Unicode *str)
{
   int len = eina_share_common_length(ustringshare_share, (const char *)str);
   len = (len > 0) ? len / (int)sizeof(Eina_Unicode) : -1;
   return len;
}

EAPI void
eina_ustringshare_dump(void)
{
   eina_share_common_dump(ustringshare_share, NULL, 0);
}
