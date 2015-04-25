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
#include <assh/assh_packet.h>

struct assh_cipher_aes_context_s
{
  union {
    uint32_t    rkey[15][4];
    uint32_t	key[8];
  };
  uint32_t    iv[4];
  uint8_t     rounds;
  assh_bool_t encrypt;
};

static inline void aes_subst_bytes(void *b, uint_fast8_t s)
{
  static const uint8_t aes_sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,  0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,  0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,  0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,  0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,  0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,  0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,  0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,  0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,  0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,  0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,  0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,  0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,  0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,  0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,  0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,  0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
  };

  uint8_t *buf = b;

  while (s--)
    buf[s] = aes_sbox[buf[s]];
}

static inline void aes_unsubst_bytes(void *b)
{
  static const uint8_t aes_sbox_inv[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,  0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,  0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,  0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,  0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,  0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,  0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,  0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,  0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,  0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,  0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,  0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,  0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,  0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,  0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,  0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,  0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
  };

  uint8_t *buf = b;
  uint_fast8_t s = 16;

  while (s--)
    buf[s] = aes_sbox_inv[buf[s]];
}

static inline void aes_add_round_key(uint32_t b[4], uint32_t k[4])
{
  b[0] ^= k[0];
  b[1] ^= k[1];
  b[2] ^= k[2];
  b[3] ^= k[3];
}

static inline void aes_shift_rows(uint32_t b[4])
{
  uint32_t b1 = b[1];
  uint32_t b2 = b[2];
  uint32_t b3 = b[3];

  b[1] = (b1 << 24) | (b1 >> 8 );
  b[2] = (b2 << 16) | (b2 >> 16);
  b[3] = (b3 << 8 ) | (b3 >> 24);
}

static inline void aes_unshift_rows(uint32_t b[4])
{
  uint32_t b1 = b[1];
  uint32_t b2 = b[2];
  uint32_t b3 = b[3];

  b[1] = (b1 << 8 ) | (b1 >> 24);
  b[2] = (b2 << 16) | (b2 >> 16);
  b[3] = (b3 << 24) | (b3 >> 8 );
}

static inline void aes_transpose(uint32_t b[4])
{
  uint32_t b0 = b[0];
  uint32_t b1 = b[1];
  uint32_t b2 = b[2];
  uint32_t b3 = b[3];

  uint32_t a0 = ((b0 & 0x000000ff)      ) | ((b1 & 0x000000ff) << 8 )
              | ((b2 & 0x000000ff) << 16) | ((b3 & 0x000000ff) << 24);
  uint32_t a1 = ((b0 & 0x0000ff00) >> 8 ) | ((b1 & 0x0000ff00)      )
              | ((b2 & 0x0000ff00) << 8 ) | ((b3 & 0x0000ff00) << 16);
  uint32_t a2 = ((b0 & 0x00ff0000) >> 16) | ((b1 & 0x00ff0000) >> 8 )
              | ((b2 & 0x00ff0000)      ) | ((b3 & 0x00ff0000) << 8 );
  uint32_t a3 = ((b0 & 0xff000000) >> 24) | ((b1 & 0xff000000) >> 16)
              | ((b2 & 0xff000000) >> 8 ) | ((b3 & 0xff000000)      );

  b[0] = a0;
  b[1] = a1;
  b[2] = a2;
  b[3] = a3;
}

static inline uint32_t aes_simd_x2(uint32_t x) 
{
  uint32_t z = (x & 0x80808080) >> 7;
  uint32_t y = (x & 0x7f7f7f7f) << 1;
  z |= z << 3;
  return y ^ z ^ (z << 1);
}

static inline uint32_t aes_simd_x4(uint32_t x) 
{
  uint32_t z = (x & 0xc0c0c0c0) >> 6;
  uint32_t y = (x & 0x3f3f3f3f) << 2;
  z |= z << 3;
  return y ^ z ^ (z << 1);
}

static inline void aes_mix_columns(uint32_t b[4])
{
  uint32_t b0 = b[0];
  uint32_t b1 = b[1];
  uint32_t b2 = b[2];
  uint32_t b3 = b[3];
  uint32_t x = b0 ^ b1 ^ b2 ^ b3;
  b[0] ^= x ^ aes_simd_x2(b0 ^ b1);
  b[1] ^= x ^ aes_simd_x2(b1 ^ b2);
  b[2] ^= x ^ aes_simd_x2(b2 ^ b3);
  b[3] ^= x ^ aes_simd_x2(b3 ^ b0);
}

static inline void aes_unmix_columns(uint32_t b[4])
{
  uint32_t b0 = b[0];
  uint32_t b1 = b[1];
  uint32_t b2 = b[2];
  uint32_t b3 = b[3];
  uint32_t w = b0 ^ b1 ^ b2 ^ b3;
  uint32_t z = aes_simd_x2(w);
  uint32_t y = w ^ aes_simd_x4(z ^ b1 ^ b3);
  uint32_t x = w ^ aes_simd_x4(z ^ b0 ^ b2);
  b[0] ^= x ^ aes_simd_x2(b0 ^ b1);
  b[1] ^= y ^ aes_simd_x2(b1 ^ b2);
  b[2] ^= x ^ aes_simd_x2(b2 ^ b3);
  b[3] ^= y ^ aes_simd_x2(b3 ^ b0);
}

static void aes_expand_key(uint32_t *b, uint_fast8_t c, uint_fast8_t r)
{
  uint8_t rcon = 1;
  uint_fast8_t i, j;

  for (j = c; j < 4 * r; j += c)
    {
      uint32_t b3 = b[j - 1];
      b3 = (b3 << 24) | (b3 >> 8);
      aes_subst_bytes(&b3, 4);
      b[j] = b3 ^ b[j - c] ^ rcon;

      for (i = 1; i < c; i++)
        {
          if (i + j == 4 * r)
            return;
          uint32_t x = b[i + j - 1];
          if (c == 8 && i == 4)
            aes_subst_bytes(&x, 4);
          b[i + j] = x ^ b[i + j - c];
        }

      rcon = (rcon << 1) | (((int8_t)(rcon & 0x80) >> 7) & 0x1b);
    }
}

static void aes_encrypt(struct assh_cipher_aes_context_s *ctx, uint32_t b[4])
{
  uint_fast8_t i;

  aes_transpose(b);
  aes_add_round_key(b, ctx->rkey[0]);

  for (i = 1; i < ctx->rounds - 1; i++)
    {
      aes_subst_bytes(b, 16);
      aes_shift_rows(b);
      aes_mix_columns(b);
      aes_add_round_key(b, ctx->rkey[i]);
    }

  aes_subst_bytes(b, 16);
  aes_shift_rows(b);
  aes_add_round_key(b, ctx->rkey[i]);
  aes_transpose(b);
}

static void aes_decrypt(struct assh_cipher_aes_context_s *ctx, uint32_t b[4])
{
  uint_fast8_t i = ctx->rounds - 1;

  aes_transpose(b);
  aes_add_round_key(b, ctx->rkey[i]);
  aes_unshift_rows(b);
  aes_unsubst_bytes(b);

  while (--i > 0)
    {
      aes_add_round_key(b, ctx->rkey[i]);
      aes_unmix_columns(b);
      aes_unshift_rows(b);
      aes_unsubst_bytes(b);
    }

  aes_add_round_key(b, ctx->rkey[0]); 
  aes_transpose(b);
}

static void assh_aes_init(struct assh_cipher_aes_context_s *ctx,
			  uint_fast8_t r, uint_fast8_t l,
			  const uint8_t *key, const uint8_t *iv,
			  assh_bool_t encrypt)
{
  uint_fast8_t i;

  ctx->rounds = r;
  ctx->encrypt = encrypt;

  for (i = 0; i < l; i++)
    ctx->key[i] = assh_load_u32le(key + i * 4);

  aes_expand_key(ctx->rkey[0], l, r);
  for (i = 0; i < r; i++)
    aes_transpose(ctx->rkey[i]);

  for (i = 0; i < 4; i++)
    ctx->iv[i] = assh_load_u32le(iv + i * 4);
}

static ASSH_CIPHER_INIT_FCN(assh_aes128_init)
{
  assh_aes_init(ctx_, 11, 4, key, iv, encrypt);
  return ASSH_OK;
}

static ASSH_CIPHER_INIT_FCN(assh_aes192_init)
{
  assh_aes_init(ctx_, 13, 6, key, iv, encrypt);
  return ASSH_OK;
}

static ASSH_CIPHER_INIT_FCN(assh_aes256_init)
{
  assh_aes_init(ctx_, 15, 8, key, iv, encrypt);
  return ASSH_OK;
}

static ASSH_CIPHER_PROCESS_FCN(assh_aes_cbc_process)
{
  struct assh_cipher_aes_context_s *ctx = ctx_;
  assh_error_t err;
  uint32_t b[4];
  uint_fast8_t i;

  if (ctx->encrypt)
    {
      uint32_t *iv = ctx->iv;
      for (; len >= 16; len -= 16)
	{
	  for (i = 0; i < 4; i++)
	    b[i] = iv[i] ^ assh_load_u32le(data + i * 4);
	  aes_encrypt(ctx, b);
	  for (i = 0; i < 4; i++)
	    assh_store_u32le(data + i * 4, b[i]);
	  iv = b;
	  data += 16;
	}
      for (i = 0; i < 4; i++)
	ctx->iv[i] = iv[i];
    }
  else
    {
      uint32_t t[4];
      for (; len >= 16; len -= 16)
	{
	  for (i = 0; i < 4; i++)
	    t[i] = b[i] = assh_load_u32le(data + i * 4);
	  aes_decrypt(ctx, b);
	  for (i = 0; i < 4; i++)
	    {
	      assh_store_u32le(data + i * 4, b[i] ^ ctx->iv[i]);
	      ctx->iv[i] = t[i];
	    }
	  data += 16;
	}
    }

  ASSH_CHK_RET(len != 0, ASSH_ERR_BAD_ARG);
  return ASSH_OK;
}

static ASSH_CIPHER_PROCESS_FCN(assh_aes_ctr_process)
{
  struct assh_cipher_aes_context_s *ctx = ctx_;
  assh_error_t err;
  uint32_t b[4];
  uint32_t *iv = ctx->iv;
  uint_fast8_t i;

  for ( ;len >= 16; len -= 16)
    {
      for (i = 0; i < 4; i++)
	b[i] = iv[i];
      aes_encrypt(ctx, b);

      for (i = 0; i < 128; i += 8)
	*data++ ^= b[i / 32] >> (i % 32);

      uint64_t t = 0x100000000ULL;
      for (i = 0; i < 4; i++)
	iv[3-i] = assh_swap_u32(t = (t >> 32) + assh_swap_u32(iv[3-i]));
    }

  ASSH_CHK_RET(len != 0, ASSH_ERR_BAD_ARG);
  return ASSH_OK;
}

static ASSH_CIPHER_CLEANUP_FCN(assh_aes_cleanup)
{
}

const struct assh_algo_cipher_s assh_cipher_aes128_cbc =
{
  .algo = { .name = "aes128-cbc", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 40, .speed = 70 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 16,
  .is_stream = 0,
  .f_init = assh_aes128_init,
  .f_process = assh_aes_cbc_process,
  .f_cleanup = assh_aes_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_aes192_cbc =
{
  .algo = { .name = "aes192-cbc", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 50, .speed = 65 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 24,
  .is_stream = 0,
  .f_init = assh_aes192_init,
  .f_process = assh_aes_cbc_process,
  .f_cleanup = assh_aes_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_aes256_cbc =
{
  .algo = { .name = "aes256-cbc", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 60, .speed = 60 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 32,
  .is_stream = 0,
  .f_init = assh_aes256_init,
  .f_process = assh_aes_cbc_process,
  .f_cleanup = assh_aes_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_aes128_ctr =
{
  .algo = { .name = "aes128-ctr", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 41, .speed = 70 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 16,
  .is_stream = 0,
  .f_init = assh_aes128_init,
  .f_process = assh_aes_ctr_process,
  .f_cleanup = assh_aes_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_aes192_ctr =
{
  .algo = { .name = "aes192-ctr", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 51, .speed = 65 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 24,
  .is_stream = 0,
  .f_init = assh_aes192_init,
  .f_process = assh_aes_ctr_process,
  .f_cleanup = assh_aes_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_aes256_ctr =
{
  .algo = { .name = "aes256-ctr", .class_ = ASSH_ALGO_CIPHER,
	    .safety = 61, .speed = 60 },
  .ctx_size = sizeof(struct assh_cipher_aes_context_s),
  .block_size = 16,
  .key_size = 32,
  .is_stream = 0,
  .f_init = assh_aes256_init,
  .f_process = assh_aes_ctr_process,
  .f_cleanup = assh_aes_cleanup,
};

