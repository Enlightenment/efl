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

#ifndef EINA_STRINGSHARE_INLINE_H_
#define EINA_STRINGSHARE_INLINE_H_

#include <string.h>
#include "eina_stringshare.h"
/**
 * @addtogroup Eina_Stringshare_Group Stringshare
 *
 * @{
 */

/**
 * Replace the previously stringshared pointer with another stringshared pointer.
 *
 * The string pointed by @a p_str must be previously stringshared or
 * @c NULL and it will be eina_stringshare_del(). The new string must also
 * be stringshared and will be passed to eina_stringshare_ref() and then assigned to @c *p_str.
 * This function is identical to eina_stringshare_replace() except that it calls
 * eina_stringshare_ref() instead of eina_stringshare_add()
 *
 * @param p_str pointer to the stringhare to be replaced. Must not be
 *        @c NULL, but @c *p_str may be @c NULL as it is a valid
 *        stringshare handle.
 * @param news new string to replace with, may be @c NULL.
 *
 * @return #EINA_TRUE if the strings were different and thus replaced, #EINA_FALSE
 * if the strings were the same after shared.
 *
 * @since 1.8
 */
static inline Eina_Bool
eina_stringshare_refplace(Eina_Stringshare **p_str, Eina_Stringshare *news)
{
   if (*p_str == news) return EINA_FALSE;

   news = eina_stringshare_ref(news);
   eina_stringshare_del(*p_str);
   if (*p_str == news)
     return EINA_FALSE;
   *p_str = news;
   return EINA_TRUE;
}

/**
 * Replace the previously stringshared pointer with new content.
 *
 * The string pointed by @a p_str must be previously stringshared or
 * @c NULL and it will be eina_stringshare_del(). The new string will
 * be passed to eina_stringshare_add() and then assigned to @c *p_str.
 *
 * @param p_str pointer to the stringhare to be replaced. Must not be
 *        @c NULL, but @c *p_str may be @c NULL as it is a valid
 *        stringshare handle.
 * @param news new string to be stringshared, may be @c NULL.
 *
 * @return #EINA_TRUE if the strings were different and thus replaced, #EINA_FALSE
 * if the strings were the same after shared.
 */
static inline Eina_Bool
eina_stringshare_replace(Eina_Stringshare **p_str, const char *news)
{
   if (*p_str == news) return EINA_FALSE;

   news = eina_stringshare_add(news);
   eina_stringshare_del(*p_str);
   if (*p_str == news)
     return EINA_FALSE;
   *p_str = news;
   return EINA_TRUE;
}

/**
 * Replace the previously stringshared pointer with a new content.
 *
 * The string pointed by @a p_str must be previously stringshared or
 * @c NULL and it will be eina_stringshare_del(). The new string will
 * be passed to eina_stringshare_add_length() and then assigned to @c *p_str.
 *
 * @param p_str pointer to the stringhare to be replaced. Must not be
 *        @c NULL, but @c *p_str may be @c NULL as it is a valid
 *        stringshare handle.
 * @param news new string to be stringshared, may be @c NULL.
 * @param slen The string size (<= strlen(str)).
 *
 * @return #EINA_TRUE if the strings were different and thus replaced, #EINA_FALSE
 * if the strings were the same after shared.
 */
static inline Eina_Bool
eina_stringshare_replace_length(Eina_Stringshare **p_str, const char *news, unsigned int slen)
{
   if (*p_str == news) return EINA_FALSE;

   news = eina_stringshare_add_length(news, slen);
   eina_stringshare_del(*p_str);
   if (*p_str == news)
     return EINA_FALSE;
   *p_str = news;
   return EINA_TRUE;
}

/**
 * @}
 */

#endif /* EINA_STRINGSHARE_INLINE_H_ */
