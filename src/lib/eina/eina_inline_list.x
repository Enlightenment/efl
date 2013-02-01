/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_LIST_INLINE_H_
#define EINA_LIST_INLINE_H_

static inline Eina_List *
eina_list_last(const Eina_List *list)
{
   if (!list) return NULL;
   return list->accounting->last;
}

static inline Eina_List *
eina_list_next(const Eina_List *list)
{
   if (!list) return NULL;
   return list->next;
}

static inline Eina_List *
eina_list_prev(const Eina_List *list)
{
   if (!list) return NULL;
   return list->prev;
}

static inline void *
eina_list_data_get(const Eina_List *list)
{
   if (!list) return NULL;
   return list->data;
}

static inline void *
eina_list_data_set(Eina_List *list, const void *data)
{
   void *tmp;
   if (!list) return NULL;
   tmp = list->data;
   list->data = (void*) data;
   return tmp;
}

static inline unsigned int
eina_list_count(const Eina_List *list)
{
   if (!list) return 0;
   return list->accounting->count;
}


/**
 * @brief Returns the last list node's data
 *
 * @param list The list
 * @return The node's data, or @c NULL on being passed a @c NULL pointer
 *
 * This macro is a shortcut for typing eina_list_data_get(eina_list_last())
 * @since 1.8
 */
static inline void *
eina_list_last_data_get(const Eina_List *list)
{
   if (!list) return NULL;
   return eina_list_data_get(eina_list_last(list));
}
  
#endif /* EINA_LIST_INLINE_H_ */
