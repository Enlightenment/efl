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
   @short SSH signature module interface
*/

#ifndef ASSH_SIGN_H_
#define ASSH_SIGN_H_

#include "assh_algo.h"
#include "assh_key.h"

/** @internal @see assh_sign_generate_t */
#define ASSH_SIGN_GENERATE_FCN(n) ASSH_WARN_UNUSED_RESULT assh_error_t(n) \
  (struct assh_context_s *c,						\
   const struct assh_key_s *key, size_t data_count,			\
   const uint8_t * const data[], size_t const data_len[],		\
   uint8_t *sign, size_t *sign_len)

/** @internal @This defines the function type for the signature
    generation operation of the signature module interface.
    @see assh_sign_generate */
typedef ASSH_SIGN_GENERATE_FCN(assh_sign_generate_t);

/** @internal @see assh_sign_check_t */
#define ASSH_SIGN_CHECK_FCN(n) ASSH_WARN_UNUSED_RESULT assh_error_t (n) \
  (struct assh_context_s *c,						\
   const struct assh_key_s *key, size_t data_count,			\
   const uint8_t * const data[], size_t const data_len[],		\
   const uint8_t *sign, size_t sign_len)

/** @internal @This defines the function type for the signature
    checking operation of the signature module interface.
    @see assh_sign_check */
typedef ASSH_SIGN_CHECK_FCN(assh_sign_check_t);


struct assh_algo_sign_s
{
  struct assh_algo_s algo;
  assh_sign_generate_t *f_generate;
  assh_sign_check_t *f_check;
};

/** @internal @This computes the signature of the passed data using
    the provided key then writes it to the @tt sign buffer. The @tt
    sign_len parameter indicates the size of the buffer and is updated
    with the actual size of the signature blob.

    The data to sign can be split into multiple buffers. The @tt
    data_count parameter must specify the number of data buffers to use.

    If the @tt sign parameter is @tt NULL, the function updates the
    @tt sign_len parmeter with a size value which is greater or equal
    to what is needed to hold the signature blob. In this case, the
    @tt data_* parameters are not used. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_sign_generate(struct assh_context_s *c, const struct assh_algo_sign_s *algo,
                   const struct assh_key_s *key, size_t data_count,
                   const uint8_t * const data[], size_t const data_len[],
                   uint8_t *sign, size_t *sign_len)
{
  return algo->f_generate(c, key, data_count, data, data_len, sign, sign_len);
}

/** @internal @This checks the signature of the passed data using the
    provided key. The data can be split into multiple buffers. The @tt
    data_count parameter must specify the number of data buffers used. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_sign_check(struct assh_context_s *c, const struct assh_algo_sign_s *algo,
                 const struct assh_key_s *key, size_t data_count,
                 const uint8_t * const data[], size_t const data_len[],
                 const uint8_t *sign, size_t sign_len)
{
  return algo->f_check(c, key, data_count, data, data_len, sign, sign_len);
}

/** Dummy signature algorithm */
extern const struct assh_algo_sign_s assh_sign_none;

/** Use SHA1 and a dsa key with L = 1024 and N = 160. */
extern const struct assh_algo_sign_s assh_sign_dsa;

/** Use SHA224 and a dsa key with L >= 2048 and N = 224. */
extern const struct assh_algo_sign_s assh_sign_dsa2048_sha224;

/** Use SHA256 and a dsa key with L >= 2048 and N = 256. */
extern const struct assh_algo_sign_s assh_sign_dsa2048_sha256;

/** Use SHA256 and a dsa key with L >= 3072 and N = 256. */
extern const struct assh_algo_sign_s assh_sign_dsa3072_sha256;

/** Accept sha* and md5 RSA signatures, generate sha1 signatures.
    Reject keys with modulus size less than 768 bits. */
extern const struct assh_algo_sign_s assh_sign_rsa_sha1_md5;

/** Accept sha* RSA signatures, generate sha1 signatures,
    Reject keys with modulus size less than 1024 bits. */
extern const struct assh_algo_sign_s assh_sign_rsa_sha1;

/** Accept sha* RSA signatures, generate sha1 signatures. 
    Reject keys with modulus size less than 2048 bits. */
extern const struct assh_algo_sign_s assh_sign_rsa_sha1_2048;

/** Accept sha2, RSA signatures, generate sha256 signatures. 
    Reject keys with modulus size less than 2048 bits. */
extern const struct assh_algo_sign_s assh_sign_rsa_sha256_2048;

/** Accept sha2 RSA signatures, generate sha256 signatures. 
    Reject keys with modulus size less than 3072 bits. */
extern const struct assh_algo_sign_s assh_sign_rsa_sha256_3072;

/** The ssh-ed25519 algorithm as implemented by openssh. This offerrs
    125 bits security and relies on an Edward elliptic curve. 

    See @url {http://safecurves.cr.yp.to/} */
extern const struct assh_algo_sign_s assh_sign_ed25519;

/** Same algorithm as @ref assh_sign_ed25519 with the stronger E382
    edward curve and the shake256 hash function.

    See @url {http://safecurves.cr.yp.to/} */
extern const struct assh_algo_sign_s assh_sign_eddsa_e382;

/** Same algorithm as @ref assh_sign_ed25519 with the stronger E521
    edward curve and the shake256 hash function.

    See @url {http://safecurves.cr.yp.to/} */
extern const struct assh_algo_sign_s assh_sign_eddsa_e521;

#endif

