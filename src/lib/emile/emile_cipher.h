#ifndef EMILE_CIPHER_H_
#define EMILE_CIPHER_H_

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

EAPI Eina_Bool emile_binbuf_sha1(const char *key,
                                 unsigned int key_len,
                                 const Eina_Binbuf *data,
                                 unsigned char digest[20]);


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

#endif
