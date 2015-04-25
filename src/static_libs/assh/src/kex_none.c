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

#include <assh/assh_kex.h>
#include <assh/assh_session.h>
#include <assh/assh_bignum.h>
#include <assh/assh_hash.h>

#include <string.h>

static ASSH_KEX_PROCESS_FCN(assh_kex_none_process)
{
  assh_error_t err;

  assert(p == NULL);

  /* shared secret is 42 (32 bits) */
  uint8_t secret[8] = {
    /* len */ 0, 0, 0, 4,
    /* str */ 0, 0, 0, 42
  };

  /* exchange hash is zero (160 bits) */
  uint8_t ex_hash[20];
  memset(ex_hash, 0, sizeof(ex_hash));

  /* no packet exchange, setup new key */
  ASSH_ERR_RET(assh_kex_new_keys(s, &assh_hash_sha1, ex_hash, secret));
  ASSH_ERR_RET(assh_kex_end(s, 1));

  return ASSH_OK;
}

static ASSH_KEX_CLEANUP_FCN(assh_kex_none_cleanup)
{
}

static ASSH_KEX_INIT_FCN(assh_kex_none_init)
{
  return ASSH_OK;
}

const struct assh_algo_kex_s assh_kex_none =
{
  .algo = { .name = "none@libassh.org", .class_ = ASSH_ALGO_KEX,
            .safety = 0, .speed = 99 },
  .f_init = assh_kex_none_init,
  .f_cleanup = assh_kex_none_cleanup,
  .f_process = assh_kex_none_process,
};

