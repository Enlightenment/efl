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

#include <assh/assh_cipher.h>

static ASSH_CIPHER_INIT_FCN(assh_none_init)
{
  return ASSH_OK;
}

static ASSH_CIPHER_PROCESS_FCN(assh_none_process)
{
  return ASSH_OK;
}

static ASSH_CIPHER_CLEANUP_FCN(assh_none_cleanup)
{
}

const struct assh_algo_cipher_s assh_cipher_none =
{
  .algo = { .name = "none", .class_ = ASSH_ALGO_CIPHER, .safety = 0, .speed = 99 },
  .ctx_size = 0,
  .block_size = 8,
  .key_size = 8,
  .is_stream = 0,
  .f_init = assh_none_init,
  .f_process = assh_none_process,
  .f_cleanup = assh_none_cleanup,
};

