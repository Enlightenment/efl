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
   @short Main context structure and related functions

   The library main context structure hold stuff common to multiple
   sessions. This includes registered algorithms and host keys.
*/

#ifndef ASSH_CONTEXT_H_
#define ASSH_CONTEXT_H_

#include "assh.h"

/** This specifies the type of ssh session. */
enum assh_context_type_e
{
  ASSH_SERVER,
  ASSH_CLIENT,
  ASSH_CLIENT_SERVER,
};

/** @internal @This is the packet pool allocator bucket
    structure. Freed packets are inserted in the linked list of the
    bucket associated with their size. */
struct assh_packet_pool_s
{
  struct assh_packet_s *pck;
  size_t count;
  size_t size;
};

/** @internalmembers @This is the library main context structure. */
struct assh_context_s
{
  /** User private data */
  void *user_pv;

  /** Number of initialized sessions attached to this context. */
  unsigned int session_count;

  /** Client/server context type. */
  enum assh_context_type_e type;

  /** Memory allocator function */
  assh_allocator_t *f_alloc;
  /** Memory allocator private data */
  void *alloc_pv;

  /** Pseudo random number generator */
  const struct assh_prng_s *prng;
  /** Pseudo random number generator private data, allocated by the
      @ref assh_prng_init_t function and freed by @ref
      assh_prng_cleanup_t. */
  void *prng_pv;
  /** Current amount of entropy in the prng pool. a negative value
      will make the @ref assh_event_get function return an @ref
      ASSH_EVENT_PRNG_FEED event.  */
  int prng_entropy;

  /** Head of loaded keys list */
  const struct assh_key_s *keys;

  /** Estimated size of the kex init packet, computed when new
      algorithm are registered. */
  size_t kex_init_size;

  /** Packet pool maximum allocated size in a single bucket. */
  size_t pck_pool_max_bsize;
  /** Packet pool maximum allocated size. */
  size_t pck_pool_max_size;
  /** Packet pool current allocated size. */
  size_t pck_pool_size;

  /** Packet pool buckets of spare packets by size. */
  struct assh_packet_pool_s pool[ASSH_PCK_POOL_SIZE];

  /** Registered algorithms */
  const struct assh_algo_s *algos[ASSH_MAX_ALGORITHMS];
  /** Number of registered algorithms */
  size_t algos_count;

  /** Registered services. */
  const struct assh_service_s *srvs[ASSH_MAX_SERVICES];
  /** Number of registered services */
  size_t srvs_count;

  /** Big number engine */
  const struct assh_bignum_algo_s *bignum;
};

/** @This allocates and initializes a context. The @tt alloc parameter
    may be @tt NULL. @see assh_context_init */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_context_create(struct assh_context_s **ctx,
		    enum assh_context_type_e type,
		    assh_allocator_t *alloc, void *alloc_pv);

/** @This initializes a context for use as a client or server. This
    can be used to initialize a statically allocated context
    object. The @tt alloc parameter may be @tt NULL. 

    When a stable ABI is needed, use the @ref assh_context_create
    function instead. */
void assh_context_init(struct assh_context_s *ctx,
                       enum assh_context_type_e type,
		       assh_allocator_t *alloc,
		       void *alloc_pv);

/** @This releases resources associated with a context. All existing
    @ref assh_session_s objects must have been released when calling
    this function. @see assh_context_init */
void assh_context_cleanup(struct assh_context_s *ctx);

/** @This cleanups and releases a context created by the @ref
    assh_context_create function. All existing @ref assh_session_s
    objects must have been released when calling this function. @see
    assh_context_create */
void assh_context_release(struct assh_context_s *ctx);

/** @This set the user private pointer of the context. */
ASSH_INLINE void
assh_context_set_pv(struct assh_context_s *ctx,
                    void *private)
{
  ctx->user_pv = private;
}

/** @This get the user private pointer of the context. */
ASSH_INLINE void *
assh_context_get_pv(struct assh_context_s *ctx)
{
  return ctx->user_pv;
}

/** @This setups the pseudo-random number generator for use with
    this context. If an other prng has already been setup, it will be
    released.

    If this function is called with @tt NULL as @tt prng parameter and
    no prng has already been registered, a default prng is setup.
    This is performed automatically when calling the @ref
    assh_session_init function. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_context_prng(struct assh_context_s *s,
		  const struct assh_prng_s *prng);

#endif

