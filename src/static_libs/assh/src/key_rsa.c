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

#include <assh/key_rsa.h>
#include <assh/assh_packet.h>
#include <assh/assh_alloc.h>

#include <string.h>

static ASSH_KEY_OUTPUT_FCN(assh_key_rsa_output)
{
  struct assh_key_rsa_s *k = (void*)key;
  assh_error_t err;

  assert(key->algo == &assh_key_rsa);

  struct assh_bignum_s *bn_[4] = { &k->en, &k->nn, NULL, NULL };

  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6: {
      /* add algo identifier */
      size_t l = ASSH_RSA_ID_LEN;
      if (blob != NULL)
        {
          ASSH_CHK_RET(ASSH_RSA_ID_LEN > *blob_len, ASSH_ERR_OUTPUT_OVERFLOW);
          memcpy(blob, ASSH_RSA_ID, ASSH_RSA_ID_LEN);
          *blob_len -= ASSH_RSA_ID_LEN;
          blob += ASSH_RSA_ID_LEN;
        }

      /* add key integers */
      struct assh_bignum_s **bn = bn_;
      for (bn = bn_; *bn != NULL; bn++)
        {
          size_t s = assh_bignum_size_of_num(ASSH_BIGNUM_MPINT, *bn);
          if (blob != NULL)
            {
              ASSH_CHK_RET(s > *blob_len, ASSH_ERR_OUTPUT_OVERFLOW);
              ASSH_ERR_RET(assh_bignum_convert(c, ASSH_BIGNUM_NATIVE,
                             ASSH_BIGNUM_MPINT, *bn, blob));
              s = assh_load_u32(blob) + 4;
              *blob_len -= s;
              blob += s;
            }
          l += s;
        }
      *blob_len = l;
      return ASSH_OK;
    }

#warning dsa key output
#if 0
    case ASSH_KEY_FMT_PV_PEM_ASN1: {
      ASSH_CHK_RET(assh_bignum_isempty(&k->xn), ASSH_ERR_NOTSUP);
      bn_[4] = k->xn;
      return ASSH_OK;
    }
#endif

    default:
      ASSH_ERR_RET(ASSH_ERR_NOTSUP);
    }

  return ASSH_OK;
}

static ASSH_KEY_CMP_FCN(assh_key_rsa_cmp)
{
  assert(key->algo == &assh_key_rsa);

  if (key->algo != b->algo)
    return 0;

  struct assh_key_rsa_s *k = (void*)key;
  struct assh_key_rsa_s *l = (void*)b;

  enum bytecode_args_e
  {
    N0, N1, E0, E1, D0, D1
  };

  static const assh_bignum_op_t *bc, bytecode[] = {
    ASSH_BOP_CMPEQ(     D1,     D0,	0       ),
    ASSH_BOP_CMPEQ(     E1,     E0,	0       ),
    ASSH_BOP_CMPEQ(     N1,     N0,	0       ),
    ASSH_BOP_END(),
  };

  bc = bytecode;

  if (pub)
    {
      /* skip compare of D */
      bc++;
    }
  else
    {
      if (assh_bignum_isempty(&k->dn) != 
          assh_bignum_isempty(&l->dn))
        return 0;
      if (assh_bignum_isempty(&l->dn))
        bc++;
    }

  return assh_bignum_bytecode(c, bc, "NNNNNNNN",
    &k->nn, &l->nn, &k->en, &l->en, &k->dn, &l->dn) == 0;
}

static ASSH_KEY_CREATE_FCN(assh_key_rsa_create)
{
  assh_error_t err;

  ASSH_CHK_RET(bits < 1024, ASSH_ERR_NOTSUP);

  struct assh_key_rsa_s *k;

  ASSH_ERR_RET(assh_alloc(c, sizeof(struct assh_key_rsa_s),
                          ASSH_ALLOC_SECUR, (void**)&k));

  k->key.algo = &assh_key_rsa;

  /* init numbers */
  assh_bignum_init(c, &k->nn, bits, 0);
  assh_bignum_init(c, &k->dn, bits, 1);
  assh_bignum_init(c, &k->en, 17, 0);

  enum bytecode_args_e
  {
    N, D, E,
    P, Q, T0, T1, MT
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZEM(     P,      N,      0,      -1      ),
    ASSH_BOP_SIZEM(     Q,      N,      0,      -1      ),
    ASSH_BOP_SIZE(      T0,     P                       ),
    ASSH_BOP_SIZE(      T1,     N                       ),

    /* generate 2 prime numbers with the 2 most significant bits set */
    ASSH_BOP_UINT(      T0,     3                       ),
    ASSH_BOP_SHL(       T0,     T0,     -2,     P       ),
    ASSH_BOP_PRIME(     P,      T0,     ASSH_BOP_NOREG  ),
    ASSH_BOP_PRIME(     Q,      T0,     ASSH_BOP_NOREG  ),
    ASSH_BOP_CMPNE(     P,      Q,      0 /* sanity check */ ),

    ASSH_BOP_MUL(       N,      P,      Q               ),

    // FIXME could use T1 = N - (P + Q -1)
    ASSH_BOP_UINT(      T0,     1                       ),
    ASSH_BOP_SUB(       P,      P,      T0              ),
    ASSH_BOP_SUB(       Q,      Q,      T0              ),
    ASSH_BOP_MUL(       T1,     P,      Q               ),
#warning  (p-1)(q-1) must be secret

    ASSH_BOP_UINT(      E,      65537                   ),
    ASSH_BOP_INV(       D,      E,      T1              ),

    ASSH_BOP_PRIVACY(   D,      1                       ),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "NNNXXTTm",
                        &k->nn, &k->dn, &k->en), err_key);

  assert(!k->nn.secret && !k->en.secret && k->dn.secret);

  *key = &k->key;
  return ASSH_OK;
 err_key:
  assh_free(c, k);
  return err;
}

static ASSH_KEY_VALIDATE_FCN(assh_key_rsa_validate)
{
#if 0
  struct assh_key_rsa_s *k = (void*)key;
  assh_error_t err = ASSH_OK;

  unsigned int n = assh_bignum_bits(&k->nn);

  /* check key size */
  if (n < 768 || n > 8192 || n % 8)
    return ASSH_OK;

  enum bytecode_args_e
  {
    E
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_END(),
  };

  err = assh_bignum_bytecode(c, bytecode, "NNNNTT");

  if (err != ASSH_ERR_NUM_COMPARE_FAILED)
    ASSH_ERR_RET(err);
#endif
#warning rsa key validate

  return ASSH_OK;
}

static inline unsigned int
assh_rsa_mpint_strip(uint8_t *num, unsigned int len)
{
  /* discard null MSB */
  if (len && num[0] == 0)
    len--;
  return len * 8;
}

static ASSH_KEY_LOAD_FCN(assh_key_rsa_load)
{
  assh_error_t err;

  unsigned int n_len, e_len, d_len;
  uint8_t *n_str, *e_str, *d_str;

  /* parse the key blob */
  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6: {

      ASSH_CHK_RET(blob_len < ASSH_RSA_ID_LEN, ASSH_ERR_INPUT_OVERFLOW);
      ASSH_CHK_RET(memcmp(ASSH_RSA_ID, blob, ASSH_RSA_ID_LEN), ASSH_ERR_BAD_DATA);

      e_str = (uint8_t*)blob + ASSH_RSA_ID_LEN;
      ASSH_ERR_RET(assh_check_string(blob, blob_len, e_str, &n_str));
      e_len = assh_rsa_mpint_strip(e_str + 4, assh_load_u32(e_str));

      ASSH_ERR_RET(assh_check_string(blob, blob_len, n_str, NULL));
      n_len = assh_rsa_mpint_strip(n_str + 4, assh_load_u32(n_str));

      d_len = 0;
      d_str = NULL;
      break;
    }

    case ASSH_KEY_FMT_PV_PEM_ASN1: {
      uint8_t *seq, *seq_end, *p_str, *version, *val;
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, blob, &seq, &seq_end));
      /* sequence type */
      ASSH_CHK_RET(blob[0] != 0x30, ASSH_ERR_BAD_DATA);

      /* skip first value */
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, seq, &version, &n_str));

      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, n_str, &val, &e_str));
      n_len = assh_rsa_mpint_strip(val, e_str - val);

      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, e_str, &val, &d_str));
      e_len = assh_rsa_mpint_strip(val, d_str - val);

      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, d_str, &val, &p_str));
      d_len = assh_rsa_mpint_strip(val, p_str - val);
      break;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_NOTSUP);
    }

  /* allocate key structure */
  ASSH_CHK_RET(n_len < 768 || n_len > 8192, ASSH_ERR_NOTSUP);
  ASSH_CHK_RET(e_len < 1 || e_len > 32, ASSH_ERR_NOTSUP);
  ASSH_CHK_RET(d_str != NULL && (d_len < 768 || d_len > 8192), ASSH_ERR_NOTSUP);

  struct assh_key_rsa_s *k;

  ASSH_ERR_RET(assh_alloc(c, sizeof(struct assh_key_rsa_s),
                          ASSH_ALLOC_SECUR, (void**)&k));

  k->key.algo = &assh_key_rsa;

  /* init numbers */
  assh_bignum_init(c, &k->nn, n_len, 0);
  assh_bignum_init(c, &k->en, e_len, 0);
  assh_bignum_init(c, &k->dn, d_len, 1);

  /* convert numbers from blob representation */
  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6:
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       n_str, &k->nn), err_num);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       e_str, &k->en), err_num);
      break;

    case ASSH_KEY_FMT_PV_PEM_ASN1:
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       n_str, &k->nn), err_num);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       e_str, &k->en), err_num);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       d_str, &k->dn), err_num);
    default:
      break;
    }

  *key = &k->key;
  return ASSH_OK;

 err_num:
  assh_bignum_release(c, &k->nn);
  assh_bignum_release(c, &k->en);
  assh_bignum_release(c, &k->dn);
  assh_free(c, k);
  return err;
}

static ASSH_KEY_CLEANUP_FCN(assh_key_rsa_cleanup)
{
  struct assh_key_rsa_s *k = (void*)key;

  assh_bignum_release(c, &k->nn);
  assh_bignum_release(c, &k->en);
  assh_bignum_release(c, &k->dn);
  assh_free(c, k);
}

const struct assh_key_ops_s assh_key_rsa =
{
  .type = "ssh-rsa",
  .f_output = assh_key_rsa_output,
  .f_create = assh_key_rsa_create,
  .f_validate = assh_key_rsa_validate,
  .f_cmp = assh_key_rsa_cmp,
  .f_load = assh_key_rsa_load,
  .f_cleanup = assh_key_rsa_cleanup,
};

