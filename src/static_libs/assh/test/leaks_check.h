/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

#include <assh/assh_context.h>
#include <assh/assh_alloc.h>

#include <stdlib.h>

static size_t alloc_size = 0;

static ASSH_ALLOCATOR(assh_leaks_allocator)
{
  if (size == 0)
    {
      size_t *bsize = *ptr;
      bsize--;
      alloc_size -= *bsize;
      memset((void*)bsize, 0x5a, *bsize);
      free((void*)bsize);
      return ASSH_OK;
    }
  else if (*ptr == NULL)
    {
      size_t *bsize = malloc(sizeof(size_t) + size);
      if (bsize != NULL)
	{
	  *ptr = bsize + 1;
	  *bsize = size;
	  alloc_size += size;
	  memset(*ptr, 0xa5, size);
	  return ASSH_OK;
	}
      return ASSH_ERR_MEM;
    }
  else
    {
      size_t *bsize = *ptr;
      bsize = realloc(bsize - 1, sizeof(size_t) + size);
      if (bsize != NULL)
	{
	  alloc_size -= *bsize;
	  alloc_size += size;
	  *ptr = bsize + 1;
	  *bsize = size;
	  return ASSH_OK;
	}
      return ASSH_ERR_MEM;
    }
}

