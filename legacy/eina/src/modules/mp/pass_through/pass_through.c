/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "eina_types.h"
#include "eina_module.h"
#include "eina_private.h"

static void *
eina_pass_through_malloc(__UNUSED__ void *data, unsigned int size)
{
   return malloc(size);
}

static void
eina_pass_through_free(__UNUSED__ void *data, void *ptr)
{
   free(ptr);
}

static void*
eina_pass_through_realloc(__UNUSED__ void *data, void *ptr, unsigned int size)
{
   return realloc(ptr, size);
}

static void*
eina_pass_through_init(__UNUSED__ const char *context, __UNUSED__ const char *option, __UNUSED__ va_list args)
{
   return (void*) 0x1;
}

static void
eina_pass_through_shutdown(__UNUSED__ void *data)
{
}

static Eina_Mempool_Backend mp_backend = {
  .init = &eina_pass_through_init,
  .shutdown = &eina_pass_through_shutdown,
  .realloc = &eina_pass_through_realloc,
  .alloc = &eina_pass_through_malloc,
  .free = &eina_pass_through_free,
  .garbage_collect = NULL,
  .statistics = NULL
};

EINA_MODULE("pass_through", "mp", NULL, &mp_backend);

