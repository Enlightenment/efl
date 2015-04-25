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

/**
   @file
   @short Key support for the EdDSA signature algorithm
   @internal
*/

#ifndef ASSH_KEY_EDDSA_H_
#define ASSH_KEY_EDDSA_H_

#include <assh/assh_key.h>
#include <assh/assh_bignum.h>

/** @internal Edward elliptic curve parameters decriptor.
    @em {a*x^2+y^2 = 1+d*x^2y^2} */
struct assh_edward_curve_s
{
  const uint8_t *p;
  const uint8_t *l; /* order */
  const uint8_t *bx; /* basepoint x */
  const uint8_t *by; /* basepoint y */
  const uint8_t *a;
  const uint8_t *d;
  const uint8_t *i; /* sqrt(-1), used when p%8 == 5 */
  size_t bits;
  uint_fast8_t cofactor;
};

/** @internal EdDSA key storage */
struct assh_key_eddsa_s
{
  struct assh_key_s key;

  const struct assh_edward_curve_s *curve;
  const struct assh_hash_algo_s *hash;

  assh_bool_t private;

  /** public + secret key data */
  uint8_t data[0];
};

ASSH_FIRST_FIELD_ASSERT(assh_key_eddsa_s, key);

/** @multiple @internal Key operations descriptor for EdDSA keys */
extern const struct assh_key_ops_s assh_key_ed25519;
extern const struct assh_key_ops_s assh_key_eddsa_e382;
extern const struct assh_key_ops_s assh_key_eddsa_e521;

/** @multiple @internal Edward curve parameters */
extern const struct assh_edward_curve_s assh_ed25519_curve;
extern const struct assh_edward_curve_s assh_e382_curve;
extern const struct assh_edward_curve_s assh_e521_curve;

/** @internal Adjust blob for use with edward curvre */
ASSH_INLINE void
assh_edward_adjust(const struct assh_edward_curve_s *curve, uint8_t *blob)
{
  uint_fast8_t i = (8 - curve->bits) & 7;
  uint_fast8_t j = (curve->bits - 1) / 8;

  blob[0] -= blob[0] % curve->cofactor;
  blob[j] &= 0xff >> i;
  blob[j] |= 0x80 >> i;
}

/** @internal Edward curve point encoding */
ASSH_INLINE void 
assh_edward_encode(const struct assh_edward_curve_s *curve,
                   uint8_t y[], const uint8_t x[])
{
  uint_fast8_t j = (curve->bits - 1) / 8;
  y[j] |= ((x[0] & 1) << 7);
}

/** @internal addition on twisted edward curve, projective
    coordinate. 20 ops */
#define ASSH_BOP_TEDWARD_PADD(X3, Y3, Z3, X1, Y1, Z1,		\
			      X2, Y2, Z2, T0, T1, A, D, P)	\
    /* A = Z1*Z2 */						\
    ASSH_BOP_MULM(	Z3,	Z1,	Z2,	P	),	\
    /* I = (X1+Y1)*(X2+Y2) */					\
    ASSH_BOP_ADDM(	T0,	X1,	Y1,	P	),	\
    ASSH_BOP_ADDM(	T1,	X2,	Y2,	P	),	\
    ASSH_BOP_MULM(	T0,	T0,	T1,	P	),	\
    /* C = X1*X2 */						\
    ASSH_BOP_MULM(	X3,	X1,	X2,	P	),	\
    /* D = Y1*Y2 */						\
    ASSH_BOP_MULM(	Y3,	Y1,	Y2,	P	),	\
    /* E = d*C*D */						\
    ASSH_BOP_MULM(	T1,	X3,	Y3,	P	),	\
    ASSH_BOP_MULM(	T1,	T1,	D ,	P	),	\
    /* H = A*(I-C-D) */						\
    ASSH_BOP_SUBM(	T0,	T0,	X3,	P	),	\
    ASSH_BOP_SUBM(	T0,	T0,	Y3,	P	),	\
    ASSH_BOP_MULM(	T0,	T0,	Z3,	P	),	\
    /* J = (D-a*C)*A */						\
    ASSH_BOP_MULM(	X3,	X3,	A,	P	),	\
    ASSH_BOP_SUBM(	Y3,	Y3,	X3,	P	),	\
    ASSH_BOP_MULM(	Y3,	Y3,	Z3,	P	),	\
    /* F = A*A - E */						\
    ASSH_BOP_MULM(	Z3,	Z3,	Z3,	P	),	\
    ASSH_BOP_SUBM(	X3,	Z3,	T1,	P	),	\
    /* G = A*A + E */						\
    ASSH_BOP_ADDM(	Z3,	Z3,	T1,	P	),	\
    /* Y3 = G*J */						\
    ASSH_BOP_MULM(	Y3,	Y3,	Z3,	P	),	\
    /* Z3 = F*G */						\
    ASSH_BOP_MULM(	Z3,	Z3,	X3,	P	),	\
    /* X3 = F*H */						\
    ASSH_BOP_MULM(	X3,	X3,	T0,	P	)

/** @internal doubling on twisted edward curve, projective
    coordinate. 15 ops */
#define ASSH_BOP_TEDWARD_PDBL(X3, Y3, Z3, X1, Y1, Z1, T0, T1, P)\
    /* C = X1^2 */						\
    ASSH_BOP_MULM(	X3,	X1,	X1,	P	),	\
    /* D = Y1^2 */						\
    ASSH_BOP_MULM(	Y3,	Y1,	Y1,	P	),	\
    /* B = (X1+Y1)^2-C-D */					\
    ASSH_BOP_ADDM(	T0,	X1,	Y1,	P	),	\
    ASSH_BOP_MULM(	T0,	T0,	T0,	P	),	\
    ASSH_BOP_SUBM(	T0,	T0,	X3,	P	),	\
    ASSH_BOP_SUBM(	T0,	T0,	Y3,	P	),	\
    /* E = a*C */						\
    ASSH_BOP_MULM(	X3,	X3,	A,	P	),	\
    /* F = E+D */						\
    ASSH_BOP_ADDM(	Z3,	X3,	Y3,	P	),	\
    /* Y3 = F*(E-D) */						\
    ASSH_BOP_SUBM(	Y3,	X3,	Y3,	P	),	\
    ASSH_BOP_MULM(	Y3,	Z3,	Y3,	P	),	\
    /* J = F-2*Z1^2 */						\
    ASSH_BOP_MULM(	T1,	Z1,	Z1,	P	),	\
    ASSH_BOP_ADDM(	T1,	T1,	T1,	P	),	\
    ASSH_BOP_SUBM(	T1,	Z3,	T1,	P	),	\
    /* X3 = B*J */						\
    ASSH_BOP_MULM(	X3,	T0,	T1,	P	),	\
    /* Z3 = F*J */						\
    ASSH_BOP_MULM(	Z3,	Z3,	T1,	P	)

#endif

