/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_INLINE_TRASH_X__
#define EINA_INLINE_TRASH_X__

static inline void
eina_trash_init(Eina_Trash **trash)
{
   *trash = NULL;
}

static inline void
eina_trash_push(Eina_Trash **trash, void *data)
{
   Eina_Trash *tmp;

   tmp = (Eina_Trash *)data;
   tmp->next = *trash;
   *trash = tmp;
}

static inline void*
eina_trash_pop(Eina_Trash **trash)
{
   void *tmp;

   tmp = *trash;

   if (*trash)
     *trash = (*trash)->next;

   return tmp;
}

#endif
