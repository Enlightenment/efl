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

#include <gcrypt.h>

struct assh_cipher_gcrypt_context_s
{
  gcry_cipher_hd_t hd;
  assh_bool_t encrypt;
};

static assh_error_t
assh_cipher_gcrypt_init(const struct assh_algo_cipher_s *cipher,
			struct assh_cipher_gcrypt_context_s *ctx,
			const uint8_t *key, const uint8_t *iv,
			int algo, int mode, assh_bool_t encrypt)
{
  assh_error_t err;

  ASSH_CHK_RET(gcry_cipher_open(&ctx->hd, algo, mode, 0),
	       ASSH_ERR_CRYPTO);

  ASSH_CHK_GTO(gcry_cipher_setkey(ctx->hd, key, cipher->key_size),
	       ASSH_ERR_CRYPTO, err_open);

  ctx->encrypt = encrypt;

  switch (mode)
    {
    case GCRY_CIPHER_MODE_CBC:
      ASSH_CHK_GTO(gcry_cipher_setiv(ctx->hd, iv, cipher->block_size),
		   ASSH_ERR_CRYPTO, err_open);
      break;

    case GCRY_CIPHER_MODE_CTR:
      ASSH_CHK_GTO(gcry_cipher_setctr(ctx->hd, iv, cipher->block_size),
		   ASSH_ERR_CRYPTO, err_open);
      break;

    case GCRY_CIPHER_MODE_STREAM:
      assert(cipher->is_stream);

      if (cipher == &assh_cipher_arc4_128 ||
	  cipher == &assh_cipher_arc4_256)
	{
	  uint8_t dummy[128];
	  unsigned int i;

	  memset(dummy, 0, sizeof(dummy));
	  for (i = 0; i < 1536; i += sizeof(dummy))
	    if (encrypt)
	      ASSH_CHK_GTO(gcry_cipher_encrypt(ctx->hd, dummy, sizeof(dummy), NULL, 0),
			   ASSH_ERR_CRYPTO, err_open);
	    else
	      ASSH_CHK_GTO(gcry_cipher_decrypt(ctx->hd, dummy, sizeof(dummy), NULL, 0),
			   ASSH_ERR_CRYPTO, err_open);
	}
      break;
    }

  return ASSH_OK;

 err_open:
  gcry_cipher_close(ctx->hd);
  return err;
}

static ASSH_CIPHER_PROCESS_FCN(assh_cipher_gcrypt_process)
{
  assh_error_t err;
  struct assh_cipher_gcrypt_context_s *ctx = ctx_;

  if (ctx->encrypt)
    ASSH_CHK_RET(gcry_cipher_encrypt(ctx->hd, data, len, NULL, 0),
		 ASSH_ERR_CRYPTO);
  else
    ASSH_CHK_RET(gcry_cipher_decrypt(ctx->hd, data, len, NULL, 0),
		 ASSH_ERR_CRYPTO);

  return ASSH_OK;
}

static ASSH_CIPHER_CLEANUP_FCN(assh_cipher_gcrypt_cleanup)
{
  struct assh_cipher_gcrypt_context_s *ctx = ctx_;
  gcry_cipher_close(ctx->hd);
}

#define ASSH_GCRYPT_CIPHER(id_, name_, algo_, mode_, bsize_, ksize_, saf_, spd_, is_stream_) \
extern const struct assh_algo_cipher_s assh_cipher_##id_;		\
									\
static ASSH_CIPHER_INIT_FCN(assh_cipher_gcrypt_##id_##_init)		\
{									\
  return assh_cipher_gcrypt_init(&assh_cipher_##id_, ctx_, key, iv,	\
				 algo_, mode_, encrypt);		\
}									\
									\
const struct assh_algo_cipher_s assh_cipher_##id_ =			\
{									\
  .algo = { .name = name_, .class_ = ASSH_ALGO_CIPHER, .safety = saf_, .speed = spd_ }, \
  .ctx_size = sizeof(struct assh_cipher_gcrypt_context_s),		\
  .block_size = bsize_,							\
  .key_size = ksize_,							\
  .is_stream = is_stream_,						\
  .f_init = assh_cipher_gcrypt_##id_##_init,				\
  .f_process = assh_cipher_gcrypt_process,				\
  .f_cleanup = assh_cipher_gcrypt_cleanup,				\
};

ASSH_GCRYPT_CIPHER(arc4,           "arcfour",        GCRY_CIPHER_ARCFOUR,    GCRY_CIPHER_MODE_STREAM, 1,  16, 5,  80, 1);
ASSH_GCRYPT_CIPHER(arc4_128,       "arcfour128",     GCRY_CIPHER_ARCFOUR,    GCRY_CIPHER_MODE_STREAM, 1,  16, 10, 80, 1);
ASSH_GCRYPT_CIPHER(arc4_256,       "arcfour256",     GCRY_CIPHER_ARCFOUR,    GCRY_CIPHER_MODE_STREAM, 1,  32, 15, 80, 1);

ASSH_GCRYPT_CIPHER(tdes_cbc,       "3des-cbc",       GCRY_CIPHER_3DES,       GCRY_CIPHER_MODE_CBC,    8,  24, 20, 30, 0);
ASSH_GCRYPT_CIPHER(tdes_ctr,       "3des-ctr",       GCRY_CIPHER_3DES,       GCRY_CIPHER_MODE_CTR,    8,  24, 21, 30, 0);

ASSH_GCRYPT_CIPHER(cast128_cbc,    "cast128-cbc",    GCRY_CIPHER_CAST5,      GCRY_CIPHER_MODE_CBC,    16, 16, 25, 50, 0);
ASSH_GCRYPT_CIPHER(cast128_ctr,    "cast128-ctr",    GCRY_CIPHER_CAST5,      GCRY_CIPHER_MODE_CTR,    16, 16, 26, 50, 0);

ASSH_GCRYPT_CIPHER(blowfish_cbc,   "blowfish-cbc",   GCRY_CIPHER_BLOWFISH,   GCRY_CIPHER_MODE_CBC,    8,  16, 30, 60, 0);
ASSH_GCRYPT_CIPHER(blowfish_ctr,   "blowfish-ctr",   GCRY_CIPHER_BLOWFISH,   GCRY_CIPHER_MODE_CTR,    8,  32, 35, 60, 0);

ASSH_GCRYPT_CIPHER(aes128_cbc,     "aes128-cbc",     GCRY_CIPHER_AES128,     GCRY_CIPHER_MODE_CBC,    16, 16, 40, 70, 0);
ASSH_GCRYPT_CIPHER(aes192_cbc,     "aes192-cbc",     GCRY_CIPHER_AES192,     GCRY_CIPHER_MODE_CBC,    16, 24, 50, 65, 0);
ASSH_GCRYPT_CIPHER(aes256_cbc,     "aes256-cbc",     GCRY_CIPHER_AES256,     GCRY_CIPHER_MODE_CBC,    16, 32, 60, 60, 0);
ASSH_GCRYPT_CIPHER(aes128_ctr,     "aes128-ctr",     GCRY_CIPHER_AES128,     GCRY_CIPHER_MODE_CTR,    16, 16, 41, 70, 0);
ASSH_GCRYPT_CIPHER(aes192_ctr,     "aes192-ctr",     GCRY_CIPHER_AES192,     GCRY_CIPHER_MODE_CTR,    16, 24, 51, 65, 0);
ASSH_GCRYPT_CIPHER(aes256_ctr,     "aes256-ctr",     GCRY_CIPHER_AES256,     GCRY_CIPHER_MODE_CTR,    16, 32, 61, 60, 0);

ASSH_GCRYPT_CIPHER(twofish128_cbc, "twofish128-cbc", GCRY_CIPHER_TWOFISH128, GCRY_CIPHER_MODE_CBC,    16, 16, 50, 60, 0);
ASSH_GCRYPT_CIPHER(twofish256_cbc, "twofish256-cbc", GCRY_CIPHER_TWOFISH   , GCRY_CIPHER_MODE_CBC,    16, 32, 70, 60, 0);
ASSH_GCRYPT_CIPHER(twofish128_ctr, "twofish128-ctr", GCRY_CIPHER_TWOFISH128, GCRY_CIPHER_MODE_CTR,    16, 16, 51, 60, 0);
ASSH_GCRYPT_CIPHER(twofish256_ctr, "twofish256-ctr", GCRY_CIPHER_TWOFISH,    GCRY_CIPHER_MODE_CTR,    16, 32, 71, 60, 0);

ASSH_GCRYPT_CIPHER(serpent128_cbc, "serpent128-cbc", GCRY_CIPHER_SERPENT128, GCRY_CIPHER_MODE_CBC,    16, 16, 55, 40, 0);
ASSH_GCRYPT_CIPHER(serpent192_cbc, "serpent192-cbc", GCRY_CIPHER_SERPENT192, GCRY_CIPHER_MODE_CBC,    16, 24, 65, 40, 0);
ASSH_GCRYPT_CIPHER(serpent256_cbc, "serpent256-cbc", GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CBC,    16, 32, 75, 40, 0);
ASSH_GCRYPT_CIPHER(serpent128_ctr, "serpent128-ctr", GCRY_CIPHER_SERPENT128, GCRY_CIPHER_MODE_CTR,    16, 16, 56, 40, 0);
ASSH_GCRYPT_CIPHER(serpent192_ctr, "serpent192-ctr", GCRY_CIPHER_SERPENT192, GCRY_CIPHER_MODE_CTR,    16, 24, 66, 40, 0);
ASSH_GCRYPT_CIPHER(serpent256_ctr, "serpent256-ctr", GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CTR,    16, 32, 76, 40, 0);

assh_error_t assh_cipher_register_gcrypt(struct assh_context_s *c, unsigned int safety,
					 unsigned int min_safety)
{
  return assh_algo_register_va(c, safety, min_safety,
    &assh_cipher_arc4, &assh_cipher_arc4_128, &assh_cipher_arc4_256,
    &assh_cipher_tdes_cbc, &assh_cipher_tdes_ctr,
    &assh_cipher_cast128_cbc, &assh_cipher_cast128_ctr,
    &assh_cipher_blowfish_cbc, &assh_cipher_blowfish_ctr,
    &assh_cipher_aes128_cbc, &assh_cipher_aes192_cbc, &assh_cipher_aes256_cbc,
    &assh_cipher_aes128_ctr, &assh_cipher_aes192_ctr, &assh_cipher_aes256_ctr,
    &assh_cipher_twofish128_cbc, &assh_cipher_twofish256_cbc,
    &assh_cipher_twofish128_ctr, &assh_cipher_twofish256_ctr,
    &assh_cipher_serpent128_cbc, &assh_cipher_serpent192_cbc, &assh_cipher_serpent256_cbc,
    &assh_cipher_serpent128_ctr, &assh_cipher_serpent192_ctr, &assh_cipher_serpent256_ctr,
    NULL);
}

