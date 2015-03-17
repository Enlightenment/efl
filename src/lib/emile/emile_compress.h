#ifndef EMILE_COMPRESSION_H_
#define EMILE_COMPRESSION_H_

/**
 * @defgroup Emile_Group_Compression Non destructive general purpose compression functions.
 * @ingroup Emile
 * Function that allow the compression and expansion of Eina_Binbuf with
 * non destructive algorithm.
 *
 * @{
 */

/**
 * Supported type of compression algorithm.
 * @since 1.14
 *
 * @see emile_binbuf_compress()
 * @see emile_binbuf_uncompress()
 * @see emile_binbuf_expand()
 */
typedef enum
{
  EMILE_ZLIB,
  EMILE_LZ4,
  EMILE_LZ4HC
} Emile_Compressor_Type;

/**
 * Compression level to apply.
 * @since 1.14
 *
 * @see emile_binbuf_compress();
 */
typedef enum
{
  EMILE_COMPRESSOR_DEFAULT = -1,
  EMILE_COMPRESSOR_NONE = 0,
  EMILE_COMPRESSOR_FAST = 1,
  EMILE_COMPRESSOR_BEST = 9
} Emile_Compressor_Level;

/**
 * @brief Compress an Eina_Binbuf into a new Eina_Binbuf
 *
 * @param in Buffer to compress.
 * @param t Type of compression logic to use.
 * @param level Level of compression to apply.
 *
 * @return On success it will return a buffer that contains
 * the compressed data, @c NULL otherwise.
 *
 * @since 1.14
 */
EAPI Eina_Binbuf *emile_compress(const Eina_Binbuf * in, Emile_Compressor_Type t, Emile_Compressor_Level level);
/**
 * @brief Uncompress a buffer into a newly allocated buffer.
 *
 * @param in Buffer to uncompress.
 * @param t Type of compression logic to use.
 * @param dest_length Expected length of the decompressed data.
 *
 * @return a newly allocated buffer with the uncompressed data,
 * @c NULL if it failed.
 *
 * @since 1.14
 *
 * @note That if dest_length doesn't match the expanded data, it will
 * just fail and return @c NULL.
 */
EAPI Eina_Binbuf *emile_decompress(const Eina_Binbuf * in, Emile_Compressor_Type t, unsigned int dest_length);

/**
 * @brief Uncompress a buffer into an existing buffer.
 *
 * @param in Buffer to uncompress.
 * @param out Buffer to expand data into.
 * @param t Type of compression logic to use.
 *
 * @return EINA_TRUE if it succeed, EINA_FALSE if it failed.
 * @since 1.14
 *
 * @note The out buffer should have the necessary size to hold the
 * expanded data or it will fail. In case of failure, random garbage
 * could fill the out buffer.
 */
EAPI Eina_Bool emile_expand(const Eina_Binbuf * in, Eina_Binbuf * out, Emile_Compressor_Type t);
/**
 * @}
 */
#endif
