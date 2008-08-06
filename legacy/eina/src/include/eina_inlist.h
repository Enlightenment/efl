/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri
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

#ifndef EINA_INLIST_H_
#define EINA_INLIST_H_

#include "eina_types.h"

/**
 * @defgroup Inline_List_Group Inline List
 * @{
 */

/* TODO change the prototype to use an Eina_Inlist */
typedef struct _Eina_Inlist Eina_Inlist;

struct _Eina_Inlist
{
   Eina_Inlist *next;
   Eina_Inlist *prev;
   Eina_Inlist *last;
};

EAPI void * eina_inlist_append(void *in_list, void *in_item);
EAPI void * eina_inlist_prepend(void *in_list, void *in_item);
EAPI void * eina_inlist_append_relative(void *in_list, void *in_item, void *in_relative);
EAPI void * eina_inlist_prepend_relative(void *in_list, void *in_item, void *in_relative);
EAPI void * eina_inlist_remove(void *in_list, void *in_item);
EAPI void * eina_inlist_find(void *in_list, void *in_item);

#define EINA_INLIST_ITER_NEXT(list, l) for (l = (void*)(Eina_Inlist *)list; l; l = (void*)((Eina_Inlist *)l)->next)
#define EINA_INLIST_ITER_LAST(list, l) for (l = (void*)((Eina_Inlist *)list)->last; l; l = (void*)((Eina_Inlist *)l)->prev)

/** @} */

#endif /*EINA_INLIST_H_*/
