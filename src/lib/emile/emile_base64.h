#ifndef EMILE_BASE64_H_
#define EMILE_BASE64_H_

/**
 * @defgroup Emile_Group_Base64 Non destructive base64 manipulation functions.
 * @ingroup Emile
 * Function that allow the encoding and decoding of base64 Eina_Binbuf.
 *
 * @{
 */

/**
 * @brief base64 encoding function.
 * @param in The buffer to be encoded.
 * @return the base64 encoded string.
 *
 * This will create a string which is base64 encode of the buffer. The caller has
 * to free the returned string using eina_strbuf_free().
 *
 * @since 1.17.0
 */
EAPI Eina_Strbuf *emile_base64_encode(const Eina_Binbuf *in);

/**
 * @brief base64 url and filename safe encoding function.
 * @param in The buffer to be encoded.
 * @return the base64 url encoded string.
 *
 * This will create a string which is base64 encoded with url and
 * filename safe alphabet of the src. The caller has to free the
 * returned string using eina_strbuf_free(). There will be no padding in the
 * encoded string.
 *
 * @since 1.17.0
 */
EAPI Eina_Strbuf *emile_base64url_encode(const Eina_Binbuf *in);

/**
 * @brief base64 decoding function.
 * @param in The string to be decoded.
 * @return the base64 decoded buffer.
 *
 * This will create a buffer which is base64 decode of the src.
 * The caller has to free the returned string using eina_binbuf_free().
 *
 * @since 1.17.0
 */
EAPI Eina_Binbuf* emile_base64_decode(const Eina_Strbuf *in);

/**
 * @brief decoding function for base64 url and filename safe encoding.
 * @param in The string to be decoded.
 * @return the base64 url decoded buffer.
 *
 * This will create a buffer which is base64 url decode of the src.
 * The caller has to free the returned string using eina_binbuf_free().
 *
 * @since 1.17.0
 */
EAPI Eina_Binbuf* emile_base64url_decode(const Eina_Strbuf *in);

/**
 * @}
 */
#endif
