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

/* based on public domain code from Colin Plumb, 1993 */

#include <string.h>
#include <arpa/inet.h>

#define MD5_BLOCK_LENGTH 64

struct assh_hash_md5_context_s
{
  struct assh_hash_ctx_s ctx;
  uint32_t state[4];
  uint64_t count;
  uint8_t buffer[MD5_BLOCK_LENGTH];
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_md5_context_s, ctx);

static ASSH_HASH_INIT_FCN(assh_md5_init)
{
  struct assh_hash_md5_context_s *ctx = (void*)hctx;

  ctx->count = 0;
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_md5_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_md5_context_s));

  return ASSH_OK;
}

#define MD5F1(x, y, z) (z ^ (x & (y ^ z)))
#define MD5F2(x, y, z) MD5F1(z, x, y)
#define MD5F3(x, y, z) (x ^ y ^ z)
#define MD5F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s)		\
  do {						\
    w += MD5##f(x, y, z) + data;		\
    w = w<<s | w>>(32-s);			\
    w += x;					\
  } while (0)

static void
assh_md5_transform(uint32_t state[4], const uint8_t block[MD5_BLOCK_LENGTH])
{
  uint32_t a = state[0];
  uint32_t b = state[1];
  uint32_t c = state[2];
  uint32_t d = state[3];
  uint32_t x;
  uint_fast8_t i;

  static const uint32_t key[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
  };

  for (i = 0; i < 16; i++)
    {
      MD5STEP(F1, a, b, c, d, assh_load_u32le(block + i * 4) + key[i],
	      ((0x16110c07 >> ((i & 3) * 8)) & 0xff));
      (x = a), (a = d), (d = c), (c = b), (b = x);
    }

  for (; i < 32; i++)
    {
      MD5STEP(F2, a, b, c, d, assh_load_u32le(block + (20 * i + 4) % 64) + key[i],
	      ((0x140e0905 >> ((i & 3) * 8)) & 0xff));
      (x = a), (a = d), (d = c), (c = b), (b = x);
    }

  for (; i < 48; i++)
    {
      MD5STEP(F3, a, b, c, d, assh_load_u32le(block + (12 * i + 20) % 64) + key[i],
	      ((0x17100b04 >> ((i & 3) * 8)) & 0xff));
      (x = a), (a = d), (d = c), (c = b), (b = x);
    }

  for (; i < 64; i++)
    {
      MD5STEP(F4, a, b, c, d, assh_load_u32le(block + (28 * i) % 64) + key[i],
	      ((0x150f0a06 >> ((i & 3) * 8)) & 0xff));
      (x = a), (a = d), (d = c), (c = b), (b = x);
    }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}


static ASSH_HASH_UPDATE_FCN(assh_md5_update)
{
  struct assh_hash_md5_context_s *ctx = (void*)hctx;
  size_t have, need;

  have = (ctx->count / 8) % MD5_BLOCK_LENGTH;
  need = MD5_BLOCK_LENGTH - have;

  ctx->count += len * 8;

  if (len >= need)
    {
      /* Append to data from a previous call and transform 1 block. */
      if (have != 0)
	{
	  memcpy(ctx->buffer + have, data, need);
	  assh_md5_transform(ctx->state, ctx->buffer);
	  data += need;
	  len -= need;
	  have = 0;
	}

      /* Process data in MD5_BLOCK_LENGTH chunks. */
      while (len >= MD5_BLOCK_LENGTH)
	{
	  assh_md5_transform(ctx->state, data);
	  data += MD5_BLOCK_LENGTH;
	  len -= MD5_BLOCK_LENGTH;
	}
    }

  /* Keep any remaining bytes of data for next call. */
  if (len != 0)
    memcpy(ctx->buffer + have, data, len);
}

static const uint8_t assh_md5_padding[16 + 1] = { 0x80 };

static ASSH_HASH_FINAL_FCN(assh_md5_final)
{
  struct assh_hash_md5_context_s *ctx = (void*)hctx;

  assert(len == 16);

  uint8_t count[8];
  size_t padlen;
  int i;

  assh_store_u64le(count, ctx->count);

  /* Pad out to 56 mod 64. */
  padlen = MD5_BLOCK_LENGTH - (ctx->count / 8) % MD5_BLOCK_LENGTH;
  if (padlen < 1 + 8)
    padlen += MD5_BLOCK_LENGTH;
  /* padlen - 8 <= 64 */
  padlen -= 8;

  const uint8_t *pad = assh_md5_padding;
  while (padlen > 16)
    {
      assh_md5_update(hctx, pad, 16);
      pad = assh_md5_padding + 1;
      padlen -= 16;
    }
  assh_md5_update(hctx, pad, padlen);


  assh_md5_update(hctx, count, 8);

  if (hash != NULL)
      for (i = 0; i < 4; i++)
	assh_store_u32le(hash + i * 4, ctx->state[i]);
}

const struct assh_hash_algo_s assh_hash_md5 = 
{
  .name = "md5",
  .ctx_size = sizeof(struct assh_hash_md5_context_s),
  .hash_size = 16,
  .block_size = 64,
  .f_init = assh_md5_init,
  .f_copy = assh_md5_copy,
  .f_update = assh_md5_update,
  .f_final = assh_md5_final,
};

