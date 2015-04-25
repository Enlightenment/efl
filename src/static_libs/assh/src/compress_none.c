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


#include <assh/assh_compress.h>

static ASSH_COMPRESS_INIT_FCN(assh_compress_none_init)
{
  return ASSH_OK;
}

static ASSH_COMPRESS_PROCESS_FCN(assh_compress_none_process)
{
  return ASSH_OK;
}

static ASSH_COMPRESS_CLEANUP_FCN(assh_compress_none_cleanup)
{
}

const struct assh_algo_compress_s assh_compress_none =
{
  .algo = { .name = "none", .class_ = ASSH_ALGO_COMPRESS, .safety = 99, .speed = 99 },
  .ctx_size = 0,
  .f_init = assh_compress_none_init,
  .f_process = assh_compress_none_process,
  .f_cleanup = assh_compress_none_cleanup,
};

