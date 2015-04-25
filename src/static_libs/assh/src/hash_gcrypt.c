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

#include <gcrypt.h>

struct assh_hash_gcrypt_context_s
{
  struct assh_hash_ctx_s ctx;
  gcry_md_hd_t hd;
};

ASSH_FIRST_FIELD_ASSERT(assh_hash_gcrypt_context_s, ctx);

static ASSH_HASH_COPY_FCN(assh_gcrypt_hash_copy)
{
  const struct assh_hash_gcrypt_context_s *src = (const void*)hctx_src;
  struct assh_hash_gcrypt_context_s *dst = (void*)hctx_dst;
  assh_error_t err;

  dst->ctx = src->ctx;
  ASSH_CHK_RET(gcry_md_copy(&dst->hd, src->hd),
	       ASSH_ERR_CRYPTO);

  return ASSH_OK;
}

static ASSH_HASH_UPDATE_FCN(assh_gcrypt_hash_update)
{
  struct assh_hash_gcrypt_context_s *gctx = (void*)hctx;

  gcry_md_write(gctx->hd, data, len);
}

#define ASSH_GCRYPT_HASH(id_, algo_, hsize_, bsize_)                    \
                                                                        \
static ASSH_HASH_INIT_FCN(assh_gcrypt_hash_##id_##_init)                \
{                                                                       \
  struct assh_hash_gcrypt_context_s *gctx = (void*)hctx;                \
  assh_error_t err;                                                     \
                                                                        \
  ASSH_CHK_RET(gcry_md_open(&gctx->hd, algo_, GCRY_MD_FLAG_SECURE),     \
	       ASSH_ERR_CRYPTO);                                        \
                                                                        \
  return ASSH_OK;                                                       \
}                                                                       \
                                                                        \
static ASSH_HASH_FINAL_FCN(assh_gcrypt_hash_##id_##_final)              \
{                                                                       \
  struct assh_hash_gcrypt_context_s *gctx = (void*)hctx;                \
                                                                        \
  assert(len == hsize_);                                                \
                                                                        \
  if (hash != NULL)                                                     \
    memcpy(hash, gcry_md_read(gctx->hd, 0), hsize_);                    \
                                                                        \
  gcry_md_close(gctx->hd);                                              \
}                                                                       \
                                                                        \
const struct assh_hash_algo_s assh_hash_##id_ =                         \
{                                                                       \
  .name = #id_,                                                         \
  .ctx_size = sizeof(struct assh_hash_gcrypt_context_s),                \
  .hash_size = hsize_,                                                  \
  .block_size = bsize_,                                                 \
  .f_init = assh_gcrypt_hash_##id_##_init,                              \
  .f_copy = assh_gcrypt_hash_copy,                                      \
  .f_update = assh_gcrypt_hash_update,                                  \
  .f_final = assh_gcrypt_hash_##id_##_final,                            \
};

ASSH_GCRYPT_HASH(md5, GCRY_MD_MD5, 16, 64);
ASSH_GCRYPT_HASH(sha1, GCRY_MD_SHA1, 20, 64);
ASSH_GCRYPT_HASH(sha224, GCRY_MD_SHA224, 28, 64);
ASSH_GCRYPT_HASH(sha256, GCRY_MD_SHA256, 32, 64);
ASSH_GCRYPT_HASH(sha384, GCRY_MD_SHA384, 48, 128);
ASSH_GCRYPT_HASH(sha512, GCRY_MD_SHA512, 64, 128);

