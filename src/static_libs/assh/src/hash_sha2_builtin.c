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

struct assh_hash_sha512_context_s
{
  struct assh_hash_ctx_s ctx;
  uint64_t count;
  uint64_t state[8];
  uint8_t buffer[128];
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_sha512_context_s, ctx);

struct assh_hash_sha256_context_s
{
  struct assh_hash_ctx_s ctx;
  uint64_t count;
  uint32_t state[8];
  uint8_t buffer[64];
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_sha256_context_s, ctx);

/*
 *  FIPS-180-2 compliant SHA2 implementation
 *  based on PUBLIC DOMAIN sha256 by Christophe Devine
 */

static inline uint32_t ror32(uint32_t value, unsigned int bits)
{
  return (((value) >> (bits)) | ((value) << (32 - (bits))));
}

static inline uint64_t ror64(uint64_t value, unsigned int bits)
{
  return (((value) >> (bits)) | ((value) << (64 - (bits))));
}


#define S256_0(x) (ror32(x,  7) ^ ror32(x, 18) ^ (x >> 3))
#define S256_1(x) (ror32(x, 17) ^ ror32(x, 19) ^ (x >> 10))

#define S256_2(x) (ror32(x,  2) ^ ror32(x, 13) ^ ror32(x, 22))
#define S256_3(x) (ror32(x,  6) ^ ror32(x, 11) ^ ror32(x, 25))

#define S512_0(x) (ror64(x,  1) ^ ror64(x, 8) ^ (x >> 7))
#define S512_1(x) (ror64(x, 19) ^ ror64(x, 61) ^ (x >> 6))

#define S512_2(x) (ror64(x,  28) ^ ror64(x, 34) ^ ror64(x, 39))
#define S512_3(x) (ror64(x,  14) ^ ror64(x, 18) ^ ror64(x, 41))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define F1(x,y,z) (z ^ (x & (y ^ z)))

#define R(A, t)					\
(                                               \
    w[t] = S##A##_1(w[t -  2]) + w[t -  7] +          \
	   S##A##_0(w[t - 15]) + w[t - 16]            \
)

#define P(A, a, b, c, d, e, f, g, h, x, K)	\
{                                               \
    temp1 = h + S##A##_3(e) + F1(e,f,g) + K + x;      \
    temp2 = S##A##_2(a) + F0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
}

static const uint8_t assh_sha2_padding[16 + 1] = { 0x80 };

static void sha256_process(struct assh_hash_sha256_context_s *ctx,
			   const uint8_t data[64])
{
  uint32_t temp1, temp2, w[64];
  uint_fast8_t i;

  for (i = 0; i < 16; i++)
    w[i] = assh_load_u32(data + i * 4);

  uint32_t a = ctx->state[0];
  uint32_t b = ctx->state[1];
  uint32_t c = ctx->state[2];
  uint32_t d = ctx->state[3];
  uint32_t e = ctx->state[4];
  uint32_t f = ctx->state[5];
  uint32_t g = ctx->state[6];
  uint32_t h = ctx->state[7];

  static const uint32_t keys[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
  };

  for (i = 0; i < 16; i++)
    {
      P(256, a, b, c, d, e, f, g, h, w[i], keys[i]);
      uint32_t x = h;
      (h = g), (g = f), (f = e), (e = d), (d = c), (c = b), (b = a), (a = x);
    }

  for (; i < 64; i++)
    {
      P(256, a, b, c, d, e, f, g, h, R(256, i), keys[i]);
      uint32_t x = h;
      (h = g), (g = f), (f = e), (e = d), (d = c), (c = b), (b = a), (a = x);
    }

  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
}

static void sha512_process(struct assh_hash_sha512_context_s *ctx,
			   const uint8_t data[128])
{
  uint64_t temp1, temp2, w[80];
  uint_fast8_t i;

  for (i = 0; i < 16; i++)
    w[i] = assh_load_u64(data + i * 8);

  uint64_t a = ctx->state[0];
  uint64_t b = ctx->state[1];
  uint64_t c = ctx->state[2];
  uint64_t d = ctx->state[3];
  uint64_t e = ctx->state[4];
  uint64_t f = ctx->state[5];
  uint64_t g = ctx->state[6];
  uint64_t h = ctx->state[7];

  static const uint64_t keys[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538,
    0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe,
    0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235,
    0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab,
    0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
    0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed,
    0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218,
    0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53,
    0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373,
    0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c,
    0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6,
    0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc,
    0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
  };

  for (i = 0; i < 16; i++)
    {
      P(512, a, b, c, d, e, f, g, h, w[i], keys[i]);
      uint64_t x = h;
      (h = g), (g = f), (f = e), (e = d), (d = c), (c = b), (b = a), (a = x);
    }

  for (; i < 80; i++)
    {
      P(512, a, b, c, d, e, f, g, h, R(512, i), keys[i]);
      uint64_t x = h;
      (h = g), (g = f), (f = e), (e = d), (d = c), (c = b), (b = a), (a = x);
    }

  ctx->state[0] += a;
  ctx->state[1] += b;
  ctx->state[2] += c;
  ctx->state[3] += d;
  ctx->state[4] += e;
  ctx->state[5] += f;
  ctx->state[6] += g;
  ctx->state[7] += h;
}

static void assh_sha256_update_(struct assh_hash_sha256_context_s *ctx,
				const void *data, size_t len)
{
  uint32_t left, fill;
  const uint8_t *input = data;

  if (len == 0)
    return;

  left = ctx->count & 0x3f;
  fill = 64 - left;

  ctx->count += len;

  if (left && len >= fill)
    {
      memcpy((ctx->buffer + left), input, fill);
      sha256_process(ctx, ctx->buffer);
      len -= fill;
      input += fill;
      left = 0;
    }

  while (len >= 64)
    {
      sha256_process(ctx, input);
      len -= 64;
      input += 64;
    }

  if (len)
    memcpy((ctx->buffer + left), input, len);
}

static void assh_sha256_final_(struct assh_hash_sha256_context_s *ctx)
{
  uint32_t last, padn;
  uint8_t msglen[8];

  assh_store_u64(msglen, ctx->count * 8);

  last = ctx->count & 63;
  padn = (last < 56) ? (56 - last) : (120 - last);

  const uint8_t *pad = assh_sha2_padding;
  while (padn > 16)
    {
      assh_sha256_update_(ctx, pad, 16);
      pad = assh_sha2_padding + 1;
      padn -= 16;
    }
  assh_sha256_update_(ctx, pad, padn);
  assh_sha256_update_(ctx, msglen, 8);
}

static void assh_sha512_update_(struct assh_hash_sha512_context_s *ctx,
				const void *data, size_t len)
{
  uint64_t left, fill;
  const uint8_t *input = data;

  if (len == 0)
    return;

  left = ctx->count & 0x7f;
  fill = 128 - left;

  ctx->count += len;

  if (left && len >= fill)
    {
      memcpy((ctx->buffer + left), input, fill);
      sha512_process(ctx, ctx->buffer);
      len -= fill;
      input += fill;
      left = 0;
    }

  while (len >= 128)
    {
      sha512_process(ctx, input);
      len -= 128;
      input += 128;
    }

  if (len)
    memcpy((ctx->buffer + left), input, len);
}

static void assh_sha512_final_(struct assh_hash_sha512_context_s *ctx)
{
  uint64_t last, padn;
  uint8_t msglen[16];

  assh_store_u64(msglen, 0);
  assh_store_u64(msglen + 8, ctx->count * 8);

  last = ctx->count & 127;
  padn = (last < 112) ? (112 - last) : (240 - last);

  const uint8_t *pad = assh_sha2_padding;
  while (padn > 16)
    {
      assh_sha512_update_(ctx, pad, 16);
      pad = assh_sha2_padding + 1;
      padn -= 16;
    }
  assh_sha512_update_(ctx, pad, padn);
  assh_sha512_update_(ctx, msglen, 16);
}




static ASSH_HASH_INIT_FCN(assh_sha224_init)
{
  struct assh_hash_sha256_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  ctx->count = 0;

  static const uint32_t init[8] = {
    0xc1059ed8,  0x367cd507,  0x3070dd17,  0xf70e5939,
    0xffc00b31,  0x68581511,  0x64f98fa7,  0xbefa4fa4,
  };

  for (i = 0; i < 8; i++)
    ctx->state[i] = init[i];

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha224_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha256_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha224_update)
{
  assh_sha256_update_((void*)hctx, data, len);
}

static ASSH_HASH_FINAL_FCN(assh_sha224_final)
{
  struct assh_hash_sha256_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  assert(len == 28);

  assh_sha256_final_(ctx);

  if (hash != NULL)
    for (i = 0; i < 7; i++)
      assh_store_u32(hash + i * 4, ctx->state[i]);
}

const struct assh_hash_algo_s assh_hash_sha224 =
{
  .name = "sha224",
  .ctx_size = sizeof(struct assh_hash_sha256_context_s),
  .hash_size = 28,
  .block_size = 64,
  .f_init = assh_sha224_init,
  .f_copy = assh_sha224_copy,
  .f_update = assh_sha224_update,
  .f_final = assh_sha224_final,
};



static ASSH_HASH_INIT_FCN(assh_sha256_init)
{
  struct assh_hash_sha256_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  ctx->count = 0;

  static const uint32_t init[8] = {
    0x6a09e667,  0xbb67ae85,  0x3c6ef372,  0xa54ff53a,
    0x510e527f,  0x9b05688c,  0x1f83d9ab,  0x5be0cd19,
  };

  for (i = 0; i < 8; i++)
    ctx->state[i] = init[i];

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha256_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha256_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha256_update)
{
  assh_sha256_update_((void*)hctx, data, len);
}

static ASSH_HASH_FINAL_FCN(assh_sha256_final)
{
  struct assh_hash_sha256_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  assert(len == 32);

  assh_sha256_final_(ctx);

  if (hash != NULL)
    for (i = 0; i < 8; i++)
      assh_store_u32(hash + i * 4, ctx->state[i]);
}

const struct assh_hash_algo_s assh_hash_sha256 =
{
  .name = "sha256",
  .ctx_size = sizeof(struct assh_hash_sha256_context_s),
  .hash_size = 32,
  .block_size = 64,
  .f_init = assh_sha256_init,
  .f_copy = assh_sha256_copy,
  .f_update = assh_sha256_update,
  .f_final = assh_sha256_final,
};


static ASSH_HASH_INIT_FCN(assh_sha384_init)
{
  struct assh_hash_sha512_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  ctx->count = 0;

  static const uint64_t init[8] = {
    0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,
    0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4,
  };

  for (i = 0; i < 8; i++)
    ctx->state[i] = init[i];

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha384_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha512_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha384_update)
{
  assh_sha512_update_((void*)hctx, data, len);
}

static ASSH_HASH_FINAL_FCN(assh_sha384_final)
{
  struct assh_hash_sha512_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  assert(len == 48);

  assh_sha512_final_(ctx);

  if (hash != NULL)
    for (i = 0; i < 6; i++)
      assh_store_u64(hash + i * 8, ctx->state[i]);
}

const struct assh_hash_algo_s assh_hash_sha384 =
{
  .name = "sha384",
  .ctx_size = sizeof(struct assh_hash_sha512_context_s),
  .hash_size = 48,
  .block_size = 128,
  .f_init = assh_sha384_init,
  .f_copy = assh_sha384_copy,
  .f_update = assh_sha384_update,
  .f_final = assh_sha384_final,
};


static ASSH_HASH_INIT_FCN(assh_sha512_init)
{
  struct assh_hash_sha512_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  ctx->count = 0;

  static const uint64_t init[8] = {
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
  };

  for (i = 0; i < 8; i++)
    ctx->state[i] = init[i];

  return ASSH_OK;
}

static ASSH_HASH_COPY_FCN(assh_sha512_copy)
{
  memcpy(hctx_dst, hctx_src, sizeof(struct assh_hash_sha512_context_s));

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_sha512_update)
{
  assh_sha512_update_((void*)hctx, data, len);
}

static ASSH_HASH_FINAL_FCN(assh_sha512_final)
{
  struct assh_hash_sha512_context_s *ctx = (void*)hctx;
  uint_fast8_t i;

  assert(len == 64);

  assh_sha512_final_(ctx);

  if (hash != NULL)
    for (i = 0; i < 8; i++)
      assh_store_u64(hash + i * 8, ctx->state[i]);
}

const struct assh_hash_algo_s assh_hash_sha512 =
{
  .name = "sha512",
  .ctx_size = sizeof(struct assh_hash_sha512_context_s),
  .hash_size = 64,
  .block_size = 128,
  .f_init = assh_sha512_init,
  .f_copy = assh_sha512_copy,
  .f_update = assh_sha512_update,
  .f_final = assh_sha512_final,
};
