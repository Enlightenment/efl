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
   @short Key support for the RSA signature algorithm
   @internal
*/

#ifndef ASSH_KEY_RSA_H_
#define ASSH_KEY_RSA_H_

#include <assh/assh_key.h>
#include <assh/assh_bignum.h>

/** @internal RSA key storage */
struct assh_key_rsa_s
{
  struct assh_key_s key;
  /** RSA modulus */
  struct assh_bignum_s nn;
  /** RSA exponent */
  struct assh_bignum_s en;
  /** RSA exponent */
  struct assh_bignum_s dn;
};

ASSH_FIRST_FIELD_ASSERT(assh_key_rsa_s, key);

/** @internal Key operations descriptor for RSA keys */
extern const struct assh_key_ops_s assh_key_rsa;

/** @internal */
#define ASSH_RSA_ID     "\x00\x00\x00\x07ssh-rsa"
/** @internal */
#define ASSH_RSA_ID_LEN (sizeof(ASSH_RSA_ID) - 1)

#endif

