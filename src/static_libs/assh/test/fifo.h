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

#ifndef ASSH_TEST_FIFO_H_
#define ASSH_TEST_FIFO_H_

#include <stdint.h>

#ifndef FIFO_BUF_SIZE
# define FIFO_BUF_SIZE 128
#endif

struct fifo_s
{
  uint8_t buf[FIFO_BUF_SIZE];
  unsigned ptr;
  size_t size;
};

static inline void fifo_init(struct fifo_s *f)
{
  f->ptr = 0;
  f->size = 0;
}

static inline size_t fifo_read(struct fifo_s *f, uint8_t *data, size_t size)
{
  size_t osize = size;
  while (f->size > 0 && size > 0)
    {
      *data++ = f->buf[f->ptr++ % sizeof(f->buf)];
      f->size--, size--;
    }
  return osize - size;
}

static inline size_t fifo_write(struct fifo_s *f, const uint8_t *data, size_t size)
{
  size_t osize = size;
  while (f->size < sizeof(f->buf) && size > 0)
    {
      f->buf[(f->ptr + f->size++) % sizeof(f->buf)] = *data++;
      size--;
    }
  return osize - size;
}

#endif

