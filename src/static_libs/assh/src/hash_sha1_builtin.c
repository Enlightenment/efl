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

/*
  based on public domain SHA-1 in C
  by Steve Reid <steve@edmweb.com>
*/

#include <string.h>
#include <arpa/inet.h>

struct assh_hash_sha1_context_s
{
  struct assh_hash_ctx_s ctx;
  uint32_t state[5];
  uint64_t count;
  uint8_t buffer[64];
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_sha1_context_s, ctx);

static inline uint32_t rol(uint32_t value, unsigned int bits)
{
  return (((value) << (bits)) | ((value) >> (32 - (bits))));
}

/* BLK0() and BLK() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

#define BLK0(i) (block[i])
#define BLK(i) (block[i&15] = rol(block[(i + 13)&15] ^ block[(i + 8)&15] \
                ^ block[(i + 2)&15] ^ block[i&15], 1))

/* (R0 + R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v, w, x, y, z, i) do { z += ((w&(x ^ y)) ^ y) + BLK0(i) + 0x5A827999 + rol(v, 5);w = rol(w, 30); } while (0)
#define R1(v, w, x, y, z, i) do { z += ((w&(x ^ y)) ^ y) + BLK(i) + 0x5A827999 + rol(v, 5);w = rol(w, 30); } while (0)
#define R2(v, w, x, y, z, i) do { z += (w ^ x ^ y) + BLK(i) + 0x6ED9EBA1 + rol(v, 5);w = rol(w, 30); } while (0)
#define R3(v, w, x, y, z, i) do { z += (((w|x)&y)|(w&x)) + BLK(i) + 0x8F1BBCDC + rol(v, 5);w = rol(w, 30); } while (0)
#define R4(v, w, x, y, z, i) do { z += (w ^ x ^ y) + BLK(i) + 0xCA62C1D6 + rol(v, 5);w = rol(w, 30); } while (0)


/* Hash a single 512-bit block. This is the core of the algorithm. */
static void assh_sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
  uint32_t a, b, c, d, e, x;
  uint32_t block[16];
  uint_fast8_t i;

  for (i = 0; i < 16; i++)
    block[i] = assh_load_u32(buffer + i * 4);

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  /* 4 rounds of 20 operations each. */  

  for (i = 0; i <= 15; i++)
    {
      R0(a, b, c, d, e, i);
      (x = a), (a = e), (e = d), (d = c), (c = b), (b = x);
    }
  for (; i <= 19; i++)
    {
      R1(a, b, c, d, e, i);
      (x = a), (a = e), (e = d), (d = c), (c = b), (b = x);
    }
  for (; i <= 39; i++)
    {
      R2(a, b, c, d, e, i);
      (x = a), (a = e), (e = d), (d = c), (c = b), (b = x);
    }
  for (; i <= 59; i++)
    {
      R3(a, b, c, d, e, i);
      (x = a), (a = e), (e = d), (d = c), (c = b), (b = x);
    }
  for (; i <= 79; i++)
    {
      R4(a, b, c, d, e, i);
      (x = a), (a = e), (e = d), (d = c), (c = b), (b = x);
    }

  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}


static ASSH_HASH_INIT_FCN(assh_sha1_init)
{
  struct assh_hash_sha1_context_s *ctx = (void*)hctx;

  /* SHA1 initialization constants */
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xc3d2e1f0;
  ctx->count = 0;

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha1_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha1_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha1_update)
{
  struct assh_hash_sha1_context_s *ctx = (void*)hctx;
  const uint8_t *data_ = data;
  uint32_t i, j;

  j = (ctx->count >> 3) & 63;
  ctx->count += len * 8;

  if ((j + len) > 63)
    {
      i = 64 - j;
      memcpy(&ctx->buffer[j], data_, i);

      assh_sha1_transform(ctx->state, ctx->buffer);
      for ( ; i + 63 < len; i += 64)
	assh_sha1_transform(ctx->state, &data_[i]);

      j = 0;
    }
  else
    {
      i = 0;
    }

  memcpy(&ctx->buffer[j], &data_[i], len - i);
}


static ASSH_HASH_FINAL_FCN(assh_sha1_final)
{
  struct assh_hash_sha1_context_s *ctx = (void*)hctx;

  assert(len == 20);

  unsigned int i;
  uint8_t count[8];
  uint8_t c;

  if (hash == NULL)
    return;

  assh_store_u64(count, ctx->count);

  c = 0200;
  assh_sha1_update(hctx, &c, 1);
  while ((ctx->count & 504) != 448)
    {
      c = 0000;
      assh_sha1_update(hctx, &c, 1);
    }

  assh_sha1_update(hctx, count, 8);
  for (i = 0; i < 20; i++)
    hash[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8) ) & 0xff);
}

const struct assh_hash_algo_s assh_hash_sha1 = 
{
  .name = "sha1",
  .ctx_size = sizeof(struct assh_hash_sha1_context_s),
  .hash_size = 20,
  .block_size = 64,
  .f_init = assh_sha1_init,
  .f_copy = assh_sha1_copy,
  .f_update = assh_sha1_update,
  .f_final = assh_sha1_final,
};

