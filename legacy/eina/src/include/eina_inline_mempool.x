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

#ifndef EINA_INLINE_MEMPOOL_X_
#define EINA_INLINE_MEMPOOL_X_

/* Memory Pool */
typedef struct _Eina_Mempool_Backend Eina_Mempool_Backend;
struct _Eina_Mempool_Backend
{
   const char *name;
   void *(*init)(const char *context, const char *options, va_list args);
   void (*free)(void *data, void *element);
   void *(*alloc)(void *data, unsigned int size);
   void *(*realloc)(void *data, void *element, unsigned int size);
   void (*garbage_collect)(void *data);
   void (*statistics)(void *data);
   void (*shutdown)(void *data);
};

struct _Eina_Mempool
{
   Eina_Mempool_Backend backend;
   void *backend_data;
};

static inline void *
eina_mempool_realloc(Eina_Mempool *mp, void *element, unsigned int size)
{
   return mp->backend.realloc(mp->backend_data, element, size);
}

static inline void *
eina_mempool_alloc(Eina_Mempool *mp, unsigned int size)
{
   return mp->backend.alloc(mp->backend_data, size);
}

static inline void
eina_mempool_free(Eina_Mempool *mp, void *element)
{
   mp->backend.free(mp->backend_data, element);
}

#endif
