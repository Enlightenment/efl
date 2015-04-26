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
#include <assh/assh_packet.h>
#include <assh/assh_context.h>
#include <assh/assh_prng.h>

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

#include "prng_weak.c"
#include <stdlib.h>

 /* 1024 bits prime number */
static const uint8_t *prime1 = (const uint8_t*)"\x00\x00\x00\x81"
  "\x00\xff\xff\xff\xff\xff\xff\xff\xff\xc9\x0f\xda\xa2\x21\x68\xc2\x34"
  "\xc4\xc6\x62\x8b\x80\xdc\x1c\xd1\x29\x02\x4e\x08\x8a\x67\xcc\x74"
  "\x02\x0b\xbe\xa6\x3b\x13\x9b\x22\x51\x4a\x08\x79\x8e\x34\x04\xdd"
  "\xef\x95\x19\xb3\xcd\x3a\x43\x1b\x30\x2b\x0a\x6d\xf2\x5f\x14\x37"
  "\x4f\xe1\x35\x6d\x6d\x51\xc2\x45\xe4\x85\xb5\x76\x62\x5e\x7e\xc6"
  "\xf4\x4c\x42\xe9\xa6\x37\xed\x6b\x0b\xff\x5c\xb6\xf4\x06\xb7\xed"
  "\xee\x38\x6b\xfb\x5a\x89\x9f\xa5\xae\x9f\x24\x11\x7c\x4b\x1f\xe6"
  "\x49\x28\x66\x51\xec\xe6\x53\x81\xff\xff\xff\xff\xff\xff\xff\xff";

static const uint8_t *prime2 = (const uint8_t*)"\x00\x00\x00\x81"
  "\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x83";


struct assh_context_s context;

#define ABORT() do {                                            \
    fprintf(stderr, "%s:%u:%s\n",                               \
            __FILE__, __LINE__, __func__);                      \
    abort();                                                    \
  } while (0)

assh_error_t test_convert()
{
  struct assh_bignum_s n, m;
  uint8_t buf[32];

  assh_bignum_init(&context, &n, 128, 0);

  /********************/
  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x01\x55", &n))
    ABORT();

  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, &n, buf))
    ABORT();

  if (memcmp(buf, "\x00\x00\x00\x01\x55\xaa", 6))
    ABORT();

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x01\x85", &n))
    ABORT();

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x01\x00", &n))
    ABORT();

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x02\x00\x00", &n))
    ABORT();

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x02\x00\x10", &n))
    ABORT();

  /********************/
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x11\x00\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", &n))
    ABORT();

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x11\x01\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", &n))
    ABORT();

  assh_bignum_release(&context, &n);
  assh_bignum_init(&context, &n, 125, 0);

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x11\x00\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", &n))
    ABORT();
  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x10\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", &n))
    ABORT();

  assh_bignum_release(&context, &n);
  assh_bignum_init(&context, &n, 117, 0);

  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x0f\x10\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04", &n))
    ABORT();
  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, &n, buf))
    ABORT();
  if (memcmp(buf, "\x00\x00\x00\x0f\x10\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\xaa", 20))
    ABORT();


  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MSB_RAW, ASSH_BIGNUM_NATIVE, "\x90\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04", &n))
    ABORT();
  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, &n, buf))
    ABORT();
  if (memcmp(buf, "\x00\x00\x00\x0f\x10\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\xaa", 20))
    ABORT();


  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_LSB_RAW, ASSH_BIGNUM_NATIVE, "\x04\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x01\x00\x10", &n))
    ABORT();
  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, &n, buf))
    ABORT();
  if (memcmp(buf, "\x00\x00\x00\x0f\x10\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\xaa", 20))
    ABORT();

  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_LSB_RAW, &n, buf))
    ABORT();
  if (memcmp(buf, "\x04\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x01\x00\x10\xaa", 16))
    ABORT();

  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MSB_RAW, &n, buf))
    ABORT();
  if (memcmp(buf, "\x10\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\xaa", 16))
    ABORT();

  /* value to large */
  assh_bignum_init(&context, &m, 64, 0);
  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_NATIVE, &n, &m))
    ABORT();

  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x09\x01\xf0\x00\x00\x00\xf0\x00\x00\x00", &n))
    ABORT();
  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_NATIVE, &n, &m))
    ABORT();

  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x09\x00\xf0\x00\x00\x00\xf0\x00\x00\x00", &n))
    ABORT();
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_NATIVE, &n, &m))
    ABORT();

  assh_bignum_release(&context, &m);
  assh_bignum_init(&context, &m, 60, 0);

  if (!assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_NATIVE, &n, &m))
    ABORT();

  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_MPINT, ASSH_BIGNUM_NATIVE, "\x00\x00\x00\x08\x0f\x00\x00\x00\xf0\x00\x00\x00", &n))
    ABORT();
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_NATIVE, &n, &m))
    ABORT();
  memset(buf, 0xaa, sizeof(buf));
  if (assh_bignum_convert(&context,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, &n, buf))
    ABORT();
  if (memcmp(buf, "\x00\x00\x00\x08\x0f\x00\x00\x00\xf0\x00\x00\x00\xaa", 9))
    ABORT();

  /********************/

  fprintf(stderr, "v");
  return ASSH_OK;

  fprintf(stderr, "Convert error\n");
  ABORT();
}

assh_error_t test_cmp()
{
#warning move cmp tests in test_ops() ?

  enum bytecode_args_e
  {
    X_hex, Y_hex, X, Y, S
  };

  assh_bignum_op_t bytecode1[] = {
    ASSH_BOP_SIZE(	X,	S			),
    ASSH_BOP_MOVE(	X,	X_hex			),

    ASSH_BOP_SIZE(	Y,	S			),
    ASSH_BOP_MOVE(	Y,	Y_hex			),

    ASSH_BOP_CMPNE(     X,      Y,      0		),
    ASSH_BOP_CMPLT(     X,      Y,      0		),

    ASSH_BOP_UINT(      X,      16                      ),
    ASSH_BOP_CMPEQ(     X,      Y,      0		),
    ASSH_BOP_CMPLTEQ(   X,      Y,      0		),
    ASSH_BOP_CMPLTEQ(   Y,      X,      0		),

    ASSH_BOP_END(),
  };

  if (assh_bignum_bytecode(&context, bytecode1, "MMTTs", "\x00\x00\x00\x01\x01",
                           "\x00\x00\x00\x01\x10", (size_t)64))
    ABORT();

  assh_bignum_op_t bytecode2[] = {
    ASSH_BOP_SIZE(	X,	S			),
    ASSH_BOP_MOVE(	X,	X_hex			),

    ASSH_BOP_SIZE(	Y,	S			),
    ASSH_BOP_MOVE(	Y,	Y_hex			),

    ASSH_BOP_CMPEQ(     X,      Y,      0		),

    ASSH_BOP_END(),
  };

  if (assh_bignum_bytecode(&context, bytecode2, "MMTTs",
        "\x00\x00\x00\x01\x01", "\x00\x00\x00\x01\x10", (size_t)64)
      != ASSH_ERR_NUM_COMPARE_FAILED)
    ABORT();

  assh_bignum_op_t bytecode3[] = {
    ASSH_BOP_SIZE(	X,	S			),
    ASSH_BOP_MOVE(	X,	X_hex			),

    ASSH_BOP_SIZE(	Y,	S			),
    ASSH_BOP_MOVE(	Y,	Y_hex			),

    ASSH_BOP_CMPLT(     Y,      X,      0		),

    ASSH_BOP_END(),
  };

  if (assh_bignum_bytecode(&context, bytecode3, "MMTTs",
        "\x00\x00\x00\x01\x01", "\x00\x00\x00\x01\x10", (size_t)64)
      != ASSH_ERR_NUM_COMPARE_FAILED)
    ABORT();

  fprintf(stderr, "c");
  return ASSH_OK;
}

assh_error_t test_ops()
{
  {
    struct op_test_s
    {
      size_t abits, bbits, rbits;
      assh_bool_t err;
      const assh_bignum_op_t *bytecode;
      const char *a, *b, *r, *m;
    };

    enum bytecode_args_e
    {
      A, B, R, M, A_mpint, B_mpint, R_mpint, M_mpint, MT
    };

    static const assh_bignum_op_t bytecode_shl[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SHL(	R,	A,	0,	B	),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_shr[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SHR(	R,	A,	0,	B	),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_add[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_ADD(	R,	A,	B	),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_sub[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SUB(	R,	A,	B	),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_addm[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SIZE(	M,	R		),
      ASSH_BOP_MOVE(	M,	M_mpint		),
      ASSH_BOP_MTINIT(	MT,     M               ),
      ASSH_BOP_MTTO(    A,      A,      A,      MT      ),
      ASSH_BOP_MTTO(    B,      B,      B,      MT      ),
      ASSH_BOP_ADDM(	R,	A,	B,	MT	),
      ASSH_BOP_MTFROM(  R,      R,      R,      MT      ),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_subm[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SIZE(	M,	R		),
      ASSH_BOP_MOVE(	M,	M_mpint		),
      ASSH_BOP_MTINIT(	MT,     M               ),
      ASSH_BOP_MTTO(    A,      A,      A,      MT      ),
      ASSH_BOP_MTTO(    B,      B,      B,      MT      ),
      ASSH_BOP_SUBM(	R,	A,	B,	MT	),
      ASSH_BOP_MTFROM(  R,      R,      R,      MT      ),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_mulm_mt[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_MOVE(	B,	B_mpint		),
      ASSH_BOP_SIZE(	M,	R		),
      ASSH_BOP_MOVE(	M,	M_mpint		),
      ASSH_BOP_MTINIT(	MT,     M               ),
      ASSH_BOP_MTTO(    A,      A,      A,      MT      ),
      ASSH_BOP_MTTO(    B,      B,      B,      MT      ),
      ASSH_BOP_MOVE(	R,	A		),
      ASSH_BOP_MULM(	R,	R,	A,	MT	),
      ASSH_BOP_MTFROM(  R,      R,      R,      MT      ),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const assh_bignum_op_t bytecode_modinv_mt[] = {
      ASSH_BOP_MOVE(	A,	A_mpint		),
      ASSH_BOP_SIZE(	M,	R		),
      ASSH_BOP_MOVE(	M,	M_mpint		),
      ASSH_BOP_MTINIT(	MT,     M               ),
      ASSH_BOP_MTTO(    A,      A,      A,      MT      ),
      ASSH_BOP_INV(	R,	A,	MT	),
      ASSH_BOP_MTFROM(  R,      R,      R,      MT      ),
      ASSH_BOP_MOVE(	R_mpint,	R	),
      ASSH_BOP_END(),
    };

    static const struct op_test_s tests[] = {
      {
	128, 32, 128, 0, bytecode_shr,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x0c\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98",
      },

      {
	128, 31, 128, 0, bytecode_shr,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x0c\x3e\x46\x8a\xcf\x13\x57\x9a\x1d\xc1\xb9\x57\x30",
      },

      {
	128, 101, 128, 0, bytecode_shr,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x04\x00\xf9\x1a\x2b",
      },

      {
	128, 5, 128, 0, bytecode_shr,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x10\x00\xf9\x1a\x2b\x3c\x4d\x5e\x68\x77\x06\xe5\x5c\xc3\xb2\xa1\x97",
      },

      {
	128, 0, 128, 0, bytecode_shr,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
      },

      {
	128, 32, 128, 0, bytecode_shl,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x11\x00\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1\x00\x00\x00\x00",
      },

      {
	128, 31, 128, 0, bytecode_shl,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x11\x00\xc4\xd5\xe6\x87\x70\x6e\x55\xcc\x3b\x2a\x19\x78\x80\x00\x00\x00",
      },

      {
	128, 101, 128, 0, bytecode_shl,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x11\x00\xca\x86\x5e\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      },

      {
	128, 5, 128, 0, bytecode_shl,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x11\x00\xe4\x68\xac\xf1\x35\x79\xa1\xdc\x1b\x95\x73\x0e\xca\x86\x5e\x20",
      },

      {
	128, 0, 128, 0, bytecode_shl,
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x10\x1f\x23\x45\x67\x89\xab\xcd\x0e\xe0\xdc\xab\x98\x76\x54\x32\xf1",
      },

      {
	128, 128, 128, 0, bytecode_add,
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x00",
      },

      {
	8, 8, 8, 0, bytecode_add,
	"\x00\x00\x00\x01" "\x02",
	"\x00\x00\x00\x01" "\x03",
	"\x00\x00\x00\x01" "\x05",
      },
      {
	128, 128, 128, 0, bytecode_add,
	"\x00\x00\x00\x10" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10" "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00",
      },
      {
	128, 128, 256, 0, bytecode_add,
	"\x00\x00\x00\x10" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10" "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00",
      },
      {
	128, 128, 128, 1, bytecode_add, /* overflow */
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	128, 128, 129, 0, bytecode_add,
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00",
      },
      {
	127, 127, 128, 0, bytecode_add,
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x81\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00",
      },
      {
	127, 127, 127, 1, bytecode_add, /* overflow */
	"\x00\x00\x00\x10" "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10" "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	64, 64, 128, 0, bytecode_add,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x09" "\x01\x01\x01\x01\x01\x01\x01\x01\x00",
      },

      {
	32, 64, 128, 0, bytecode_add,
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
      },

      {
	64, 32, 128, 0, bytecode_add,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
      },

      {
	128, 128, 128, 0, bytecode_sub,
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x00",
      },
      {
	8, 8, 8, 0, bytecode_sub,
	"\x00\x00\x00\x01" "\x05",
	"\x00\x00\x00\x01" "\x02",
	"\x00\x00\x00\x01" "\x03",
      },
      {
	8, 8, 8, 0, bytecode_sub,
	"\x00\x00\x00\x01" "\x05",
	"\x00\x00\x00\x01" "\x05",
	"\x00\x00\x00\x00",
      },
      {
	8, 8, 8, 1, bytecode_sub, /* overflow */
	"\x00\x00\x00\x01" "\x05",
	"\x00\x00\x00\x01" "\x06",
      },
      {
	128, 64, 128, 0, bytecode_sub,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	128, 64, 256, 0, bytecode_sub,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	64, 128, 128, 0, bytecode_sub,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	64, 128, 128, 1, bytecode_sub, /* overflow */
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	128, 64, 128, 0, bytecode_sub,
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },
      {
	128, 64, 128, 1, bytecode_sub, /* overflow */
	"\x00\x00\x00\x05" "\x00\xff\xff\xff\xff",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x81\x80\x80\x80\x7f",
	"\x00\x00\x00\x09" "\x00\x80\x80\x80\x80\x80\x80\x80\x80",
      },

      {
	128, 128, 128, 0, bytecode_addm, /* carry */
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10"     "\x0b\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab",
	"\x00\x00\x00\x11" "\x00\xf5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
      },

      {
	128, 128, 128, 0, bytecode_addm, /* no carry */
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10"     "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01",
	"\x00\x00\x00\x10"     "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
      },

      {
	128, 128, 128, 0, bytecode_addm, /* no carry */
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x0f"         "\x2b\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xab\xac\x04",
	"\x00\x00\x00\x10"     "\x00\xf5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
      },

      {
	128, 128, 128, 0, bytecode_subm, /* carry */
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x11" "\x00\xb5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
	"\x00\x00\x00\x11" "\x00\xf5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
      },

      {
	128, 128, 128, 0, bytecode_subm, /* no carry */
	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x10"     "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x11" "\x00\xf5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
      },

//      {
//	128, 128, 128, 0, bytecode_mulm_mt,
//	"\x00\x00\x00\x11" "\x00\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
//	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
//	"\x00\x00\x00\x10"     "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
//	"\x00\x00\x00\x11" "\x00\xf5\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",
//      },

      {
	128, 128, 128, 0, bytecode_modinv_mt,
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x10"     "\x14\xc5\x85\x54\x72\xa2\x41\x05\x69\xb3\x6c\x28\x83\x80\xe1\x4d",
	"\x00\x00\x00\x11" "\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1d",
      },

      {
	128, 128, 128, 0, bytecode_modinv_mt,
	"\x00\x00\x00\x10"     "\x40\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80",
	"\x00\x00\x00\x00",
	"\x00\x00\x00\x10"     "\x15\xae\x4d\x65\x16\xb5\x64\xda\xc8\x42\x79\x00\x39\xeb\xf3\xc8",
	"\x00\x00\x00\x11" "\x00\x98\xcc\x60\x26\xdc\x2d\xb6\x92\x2c\x5a\x00\x94\x00\x00\x00\x01",
      },

      { 0 }
    };

    int i;
    for (i = 0; tests[i].abits; i++)
      {
	const struct op_test_s *t = &tests[i];
	size_t bytes = ASSH_ALIGN8(t->rbits) / 8;
	uint8_t buf[5 + bytes];

	struct assh_bignum_s a, b, r;
	assh_bignum_init(&context, &a, t->abits, 0);
	assh_bignum_init(&context, &b, t->bbits, 0);
	assh_bignum_init(&context, &r, t->rbits, 0);

	memset(buf, 0, sizeof(buf));
	assh_error_t e = assh_bignum_bytecode(&context, t->bytecode, "NNNTMMMMm",
					      &a, &b, &r, t->a, t->b, buf, t->m);

	if (t->err)
	  {
	    if (!e)
	      ABORT();
	  }
	else
	  {
	    if (e)
	      ABORT();
	    size_t s = 4 + assh_load_u32((const uint8_t*)t->r);
	    if (memcmp(buf, t->r, s))
	      {
		assh_hexdump("result", buf, s);
		assh_hexdump("expected", t->r, s);
		ABORT();
	      }
	  }

	assh_bignum_release(&context, &a);
	assh_bignum_release(&context, &b);
	assh_bignum_release(&context, &r);
      }

  }

  fprintf(stderr, "o");
  return ASSH_OK;
}

assh_error_t test_add_sub(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {
      enum bytecode_args_e
      {
        A, B, C, D, S, L
      };

      size_t s = 27 + rand() % 100;
      size_t l = 3 + rand() % 12;

      static const assh_bignum_op_t bytecode[] = {
        ASSH_BOP_SIZE(  A,      S                       ),
        ASSH_BOP_SIZE(  B,      S                       ),
        ASSH_BOP_SIZE(  C,      S                       ),

        ASSH_BOP_RAND(  A,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_RAND(  B,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        // ASSH_BOP_PRINT( A,      'A'                     ),
        ASSH_BOP_SHR(   A,      A,      0,      L       ),
        // ASSH_BOP_PRINT( A,      'A'                     ),
        ASSH_BOP_SHR(   B,      B,      0,      L       ),

        ASSH_BOP_MOVE(  C,      B                       ),
        ASSH_BOP_CMPEQ( C,      B,      0               ),

        // ASSH_BOP_PRINT( A,      'A'                     ),
        // ASSH_BOP_PRINT( B,      'B'                     ),

        ASSH_BOP_ADD(   B,      B,      A               ),
      //ASSH_BOP_PRINT( B,      'B'                     ),
        ASSH_BOP_CMPNE( C,      B,      0               ),

        ASSH_BOP_SUB(   B,      B,      A               ),
      //ASSH_BOP_PRINT( B,      'B'                     ),
      //ASSH_BOP_PRINT( C,      'C'                     ),
        ASSH_BOP_CMPEQ( C,      B,      0               ),

        ASSH_BOP_MOVE(  B,      A                       ),
        ASSH_BOP_ADD(   B,      A,      B               ),
        ASSH_BOP_ADD(   B,      B,      A               ),
        ASSH_BOP_ADD(   B,      A,      B               ),
        ASSH_BOP_ADD(   B,      B,      A               ),

        ASSH_BOP_SIZEM( D,      S,      0,      2       ),
        ASSH_BOP_UINT(  C,      5                       ),
        ASSH_BOP_MUL(   D,      A,      C               ),
     // ASSH_BOP_PRINT( A,      'A'                     ),
     // ASSH_BOP_PRINT( B,      'B'                     ),
     // ASSH_BOP_PRINT( C,      'C'                     ),
     // ASSH_BOP_PRINT( D,      'D'                     ),

        ASSH_BOP_CMPEQ( D,      B,      0               ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "TTTTss", s, l));
    }

  fprintf(stderr, "a");
  return ASSH_OK;
}

assh_error_t test_div(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {
      enum bytecode_args_e
      {
        A, B, C, D, E, S
      };

      static const assh_bignum_op_t bytecode[] = {
        ASSH_BOP_SIZE(  A,      S                       ),
        ASSH_BOP_SIZE(  B,      S                       ),
        ASSH_BOP_SIZE(  C,      S                       ),
        ASSH_BOP_SIZE(  D,      S                       ),
        ASSH_BOP_SIZEM( E,      S,      0,      2       ),

        ASSH_BOP_RAND(  A,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_RAND(  B,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),

        ASSH_BOP_DIVMOD(C,      D,      A,      B       ),

        ASSH_BOP_MUL(   E,      B,      C               ),
        ASSH_BOP_ADD(   E,      E,      D               ),

        ASSH_BOP_CMPEQ( E,      A,      0               ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "TTTTTs", (size_t)256));
    }

  fprintf(stderr, "d");
  return ASSH_OK;
}

assh_error_t test_move(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {
      enum bytecode_args_e
      {
        A, B, L,
        A_mpint, B_mpint
      };

      size_t s = 1 + rand() % 64;

      uint8_t mpa[s+4];
      uint8_t mpb[s+4];

      memset(mpa + 4, rand() & 127, s);

      if (mpa[4] == 0)
        s = 0;
      assh_store_u32(mpa, s);
      assh_store_u32(mpb, s);

      static const assh_bignum_op_t bytecode[] = {
      //ASSH_BOP_PRINT( A,      'A'                     ),

        ASSH_BOP_SIZE(  A,      A_mpint                 ),
      //ASSH_BOP_PRINT( A,      'A'                     ),

        ASSH_BOP_MOVE(  A,      A_mpint                 ),
      //ASSH_BOP_PRINT( A,      'A'                     ),

        ASSH_BOP_MOVE(  B_mpint,        A               ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "TTsMM",
                                        (size_t)256, mpa, mpb));

      ASSH_CHK_RET(memcmp(mpa, mpb, s+4), ASSH_ERR_BAD_DATA);
    }

  fprintf(stderr, "c");
  return ASSH_OK;
}

/* This test the modinv operation. lshift, mul and div ops are used. */
assh_error_t test_modinv(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {
      enum bytecode_args_e
      {
        P, B, C, D, S, P_mpint
      };

      static const assh_bignum_op_t bytecode[] = {

        ASSH_BOP_SIZE(  P,      P_mpint                 ),
        ASSH_BOP_MOVE(  P,      P_mpint                 ),

        ASSH_BOP_SIZE(  B,      S                       ),
        ASSH_BOP_SIZE(  C,      P                       ),
        ASSH_BOP_SIZE(  D,      P                       ),

        ASSH_BOP_RAND(  B,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),

        ASSH_BOP_INV(   C,      B,      P               ),
        ASSH_BOP_CMPLT( C,      P,      0               ),

        ASSH_BOP_MULM(  D,      B,      C,      P       ),

        ASSH_BOP_UINT(  P,      1                       ),
        ASSH_BOP_CMPEQ( P,      D,      0               ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "TTTTsM",
                                        (size_t)(rand() % 900 + 100),
                                        i % 2 ? prime1 : prime2));
    }

  fprintf(stderr, "i");
  return ASSH_OK;
}


/* montgomery mul */
assh_error_t test_mt(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {

      enum bytecode_args_e
      {
        P, A, B, R, R2, MT, S, P_mpint
      };

      static const assh_bignum_op_t bytecode[] = {
        ASSH_BOP_SIZE(  P,      P_mpint                 ),
        ASSH_BOP_MOVE(  P,      P_mpint                 ),

        ASSH_BOP_SIZE(  A,      P                       ),
        ASSH_BOP_SIZE(  B,      P                       ),
        ASSH_BOP_SIZE(  R,      P                       ),
        ASSH_BOP_SIZE(  R2,     P                       ),

        ASSH_BOP_RAND(  A,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_RAND(  B,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),

        ASSH_BOP_MULM(  R2,      A,      B,      P      ),

        ASSH_BOP_MTINIT(MT,     P                       ),
        ASSH_BOP_MTTO(  A,      A,      A,      MT              ),
        ASSH_BOP_MTTO(  B,      B,      B,      MT              ),
        ASSH_BOP_MULM(  R,      A,      B,      MT      ),
        ASSH_BOP_MTFROM(R,      R,      R,      MT              ),

        ASSH_BOP_CMPEQ( R,      R2,      0               ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode, "TTTTTmsM",
                                        (size_t)1024 /*(rand() % 900 + 100)*/,
                                        i % 2 ? prime1 : prime2));
    }

  fprintf(stderr, "m");
  return ASSH_OK;
}

/* test expmod. uses div, mul, modinv */
assh_error_t test_expmod(unsigned int count)
{
  assh_error_t err;
  int i;

  for (i = 0; i < count; i++)
    {

      enum bytecode_args_e
      {
        P, A, IA, X, E, R1, R2, R3, R4, R5, P_mpint, MT
      };

      static const assh_bignum_op_t bytecode[] = {
        ASSH_BOP_SIZE(  P,      P_mpint                 ),
        ASSH_BOP_MOVE(  P,      P_mpint                 ),

        ASSH_BOP_SIZE(  A,      P                       ),
        ASSH_BOP_SIZE(  IA,     P                       ),
        ASSH_BOP_SIZE(  X,      P                       ),
        ASSH_BOP_SIZE(  E,      P                       ),
        ASSH_BOP_SIZE(  R1,     P                       ),
        ASSH_BOP_SIZE(  R2,     P                       ),
        ASSH_BOP_SIZE(  R3,     P                       ),
        ASSH_BOP_SIZE(  R4,     P                       ),
        ASSH_BOP_SIZE(  R5,     P                       ),
        ASSH_BOP_MTINIT(MT,     P                       ),

        ASSH_BOP_RAND(  A,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_MOD(   A,      A,      P               ),
        ASSH_BOP_RAND(  E,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_RAND(  X,      ASSH_BOP_NOREG, ASSH_BOP_NOREG,
                                ASSH_PRNG_QUALITY_WEAK  ),
        ASSH_BOP_INV(   IA,     A,      P               ),

        // ASSH_BOP_PRINT( X,     'X'                    ),
        // ASSH_BOP_PRINT( E,     'E'                    ),
        // ASSH_BOP_PRINT( P,     'P'                    ),

        /* ((((a * x) % p)^e) % p) * ((inv(a)^e) % p) == x^e % p */
        ASSH_BOP_MULM(  R3,     A,      X,      P       ),
        ASSH_BOP_MTTO(  R3,     R3,     R3,     MT              ),
        ASSH_BOP_EXPM(  R2,     R3,     E,      MT      ),
        ASSH_BOP_MTTO(  IA,     IA,     IA,     MT              ),
        ASSH_BOP_EXPM(  R1,     IA,     E,      MT      ),
        ASSH_BOP_MULM(  R4,     R1,     R2,     MT      ),
        ASSH_BOP_MTTO(  X,      X,      X,      MT              ),
        ASSH_BOP_EXPM(  R5,     X,      E,      MT      ),

        ASSH_BOP_CMPEQ( R4,     R5,     0               ),

        ASSH_BOP_MTFROM(R4,     R4,     R4,     MT              ),
        ASSH_BOP_MTFROM(R5,     R5,     R5,     MT              ),
        ASSH_BOP_CMPEQ( R4,     R5,     0               ),

        // ASSH_BOP_PRINT( R4,     '4'                    ),
        // ASSH_BOP_PRINT( R5,     '5'                    ),

        ASSH_BOP_END(),
      };

      ASSH_ERR_RET(assh_bignum_bytecode(&context, bytecode,
                                        "TTTTTTTTTTMm", prime1));
    }

  fprintf(stderr, "e");
  return ASSH_OK;
}

int main(int argc, char **argv)
{
  assh_error_t err;

#ifdef CONFIG_ASSH_USE_GCRYPT
  if (!gcry_check_version(GCRYPT_VERSION))
    return -1;
#endif

  assh_context_init(&context, ASSH_SERVER, NULL, NULL);

  ASSH_ERR_RET(assh_context_prng(&context, NULL));
#warning weak
  //  ASSH_ERR_RET(assh_context_prng(&context, &assh_prng_weak));

  test_convert();
  test_cmp();
  test_ops();

  int i, count = 10;
  if (argc > 1)
    count = atoi(argv[1]);

  for (i = 0; count <= 0 || i < count; i++)
    {
      ASSH_ERR_RET(test_add_sub(0x100));
      ASSH_ERR_RET(test_div(0x100));
      ASSH_ERR_RET(test_move(0x100));
      ASSH_ERR_RET(test_modinv(0x1000));
      ASSH_ERR_RET(test_mt(0x1000));
      ASSH_ERR_RET(test_expmod(0x10));
    }

  fprintf(stderr, "\nDone\n");
  return 0;
}

