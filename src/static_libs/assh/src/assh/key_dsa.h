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
   @short Key support for the Digitial Signature Algorithm
   @internal
*/

#ifndef ASSH_KEY_DSA_H_
#define ASSH_KEY_DSA_H_

#include <assh/assh_key.h>
#include <assh/assh_bignum.h>

/** @internal DSA key storage */
struct assh_key_dsa_s
{
  struct assh_key_s key;
  /** public p */
  struct assh_bignum_s pn;
  /** public q */
  struct assh_bignum_s qn;
  /** public g */
  struct assh_bignum_s gn;
  /** public y */
  struct assh_bignum_s yn;
  /** private x, may be empty */
  struct assh_bignum_s xn;
};

ASSH_FIRST_FIELD_ASSERT(assh_key_dsa_s, key);

/** @internal Key operations descriptor for DSA keys */
extern const struct assh_key_ops_s assh_key_dsa;

/** @internal */
#define ASSH_DSA_ID     "\x00\x00\x00\x07ssh-dss"
/** @internal */
#define ASSH_DSA_ID_LEN (sizeof(ASSH_DSA_ID) - 1)

#endif

