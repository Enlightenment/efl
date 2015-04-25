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

#include <assh/assh_packet.h>
#include <assh/assh_bignum.h>
#include <assh/assh_sign.h>
#include <assh/key_rsa.h>
#include <assh/assh_hash.h>
#include <assh/assh_prng.h>
#include <assh/assh_alloc.h>

#include <string.h>

enum assh_rsa_digest_e
{
  RSA_DIGEST_MD2,
  RSA_DIGEST_MD5,
  RSA_DIGEST_SHA1,
  RSA_DIGEST_SHA256,
  RSA_DIGEST_SHA384,
  RSA_DIGEST_SHA512,
  RSA_DIGEST_count,
};

struct assh_rsa_digest_s
{
  /* asn1 DER digest algorithm identifier */
  uint_fast8_t oid_len;
  const char *oid;

  const struct assh_hash_algo_s *algo;
};

static const struct assh_rsa_digest_s assh_rsa_digests[RSA_DIGEST_count] =
{
 /* len   DigestInfo header */
  { 18, "\x30\x20\x30\x0c\x06\x08\x2a\x86\x48\x86\xf7\x0d\x02\x02\x05\x00\x04\x10",
    NULL /* md2 */ },
  { 18, "\x30\x20\x30\x0c\x06\x08\x2a\x86\x48\x86\xf7\x0d\x02\x05\x05\x00\x04\x10",
    &assh_hash_md5 },
  { 15, "\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14",
    &assh_hash_sha1 },
  { 19, "\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20",
    &assh_hash_sha256 },
  { 19, "\x30\x41\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02\x05\x00\x04\x30",
    &assh_hash_sha384 },
  { 19, "\x30\x51\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03\x05\x00\x04\x40",
    &assh_hash_sha512 },
};

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_sign_rsa_generate(struct assh_context_s *c,
                       const struct assh_key_s *key,
                       size_t data_count,
                       const uint8_t * const data[],
                       size_t const data_len[],
                       uint8_t *sign, size_t *sign_len,
                       enum assh_rsa_digest_e digest_id)
{
  const struct assh_key_rsa_s *k = (const void*)key;
  assh_error_t err;

  assert(key->algo == &assh_key_rsa);

  /* check availability of the private key */
  ASSH_CHK_RET(assh_bignum_isempty(&k->dn), ASSH_ERR_MISSING_KEY);

  unsigned int n = assh_bignum_bits(&k->nn);

  /* check/return signature length */
  size_t len = ASSH_RSA_ID_LEN + 4 + n / 8;

  if (sign == NULL)
    {
      *sign_len = len;
      return ASSH_OK;
    }

  ASSH_CHK_RET(*sign_len < len, ASSH_ERR_OUTPUT_OVERFLOW);
  *sign_len = len;

  const struct assh_rsa_digest_s *digest = assh_rsa_digests + digest_id;
  ASSH_CHK_RET(digest->algo == NULL, ASSH_ERR_NOTSUP);

  /* build encoded message buffer */
  unsigned int ps_len = n / 8 - 3 - digest->oid_len - digest->algo->hash_size;

  ASSH_CHK_RET(ps_len < 8, ASSH_ERR_BAD_DATA);

  ASSH_SCRATCH_ALLOC(c, uint8_t, scratch,
                     digest->algo->ctx_size + n / 8,
                     ASSH_ERRSV_CONTINUE, err_);

  uint8_t *em_buf = scratch + digest->algo->ctx_size;
  uint8_t *em = em_buf;

  *em++ = 0x00;
  *em++ = 0x01;
  memset(em, 0xff, ps_len);
  em += ps_len;
  *em++ = 0x00;
  memcpy(em, digest->oid, digest->oid_len);
  em += digest->oid_len;

  uint_fast16_t i;
  void *hash_ctx = scratch;

  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, digest->algo), err_scratch);
  for (i = 0; i < data_count; i++)
    assh_hash_update(hash_ctx, data[i], data_len[i]);
  assh_hash_final(hash_ctx, em, digest->algo->hash_size);
  assh_hash_cleanup(hash_ctx);

  /* build signature blob */
  memcpy(sign, ASSH_RSA_ID, ASSH_RSA_ID_LEN);
  assh_store_u32(sign + ASSH_RSA_ID_LEN, n / 8);
  uint8_t *c_str = sign + ASSH_RSA_ID_LEN + 4;

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("rsa generate em", em_buf, n / 8);
#endif

  enum bytecode_args_e
  {
    C_data, EM_data,            /* data buffers */
    N, D,                       /* big number inputs */
    C, EM,                      /* big number temporaries */
    MT
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZE(      C,      N			),
    ASSH_BOP_SIZE(      EM,     N			),
    ASSH_BOP_MTINIT(    MT,     N                       ),

    ASSH_BOP_MOVE(      EM,     EM_data			),
    ASSH_BOP_MTTO(      EM,     EM,     EM,     MT      ),
    ASSH_BOP_EXPM(      C,      EM,     D,	MT	),
    ASSH_BOP_MTFROM(    C,      C,      C,      MT      ),

    ASSH_BOP_MOVE(      C_data, C			),
    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "DDNNTTm",
                   /* Data */ c_str, em_buf,
                   /* Num  */ &k->nn, &k->dn), err_scratch);

  err = ASSH_OK;

 err_scratch:
  ASSH_SCRATCH_FREE(c, scratch);
 err_:
  return err;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_sign_rsa_check(struct assh_context_s *c,
                     const struct assh_key_s *key, size_t data_count,
                     const uint8_t * const data[], size_t const data_len[],
                     const uint8_t *sign, size_t sign_len, uint8_t digest_mask)
{
  const struct assh_key_rsa_s *k = (const void*)key;
  assh_error_t err;

  assert(key->algo == &assh_key_rsa);

  unsigned int n = assh_bignum_bits(&k->nn);

  ASSH_CHK_RET(sign_len != ASSH_RSA_ID_LEN + 4 + n / 8, ASSH_ERR_INPUT_OVERFLOW);

  ASSH_CHK_RET(memcmp(sign, ASSH_RSA_ID, ASSH_RSA_ID_LEN), ASSH_ERR_BAD_DATA);

  uint8_t *c_str = (uint8_t*)sign + ASSH_RSA_ID_LEN;
  ASSH_CHK_RET(assh_load_u32(c_str) != n / 8, ASSH_ERR_INPUT_OVERFLOW);

  ASSH_SCRATCH_ALLOC(c, uint8_t, em_buf, n / 8, ASSH_ERRSV_CONTINUE, err_);
  uint8_t *em = em_buf;

  enum bytecode_args_e
  {
    C_data, EM_data,            /* data buffers */
    N, E,                       /* big number inputs */
    C, EM,                      /* big number temporaries */
    MT
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZE(      C,      N			),
    ASSH_BOP_SIZE(      EM,     N			),
    ASSH_BOP_MTINIT(    MT,     N                       ),

    ASSH_BOP_MOVE(      C,      C_data                  ),
    ASSH_BOP_MTTO(      C,      C,      C,      MT      ),
    ASSH_BOP_EXPM(      EM,     C,      E,	MT	),
    ASSH_BOP_MTFROM(    EM,     EM,     EM,     MT      ),

    ASSH_BOP_MOVE(      EM_data, EM                     ),
    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "DDNNTTm",
                   /* Data */ c_str + 4, em,
                   /* Nun  */ &k->nn, &k->en), err_em);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("rsa check em", em, n / 8);
#endif

  uint8_t *em_end = em + n / 8;
  uint_fast16_t i;

  /* check padding */
  ASSH_CHK_GTO(*em++ != 0x00, ASSH_ERR_BAD_DATA, err_em);
  ASSH_CHK_GTO(*em++ != 0x01, ASSH_ERR_BAD_DATA, err_em);
  for (i = 0; em + 1 < em_end && *em == 0xff; em++)
    i++;
  ASSH_CHK_GTO(i < 8, ASSH_ERR_BAD_DATA, err_em);
  ASSH_CHK_GTO(*em++ != 0x00, ASSH_ERR_BAD_DATA, err_em);

  /* lookup digest algorithm in use */
  const struct assh_rsa_digest_s *digest;
  for (i = 0; i < RSA_DIGEST_count; i++)
    {
      digest = assh_rsa_digests + i;
      if (digest->algo == NULL)
        continue;
      if (digest->oid_len + digest->algo->hash_size != em_end - em)
        continue;
      if (!memcmp(digest->oid, em, digest->oid_len))
        break;
    }

  ASSH_CHK_GTO(i == RSA_DIGEST_count, ASSH_ERR_NOTSUP, err_em);
  ASSH_CHK_GTO(!((digest_mask >> i) & 1), ASSH_ERR_WEAK_ALGORITHM, err_em);

  /* compute message hash */
  em += digest->oid_len;
  ASSH_SCRATCH_ALLOC(c, void, hash_ctx, digest->algo->ctx_size +
                     digest->algo->hash_size,
                     ASSH_ERRSV_CONTINUE, err_em);

  uint8_t *hash = hash_ctx + digest->algo->ctx_size;

  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, digest->algo), err_hash);
  for (i = 0; i < data_count; i++)
    assh_hash_update(hash_ctx, data[i], data_len[i]);
  assh_hash_final(hash_ctx, hash, digest->algo->hash_size);
  assh_hash_cleanup(hash_ctx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("rsa check hash", hash, digest->algo->hash_size);
#endif

  ASSH_CHK_GTO(assh_memcmp(hash, em, digest->algo->hash_size),
               ASSH_ERR_NUM_COMPARE_FAILED, err_hash);

  err = ASSH_OK;

 err_hash:
  ASSH_SCRATCH_FREE(c, hash_ctx);
 err_em:
  ASSH_SCRATCH_FREE(c, em_buf);
 err_:
  return err;
}

static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_rsa_suitable_key_768)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  if (key->algo != &assh_key_rsa)
    return 0;
  const struct assh_key_rsa_s *k = (const void*)key;
  return assh_bignum_bits(&k->nn) >= 768;
}

static ASSH_SIGN_CHECK_FCN(assh_sign_rsa_check_sha1_md5)
{
  return assh_sign_rsa_check(c, key, data_count, data, data_len,
                              sign, sign_len, (1 << RSA_DIGEST_SHA1)
                              | (1 << RSA_DIGEST_MD5)
                              | (1 << RSA_DIGEST_SHA256)
                              | (1 << RSA_DIGEST_SHA384)
                              | (1 << RSA_DIGEST_SHA512));
}

static ASSH_SIGN_GENERATE_FCN(assh_sign_rsa_generate_sha1)
{
  return assh_sign_rsa_generate(c, key, data_count, data, data_len,
                                sign, sign_len, RSA_DIGEST_SHA1);
}

const struct assh_algo_sign_s assh_sign_rsa_sha1_md5 =
{
  .algo = {
    .name = "ssh-rsa", .variant = "sha*, md5, 768+ bits keys",
    .class_ = ASSH_ALGO_SIGN,
    .priority = 2, .safety = 15, .speed = 40,
    .f_suitable_key = assh_sign_rsa_suitable_key_768,
    .key = &assh_key_rsa,
  },
  .f_generate = assh_sign_rsa_generate_sha1,
  .f_check = assh_sign_rsa_check_sha1_md5,
};



static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_rsa_suitable_key_1024)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  if (key->algo != &assh_key_rsa)
    return 0;
  const struct assh_key_rsa_s *k = (const void*)key;
  return assh_bignum_bits(&k->nn) >= 1024;
}

static ASSH_SIGN_CHECK_FCN(assh_sign_rsa_check_sha1)
{
  return assh_sign_rsa_check(c, key, data_count, data, data_len,
                              sign, sign_len, (1 << RSA_DIGEST_SHA1)
                              | (1 << RSA_DIGEST_SHA256)
                              | (1 << RSA_DIGEST_SHA384)
                              | (1 << RSA_DIGEST_SHA512));
}

const struct assh_algo_sign_s assh_sign_rsa_sha1 =
{
  .algo = {
    .name = "ssh-rsa",  .variant = "sha*, 1024+ bits keys",
    .class_ = ASSH_ALGO_SIGN,
    .priority = 1, .safety = 20, .speed = 40,
    .f_suitable_key = assh_sign_rsa_suitable_key_1024,
    .key = &assh_key_rsa,
  },
  .f_generate = assh_sign_rsa_generate_sha1,
  .f_check = assh_sign_rsa_check_sha1,
};



static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_rsa_suitable_key_2048)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  if (key->algo != &assh_key_rsa)
    return 0;
  const struct assh_key_rsa_s *k = (const void*)key;
  return assh_bignum_bits(&k->nn) >= 2048;
}

const struct assh_algo_sign_s assh_sign_rsa_sha1_2048 =
{
  .algo = {
    .name = "ssh-rsa", .variant = "sha1, 2048+ bits keys",
    .class_ = ASSH_ALGO_SIGN,
    .priority = 0, .safety = 25, .speed = 30,
    .f_suitable_key = assh_sign_rsa_suitable_key_2048,
    .key = &assh_key_rsa,
  },
  .f_generate = assh_sign_rsa_generate_sha1,
  .f_check = assh_sign_rsa_check_sha1,
};



static ASSH_SIGN_CHECK_FCN(assh_sign_rsa_check_sha256)
{
  return assh_sign_rsa_check(c, key, data_count, data, data_len,
                              sign, sign_len, (1 << RSA_DIGEST_SHA256)
                              | (1 << RSA_DIGEST_SHA384)
                              | (1 << RSA_DIGEST_SHA512));
}

static ASSH_SIGN_GENERATE_FCN(assh_sign_rsa_generate_sha256)
{
  return assh_sign_rsa_generate(c, key, data_count, data, data_len,
                                sign, sign_len, RSA_DIGEST_SHA256);
}

const struct assh_algo_sign_s assh_sign_rsa_sha256_2048 =
{
  .algo = {
    .name = "rsa2048-sha256@libassh.org",
    .class_ = ASSH_ALGO_SIGN,
    .safety = 40, .speed = 30,
    .f_suitable_key = assh_sign_rsa_suitable_key_2048,
    .key = &assh_key_rsa,
  },
  .f_generate = assh_sign_rsa_generate_sha256,
  .f_check = assh_sign_rsa_check_sha256,
};



static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_rsa_suitable_key_3072)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  if (key->algo != &assh_key_rsa)
    return 0;
  const struct assh_key_rsa_s *k = (const void*)key;
  return assh_bignum_bits(&k->nn) >= 3072;
}

const struct assh_algo_sign_s assh_sign_rsa_sha256_3072 =
{
  .algo = {
    .name = "rsa3072-sha256@libassh.org",
    .class_ = ASSH_ALGO_SIGN,
    .safety = 50, .speed = 20,
    .f_suitable_key = assh_sign_rsa_suitable_key_3072,
    .key = &assh_key_rsa,
  },
  .f_generate = assh_sign_rsa_generate_sha256,
  .f_check = assh_sign_rsa_check_sha256,
};

