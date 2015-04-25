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

#include <assh/key_dsa.h>
#include <assh/assh_bignum.h>
#include <assh/assh_packet.h>
#include <assh/assh_alloc.h>
#include <assh/assh_prng.h>

#include <string.h>

static ASSH_KEY_OUTPUT_FCN(assh_key_dsa_output)
{
  struct assh_key_dsa_s *k = (void*)key;
  assh_error_t err;

  assert(key->algo == &assh_key_dsa);

  struct assh_bignum_s *bn_[6] = { &k->pn, &k->qn, &k->gn, &k->yn, &k->xn };

  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6: {
      /* add algo identifier */
      size_t l = ASSH_DSA_ID_LEN;
      if (blob != NULL)
        {
          ASSH_CHK_RET(ASSH_DSA_ID_LEN > *blob_len, ASSH_ERR_OUTPUT_OVERFLOW);
          memcpy(blob, ASSH_DSA_ID, ASSH_DSA_ID_LEN);
          *blob_len -= ASSH_DSA_ID_LEN;
          blob += ASSH_DSA_ID_LEN;
        }

      /* add key integers */
      uint_fast8_t i;
      for (i = 0; i < 4; i++)
        {
          struct assh_bignum_s *bn = bn_[i];
          size_t s = assh_bignum_size_of_num(ASSH_BIGNUM_MPINT, bn);
          if (blob != NULL)
            {
              ASSH_CHK_RET(s > *blob_len, ASSH_ERR_OUTPUT_OVERFLOW);
              ASSH_ERR_RET(assh_bignum_convert(c, ASSH_BIGNUM_NATIVE,
                             ASSH_BIGNUM_MPINT, bn, blob));
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
      uint_fast8_t i;
      for (i = 0; i < 5; i++)
        {
          struct assh_bignum_s *bn = bn_[i];
          size_t s = assh_bignum_size_of_num(ASSH_BIGNUM_ASN1, bn);
          if (blob != NULL)
            {
              ASSH_CHK_RET(s > *blob_len, ASSH_ERR_OUTPUT_OVERFLOW);
              ASSH_ERR_RET(assh_bignum_convert(c, ASSH_BIGNUM_NATIVE,
                             ASSH_BIGNUM_MPINT, bn, blob));
            }
        }
      return ASSH_OK;
    }
#endif

    default:
      ASSH_ERR_RET(ASSH_ERR_NOTSUP);
    }

  return ASSH_OK;
}

static ASSH_KEY_CMP_FCN(assh_key_dsa_cmp)
{
  assert(key->algo == &assh_key_dsa);

  if (key->algo != b->algo)
    return 0;

  struct assh_key_dsa_s *k = (void*)key;
  struct assh_key_dsa_s *l = (void*)b;

  enum bytecode_args_e
  {
    P0, P1, Q0, Q1, G0, G1, Y0, Y1, X0, X1
  };

  static const assh_bignum_op_t *bc, bytecode[] = {
    ASSH_BOP_CMPEQ(     X1,     X0,	0       ),
    ASSH_BOP_CMPEQ(     P1,     P0,	0       ),
    ASSH_BOP_CMPEQ(     Q1,     Q0,	0       ),
    ASSH_BOP_CMPEQ(     G1,     G0,	0       ),
    ASSH_BOP_CMPEQ(     Y1,     Y0,	0       ),
    ASSH_BOP_END(),
  };

  bc = bytecode;

  if (pub)
    {
      /* skip compare of X */
      bc++;
    }
  else
    {
      if (assh_bignum_isempty(&k->xn) != 
          assh_bignum_isempty(&l->xn))
        return 0;
      if (assh_bignum_isempty(&l->xn))
        bc++;
    }

  return assh_bignum_bytecode(c, bc, "NNNNNNNN",
    &k->pn, &l->pn, &k->qn, &l->qn, &k->gn, &l->gn, &k->yn, &l->yn) == 0;
}

static ASSH_KEY_CREATE_FCN(assh_key_dsa_create)
{
  assh_error_t err;

  ASSH_CHK_RET(bits < 1024 || bits > 4096, ASSH_ERR_NOTSUP);

  /* DSA domain parameters used are:
       q = 2^N - e,
       p = 2^L - (2^L-1) % q - q * f
       g = 2^((p-1)/q) % p
     with e and f small
  */

  size_t l = ASSH_ALIGN8(bits);
  size_t n;
  uint16_t e;

  if (l == 1024)
    {
      n = 160;
      e = 47;
    }
  else if (l < 2048)
    {
      n = 224;
      e = 63;
    }
  else
    {
      n = 256;
      e = 189;
    }

  static const uint16_t f[385] = {
    /* N = 160, e = 47, L = 1024 */
      504,
    /* N = 224, e = 63, L in (1024, 2048) */
      684,    40,  3124,  1198,   190,  1416,  1044,   348,   414,  1410,
      935,   987,   196,   294,   564,   636,   832,  1552,   240,   856,
     1402,    22,  1504,   766,   412,   666,   646,   156,  3162,  1066,
      682,  2202,    22,   174,   984,   550,   285,   879,    15,   537,
     1420,  1044,  1990,   462,   880,   232,   280,   280,   574,   118,
       40,   274,   538,   630,  1692,   870,   490,   178,  2004,   168,
      552,  1014,   282,   156,   286,  1586,  1404,  1423,  1012,   582,
     3768,   718,   372,  1558,  1374,  2884,  1246,  1318,   696,   744,
       28,   936,  1318,   252,   814,   994,   942,     6,   466,   336,
     6145,   515,   989,  2198,  1159,  2191,  1288,  3880,   144,  1696,
     2076,   318,    16,   586,   234,  1440,   804,  3070,  1156,   996,
     1380,   840,  1960,   780,   316,  1020,   766,   328,   925,   856,
      337,  2773,  2250,  1189,  2506,   264,   136,
    /* N = 256, e = 189, L in [2048, 4096] */
      387,  4774,  3472,    22,   346,    72,  1090,  1474,  3820,   490,
     4372,  2380,   366,   300,   420,  5922,  2592,  1312,   154,  3592,
     1012,  2170,   330,  3556,  2436,  4214,   910,   807,    83,   103,
     9558,  3551,   531,   232,  1558,  1936,  3238,   346,  4960,  1066,
     2206,    18,   688,  3408,   240,  2706,  1200,  1908,  3126,  2578,
     1446,   990,   750,    12,  3790,   388,  1052,   268,  1090,  1168,
     2853,   495,  2887,  1355,    45,  2530,  1386,   714,  2520,  1810,
     3072,  1054,  5226,  4684,  1522,     4,   696,   114,    94,  3474,
      324,    66,  1882,  4392,   694,  5014,  1914,  1192,   528,    66,
     2033,  1290,  2165,  2151,  3205,   284,   855,   912,   702,  3136,
     2020,   598,  4152,  4782,  4500,   796,  3550,   226,  1932,  5386,
     3192,  1510,  1248,  3190,  1962,  8992,  4302,  3558,  3775,  1133,
      267,  1637,   879,    77,    31,   211,  2761,  1948,  2061,  1950,
      394,   444, 10120,  3076,  4974,  4926,  2320,  1960,   904,  3196,
     1924,  1096,   714,   396,   334,  5334,   126,   582,   996,   288,
      554,   138,  6254,    84,  1262,  4023,  9740,   871,  1420,   846,
     1045,   318,  1146,  7282,  2380,  1432,   598,   256,  3012,   136,
      796,  1792,  4728,  9432,   340,    18,  1020,  1036,   136,     0,
       22,  2934,   182,  6380,  4978,  1805,  5852,  1722,  1575,   492,
      927,  1635,  5205,  2470,    82,   396,  3606,  5422,   862,   352,
     4734,    46, 12180,   370,  1942,   816,  1396,  7414,  5370,  9264,
      270,  3829,  3102,    73,  5152,   872,  2085,  3492,  2902,   261,
      713,  4105,  1555,     9,  2623,  7960,  1170,  3862, 15172,  7560,
      268,   138,  2208,  3906,   280,   456,  1260,   408,  1206,  3012,
     3160,   556,  3582,  1207,  3996,   289,  2278,   481,  1924,  1468,
     3483,     6,  2683,  1459,  4055,  1504,    69, 
  };

  struct assh_key_dsa_s *k;

  ASSH_ERR_RET(assh_alloc(c, sizeof(struct assh_key_dsa_s),
                          ASSH_ALLOC_SECUR, (void**)&k));

  k->key.algo = &assh_key_dsa;

  /* init numbers */
  assh_bignum_init(c, &k->pn, l, 0);
  assh_bignum_init(c, &k->qn, n, 0);
  assh_bignum_init(c, &k->gn, l, 0);
  assh_bignum_init(c, &k->yn, l, 0);
  assh_bignum_init(c, &k->xn, n, 1);

  enum bytecode_args_e
  {
    P, Q, G, Y, X,
    E_x, F_x,
    T0, T1, T2, MT,
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZEM(     T0,     Q,      1,      1       ),
    ASSH_BOP_SIZEM(     T1,     P,      1,      1       ),
    ASSH_BOP_SIZE(      T2,     P                       ),

    /* compute DSA parameters */
    ASSH_BOP_UINT(      T0,     1                       ),
    ASSH_BOP_SHL(       T0,     T0,     0,      Q       ),
    ASSH_BOP_MOVE(      Q,      E_x                     ),
    ASSH_BOP_SUB(       T0,     T0,     Q               ),
    ASSH_BOP_MOVE(      Q,      T0                      ),

    ASSH_BOP_UINT(      T1,     1                       ),
    ASSH_BOP_SHL(       T1,     T1,     0,      P       ),
    ASSH_BOP_UINT(      T2,     1                       ),
    ASSH_BOP_SUB(       T1,     T1,     T2              ),
    ASSH_BOP_MOVE(      P,      T1                      ),

    ASSH_BOP_MOVE(      T2,     F_x                     ),
    ASSH_BOP_MULM(      T2,     Q,      T2,     P       ),
    ASSH_BOP_SUB(       P,      P,      T2              ),

    ASSH_BOP_MOD(       T2,     T1,     Q               ),
    ASSH_BOP_SUB(       P,      P,      T2              ),

    ASSH_BOP_DIV(       T2,     P,      Q               ),

    ASSH_BOP_UINT(      T0,     1                       ),
    ASSH_BOP_ADD(       P,      P,      T0              ),

    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_UINT(      G,      2                       ),
    ASSH_BOP_MTTO(      G,      G,      G,      MT      ),
    ASSH_BOP_EXPM(      G,      G,      T2,     MT      ),

#ifdef CONFIG_ASSH_DEBUG_SIGN
    ASSH_BOP_PRINT(     P,      'P'                     ),
    ASSH_BOP_PRINT(     Q,      'Q'                     ),
    ASSH_BOP_PRINT(     G,      'G'                     ),
#endif

    /* generate key pair */
    ASSH_BOP_RAND(      X,      T0,     Q,
                        ASSH_PRNG_QUALITY_LONGTERM_KEY  ),
    ASSH_BOP_EXPM(    Y,      G,      X,      MT        ),
    ASSH_BOP_PRIVACY( Y,      0 			),
    ASSH_BOP_MTFROM(  Y,      Y,      Y,      MT        ),
    ASSH_BOP_MTFROM(  G,      G,      G,      MT        ),

#ifdef CONFIG_ASSH_DEBUG_SIGN
    ASSH_BOP_PRINT(     Y,      'Y'                     ),
    ASSH_BOP_PRINT(     X,      'X'                     ),
#endif

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "NNNNNiiTTTm",
                 &k->pn, &k->qn, &k->gn, &k->yn, &k->xn,
                 (uintptr_t)e, (uintptr_t)f[(l-1024)/8]), err_key);

  assert(!k->pn.secret && !k->qn.secret &&
         !k->gn.secret && !k->yn.secret && k->xn.secret);

  *key = &k->key;
  return ASSH_OK;

 err_key:
  assh_free(c, k);
  return err;
}

static ASSH_KEY_VALIDATE_FCN(assh_key_dsa_validate)
{
  struct assh_key_dsa_s *k = (void*)key;
  assh_error_t err = ASSH_OK;

  /*
   * FIPS 186-4 Appendix A2.2
   * SP 800-89 section 5.3.1
   */

  unsigned int l = assh_bignum_bits(&k->pn);
  unsigned int n = assh_bignum_bits(&k->qn);

  /* check key size */
  ASSH_CHK_RET(l < 1024 || n < 160 || l > 4096 || n > 256 || l % 8 || n % 8,
               ASSH_ERR_BAD_DATA);

  enum bytecode_args_e
  {
    P, Q, G, X, Y, T1, T2, MT
  };

  static const assh_bignum_op_t bytecode1[] = {
    ASSH_BOP_SIZER(     T1,     T2,     P               ),

    /* check q prime */
    ASSH_BOP_TESTS(     Q,      1,      Q,      0       ),
    ASSH_BOP_ISPRIM(    Q,      0                       ),

    /* check p prime */
    ASSH_BOP_TESTS(     P,      1,      P,      0       ),
    ASSH_BOP_ISPRIM(    P,      0                       ),

    /* check (p-1)%q < 1 */
    ASSH_BOP_UINT(      T1,     1                       ),
    ASSH_BOP_SUB(       T2,     P,      T1              ),
    ASSH_BOP_MOD(       T2,     T2,     Q               ),
    ASSH_BOP_CMPLT(     T2,     T1,     0               ),

    /* check generator range */
    ASSH_BOP_CMPLT(     T1,     G,      0 /* g > 1 */   ),
    ASSH_BOP_CMPLT(     G,      P,      0 /* g < p */   ),

    ASSH_BOP_MTINIT(    MT,     P                       ),

    /* check generator order in the group */
    ASSH_BOP_MTTO(      T2,     T2,     G,      MT      ),
    ASSH_BOP_EXPM(      T2,     T2,     Q,      MT      ),
    ASSH_BOP_MTFROM(    T2,     T2,     T2,     MT      ),
    ASSH_BOP_CMPEQ(     T1,     T2,     0               ),

    /* check public key range */
    ASSH_BOP_CMPLT(     T1,     Y,      0  /* y > 1 */  ),
    ASSH_BOP_SUB(       T2,     P,      T1              ),
    ASSH_BOP_CMPLT(     Y,      T2,     0 /* y < p-1 */ ),

    /* check public key order in the group */
    ASSH_BOP_MTTO(      T2,     T2,     Y,      MT      ),
    ASSH_BOP_EXPM(      T2,     T2,     Q,      MT      ),
    ASSH_BOP_MTFROM(    T2,     T2,     T2,     MT      ),
    ASSH_BOP_CMPEQ(     T1,     T2,     0               ),

    /* check private key */
    ASSH_BOP_CMPEQ(     X,      ASSH_BOP_NOREG, 4       ),
    ASSH_BOP_MTTO(      T2,     T2,     G,      MT      ),
    ASSH_BOP_EXPM(      T2,     T2,     X,      MT      ),
    ASSH_BOP_MTFROM(    T2,     T2,     T2,     MT      ),
    ASSH_BOP_CMPEQ(     T2,     Y,      0               ),

    ASSH_BOP_END(),
  };

  ASSH_ERR_RET(assh_bignum_bytecode(c, bytecode1, "NNNNNTTm",
                             &k->pn, &k->qn, &k->gn, &k->xn, &k->yn));

  return ASSH_OK;
}

static ASSH_KEY_LOAD_FCN(assh_key_dsa_load)
{
  assh_error_t err;

  unsigned int l, n;
  uint8_t *p_str, *q_str, *g_str, *y_str, *x_str;

  /* parse the key blob */
  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6: {

      ASSH_CHK_RET(blob_len < ASSH_DSA_ID_LEN, ASSH_ERR_INPUT_OVERFLOW);
      ASSH_CHK_RET(memcmp(ASSH_DSA_ID, blob, ASSH_DSA_ID_LEN), ASSH_ERR_BAD_DATA);

      p_str = (uint8_t*)blob + ASSH_DSA_ID_LEN;
      ASSH_ERR_RET(assh_check_string(blob, blob_len, p_str, &q_str));
      l = (assh_load_u32(p_str) * 8) & 0xfffffc00;
      ASSH_ERR_RET(assh_check_string(blob, blob_len, q_str, &g_str));
      n = (assh_load_u32(q_str) * 8) & 0xffffffe0;
      ASSH_ERR_RET(assh_check_string(blob, blob_len, g_str, &y_str));
      ASSH_ERR_RET(assh_check_string(blob, blob_len, y_str, NULL));
      x_str = NULL;
      break;
    }

    case ASSH_KEY_FMT_PV_PEM_ASN1: {
      uint8_t *seq, *seq_end, *val;
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, blob, &seq, &seq_end));

      /* sequence type */
      ASSH_CHK_RET(blob[0] != 0x30, ASSH_ERR_BAD_DATA);

      /* skip first value */
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, seq, NULL, &p_str));

      /* parse p, q, g, y, x */
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, p_str, &val, &q_str));
      l = ((q_str - val) * 8) & 0xfffffc00;
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, q_str, &val, &g_str));
      n = ((g_str - val) * 8) & 0xffffffe0;
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, g_str, NULL, &y_str));
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, y_str, NULL, &x_str));
      ASSH_ERR_RET(assh_check_asn1(blob, blob_len, x_str, NULL, NULL));
      break;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_NOTSUP);
    }

  /* allocate key structure */
  ASSH_CHK_RET(l < 1024 || n < 160 || l % 8 || n % 8, ASSH_ERR_BAD_DATA);
  ASSH_CHK_RET(l > 4096 || n > 256, ASSH_ERR_NOTSUP);

  struct assh_key_dsa_s *k = (void*)*key;

  ASSH_ERR_RET(assh_alloc(c, sizeof(struct assh_key_dsa_s),
                          ASSH_ALLOC_SECUR, (void**)&k));

  k->key.algo = &assh_key_dsa;

  /* init numbers */
  assh_bignum_init(c, &k->pn, l, 0);
  assh_bignum_init(c, &k->qn, n, 0);
  assh_bignum_init(c, &k->gn, l, 0);
  assh_bignum_init(c, &k->yn, l, 0);
  assh_bignum_init(c, &k->xn, n, 1);

  /* convert numbers from blob representation */
  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4253_6_6:
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       p_str, &k->pn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       q_str, &k->qn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       g_str, &k->gn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE,
                                       y_str, &k->yn), err_xn);
      break;

    case ASSH_KEY_FMT_PV_PEM_ASN1:
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       p_str, &k->pn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       q_str, &k->qn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       g_str, &k->gn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       y_str, &k->yn), err_xn);
      ASSH_ERR_GTO(assh_bignum_convert(c, ASSH_BIGNUM_ASN1, ASSH_BIGNUM_NATIVE,
                                       x_str, &k->xn), err_xn);
    default:
      break;
    }

  *key = &k->key;
  return ASSH_OK;

 err_xn:
  assh_bignum_release(c, &k->xn);
  assh_bignum_release(c, &k->yn);
  assh_bignum_release(c, &k->gn);
  assh_bignum_release(c, &k->qn);
  assh_bignum_release(c, &k->pn);
  assh_free(c, k);
  return err;
}

static ASSH_KEY_CLEANUP_FCN(assh_key_dsa_cleanup)
{
  struct assh_key_dsa_s *k = (void*)key;

  assh_bignum_release(c, &k->xn);
  assh_bignum_release(c, &k->yn);
  assh_bignum_release(c, &k->gn);
  assh_bignum_release(c, &k->qn);
  assh_bignum_release(c, &k->pn);
  assh_free(c, k);
}

const struct assh_key_ops_s assh_key_dsa =
{
  .type = "ssh-dss",
  .f_output = assh_key_dsa_output,
  .f_create = assh_key_dsa_create,
  .f_validate = assh_key_dsa_validate,
  .f_cmp = assh_key_dsa_cmp,
  .f_load = assh_key_dsa_load,
  .f_cleanup = assh_key_dsa_cleanup,
};

