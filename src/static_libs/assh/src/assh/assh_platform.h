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

#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/**
   @file
   @short PLatform dependent definitions
   @internal
*/

#ifndef ASSH_PLATFORM_H_
#define ASSH_PLATFORM_H_

#ifdef __GNUC__
# define ASSH_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define ASSH_WARN_UNUSED_RESULT
#endif

#define ASSH_INLINE static inline

#define CONFIG_ASSH_NONALIGNED_ACCESS

/** @internal @This stores a 32 bits value in network byte
    order into a non-aligned location. */
ASSH_INLINE void assh_store_u32(uint8_t *s, uint32_t x)
{
#ifdef CONFIG_ASSH_NONALIGNED_ACCESS
  *(uint32_t*)s = htonl(x);
#else
  s[0] = x >> 24;
  s[1] = x >> 16;
  s[2] = x >> 8;
  s[3] = x;
#endif
}

/** @internal @This stores a 32 bits value in little endian byte
    order into a non-aligned location. */
ASSH_INLINE void assh_store_u32le(uint8_t *s, uint32_t x)
{
  s[3] = x >> 24;
  s[2] = x >> 16;
  s[1] = x >> 8;
  s[0] = x;
}

/** @internal @This stores a 64 bits value in network byte
    order into a non-aligned location. */
ASSH_INLINE void assh_store_u64(uint8_t *s, uint64_t x)
{
#ifdef CONFIG_ASSH_NONALIGNED_ACCESS
  *(uint32_t*)(s) = htonl(x >> 32);
  *(uint32_t*)(s + 4) = htonl(x);
#else
  s[0] = x >> 56;
  s[1] = x >> 48;
  s[2] = x >> 40;
  s[3] = x >> 32;
  s[4] = x >> 24;
  s[5] = x >> 16;
  s[6] = x >> 8;
  s[7] = x;
#endif
}

/** @internal @This stores a 64 bits value in little endian byte
    order into a non-aligned location. */
ASSH_INLINE void assh_store_u64le(uint8_t *s, uint64_t x)
{
  s[7] = x >> 56;
  s[6] = x >> 48;
  s[5] = x >> 40;
  s[4] = x >> 32;
  s[3] = x >> 24;
  s[2] = x >> 16;
  s[1] = x >> 8;
  s[0] = x;
}

/** @internal @This loads a 32 bits value in network byte
    order from a non-aligned location. */
ASSH_INLINE uint32_t assh_load_u32(const uint8_t *s)
{
#ifdef CONFIG_ASSH_NONALIGNED_ACCESS
  return htonl(*(const uint32_t*)s);
#else
  return s[3] + (s[2] << 8) + (s[1] << 16) + (s[0] << 24);
#endif
}

/** @internal @This loads a 32 bits value in little endian
    byte order from a non-aligned location. */
ASSH_INLINE uint32_t assh_load_u32le(const uint8_t *s)
{
  return s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
}

/** @internal @This loads a 64 bits value in network byte
    order from a non-aligned location. */
ASSH_INLINE uint64_t assh_load_u64(const uint8_t *s)
{
  return ((uint64_t)s[7] << 0)  + ((uint64_t)s[6] << 8) +
         ((uint64_t)s[5] << 16) + ((uint64_t)s[4] << 24) +
         ((uint64_t)s[3] << 32) + ((uint64_t)s[2] << 40) +
         ((uint64_t)s[1] << 48) + ((uint64_t)s[0] << 56);
}

/** @internal @This loads a 64 bits value in little endian
    byte order from a non-aligned location. */
ASSH_INLINE uint64_t assh_load_u64le(const uint8_t *s)
{
  return ((uint64_t)s[0] << 0)  + ((uint64_t)s[1] << 8) +
         ((uint64_t)s[2] << 16) + ((uint64_t)s[3] << 24) +
         ((uint64_t)s[4] << 32) + ((uint64_t)s[5] << 40) +
         ((uint64_t)s[6] << 48) + ((uint64_t)s[7] << 56);
}

/** @internal @This performs a byte swap of a 32 bits value. */
ASSH_INLINE uint32_t assh_swap_u32(uint32_t x)
{
  x = (x << 16) | (x >> 16);
  x = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);
  return x;
}

/** @internal */
#define ASSH_MAX(a, b) ({ typeof(a) __a = (a); typeof(b) __b = (b); __a > __b ? __a : __b; })

/** @internal */
#define ASSH_MIN(a, b) ({ typeof(a) __a = (a); typeof(b) __b = (b); __a < __b ? __a : __b; })

/** @internal */
#define ASSH_SWAP(a, b) do { typeof(a) __a = (a); typeof(b) __b = (b); (a) = __b; (b) = __a; } while(0)

/** @internal */
#define ASSH_CLZ8(x)  (__builtin_clz((uint8_t)(x)) + 8 - sizeof(int) * 8)
/** @internal */
#define ASSH_CLZ16(x) (__builtin_clz((uint16_t)(x)) + 16 - sizeof(int) * 8)
/** @internal */
#define ASSH_CLZ32(x) (__builtin_clzl((uint32_t)(x)) + 32 - sizeof(long) * 8)
/** @internal */
#define ASSH_CLZ64(x) (__builtin_clzll((uint64_t)(x)) + 64 - sizeof(long long) * 8)

/** @internal */
#define ASSH_CTZ8(x) __builtin_ctz(x)
/** @internal */
#define ASSH_CTZ16(x) __builtin_ctz(x)
/** @internal */
#define ASSH_CTZ32(x) __builtin_ctzl(x)
/** @internal */
#define ASSH_CTZ64(x) __builtin_ctzll(x)

/** @internal */
#define ASSH_ALIGN8(x) ((((x) - 1) | 7) + 1)

#endif
