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

#include <assh/assh_prng.h>
#include <assh/assh_context.h>

#include <string.h>
#include <gcrypt.h>

static ASSH_PRNG_INIT_FCN(assh_prng_gcrypt_init)
{
  c->prng_entropy = 0;
  return ASSH_OK;
}

static ASSH_PRNG_GET_FCN(assh_prng_gcrypt_get)
{
  switch (quality)
    {
    case ASSH_PRNG_QUALITY_WEAK:
      gcry_create_nonce(rdata, rdata_len);
      break;
    case ASSH_PRNG_QUALITY_NONCE:
    case ASSH_PRNG_QUALITY_EPHEMERAL_KEY:
      gcry_randomize(rdata, rdata_len, GCRY_STRONG_RANDOM);
      break;
    case ASSH_PRNG_QUALITY_LONGTERM_KEY:
    default:
      gcry_randomize(rdata, rdata_len, GCRY_VERY_STRONG_RANDOM);
      break;
    }

  return ASSH_OK;
}

static ASSH_PRNG_FEED_FCN(assh_prng_gcrypt_feed)
{
  return ASSH_OK;
}

static ASSH_PRNG_CLEANUP_FCN(assh_prng_gcrypt_cleanup)
{
}

const struct assh_prng_s assh_prng_gcrypt = 
{
  .f_init = assh_prng_gcrypt_init,
  .f_get = assh_prng_gcrypt_get,
  .f_feed = assh_prng_gcrypt_feed,
  .f_cleanup = assh_prng_gcrypt_cleanup,  
};

