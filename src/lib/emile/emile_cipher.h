#ifndef EMILE_CIPHER_H_
#define EMILE_CIPHER_H_

/**
 * @defgroup Emile_Cipher_Group Top level functions
 * @ingroup Emile
 * Function that allow ciphering content.
 *
 * @{
 */

/**
 * @typedef Emile_Cipher_Backend
 *
 * Flags describing the implemented backend.
 *
 * @since 1.14
 */
typedef enum _Emile_Cipher_Backend
{
  EMILE_NONE,
  EMILE_OPENSSL,
  EMILE_GNUTLS
} Emile_Cipher_Backend;

/**
 * @typedef Emile_Cipher_Algorithm
 *
 * Flags describing known cipher algorithm.
 *
 * @since 1.14
 */
typedef enum _Emile_Cipher_Algorithm
{
  EMILE_AES256_CBC
} Emile_Cipher_Algorithm;

/**
 * Force the initialization of the underlying cipher library.
 *
 * This call force the initialisation of GNUTLS or OpenSSL, so
 * that you get the same setup for everyone.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 * @see emile_cipher_module_get
 *
 * @since 1.14
 */
EAPI Eina_Bool emile_cipher_init(void);
/**
 * Get the name of the current used backend.
 *
 * @return the name of the current cipher backend.
 * @since 1.14
 */
EAPI Emile_Cipher_Backend emile_cipher_module_get(void);

/**
 * Cipher a buffer with a defined algorithm and key.
 *
 * @param algo The algorithm to use to cipher the buffer.
 * @param in The buffer to cipher.
 * @param key The symetric key to use for ciphering.
 * @param length The length of the symetric key to be used.
 * @return the ciphered buffer or NULL on error.
 *
 * @since 1.14
 */
EAPI Eina_Binbuf *emile_binbuf_cipher(Emile_Cipher_Algorithm algo, const Eina_Binbuf * in, const char *key, unsigned int length);

/**
 * Decipher a buffer with a defined algorithm and key.
 *
 * @param algo The algorithm to use to decipher the buffer.
 * @param in The ciphered buffer to decipher.
 * @param key The symetric key used to cipher the buffer.
 * @param length The length of the symetric key used to cipher the buffer.
 * @return the clear buffer or NULL on error.
 *
 * @note This won't detect if the given key is the correct one or not. You
 * have to check that the returned data make sense. You should also not treat
 * them as safe.
 *
 * @since 1.14
 */
EAPI Eina_Binbuf *emile_binbuf_decipher(Emile_Cipher_Algorithm algo, const Eina_Binbuf * in, const char *key, unsigned int length);

#ifdef EFL_BETA_API_SUPPORT

typedef struct _Emile_SSL Emile_SSL;

typedef enum
{
  EMILE_SSLv23,
  EMILE_TLSv1
} Emile_Cipher_Type;

typedef enum
{
  EMILE_WANT_NOTHING = 0,
  EMILE_WANT_READ = 1,
  EMILE_WANT_WRITE = 3
} Emile_Want_Type;

EAPI Eina_Bool emile_binbuf_hmac_sha1(const char *key, unsigned int key_len, const Eina_Binbuf * data, unsigned char digest[20]);

EAPI Eina_Bool emile_binbuf_sha1(const Eina_Binbuf * data, unsigned char digest[20]);


EAPI Emile_SSL *emile_cipher_server_listen(Emile_Cipher_Type t);
EAPI Emile_SSL *emile_cipher_client_connect(Emile_SSL * server, int fd);
EAPI Emile_SSL *emile_cipher_server_connect(Emile_Cipher_Type t);
EAPI Eina_Bool emile_cipher_free(Emile_SSL * emile);

EAPI Eina_Bool emile_cipher_cafile_add(Emile_SSL * emile, const char *file);
EAPI Eina_Bool emile_cipher_cert_add(Emile_SSL * emile, const char *file);
EAPI Eina_Bool emile_cipher_privkey_add(Emile_SSL * emile, const char *file);
EAPI Eina_Bool emile_cipher_crl_add(Emile_SSL * emile, const char *file);
EAPI int emile_cipher_read(Emile_SSL * emile, Eina_Binbuf * buffer);
EAPI int emile_cipher_write(Emile_SSL * emile, const Eina_Binbuf * buffer);
EAPI const char *emile_cipher_error_get(const Emile_SSL * emile);
EAPI Eina_Bool emile_cipher_verify_name_set(Emile_SSL * emile, const char *name);
EAPI const char *emile_cipher_verify_name_get(const Emile_SSL * emile);
EAPI void emile_cipher_verify_set(Emile_SSL * emile, Eina_Bool verify);
EAPI void emile_cipher_verify_basic_set(Emile_SSL * emile, Eina_Bool verify_basic);
EAPI Eina_Bool emile_cipher_verify_get(const Emile_SSL * emile);
EAPI Eina_Bool emile_cipher_verify_basic_get(const Emile_SSL * emile);

#endif

/**
 * @}
 */

#endif
