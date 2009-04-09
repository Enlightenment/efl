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

/**
 * @addtogroup Eina_Stringshare_Group Stringshare
 *
 * @{
 */

static inline Eina_Bool
eina_stringshare_replace(const char **p_str, const char *news)
{
   news = eina_stringshare_add(news);
   eina_stringshare_del(*p_str);
   if (*p_str == news)
     return 0;
   *p_str = news;
   return 1;
}

/**
 * @}
 */

#endif /* EINA_STRINGSHARE_INLINE_H_ */
