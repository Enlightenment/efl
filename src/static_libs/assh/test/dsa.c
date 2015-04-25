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


#include <assh/assh_bignum.h>
#include <assh/assh_context.h>

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

static const unsigned int n = 160, l = 512;

static const uint8_t *mp_pn = (const uint8_t*)"\x00\x00\x00\x41" "\x00\xd4\x11\xa4\xa0\xe3\x93\xf6\xaa\xb0\xf0\x8b\x14\xd1\x84\x58\x66\x5b\x3e\x4d\xbd\xce\x25\x44\x54\x3f\xe3\x65\xcf\x71\xc8\x62\x24\x12\xdb\x6e\x7d\xd0\x2b\xbe\x13\xd8\x8c\x58\xd7\x26\x3e\x90\x23\x6a\xf1\x7a\xc8\xa9\xfe\x5f\x24\x9c\xc8\x1f\x42\x7f\xc5\x43\xf7";
static const uint8_t *mp_qn = (const uint8_t*)"\x00\x00\x00\x15" "\x00\xb2\x0d\xb0\xb1\x01\xdf\x0c\x66\x24\xfc\x13\x92\xba\x55\xf7\x7d\x57\x74\x81\xe5";
static const uint8_t *mp_gn = (const uint8_t*)"\x00\x00\x00\x41" "\x00\xb3\x08\x55\x10\x02\x1f\x99\x90\x49\xa9\xe7\xcd\x38\x72\xce\x99\x58\x18\x6b\x50\x07\xe7\xad\xaf\x25\x24\x8b\x58\xa3\xdc\x4f\x71\x78\x1d\x21\xf2\xdf\x89\xb7\x17\x47\xbd\x54\xb3\x23\xbb\xec\xc4\x43\xec\x1d\x3e\x02\x0d\xad\xab\xbf\x78\x22\x57\x82\x55\xc1\x04";
static const uint8_t *mp_yn = (const uint8_t*)"\x00\x00\x00\x41" "\x00\xb3\x2f\xbe\xc0\x31\x75\x79\x1d\xf0\x8c\x3f\x86\x1c\x81\xdf\x7d\xe7\xe0\xcb\xa7\xf1\xc4\xf7\x26\x9b\xb1\x2d\x6c\x62\x87\x84\xfb\x74\x2e\x66\xed\x31\x57\x54\xdf\xe3\x8b\x59\x84\xe9\x4d\x37\x25\x37\xf6\x55\xcb\x3e\xa4\x76\x7c\x87\x8c\xbd\x2d\x78\x3e\xe6\x62";
static const uint8_t *mp_xn = (const uint8_t*)"\x00\x00\x00\x14" "\x6b\x2c\xd9\x35\xd0\x19\x2d\x54\xe2\xc9\x42\xb5\x74\xc8\x01\x02\xc8\xf8\xef\x67";
static const uint8_t *mp_kn = (const uint8_t*)"\x00\x00\x00\x14" "\x79\x57\x7d\xdc\xaa\xfd\xdc\x03\x8b\x86\x5b\x19\xf8\xeb\x1a\xda\x8a\x28\x38\xc6";
static const uint8_t *mp_m  = (const uint8_t*)"\x00\x00\x00\x14" "\x01\x64\xb8\xa9\x14\xcd\x2a\x5e\x74\xc4\xf7\xff\x08\x2c\x4d\x97\xf1\xed\xf8\x80";
static const uint8_t *mp_r  = (const uint8_t*)"\x00\x00\x00\x15" "\x00\x9b\x77\xf7\x05\x4c\x81\x53\x1c\x4e\x46\xa4\x69\x2f\xbf\xe0\xf7\x7f\x7e\xbf\xf2";

assh_error_t dsa_generate(struct assh_context_s *c,
			  struct assh_bignum_s *rn,
			  struct assh_bignum_s *sn)
{
  assh_error_t err;

  enum bytecode_args_e
  {
    /* input mp strings */
    P_mp, Q_mp, G_mp, Y_mp, X_mp, K_mp, M_mp,
    /* output values */
    R, S,
    /* temporary numbers from input strings */
    P, Q, G, X, K, M,
    /* temporary numbers */
    R1, R2, R3,
    /* bit size */
    N, L,
    /* montgomery context */
    MT
  };

  assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZE(	P,	L			),
    ASSH_BOP_MOVE(      P,      P_mp			),

    ASSH_BOP_SIZE(	Q,	N			),
    ASSH_BOP_MOVE(      Q,      Q_mp			),

    ASSH_BOP_SIZE(	G,	L			),
    ASSH_BOP_MOVE(      G,      G_mp			),

    ASSH_BOP_SIZE(	X,	N			),
    ASSH_BOP_MOVE(      X,      X_mp			),

    ASSH_BOP_SIZE(	K,	N			),
    ASSH_BOP_MOVE(      K,      K_mp			),

    ASSH_BOP_SIZE(	M,	N			),
    ASSH_BOP_MOVE(      M,      M_mp			),

    ASSH_BOP_SIZE(	R1,	N			),
    ASSH_BOP_SIZE(	R2,	N			),
    ASSH_BOP_SIZE(	R3,	L			),

    ASSH_BOP_SIZE(	R,	N			),
    ASSH_BOP_SIZE(	S,	N			),

    ASSH_BOP_MTINIT(	MT,	P			),
    /* g^k mod p */
    ASSH_BOP_MTTO(      G,	G,	G,	MT	),
    ASSH_BOP_EXPM(      R3,     G,      K,	MT      ),
    ASSH_BOP_MTFROM(    R3,	R3,	R3,	MT	),
    /* r = (g^k mod p) mod q */
    ASSH_BOP_MOD(       R3,     R3,      Q		),
    ASSH_BOP_MOVE(      R,      R3			),
    /* (x * r) mod q */
    ASSH_BOP_MTINIT(	MT,	Q			),
    ASSH_BOP_MTTO(      R2,	R2,	R,	MT	),
    ASSH_BOP_MTTO(      X,	X,	X,	MT	),
    ASSH_BOP_MTTO(      M,	M,	M,	MT	),
    ASSH_BOP_MULM(      R1,     X,      R2,	MT	),
    /* sha(m) + (x * r) */
    ASSH_BOP_ADDM(      R2,     M,      R1,	MT      ),
    /* k^-1 */
    ASSH_BOP_INV(       R1,     K,      Q		),
    ASSH_BOP_MTTO(      R1,	R1,	R1,	MT	),
    /* s = k^-1 * (sha(m) + (x * r)) mod q */
    ASSH_BOP_MULM(      S,      R1,     R2,	MT      ),
    ASSH_BOP_MTFROM(    S,	S,	S,	MT	),
    ASSH_BOP_PRIVACY(   S,	0			),

    ASSH_BOP_PRINT(	R,	'R'			),
    ASSH_BOP_PRINT(	S,	'S'			),

    ASSH_BOP_END(),
  };

  ASSH_ERR_RET(assh_bignum_bytecode(c, bytecode,
	      /* mpint */ "MMMMMMM"
	      /* r, s */ "NN" /* temps: */ "TTTXTTTTT"
	      /* sizes */ "ss" /* montgomery */ "m",

	      /* char* */   mp_pn, mp_qn, mp_gn, mp_yn, mp_xn, mp_kn, mp_m ,
	      /* bignums */ rn, sn, /* bit sizes */ n, l));

  return ASSH_OK;
}

assh_error_t dsa_verify(struct assh_context_s *c,
			struct assh_bignum_s *rn,
			struct assh_bignum_s *sn,
			struct assh_bignum_s *vn)
{
  assh_error_t err;

  enum bytecode_args_e
  {
    /* input mp strings */
    P_mp, Q_mp, G_mp, Y_mp, M_mp,
    /* input values */
    R, S,
    /* output values */
    V,
    /* temporary numbers from input strings */
    P, Q, G, Y, M,
    /* temporary numbers */
    W, U1, V1, U2, V2,
    /* bit size */
    N, L,
    /* montgomery context */
    MT
  };

  assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZE(	P,	L			),
    ASSH_BOP_MOVE(      P,      P_mp			),

    ASSH_BOP_SIZE(	Q,	N			),
    ASSH_BOP_MOVE(      Q,      Q_mp			),

    ASSH_BOP_SIZE(	G,	L			),
    ASSH_BOP_MOVE(      G,      G_mp			),

    ASSH_BOP_SIZE(	Y,	L			),
    ASSH_BOP_MOVE(      Y,      Y_mp			),

    ASSH_BOP_SIZE(	M,	N			),
    ASSH_BOP_MOVE(      M,      M_mp			),

    ASSH_BOP_SIZE(	V,	N			),

    ASSH_BOP_SIZE(	W,	N			),
    ASSH_BOP_SIZE(	U1,	N			),
    ASSH_BOP_SIZE(	V1,	L			),
    ASSH_BOP_SIZE(	U2,	N			),
    ASSH_BOP_SIZE(	V2,	L			),

    ASSH_BOP_INV(       W,      S,      Q		),
    /* r * w mod q */
    ASSH_BOP_MULM(      U2,     R,      W,	Q       ),

    /* (sha(m) * w) mod q */
    ASSH_BOP_MULM(      U1,     M,      W,	Q       ),
    /* g^u1 */
    ASSH_BOP_MTINIT(	MT,	P			),
    ASSH_BOP_MTTO(      G,	G,	G,	MT	),
    ASSH_BOP_EXPM(      V1,     G,      U1,	MT	),
    /* y^u2 */
    ASSH_BOP_MTTO(      Y,	Y,	Y,	MT	),
    ASSH_BOP_EXPM(      V2,     Y,      U2,	MT	),
    /* (g^u1 * y^u2) mod p */
    ASSH_BOP_MULM(      Y,      V1,     V2,	MT	),
    /* v = (g^u1 * y^u2) mod p mod q */
    ASSH_BOP_MTFROM(    Y,	Y,	Y,	MT	),
    ASSH_BOP_MOD(       V,      Y,      Q		),

    ASSH_BOP_PRINT(	R,	'R'			),
    ASSH_BOP_PRINT(	V,	'V'			),
    ASSH_BOP_CMPEQ(     V,      R,	0		),

    ASSH_BOP_END(),
  };

  ASSH_ERR_RET(assh_bignum_bytecode(c, bytecode,
	      /* mpint */ "MMMMM"
	      /* r, s, v */ "NNN" /* temps: */ "TTTTTTTTTT"
              /* sizes */ "ss" /* montgomery */ "m",

	      /* char* */   mp_pn, mp_qn, mp_gn, mp_yn, mp_m ,
	      /* bignums */ rn, sn, vn, /* bit sizes */ n, l));

  return ASSH_OK;
}

int main()
{
  assh_error_t err;
  struct assh_context_s context;

#ifdef CONFIG_ASSH_USE_GCRYPT
  if (!gcry_check_version(GCRYPT_VERSION))
    return -1;
#endif

  assh_context_init(&context, ASSH_SERVER);

  struct assh_bignum_s rn;
  struct assh_bignum_s sn;

  assh_bignum_init(&context, &rn, n, 0);
  assh_bignum_init(&context, &sn, n, 0);

  dsa_generate(&context, &rn, &sn);

  struct assh_bignum_s vn;

  assh_bignum_init(&context, &vn, n, 0);

  dsa_verify(&context, &rn, &sn, &vn);

  /* check V against constant R */
  enum bytecode_args_e
  {
    R_mp, R, V, N
  };

  assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZE(	V,	N			),

    ASSH_BOP_SIZE(	R,	N			),
    ASSH_BOP_MOVE(      R,      R_mp			),

    ASSH_BOP_CMPEQ(     V,      R,	0		),

    ASSH_BOP_END(),
  };

  ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "MTNs",
				    mp_r, &vn, n));

  assh_bignum_release(&context, &rn);
  assh_bignum_release(&context, &sn);
  assh_bignum_release(&context, &vn);

  return 0;
}
