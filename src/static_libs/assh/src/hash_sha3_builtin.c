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

#include <assh/assh_hash.h>
#include <assh/assh_packet.h>

#include <string.h>

struct assh_hash_sha3_context_s
{
  struct assh_hash_ctx_s ctx;
  const struct assh_hash_algo_s *algo;
  uint64_t a[25];
  uint8_t buf[200];
  uint8_t padding;
  uint_fast8_t i;
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_sha3_context_s, ctx);

static inline uint64_t keccak_rotate(uint64_t x, uint8_t n)
{
  return (x << n) | (x >> (64 - n));
}

#define ASSH_KECCAK_WRAP(i) ((i) % 5)

#define ASSH_KECCAK_WRAP2(i, j) (ASSH_KECCAK_WRAP(i) + 5 * ASSH_KECCAK_WRAP(j))

#if defined(__OPTIMIZE_SIZE__)
# define ASSH_KECCAK_LOOP(var, ...)		\
  for (var = 0; var < 5; var++)			\
    { __VA_ARGS__ }
#else
# define ASSH_KECCAK_LOOP(var, ...)		\
  do {						\
    var = 0; { __VA_ARGS__ }			\
    var = 1; { __VA_ARGS__ }			\
    var = 2; { __VA_ARGS__ }			\
    var = 3; { __VA_ARGS__ }			\
    var = 4; { __VA_ARGS__ }			\
  } while (0)
#endif

static void assh_keccak(struct assh_hash_sha3_context_s *ctx)
{
  static const uint64_t rc[24] =
    {
      0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
      0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
      0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
      0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
      0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
      0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
      0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
      0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL,
    };

  static const uint8_t r[25] =
    {
       0,  1, 62, 28, 27,
      36, 44,  6, 55, 20,
       3, 10, 43, 25, 39,
      41, 45, 15, 21,  8,
      18,  2, 61, 56, 14
    };

  uint_fast8_t n, i, j, k;

  for (n = 0; n < 24; n++)
    {
      uint64_t c[5], b[25];

      ASSH_KECCAK_LOOP(i, {
	c[i] = ctx->a[ASSH_KECCAK_WRAP2(i, 0)] ^ ctx->a[ASSH_KECCAK_WRAP2(i, 1)]
	     ^ ctx->a[ASSH_KECCAK_WRAP2(i, 2)] ^ ctx->a[ASSH_KECCAK_WRAP2(i, 3)]
             ^ ctx->a[ASSH_KECCAK_WRAP2(i, 4)];
      });

      ASSH_KECCAK_LOOP(i, {
	  uint64_t d = c[ASSH_KECCAK_WRAP(i + 4)]
	             ^ keccak_rotate(c[ASSH_KECCAK_WRAP(i + 1)], 1);
	  ASSH_KECCAK_LOOP(j, {
              ctx->a[ASSH_KECCAK_WRAP2(i, j)] ^= d;
	  });
      });

      ASSH_KECCAK_LOOP(i, {
	  ASSH_KECCAK_LOOP(j, {
              k = ASSH_KECCAK_WRAP2(i, j);
              b[ASSH_KECCAK_WRAP2(j, i * 2 + j * 3)] = keccak_rotate(ctx->a[k], r[k]);
	  });
      });

      ASSH_KECCAK_LOOP(i, {
	  ASSH_KECCAK_LOOP(j, {
              k = ASSH_KECCAK_WRAP2(i, j);
              ctx->a[k] = b[k] ^ (~b[ASSH_KECCAK_WRAP2(i + 1, j)]
                                 & b[ASSH_KECCAK_WRAP2(i + 2, j)]);
	  });
      });

      ctx->a[0] ^= rc[n];
    }
}

static void assh_keccak_xorin(struct assh_hash_sha3_context_s *ctx)
{
  uint_fast8_t i;

  for (i = 0; i < ctx->ctx.algo->block_size / 8; i++)
    ctx->a[i] ^= assh_load_u64le(ctx->buf + i * 8);

  assh_keccak(ctx);
}

static ASSH_HASH_INIT_FCN(assh_sha3_init)
{
  struct assh_hash_sha3_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  ctx->algo = algo;
  ctx->i = 0;
  ctx->padding = algo->hash_size ? 0x06 : 0x1f;

  for (i = 0; i < 25; i++)
    ctx->a[i] = 0;

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha3_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha3_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha3_update)
{
  struct assh_hash_sha3_context_s *ctx = (void*)hctx;
  const uint8_t *d = data;

  assert(ctx->padding);

  uint_fast8_t i = ctx->i;
  uint_fast8_t bs = ctx->algo->block_size;

  while (len--)
    {
      ctx->buf[i++] = *d++;

      if (i == bs)
	{
	  assh_keccak_xorin(ctx);
	  i = 0;
	}
    }

  ctx->i = i;
}

static ASSH_HASH_FINAL_FCN(assh_sha3_final)
{
  struct assh_hash_sha3_context_s *ctx = (void*)hctx;

  uint_fast8_t i = ctx->i;
  uint_fast8_t bs = ctx->algo->block_size;

  assert(!ctx->algo->hash_size || ctx->algo->hash_size == len);

  if (ctx->padding)
    {
      if (i == bs - 1)
        {
          ctx->buf[i++] = ctx->padding | 0x80;
          assh_keccak_xorin(ctx);
        }
      else
        {
          ctx->buf[i++] = ctx->padding;
          while (i < bs - 1)
            ctx->buf[i++] = 0;
          ctx->buf[i++] = 0x80;
          assh_keccak_xorin(ctx);
        }

      i = 0;
      ctx->padding = 0;
    }

  while (len--)
    {
      *hash++ = ctx->a[i >> 3] >> ((i & 7) * 8);
      if (++i == bs)
	{
	  assh_keccak(ctx);
	  i = 0;
	}
    }

  ctx->i = i;
}

const struct assh_hash_algo_s assh_hash_sha3_224 =
{
  .name = "sha3-224",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .hash_size = 28,
  .block_size = 144,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};


const struct assh_hash_algo_s assh_hash_sha3_256 =
{
  .name = "sha3-256",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .hash_size = 32,
  .block_size = 136,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};


const struct assh_hash_algo_s assh_hash_sha3_384 =
{
  .name = "sha3-384",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .hash_size = 48,
  .block_size = 104,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};


const struct assh_hash_algo_s assh_hash_sha3_512 =
{
  .name = "sha3-512",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .hash_size = 64,
  .block_size = 72,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};


const struct assh_hash_algo_s assh_hash_shake_128 =
{
  .name = "shake128",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .block_size = 168,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};


const struct assh_hash_algo_s assh_hash_shake_256 =
{
  .name = "shake256",
  .ctx_size = sizeof(struct assh_hash_sha3_context_s),
  .block_size = 136,
  .f_init = assh_sha3_init,
  .f_copy = assh_sha3_copy,
  .f_update = assh_sha3_update,
  .f_final = assh_sha3_final,
};

