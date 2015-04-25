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
   @short SSH message authentication code module interface
*/

#ifndef ASSH_MAC_H_
#define ASSH_MAC_H_

#include "assh_algo.h"

/** @internal @see assh_mac_init_t */
#define ASSH_MAC_INIT_FCN(n)                                            \
  ASSH_WARN_UNUSED_RESULT assh_error_t (n)(struct assh_context_s *c,    \
                                           void *ctx_, const uint8_t *key)

/** @internal @This defines the function type for the mac
    initialization operation of the mac module interface. The @tt
    ctx_ argument must points to a buffer allocated in secure memory
    of size given by @ref assh_algo_mac_s::ctx_size. */
typedef ASSH_MAC_INIT_FCN(assh_mac_init_t);

/** @internal @see assh_mac_compute_t */
#define ASSH_MAC_COMPUTE_FCN(n)                                         \
  ASSH_WARN_UNUSED_RESULT assh_error_t (n)(void *ctx_, uint32_t seq,    \
                                           const uint8_t *data, size_t len, \
                                           uint8_t *mac)

/** @internal @This defines the function type for the mac computation
    operation of the mac module interface. */
typedef ASSH_MAC_COMPUTE_FCN(assh_mac_compute_t);

/** @internal @see assh_mac_check_t */
#define ASSH_MAC_CHECK_FCN(n) \
  ASSH_WARN_UNUSED_RESULT assh_error_t (n)(void *ctx_, uint32_t seq, \
                                           const uint8_t *data, size_t len, \
                                           const uint8_t *mac)

/** @internal @This defines the function type for the mac checking
    operation of the mac module interface. */
typedef ASSH_MAC_CHECK_FCN(assh_mac_check_t);

/** @internal @see assh_mac_cleanup_t */
#define ASSH_MAC_CLEANUP_FCN(n) void (n)(struct assh_context_s *c, void *ctx_)

/** @internal @This defines the function type for the cleanup
    operation of the mac module interface. */
typedef ASSH_MAC_CLEANUP_FCN(assh_mac_cleanup_t);

/** @internalmembers @This is the mac algorithm descriptor
    structure. It can be casted to the @ref assh_algo_s type. */
struct assh_algo_mac_s
{
  struct assh_algo_s algo;
  /** Size of the context structure needed to initialize the algorithm. */
  size_t ctx_size;
  /** Mac key size in bytes. */
  size_t key_size;
  /** Authentication tag size. */
  size_t mac_size;
  assh_mac_init_t    *f_init;
  assh_mac_compute_t *f_compute;
  assh_mac_check_t  *f_check;
  assh_mac_cleanup_t *f_cleanup;
};

/** @multiple @This is a mac algorithm implementation descriptor. */
extern const struct assh_algo_mac_s assh_hmac_none;
extern const struct assh_algo_mac_s assh_hmac_md5;
extern const struct assh_algo_mac_s assh_hmac_md5_96;
extern const struct assh_algo_mac_s assh_hmac_sha1;
extern const struct assh_algo_mac_s assh_hmac_sha1_96;
extern const struct assh_algo_mac_s assh_hmac_sha256;
extern const struct assh_algo_mac_s assh_hmac_sha512;

#ifdef CONFIG_ASSH_USE_GCRYPT_HASH
extern const struct assh_algo_mac_s assh_hmac_ripemd160;
#endif

#endif

