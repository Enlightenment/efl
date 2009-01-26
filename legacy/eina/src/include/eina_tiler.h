/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
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
#ifndef EINA_TILER_H_
#define EINA_TILER_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_rectangle.h"

typedef struct _Eina_Tiler Eina_Tiler;

EAPI Eina_Tiler *eina_tiler_new(int w, int h);
EAPI void eina_tiler_del(Eina_Tiler *t);
EAPI void eina_tiler_tile_size_set(Eina_Tiler *t, int w, int h);
EAPI Eina_Bool eina_tiler_rect_add(Eina_Tiler *t, Eina_Rectangle *r);
EAPI void eina_tiler_rect_del(Eina_Tiler *t, Eina_Rectangle *r);
EAPI void eina_tiler_clear(Eina_Tiler *t);
EAPI Eina_Iterator * eina_tiler_iterator_new(const Eina_Tiler *t);

#endif /* EINA_TILER_H_ */
