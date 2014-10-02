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

#include <string.h>

/**
 * @addtogroup Eina_Memory_Pool_Group Memory Pool
 *
 * @{
 */

/* Memory Pool */
typedef struct _Eina_Mempool_Backend_ABI1 Eina_Mempool_Backend_ABI1;
typedef struct _Eina_Mempool_Backend_ABI2 Eina_Mempool_Backend_ABI2;

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
   void (*repack)(void *data, Eina_Mempool_Repack_Cb cb, void *cb_data);
};

struct _Eina_Mempool_Backend_ABI1
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

struct _Eina_Mempool_Backend_ABI2
{
   void (*repack)(void *data, Eina_Mempool_Repack_Cb cb, void *cb_data);
};

struct _Eina_Mempool
{
   Eina_Mempool_Backend_ABI1 backend;
   void *backend_data;
   Eina_Mempool_Backend_ABI2 *backend2;
};

static inline void *
eina_mempool_realloc(Eina_Mempool *mp, void *element, unsigned int size)
{
   return mp->backend.realloc(mp->backend_data, element, size);
}

static inline void *
eina_mempool_malloc(Eina_Mempool *mp, unsigned int size)
{
   return mp->backend.alloc(mp->backend_data, size);
}

static inline void *
eina_mempool_calloc(Eina_Mempool *mp, unsigned int size)
{
   void *r = mp->backend.alloc(mp->backend_data, size);
   if (!r) return NULL;
   memset(r, 0, size);
   return r;
}

static inline void
eina_mempool_free(Eina_Mempool *mp, void *element)
{
   if (!element) return ;
   mp->backend.free(mp->backend_data, element);
}

static inline unsigned int
eina_mempool_alignof(unsigned int size)
{
   unsigned int align;

   if (EINA_UNLIKELY(size <= 2))
     {
        align = 2;
     }
   else if (EINA_UNLIKELY(size < 8))
     {
        align = 4;
     }
   else
#if __WORDSIZE == 32
     {
        align = 8;
     }
#else
   if (EINA_UNLIKELY(size < 16))
     {
        align = 8;
     }
   else
     {
        align = 16;
     }
#endif

   return ((size / align) + (size % align ? 1 : 0)) * align;
}

/**
 * @}
 */

#endif
