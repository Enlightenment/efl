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
#include <assh/assh_alloc.h>

#include <string.h>
#include <stdlib.h>

/** Number of words in the XSWAP prng pool */
#define ASSH_PRNG_XSWAP_SIZE 128

struct assh_prng_pv_s
{
  uint32_t pool[ASSH_PRNG_XSWAP_SIZE];
  uint32_t key[4];
  uint64_t cnt;
  uint32_t i;
};

static void assh_prng_xswap_tea_cipher(struct assh_prng_pv_s *ctx,
				       uint32_t *v0_, uint32_t *v1_)
{
  uint32_t k0 = ctx->key[0], k1 = ctx->key[1], k2 = ctx->key[2], k3 = ctx->key[3];
  uint32_t v0 = *v0_, v1 = *v1_;
  uint32_t sum = 0;
  unsigned int i;

  for (i = 0; i < 32; i++)
    {
      sum += 0x9e3779b9;
      v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
      v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);  
    }
  *v0_ = v0;
  *v1_ = v1;
}

static uint32_t assh_prng_xswap_get32(struct assh_prng_pv_s *ctx)
{
  /* 
                 |    i += 2   | j = rnd(i, i+N/4) * 2 + N/8 |
    ------------------------------------------/ /---------------
          |      |      |      |      |      / /    |       |
    ----------------------------------------/ /-----------------
                     pool[i]              pool[j]
                  \___________/        \___________/
                        |                    |
                        |        cnt++       |
                        |          |         |
                        |          v         |
                        |------>(xor64)<-----|
                        |          |         |
                        |          v         |
     128 bits key >---  |  ----->(tea)       |
                        |          |         |
                        v          |         v
                     (xor64)<------|----->(xor64)
                        |          |         |
                        |          \-------  |  --->(xor32)---> result
                        |                    |
                   _____v_____          _____v_____
                  /           \        /           \
                     pool[i]              pool[j]
    ------------------------------------------/ /---------------
          |      |      |      |      |      / /    |       |
    ----------------------------------------/ /-----------------
  */

  uint32_t *i0 = ctx->pool + ((ctx->i + 0) & (ASSH_PRNG_XSWAP_SIZE - 1));
  uint32_t *i1 = ctx->pool + ((ctx->i + 1) & (ASSH_PRNG_XSWAP_SIZE - 1));

  uint32_t j = ctx->i + (*i0 & (ASSH_PRNG_XSWAP_SIZE / 4 - 1)) * 2
                                  + ASSH_PRNG_XSWAP_SIZE / 8;

  uint32_t *j0 = ctx->pool + ((j + 0) & (ASSH_PRNG_XSWAP_SIZE - 1));
  uint32_t *j1 = ctx->pool + ((j + 1) & (ASSH_PRNG_XSWAP_SIZE - 1));

  uint32_t x0 = *i0 ^ *j0 ^ ctx->cnt;
  uint32_t x1 = *i1 ^ *j1 ^ (ctx->cnt >> 32);

  assh_prng_xswap_tea_cipher(ctx, &x0, &x1);

  *i0 ^= x0;
  *i1 ^= x1;
  *j0 ^= x0;
  *j1 ^= x1;

  ctx->i += 2;
  ctx->cnt++;

  return x0 ^ x1;
}

static ASSH_PRNG_INIT_FCN(assh_prng_xswap_init)
{
  assh_error_t err;

  ASSH_ERR_RET(assh_alloc(c, sizeof(struct assh_prng_pv_s), ASSH_ALLOC_SECUR, &c->prng_pv));
  struct assh_prng_pv_s *ctx = c->prng_pv;

  assert((ASSH_PRNG_XSWAP_SIZE & (ASSH_PRNG_XSWAP_SIZE-1)) == 0 && "power of 2");
  assert(ASSH_PRNG_XSWAP_SIZE >= 32);

  c->prng_entropy = -(int)sizeof(ctx->pool);

  /* setup initial state (pool + key) */
  uint32_t i;
  for (i = 0; i < ASSH_PRNG_XSWAP_SIZE; i++)
    ctx->pool[i] = i;

  ctx->key[0] = 0x243f6a88;  /* pi */
  ctx->key[1] = 0x85a308d3;
  ctx->key[2] = 0x13198a2e;
  ctx->key[3] = 0x03707344;

  ctx->cnt = 0;
  ctx->i = 0;

  return ASSH_OK;
}

static ASSH_PRNG_GET_FCN(assh_prng_xswap_get)
{
  struct assh_prng_pv_s *ctx = c->prng_pv;
  union {
    uint8_t bytes[4];
    uint32_t word;
  } x;

  if (quality == ASSH_PRNG_QUALITY_WEAK)
    {
      while (rdata_len--)
	*rdata++ = rand();
      return ASSH_OK;
    }

  c->prng_entropy -= rdata_len;

  while (rdata_len > 0)
    {
      x.word = assh_prng_xswap_get32(ctx);

      unsigned int n, l = ASSH_MIN(sizeof(x.word), rdata_len);
      for (n = 0; n < l; n++)
	*rdata++ = x.bytes[n];
      rdata_len -= l;
    }

  return ASSH_OK;
}

static ASSH_PRNG_FEED_FCN(assh_prng_xswap_feed)
{
  struct assh_prng_pv_s *ctx = c->prng_pv;
  union {
    uint8_t bytes[4];
    uint32_t word;
  } x;

  c->prng_entropy += rdata_len & ~(size_t)3;
  uint32_t i = ctx->i;

  while (rdata_len > 0)
    {
      x.word = 0;
      unsigned int n, l = ASSH_MIN(sizeof(x.word), rdata_len);
      for (n = 0; n < l; n++)
        x.bytes[n] = *rdata++;
      rdata_len -= l;

      ctx->pool[i & (ASSH_PRNG_XSWAP_SIZE - 1)] ^= x.word;
      ctx->key[i & 3] ^= x.word;
      i += l / sizeof(x.word);
    }

  ctx->i = i;

  return ASSH_OK;
}

static ASSH_PRNG_CLEANUP_FCN(assh_prng_xswap_cleanup)
{
  struct assh_prng_pv_s *ctx = c->prng_pv;
  assh_free(c, ctx, ASSH_ALLOC_SECUR);
}

const struct assh_prng_s assh_prng_xswap = 
{
  .f_init = assh_prng_xswap_init,
  .f_get = assh_prng_xswap_get,
  .f_feed = assh_prng_xswap_feed,
  .f_cleanup = assh_prng_xswap_cleanup,  
};

