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
#include <assh/key_eddsa.h>
#include <assh/assh_hash.h>
#include <assh/assh_prng.h>
#include <assh/assh_alloc.h>

#include <string.h>

static ASSH_SIGN_GENERATE_FCN(assh_sign_eddsa_generate)
{
  const struct assh_key_eddsa_s *k = (const void*)key;
  assh_error_t err;

  const struct assh_edward_curve_s *curve = k->curve;
  const struct assh_hash_algo_s *hash = k->hash;

  /* check availability of the private key */
  ASSH_CHK_RET(!k->private, ASSH_ERR_MISSING_KEY);

  size_t n = ASSH_ALIGN8(k->curve->bits) / 8;
  size_t tlen = strlen(k->key.algo->type);
  size_t len = 4 + tlen + 4 + 2 * n;

  /* check/return signature length */
  if (sign == NULL)
    {
      *sign_len = len;
      return ASSH_OK;
    }

  ASSH_CHK_RET(*sign_len < len, ASSH_ERR_OUTPUT_OVERFLOW);
  *sign_len = len;

  assh_store_u32(sign, tlen);
  memcpy(sign + 4, k->key.algo->type, tlen);
  assh_store_u32(sign + 4 + tlen, 2 * n);
  uint8_t *r_str = sign + 4 + tlen + 4;
  uint8_t *s_str = r_str + n;

  const uint8_t *kp = k->data;
  const uint8_t *ks = k->data + n;

  ASSH_SCRATCH_ALLOC(c, uint8_t, sc,
    hash->ctx_size + /* h */ n * 2 + /* r */ n * 2 +  /* hram */ n * 2 +
    /* az */ n * 2 + /* rx */ n, ASSH_ERRSV_CONTINUE, err_);

  void    *hash_ctx = sc;
  uint8_t *h = sc + hash->ctx_size;
  uint8_t *r = h + 2 * n;
  uint8_t *hram = r + 2 * n;
  uint8_t *az = hram + 2 * n;
  uint8_t *rx = az + 2;

  unsigned int i;

  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, hash), err_scratch);
  assh_hash_update(hash_ctx, ks, n);
  assh_hash_final(hash_ctx, h, n * 2);
  assh_hash_cleanup(hash_ctx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("h", h, 2 * n);
#endif

  /* a is h[0,n-1] */
  assh_edward_adjust(curve, h);

  /* r */
  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, hash), err_scratch);
  assh_hash_update(hash_ctx, h + n, n);
  for (i = 0; i < data_count; i++)
    assh_hash_update(hash_ctx, data[i], data_len[i]);
  assh_hash_final(hash_ctx, r, n * 2);
  assh_hash_cleanup(hash_ctx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("r", r, 2 * n);
#endif

  struct assh_bignum_lad_s lad = {
    .data = r,
    .count = n * 8 * 2,
    .msbyte_1st = 0,
    .msbit_1st = 1,
  };

  {
    enum {
      /* in */
      BX_mpint, BY_mpint, A_mpint, P_mpint, D_mpint,
      L, P_n,
      /* out */
      RX_raw, RY_raw,
      /* temp */
      MT, A, D,
      RX, RY, RZ,  BX, BY, BZ,
      PX, PY, PZ,  QX, QY, QZ, T0, T1,
    };

    static const assh_bignum_op_t bytecode1[] = {

      ASSH_BOP_SIZER(   A,      T1,     P_n             ),

      /* init */
      ASSH_BOP_MOVE(    T0,     P_mpint                 ),
      ASSH_BOP_MTINIT(	MT,     T0                      ),
      ASSH_BOP_MOVE(    A,      A_mpint                 ),
      ASSH_BOP_MOVE(    D,      D_mpint                 ),

      ASSH_BOP_UINT(    RX,     0                       ),
      ASSH_BOP_UINT(    RY,     1                       ),
      ASSH_BOP_UINT(    RZ,     1                       ),
      ASSH_BOP_MOVE(    BX,     BX_mpint                ),
      ASSH_BOP_MOVE(    BY,     BY_mpint                ),
      ASSH_BOP_UINT(    BZ,     1                       ),

      ASSH_BOP_MTTO(	A,      BZ,    A,      MT       ),

      /* ladder */
      ASSH_BOP_TEDWARD_PDBL( PX, PY, PZ,  RX, RY, RZ,
                             T0, T1, MT                 ),

      ASSH_BOP_TEDWARD_PADD( QX, QY, QZ,  PX, PY, PZ,
                             BX, BY, BZ,  T0, T1, A, D, MT ),

      ASSH_BOP_MOVE(    RX,     PX                      ),
      ASSH_BOP_MOVE(    RY,     PY                      ),
      ASSH_BOP_MOVE(    RZ,     PZ                      ),

      ASSH_BOP_LADSWAP( RX,     QX,     L               ),
      ASSH_BOP_LADSWAP( RY,     QY,     L               ),
      ASSH_BOP_LADSWAP( RZ,     QZ,     L               ),

      ASSH_BOP_LADLOOP( 42,             L               ),

      /* projective to affine */
      ASSH_BOP_INV(     T0,     RZ,     MT              ),
      ASSH_BOP_MULM(    RX,     RX,     T0,     MT      ),
      ASSH_BOP_MULM(    RY,     RY,     T0,     MT      ),

      ASSH_BOP_MTFROM(	RX,     RY,     RX,     MT      ),

      ASSH_BOP_MOVE(    RX_raw, RX                      ),
      ASSH_BOP_MOVE(    RY_raw, RY                      ),

      ASSH_BOP_END(),
    };

    ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode1,
      "MMMMMLsddmTTXXXXXXTTTTTTTT", curve->bx, curve->by,
      curve->a, curve->p, curve->d, &lad, curve->bits, rx, r_str), err_scratch);
  }

  /* encode point */
  assh_edward_encode(curve, r_str, rx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("rxy", r_str, n);
#endif

  /* hram */
  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, hash), err_scratch);
  assh_hash_update(hash_ctx, r_str, n);
  assh_hash_update(hash_ctx, kp, n);
  for (i = 0; i < data_count; i++)
    assh_hash_update(hash_ctx, data[i], data_len[i]);
  assh_hash_final(hash_ctx, hram, n * 2);
  assh_hash_cleanup(hash_ctx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("hram", hram, 2 * n);
#endif

  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, hash), err_scratch);
  assh_hash_update(hash_ctx, ks, n);
  assh_hash_final(hash_ctx, az, n * 2);
  assh_hash_cleanup(hash_ctx);

  assh_edward_adjust(curve, az);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("az", az, 2 * n);
#endif

  {
    enum {
      L_mpint, H_raw, AZ_raw, R_raw, S_raw, P_n,
      L, T0, T1, S, MT
    };

    static const assh_bignum_op_t bytecode2[] = {
      ASSH_BOP_SIZEM(   T1,     P_n,    0,      1       ),
      ASSH_BOP_SIZE(    T0,     T1                      ),
      ASSH_BOP_SIZE(    L,      T1                      ),
      ASSH_BOP_SIZE(    S,      P_n                     ),

      ASSH_BOP_MOVE(    L,      L_mpint                 ),
      ASSH_BOP_MTINIT(  MT,     L                       ),

      ASSH_BOP_MOVE(    T1,     H_raw                   ),
      ASSH_BOP_MOVE(    S,      AZ_raw                  ),
      ASSH_BOP_MOVE(    T0,     S                       ),
      ASSH_BOP_MTTO(    T0,     T1,     T0,     MT      ),
      ASSH_BOP_MULM(    T1,     T1,     T0,     MT      ),

      ASSH_BOP_MOVE(    T0,     R_raw                   ),
      ASSH_BOP_MTTO(    T0,     T0,     T0,     MT      ),
      ASSH_BOP_ADDM(    T1,     T1,     T0,     MT      ),

      ASSH_BOP_MTFROM(  T1,     T1,     T1,     MT      ),
      ASSH_BOP_MOVE(    S,      T1                      ),
      ASSH_BOP_MOVE(    S_raw,  S                       ),

      ASSH_BOP_END(),
    };

    ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode2, "MddddsTXXXm",
      curve->l, hram, az, r, s_str, n * 8), err_scratch);
  }

  ASSH_SCRATCH_FREE(c, sc);
  return ASSH_OK;

 err_scratch:
  ASSH_SCRATCH_FREE(c, sc);
 err_:
  return err;
}

static ASSH_SIGN_CHECK_FCN(assh_sign_eddsa_check)
{
  const struct assh_key_eddsa_s *k = (const void*)key;
  assh_error_t err;

  const struct assh_edward_curve_s *curve = k->curve;
  const struct assh_hash_algo_s *hash = k->hash;

  size_t n = ASSH_ALIGN8(k->curve->bits) / 8;
  size_t tlen = strlen(k->key.algo->type);

  ASSH_CHK_RET(sign_len != 4 + tlen + 4 + 2 * n, ASSH_ERR_INPUT_OVERFLOW);

  ASSH_CHK_RET(tlen != assh_load_u32(sign), ASSH_ERR_BAD_DATA);
  ASSH_CHK_RET(memcmp(sign + 4, k->key.algo->type, tlen), ASSH_ERR_BAD_DATA);

  uint8_t *rs_str = (uint8_t*)sign + 4 + tlen;
  ASSH_CHK_RET(assh_load_u32(rs_str) != n * 2, ASSH_ERR_INPUT_OVERFLOW);

  const uint8_t *kp = k->data;

  ASSH_SCRATCH_ALLOC(c, uint8_t, sc,
    hash->ctx_size + /* hram */ n * 2 + /* rx */ n + /* ry */ n,
                     ASSH_ERRSV_CONTINUE, err_);

  void    *hash_ctx = sc;
  uint8_t *hram = sc + hash->ctx_size;
  uint8_t *rx = hram + 2 * n;
  uint8_t *ry = rx + n;

  unsigned int i;

  ASSH_ERR_GTO(assh_hash_init(c, hash_ctx, hash), err_scratch);
  assh_hash_update(hash_ctx, rs_str + 4, n);
  assh_hash_update(hash_ctx, kp, n);
  for (i = 0; i < data_count; i++)
    assh_hash_update(hash_ctx, data[i], data_len[i]);
  assh_hash_final(hash_ctx, hram, n * 2);
  assh_hash_cleanup(hash_ctx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("pub", kp, n);
#endif

  /* key X sign bit as mpint */
  uint8_t kx[5] = { 0, 0, 0, kp[n-1] >> 7, 1 };

  struct assh_bignum_lad_s lad1 = {
    .data = hram,
    .count = n * 8 * 2,
    .msbyte_1st = 0,
    .msbit_1st = 1,
  };

  struct assh_bignum_lad_s lad2 = {
    .data = rs_str + 4 + n,
    .count = n * 8,
    .msbyte_1st = 0,
    .msbit_1st = 1,
  };

  enum {
    /* in */
    BX_mpint, BY_mpint, A_mpint, P_mpint, D_mpint, I_mpint,
    L1, L2, P_n, KY_raw, KX_mpint,
    /* out */
    RX_raw, RY_raw,
    /* temp */
    P, A, D, T0, T1,
    BX, BY, BZ,  RX, RY, RZ,  PX, PY, PZ,
    QX, QY, QZ,  MT, U = PY, V = PZ,
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZER(     P,      QZ,     P_n            ),

    ASSH_BOP_MOVE(      P,      P_mpint                 ),
    ASSH_BOP_MOVE(      A,      A_mpint                 ),
    ASSH_BOP_MOVE(      D,      D_mpint                 ),

    ASSH_BOP_MOVE(      BY,     KY_raw                  ),

    /* u = y^2-1, v = d*y^2-a */
    ASSH_BOP_MULM(      U,      BY,     BY,     P       ),
    ASSH_BOP_MULM(      V,      U,      D,      P       ),
    ASSH_BOP_UINT(      T0,     1                       ),
    ASSH_BOP_SUBM(      U,      U,      T0,     P       ),
    ASSH_BOP_SUBM(      V,      V,      A,      P       ),

    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      U,      V,      U,      MT      ),
    ASSH_BOP_MTTO(      A,      D,      A,      MT      ),

    /* compute x = sqrt(u/v), the method depends on the value of P.
       This is tricky when p%8 == 1 (does not occur in used curves) */

    ASSH_BOP_TESTS(     P,      1,      ASSH_BOP_NOREG,  22       ),

    /*** case p%8 == 5: x = (uv^3)*(uv^7)^((p-5)/8) */

    /* v3 = v^3 */
    ASSH_BOP_MULM(      T0,     V,      V,      MT      ),
    ASSH_BOP_MULM(      T1,     T0,     V,      MT      ),

    /* x = uv^7 */
    ASSH_BOP_MULM(      PX,     T1,     T1,     MT      ),
    ASSH_BOP_MULM(      PX,     PX,     V,      MT      ),
    ASSH_BOP_MULM(      PX,     PX,     U,      MT      ),

    /* x = (uv^7)^((p-5)/8) */
    ASSH_BOP_UINT(      T0,     5                       ),
    ASSH_BOP_SUB(       T0,     P,      T0              ),
    ASSH_BOP_SHR(       T0,     T0,     3,      ASSH_BOP_NOREG  ),
    ASSH_BOP_EXPM(      BX,     PX,     T0,     MT      ),

    /* x = (uv^3)*(uv^7)^((p-5)/8) */
    ASSH_BOP_MULM(      BX,     BX,     T1,     MT      ),
    ASSH_BOP_MULM(      BX,     BX,     U,      MT      ),

    /* check v*x^2 == +/-u */
    ASSH_BOP_MULM(      PX,     BX,     BX,     MT      ),
    ASSH_BOP_MULM(      PX,     PX,     V,      MT      ),

    ASSH_BOP_SUBM(      T0,     PX,     U,      MT      ),
    ASSH_BOP_ADDM(      T1,     PX,     U,      MT      ),

    ASSH_BOP_MTFROM(    T0,     BX,     T0,     MT      ),    

    ASSH_BOP_UINT(      PX,     0                       ),
    ASSH_BOP_CMPEQ(     T0,     PX,     3               ),
    ASSH_BOP_CMPEQ(     T1,     PX,     0 /* abort */   ),
    ASSH_BOP_MOVE(      T0,     I_mpint                 ),
    ASSH_BOP_MULM(      BX,     BX,     T0,     P       ),
#warning check CMPEQs
    ASSH_BOP_JMP(       15                              ),

    /*** case p%4 == 3: x = (uv)*(uv^3)^((p-3)/4) */

    /* x = uv^3 */
    ASSH_BOP_MULM(      PX,     V,      V,      MT      ),
    ASSH_BOP_MULM(      PX,     PX,     V,      MT      ),
    ASSH_BOP_MULM(      PX,     PX,     U,      MT      ),

    /* x = (uv^3)^((p-3)/4) */
    ASSH_BOP_UINT(      T0,     3                       ),
    ASSH_BOP_SUB(       T0,     P,      T0              ),
    ASSH_BOP_SHR(       T0,     T0,     2,      ASSH_BOP_NOREG  ),
    ASSH_BOP_EXPM(      BX,     PX,     T0,     MT      ),

    /* x = (uv)*(uv^3)^((p-3)/4) */
    ASSH_BOP_MULM(      BX,     BX,     U,      MT      ),
    ASSH_BOP_MULM(      BX,     BX,     V,      MT      ),

    /* check v*x^2 == u */
    ASSH_BOP_MULM(      PX,     BX,     BX,     MT      ),
    ASSH_BOP_MULM(      PX,     PX,     V,      MT      ),

    ASSH_BOP_SUBM(      T1,     PX,     U,      MT      ),

    ASSH_BOP_MTFROM(    T1,     BX,     T1,     MT      ),
    ASSH_BOP_UINT(      PX,     0                       ),
    ASSH_BOP_CMPEQ(     T1,     PX,     0 /* abort */   ),

    /***********/

    /* x = -x if sign of x does not match sign bit in encoded key */
    ASSH_BOP_TESTS(     BX,     0,      ASSH_BOP_NOREG,  1      ),
    ASSH_BOP_SUBM(      BX,     P,      BX,     P               ),
    ASSH_BOP_MOVE(      T0,     KX_mpint                        ),
    ASSH_BOP_TESTC(     T0,     0,      ASSH_BOP_NOREG,  1      ),
    ASSH_BOP_SUBM(      BX,     P,      BX,     P               ),

#ifdef CONFIG_ASSH_DEBUG_SIGN
    ASSH_BOP_PRINT(     BX,     'X'                     ),
    ASSH_BOP_PRINT(     BY,     'Y'                     ),
#endif
    ASSH_BOP_UINT(      BZ,     1                       ),

    /* compute H(R,A,M).A */
    ASSH_BOP_UINT(      RX,     0                       ),
    ASSH_BOP_UINT(      RY,     1                       ),
    ASSH_BOP_UINT(      RZ,     1                       ),

    ASSH_BOP_MTTO(      BX,     BZ,     BX,     MT      ),
    ASSH_BOP_MTTO(      RY,     RZ,     RY,     MT      ),

    ASSH_BOP_TEDWARD_PDBL( PX, PY, PZ,  RX, RY, RZ,
                           T0, T1, MT                   ),
    ASSH_BOP_LADJMP(   L1,     4                        ),
    ASSH_BOP_MOVE(      RX,     PX                      ),
    ASSH_BOP_MOVE(      RY,     PY                      ),
    ASSH_BOP_MOVE(      RZ,     PZ                      ),
    ASSH_BOP_JMP(       20                              ),
    ASSH_BOP_TEDWARD_PADD( RX, RY, RZ,  PX, PY, PZ,
                           BX, BY, BZ,  T0, T1, A, D, MT ),
    ASSH_BOP_LADLOOP(  41,     L1                      ),

    /* compute S.B */
    ASSH_BOP_MOVE(      BX,     BX_mpint                ),
    ASSH_BOP_MOVE(      BY,     BY_mpint                ),
    ASSH_BOP_UINT(      BZ,     1                       ),

    ASSH_BOP_UINT(      QX,     0                       ),
    ASSH_BOP_UINT(      QY,     1                       ),
    ASSH_BOP_UINT(      QZ,     1                       ),

    ASSH_BOP_MTTO(      BX,     BZ,     BX,     MT      ),
    ASSH_BOP_MTTO(      QY,     QZ,     QY,     MT      ),

    ASSH_BOP_TEDWARD_PDBL( PX, PY, PZ,  QX, QY, QZ,
                           T0, T1, MT                   ),
    ASSH_BOP_LADJMP(    L2,     4                       ),
    ASSH_BOP_MOVE(      QX,     PX                      ),
    ASSH_BOP_MOVE(      QY,     PY                      ),
    ASSH_BOP_MOVE(      QZ,     PZ                      ),
    ASSH_BOP_JMP(       20                              ),
    ASSH_BOP_TEDWARD_PADD( QX, QY, QZ,  PX, PY, PZ,
                           BX, BY, BZ,  T0, T1, A, D, MT ),
    ASSH_BOP_LADLOOP(   41,     L2                      ),

    /* compute S.B + H(R,A,M).A */
    ASSH_BOP_TEDWARD_PADD( PX, PY, PZ,  RX, RY, RZ,
                           QX, QY, QZ,  T0, T1, A, D, MT ),

    ASSH_BOP_MTFROM(    PX,     PZ,     PX,     MT      ),
    ASSH_BOP_INV(       T0,     PZ,     P               ),
    ASSH_BOP_MULM(      RX,     PX,     T0,     P       ),
    ASSH_BOP_MULM(      RY,     PY,     T0,     P       ),

    ASSH_BOP_MOVE(      RX_raw, RX                      ),
    ASSH_BOP_MOVE(      RY_raw, RY                      ),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode,
          "MMMMMMLLsdMddTTTTTTTTTTTTTTTTTm", curve->bx, curve->by,
          curve->a, curve->p, curve->d, curve->i,
          &lad1, &lad2, curve->bits, kp, kx, rx, ry), err_scratch);

  /* encode point */
  assh_edward_encode(curve, ry, rx);

#ifdef CONFIG_ASSH_DEBUG_SIGN
  assh_hexdump("hram", hram, 2 * n);
  assh_hexdump("sign", rs_str + 4, 2 * n);
  assh_hexdump("rxy", ry, n);
#endif

  ASSH_CHK_GTO(assh_memcmp(rs_str + 4, ry, n),
               ASSH_ERR_NUM_COMPARE_FAILED, err_scratch);

  err = ASSH_OK;

 err_scratch:
  ASSH_SCRATCH_FREE(c, sc);
 err_:
  return err;
}


static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_ed25519_suitable_key)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  return key->algo == &assh_key_ed25519;
}

const struct assh_algo_sign_s assh_sign_ed25519 =
{
  .algo = {
    .name = "ssh-ed25519", .class_ = ASSH_ALGO_SIGN,
    .safety = 50, .speed = 90,
    .f_suitable_key = assh_sign_ed25519_suitable_key,
    .key = &assh_key_ed25519,
  },
  .f_generate = assh_sign_eddsa_generate,
  .f_check = assh_sign_eddsa_check,
};


static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_eddsa_e382_suitable_key)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  return key->algo == &assh_key_eddsa_e382;
}

const struct assh_algo_sign_s assh_sign_eddsa_e382 =
{
  .algo = {
    .name = "eddsa-e382-shake256@libassh.org", .class_ = ASSH_ALGO_SIGN,
    .safety = 70, .speed = 80,
    .f_suitable_key = assh_sign_eddsa_e382_suitable_key,
    .key = &assh_key_eddsa_e382,
  },
  .f_generate = assh_sign_eddsa_generate,
  .f_check = assh_sign_eddsa_check,
};


static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_eddsa_e521_suitable_key)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  return key->algo == &assh_key_eddsa_e521;
}

const struct assh_algo_sign_s assh_sign_eddsa_e521 =
{
  .algo = {
    .name = "eddsa-e521-shake256@libassh.org", .class_ = ASSH_ALGO_SIGN,
    .safety = 90, .speed = 70,
    .f_suitable_key = assh_sign_eddsa_e521_suitable_key,
    .key = &assh_key_eddsa_e521,
  },
  .f_generate = assh_sign_eddsa_generate,
  .f_check = assh_sign_eddsa_check,
};
