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
#include <assh/assh_packet.h>

#include <gcrypt.h>

struct assh_hmac_gcrypt_context_s
{
  const struct assh_algo_mac_s *mac;
  gcry_mac_hd_t hd;
  unsigned int gcry_mac_len;
};

static ASSH_MAC_CLEANUP_FCN(assh_hmac_gcrypt_cleanup)
{
  struct assh_hmac_gcrypt_context_s *ctx = ctx_;
  gcry_mac_close(ctx->hd);
}

static ASSH_MAC_COMPUTE_FCN(assh_hmac_gcrypt_compute)
{
  struct assh_hmac_gcrypt_context_s *ctx = ctx_;
  assh_error_t err;

  ASSH_CHK_RET(gcry_mac_reset(ctx->hd), ASSH_ERR_CRYPTO);

  uint8_t be_seq[4];
  assh_store_u32(be_seq, seq);

  ASSH_CHK_RET(gcry_mac_write(ctx->hd, be_seq, 4), ASSH_ERR_CRYPTO);
  ASSH_CHK_RET(gcry_mac_write(ctx->hd, data, len), ASSH_ERR_CRYPTO);

  size_t s = ctx->mac->mac_size;
  ASSH_CHK_RET(gcry_mac_read(ctx->hd, mac, &s), ASSH_ERR_CRYPTO);
  return ASSH_OK;
}

static ASSH_MAC_CHECK_FCN(assh_hmac_gcrypt_check)
{
  struct assh_hmac_gcrypt_context_s *ctx = ctx_;
  assh_error_t err;

  ASSH_CHK_RET(gcry_mac_reset(ctx->hd), ASSH_ERR_CRYPTO);

  uint8_t be_seq[4];
  assh_store_u32(be_seq, seq);

  ASSH_CHK_RET(gcry_mac_write(ctx->hd, be_seq, 4), ASSH_ERR_CRYPTO);
  ASSH_CHK_RET(gcry_mac_write(ctx->hd, data, len), ASSH_ERR_CRYPTO);

  ASSH_CHK_RET(gcry_mac_verify(ctx->hd, mac, ctx->mac->mac_size), ASSH_ERR_CRYPTO);
  return ASSH_OK;
}

static assh_error_t assh_hmac_gcrypt_init(const struct assh_algo_mac_s *mac,
				   struct assh_hmac_gcrypt_context_s *ctx,
				   const uint8_t *key, int algo)
{
  assh_error_t err;
  ctx->mac = mac;
  ASSH_CHK_RET(gcry_mac_open(&ctx->hd, algo, GCRY_MAC_FLAG_SECURE,
			     NULL), ASSH_ERR_CRYPTO);
  ASSH_CHK_GTO(gcry_mac_setkey(ctx->hd, key, mac->key_size),
	       ASSH_ERR_CRYPTO, err_hd);
  ctx->gcry_mac_len = gcry_mac_get_algo_maclen(algo);
  return ASSH_OK;
 err_hd:
  gcry_mac_close(ctx->hd);
  return err;
}

#define ASSH_GCRYPT_HMAC(id_, name_, algo_, ksize_, msize_, saf_, spd_) \
extern const struct assh_algo_mac_s assh_hmac_##id_;			\
									\
static ASSH_MAC_INIT_FCN(assh_hmac_gcrypt_##id_##_init)			\
{									\
  return assh_hmac_gcrypt_init(&assh_hmac_##id_, ctx_, key, algo_);	\
}									\
									\
const struct assh_algo_mac_s assh_hmac_##id_ =				\
{									\
  .algo = { .name = name_, .class_ = ASSH_ALGO_MAC,			\
		.safety = saf_, .speed = spd_ },			\
  .ctx_size = sizeof(struct assh_hmac_gcrypt_context_s),		\
  .key_size = ksize_,							\
  .mac_size = msize_,							\
  .f_init = assh_hmac_gcrypt_##id_##_init,				\
  .f_compute = assh_hmac_gcrypt_compute,				\
  .f_check  = assh_hmac_gcrypt_check,					\
  .f_cleanup = assh_hmac_gcrypt_cleanup,				\
};

ASSH_GCRYPT_HMAC(md5,       "hmac-md5"      , GCRY_MAC_HMAC_MD5,    16, 16, 30, 70);
ASSH_GCRYPT_HMAC(md5_96,    "hmac-md5-96"   , GCRY_MAC_HMAC_MD5,    16, 12, 20, 75);
ASSH_GCRYPT_HMAC(sha1,      "hmac-sha1"     , GCRY_MAC_HMAC_SHA1,   20, 20, 30, 70);
ASSH_GCRYPT_HMAC(sha1_96,   "hmac-sha1-96"  , GCRY_MAC_HMAC_SHA1,   20, 12, 20, 75);
ASSH_GCRYPT_HMAC(sha256,    "hmac-sha2-256" , GCRY_MAC_HMAC_SHA256, 32, 32, 40, 60);
ASSH_GCRYPT_HMAC(sha512,    "hmac-sha2-512" , GCRY_MAC_HMAC_SHA512, 64, 64, 50, 50);
ASSH_GCRYPT_HMAC(ripemd160, "hmac-ripemd160", GCRY_MAC_HMAC_RMD160, 20, 20, 30, 70);

