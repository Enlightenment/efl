/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
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

#ifndef EINA_COW_H_
#define EINA_COW_H_

typedef struct _Eina_Cow Eina_Cow;

EAPI Eina_Cow *eina_cow_add(const char *name, unsigned int struct_size, unsigned int step, const void *default_value);
EAPI void eina_cow_del(Eina_Cow *cow);

EAPI const void *eina_cow_alloc(Eina_Cow *cow);
EAPI void eina_cow_free(Eina_Cow *cow, const void *data);

EAPI void *eina_cow_write(Eina_Cow *cow, const void * const *src);
EAPI void eina_cow_commit(Eina_Cow *cow, const void * const *dst, const void *data);
EAPI void eina_cow_memcpy(Eina_Cow *cow, const void * const *dst, const void *src);

EAPI Eina_Bool eina_cow_gc(Eina_Cow *cow);

#endif
