/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
/*
 * This code has been partially inspired by :
 *  - http://create.stephan-brumme.com/crc32/
 */

#ifndef EINA_INLINE_CPU_X_
#define EINA_INLINE_CPU_X_

#ifdef EINA_HAVE_BYTESWAP_H
# include <byteswap.h>
#endif

#ifdef __has_builtin
# define EINA_HAS_BUILTIN(x) __has_builtin(x)
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
# define EINA_HAS_BUILTIN(x) 1
#else
# define EINA_HAS_BUILTIN(x) 0  // Compatibility for the rest of the world
#endif

static inline unsigned short
eina_swap16(unsigned short x)
{
#if defined EINA_HAVE_BSWAP16 && EINA_HAS_BUILTIN(__builtin_bswap16)
  return __builtin_bswap16(x);
#elif defined _MSC_VER          /* Windows. Apparently in <stdlib.h>. */
  return _byteswap_ushort(x);
#elif defined EINA_HAVE_BYTESWAP_H
  return bswap_16(x);
#else
  return (((x & 0xff00) >> 8) |
          ((x & 0x00ff) << 8));
#endif
}

static inline unsigned int
eina_swap32(unsigned int x)
{
#ifdef EINA_HAVE_BSWAP32
  return __builtin_bswap32(x);
#elif defined _MSC_VER          /* Windows. Apparently in <stdlib.h>. */
  return _byteswap_ulong(x);
#elif defined EINA_HAVE_BYTESWAP_H
  return bswap_32(x);
#else
  return (x >> 24) |
        ((x >>  8) & 0x0000FF00) |
        ((x <<  8) & 0x00FF0000) |
         (x << 24);
#endif
}

static inline unsigned long long
eina_swap64(unsigned long long x)
{
#ifdef EINA_HAVE_BSWAP64
  return __builtin_bswap64(x);
#elif defined _MSC_VER          /* Windows. Apparently in <stdlib.h>. */
  return _byteswap_uint64(x);
#elif defined EINA_HAVE_BYTESWAP_H
  return bswap_64(x);
#else
  return (((x & 0xff00000000000000ull) >> 56) |
          ((x & 0x00ff000000000000ull) >> 40) |
          ((x & 0x0000ff0000000000ull) >> 24) |
          ((x & 0x000000ff00000000ull) >> 8)  |
          ((x & 0x00000000ff000000ull) << 8)  |
          ((x & 0x0000000000ff0000ull) << 24) |
          ((x & 0x000000000000ff00ull) << 40) |
          ((x & 0x00000000000000ffull) << 56));
#endif
}



#endif

