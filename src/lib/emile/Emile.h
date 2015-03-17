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

typedef struct _Emile_SSL Emile_SSL;

typedef enum
{
  EMILE_SSLv23,
  EMILE_SSLv3,
  EMILE_TLSv1
} Emile_Cipher_Type;

typedef enum
{
  EMILE_WANT_NOTHING = 0,
  EMILE_WANT_READ = 1,
  EMILE_WANT_WRITE = 3
} Emile_Want_Type;

EAPI Eina_Bool emile_cipher_init(void);
EAPI const char *emile_cipher_module_get(void);

EAPI Eina_Binbuf *emile_binbuf_cipher(const Eina_Binbuf *in,
                                      const char *key, unsigned int length);

EAPI Eina_Binbuf *emile_binbuf_decipher(const Eina_Binbuf *in,
                                        const char *key, unsigned int length);

EAPI Emile_SSL *emile_cipher_server_listen(Emile_Cipher_Type t);
EAPI Emile_SSL *emile_cipher_client_connect(Emile_SSL *server, int fd);
EAPI Emile_SSL *emile_cipher_server_connect(Emile_Cipher_Type t);
EAPI Eina_Bool emile_cipher_free(Emile_SSL *emile);

EAPI Eina_Bool emile_cipher_cafile_add(Emile_SSL *emile, const char *file);
EAPI Eina_Bool emile_cipher_cert_add(Emile_SSL *emile, const char *file);
EAPI Eina_Bool emile_cipher_privkey_add(Emile_SSL *emile, const char *file);
EAPI Eina_Bool emile_cipher_crl_add(Emile_SSL *emile, const char *file);
EAPI int emile_cipher_read(Emile_SSL *emile, Eina_Binbuf *buffer);
EAPI int emile_cipher_write(Emile_SSL *emile, const Eina_Binbuf *buffer);
EAPI const char *emile_cipher_error_get(const Emile_SSL *emile);
EAPI Eina_Bool emile_cipher_verify_name_set(Emile_SSL *emile, const char *name);
EAPI const char *emile_cipher_verify_name_get(const Emile_SSL *emile);
EAPI void emile_cipher_verify_set(Emile_SSL *emile, Eina_Bool verify);
EAPI void emile_cipher_verify_basic_set(Emile_SSL *emile, Eina_Bool verify_basic);
EAPI Eina_Bool emile_cipher_verify_get(const Emile_SSL *emile);
EAPI Eina_Bool emile_cipher_verify_basic_get(const Emile_SSL *emile);

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

/* All the value from below enum should be the same as in Evas_Loader.h */
typedef enum _Emile_Colorspace
{
  EMILE_COLORSPACE_ARGB8888,/**< ARGB 32 bits per pixel, high-byte is Alpha, accessed 1 32bit word at a time */
  EMILE_COLORSPACE_YCBCR422P601_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
  EMILE_COLORSPACE_YCBCR422P709_PL, /**< YCbCr 4:2:2 Planar, ITU.BT-709 specifications. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb, then Cr rows */
  EMILE_COLORSPACE_RGB565_A5P, /**< 16bit rgb565 + Alpha plane at end - 5 bits of the 8 being used per alpha byte */
  EMILE_COLORSPACE_GRY8 = 4,
  EMILE_COLORSPACE_YCBCR422601_PL, /**<  YCbCr 4:2:2, ITU.BT-601 specifications. The data pointed to is just an array of row pointer, pointing to line of Y,Cb,Y,Cr bytes */
  EMILE_COLORSPACE_YCBCR420NV12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of row pointer, pointing to the Y rows, then the Cb,Cr rows. */
  EMILE_COLORSPACE_YCBCR420TM12601_PL, /**< YCbCr 4:2:0, ITU.BT-601 specification. The data pointed to is just an array of tiled row pointer, pointing to the Y rows, then the Cb,Cr rows. */
  EMILE_COLORSPACE_AGRY88 = 8, /**< AY 8bits Alpha and 8bits Grey, accessed 1 16bits at a time */
  EMILE_COLORSPACE_ETC1 = 9, /**< OpenGL ETC1 encoding of RGB texture (4 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_RGB8_ETC2 = 10, /**< OpenGL GL_COMPRESSED_RGB8_ETC2 texture compression format (4 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_RGBA8_ETC2_EAC = 11, /**< OpenGL GL_COMPRESSED_RGBA8_ETC2_EAC texture compression format, supports alpha (8 bit per pixel) @since 1.10 */
  EMILE_COLORSPACE_ETC1_ALPHA = 12, /**< ETC1 with alpha support using two planes: ETC1 RGB and ETC1 grey for alpha @since 1.11 */
  EMILE_COLORSPACE_RGB_S3TC_DXT1 = 13,  /**< OpenGL COMPRESSED_RGB_S3TC_DXT1_EXT format with RGB only. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT1 = 14, /**< OpenGL COMPRESSED_RGBA_S3TC_DXT1_EXT format with RGBA punchthrough. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT2 = 15, /**< DirectDraw DXT2 format with premultiplied RGBA. Not supported by OpenGL itself. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT3 = 16, /**< OpenGL COMPRESSED_RGBA_S3TC_DXT3_EXT format with RGBA. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT4 = 17, /**< DirectDraw DXT4 format with premultiplied RGBA. Not supported by OpenGL itself. @since 1.11 */
  EMILE_COLORSPACE_RGBA_S3TC_DXT5 = 18  /**< OpenGL COMPRESSED_RGBA_S3TC_DXT5_EXT format with RGBA. @since 1.11 */
} Emile_Colorspace;

typedef enum _Emile_Image_Encoding
{
  EMILE_IMAGE_LOSSLESS = 0,
  EMILE_IMAGE_JPEG = 1,
  EMILE_IMAGE_ETC1 = 2,
  EMILE_IMAGE_ETC2_RGB = 3,
  EMILE_IMAGE_ETC2_RGBA = 4,
  EMILE_IMAGE_ETC1_ALPHA = 5
} Emile_Image_Encoding;

typedef enum _Emile_Image_Scale_Hint
{
  EMILE_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
  EMILE_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
  EMILE_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Emile_Image_Scale_Hint;

typedef enum _Emile_Image_Animated_Loop_Hint
{
  EMILE_IMAGE_ANIMATED_HINT_NONE = 0,
  EMILE_IMAGE_ANIMATED_HINT_LOOP = 1,
  EMILE_IMAGE_ANIMATED_HINT_PINGPONG = 2
} Emile_Image_Animated_Loop_Hint;

typedef enum _Emile_Image_Load_Error
{
   EMILE_IMAGE_LOAD_ERROR_NONE = 0, /**< No error on load */
   EMILE_IMAGE_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EMILE_IMAGE_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EMILE_IMAGE_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission denied to an existing file (or path) */
   EMILE_IMAGE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Allocation of resources failure prevented load */
   EMILE_IMAGE_LOAD_ERROR_CORRUPT_FILE = 5, /**< File corrupt (but was detected as a known format) */
   EMILE_IMAGE_LOAD_ERROR_UNKNOWN_FORMAT = 6 /**< File is not a known format */
} Emile_Image_Load_Error; /**< Emile image load error codes one can get - see emile_load_error_str() too. */

typedef struct _Emile_Image Emile_Image;
typedef struct _Emile_Image_Property Emile_Image_Property;
typedef struct _Emile_Image_Load_Opts Emile_Image_Load_Opts;
typedef struct _Emile_Image_Animated Emile_Image_Animated;

struct _Emile_Image_Property
{
   struct {
      unsigned char l, r, t, b;
   } borders;

   const Emile_Colorspace *cspaces;
   Emile_Colorspace cspace;

   Emile_Image_Encoding encoding;

   unsigned int w;
   unsigned int h;
   unsigned int row_stride;

   unsigned char scale;

   Eina_Bool rotated;
   Eina_Bool alpha;
   Eina_Bool premul;
   Eina_Bool alpha_sparse;

   Eina_Bool flipped;
   Eina_Bool comp;
};

struct _Emile_Image_Animated
{
   Eina_List *frames;

   Emile_Image_Animated_Loop_Hint loop_hint;

   int frame_count;
   int loop_count;
   int cur_frame;

   Eina_Bool animated;
};

struct _Emile_Image_Load_Opts
{
   struct {
      int x, y, w, h;
   } region;
   struct {
      int src_x, src_y, src_w, src_h;
      int dst_w, dst_h;
      int smooth;
      Emile_Image_Scale_Hint scale_hint;
   } scale_load;
   double dpi;
   unsigned int w, h;
   unsigned int degree;
   int scale_down_by;

   Eina_Bool orientation;
};

// FIXME: Add enum for error code
// FIXME: should set region at load time, not head time

EAPI Emile_Image *emile_image_tgv_memory_open(Eina_Binbuf *source,
                                              Emile_Image_Load_Opts *opts,
                                              Emile_Image_Animated *animated,
                                              Emile_Image_Load_Error *error);
EAPI Emile_Image *emile_image_tgv_file_open(Eina_File *source,
                                            Emile_Image_Load_Opts *opts,
                                            Emile_Image_Animated *animated,
                                            Emile_Image_Load_Error *error);

EAPI Emile_Image *emile_image_jpeg_memory_open(Eina_Binbuf *source,
                                               Emile_Image_Load_Opts *opts,
                                               Emile_Image_Animated *animated,
                                               Emile_Image_Load_Error *error);
EAPI Emile_Image *emile_image_jpeg_file_open(Eina_File *source,
                                             Emile_Image_Load_Opts *opts,
                                             Emile_Image_Animated *animated,
                                             Emile_Image_Load_Error *error);

EAPI Eina_Bool emile_image_head(Emile_Image *image,
                                Emile_Image_Property *prop,
                                unsigned int property_size,
                                Emile_Image_Load_Error *error);
EAPI Eina_Bool emile_image_data(Emile_Image *image,
                                Emile_Image_Property *prop,
                                unsigned int property_size,
                                void *pixels,
                                Emile_Image_Load_Error *error);

EAPI void emile_image_close(Emile_Image *source);

EAPI const char *emile_load_error_str(Emile_Image *source,
                                      Emile_Image_Load_Error error);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef _EET_H */
