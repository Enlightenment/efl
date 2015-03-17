/* EMILE - EFL serialization, compression and crypto library.
 * Copyright (C) 2013 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@samsung.com>
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

/**
 * @brief Emile serialization, compression and ciphering public API calls.
 *
 * These routines are used for Emile Library interaction
 *
 * @date 2013 (created)
 */
#ifndef EMILE_H_
#define EMILE_H_

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef _WIN32
# ifdef EFL_EMILE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else /* ifdef DLL_EXPORT */
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else /* ifdef EFL_EET_BUILD */
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EET_BUILD */
#else /* ifdef _WIN32 */
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else /* if __GNUC__ >= 4 */
#   define EAPI
#  endif /* if __GNUC__ >= 4 */
# else /* ifdef __GNUC__ */
#  define EAPI
# endif /* ifdef __GNUC__ */
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @file Emile.h
 * @brief The file that provide the Emile function
 *
 * This header provides the Emile management functions.
 */

/**
 * @defgroup Emile_Group Top level functions
 * @ingroup Emile
 * Function that affect Emile as a whole.
 *
 * @{
 */

/**
 * Initialize the Emile library
 *
 * The first time this function is called, it will perform all the internal
 * initialization required for the library to function properly and
 * increment the initialization counter. Any subsequent call only
 * increment this counter and return its new value, so it's safe to call
 * this function more than once.
 *
 * @return The new init count. Will be 0 if initialization failed.
 *
 * @since 1.9.0
 */
EAPI int emile_init(void);

/**
 * Shut down the Emile library
 *
 * If emile_init() was called more than once for the running application,
 * emile_shutdown() will decrement the initialization counter and return its
 * new value, without doing anything else. When the counter reaches 0, all
 * of the internal elements will be shutdown and any memory used freed.
 *
 * @return The new init count.
 * @since 1.9.0
 */
EAPI int emile_shutdown(void);

/**
 * @}
 */

EAPI Eina_Bool emile_cipher_init(void);

EAPI Eina_Binbuf *emile_binbuf_cipher(const Eina_Binbuf *in,
                                      const char *key, unsigned int length);

EAPI Eina_Binbuf *emile_binbuf_decipher(const Eina_Binbuf *in,
                                        const char *key, unsigned int length);

typedef enum
{
  EMILE_ZLIB,
  EMILE_LZ4,
  EMILE_LZ4HC
} Emile_Compressor_Type;

typedef enum
{
  EMILE_DEFAULT = -1,
  EMILE_NO_COMPRESSION = 0,
  EMILE_FAST_COMPRESSION = 1,
  EMILE_BEST_COMPRESSION = 9
} Emile_Compressor_Level;

EAPI Eina_Binbuf *emile_binbuf_compress(const Eina_Binbuf *in,
                                        Emile_Compressor_Type t, int level);
EAPI Eina_Binbuf *emile_binbuf_uncompress(const Eina_Binbuf *in,
                                          Emile_Compressor_Type t,
                                          unsigned int dest_length);
EAPI Eina_Bool emile_binbuf_expand(const Eina_Binbuf *in,
                                   Eina_Binbuf *out,
                                   Emile_Compressor_Type t);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _EET_H */
