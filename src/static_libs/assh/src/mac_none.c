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


#include <assh/assh_mac.h>

#include <string.h>

static ASSH_MAC_INIT_FCN(assh_hmac_none_init)
{
  return ASSH_OK;
}

static ASSH_MAC_CLEANUP_FCN(assh_hmac_none_cleanup)
{
}

static ASSH_MAC_COMPUTE_FCN(assh_hmac_none_compute)
{
  return ASSH_OK;
}

static ASSH_MAC_CHECK_FCN(assh_hmac_none_check)
{
  return ASSH_OK;
}

const struct assh_algo_mac_s assh_hmac_none = 
{
  .algo = { .name = "none", .class_ = ASSH_ALGO_MAC,
            .safety = 0, .speed = 99 },
  .ctx_size = 0,
  .key_size = 0,
  .mac_size = 0,
  .f_init = assh_hmac_none_init,
  .f_compute = assh_hmac_none_compute,
  .f_check = assh_hmac_none_check,
  .f_cleanup = assh_hmac_none_cleanup,
};

