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
   @short SSH algorithms base descriptor structure and related functions
*/

#ifndef ASSH_ALGO_H_
#define ASSH_ALGO_H_

#include "assh.h"

/** @internal @see assh_algo_suitable_key_t */
#define ASSH_ALGO_SUITABLE_KEY_FCN(n) assh_bool_t (n)    \
  (struct assh_context_s *c,                             \
   const struct assh_algo_s *algo,                       \
   const struct assh_key_s *key)

/** @internal @This defines the function type for the key
    compatibility checking operation common to all the algorithm
    module interfaces. @see assh_algo_suitable_key */
typedef ASSH_ALGO_SUITABLE_KEY_FCN(assh_algo_suitable_key_t);

/** @internal @This specifies classes of SSH algorithm */
enum assh_algo_class_e
{
  ASSH_ALGO_KEX,
  ASSH_ALGO_SIGN,
  ASSH_ALGO_CIPHER,
  ASSH_ALGO_MAC,
  ASSH_ALGO_COMPRESS,
  ASSH_ALGO_ANY,
};

/** @internalmembers @This is the generic algorithm descriptor
    structure. Other algorithm descriptor structures iherit from this
    type. */
struct assh_algo_s
{
  /** SSH algorithm identifier, used during key exchange */
  const char *name;

  /** Variant description string, used when multiple declarations of
      the same algorithm name exist. */
  const char *variant;

  /** Class of algorithm */
  enum assh_algo_class_e class_;

  /** Pointer to associated key operations, may be @tt NULL. */
  const struct assh_key_ops_s *key;

  /** Test if a key can be used with the algorithm, may be @tt NULL. */
  assh_algo_suitable_key_t *f_suitable_key;

  /** Safety factor in range [0, 99] */
  int_fast16_t safety;
  /** Speed factor in range [0, 99] */
  int_fast16_t speed;
  /** Used to choose between variants having the same name */
  int_fast8_t priority;
};

/**
   @This registers the specified @ref assh_algo_s objects for use by
   the given context. The last parameter must be @tt NULL.

   The algorithms are sorted depending on their safety factor and
   speed factor. The @tt safety parameter indicates how algorithms
   safety must be favored over speed. Valid range for this parameter
   is [0, 99]. Algorithms with a safety factor less than @tt
   min_safety are skipped.

   The Safety factor is defined follow:

   @list
     @item 0-19: weak, broken
     @item 20-25: borderline
     @item 26-49: suitable for general use
     @item 50-99: strong
   @end list

   If multiple implementations of the same algorithm are registered,
   the variant which appears first in the list after sorting is kept
   and subsequent variants with the same name are discarded. This
   should retain the less secure variants of the same algorithm not
   filtered by the value of @tt min_safety.
*/
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_algo_register_va(struct assh_context_s *c, unsigned int safety,
		      unsigned int min_safety, ...);

/** @internal @This finds a registered algorithm with matching class
    and name. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_algo_by_name(struct assh_context_s *c,
		  enum assh_algo_class_e class_, const char *name,
                  size_t name_len, const struct assh_algo_s **algo);

/** @internal @This finds a registered algorithm which can be used
    with the given key. If the @tt pos parameter is not @tt NULL, it
    specifies the starting index of the search and it will be updated
    with the index of the matching entry. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_algo_by_key(struct assh_context_s *c,
                 const struct assh_key_s *key, uint_fast16_t *pos,
                 const struct assh_algo_s **algo);

/** @internal @This returns true if the provided key can be used with
    the algorithm and has been loaded or created for that purpose.
    When the @tt key parameter is @tt NULL, the return value indicates
    if the algorithm needs a key when used during a key exchange.

    This does not check the validity of the key, the @ref
    assh_key_validate function is provided for that purpose. */
assh_bool_t
assh_algo_suitable_key(struct assh_context_s *c,
                       const struct assh_algo_s *algo,
                       const struct assh_key_s *key);

/** @internal @This registers the default set of available algorithms
    depending on the library configuration. It relies on the @ref
    assh_algo_register_va function. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_algo_register_default(struct assh_context_s *c,
                           unsigned int safety,
			   unsigned int min_safety);

#endif

