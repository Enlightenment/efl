#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

Eina_Bool _emile_cipher_init(void)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_binbuf_hmac_sha1(const char *key EINA_UNUSED,
                       unsigned int key_len EINA_UNUSED,
                       const Eina_Binbuf *data EINA_UNUSED,
                       unsigned char digest[20] EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_binbuf_sha1(const Eina_Binbuf * data, unsigned char digest[20])
{
   return EINA_FALSE;
}

EMILE_API Eina_Binbuf *
emile_binbuf_cipher(Emile_Cipher_Algorithm algo EINA_UNUSED,
                    const Eina_Binbuf *data EINA_UNUSED,
                    const char *key EINA_UNUSED,
                    unsigned int length EINA_UNUSED)
{
   return NULL;
}

EMILE_API Eina_Binbuf *
emile_binbuf_decipher(Emile_Cipher_Algorithm algo EINA_UNUSED,
                      const Eina_Binbuf *data EINA_UNUSED,
                      const char *key EINA_UNUSED,
                      unsigned int length EINA_UNUSED)
{
   return NULL;
}

EMILE_API Emile_SSL *
emile_cipher_server_listen(Emile_Cipher_Type t EINA_UNUSED)
{
   return NULL;
}

EMILE_API Emile_SSL *
emile_cipher_client_connect(Emile_SSL *server EINA_UNUSED, int fd EINA_UNUSED)
{
   return NULL;
}

EMILE_API Emile_SSL *
emile_cipher_server_connect(Emile_Cipher_Type t EINA_UNUSED)
{
   return NULL;
}

EMILE_API Eina_Bool
emile_cipher_free(Emile_SSL *emile EINA_UNUSED)
{
   return EINA_TRUE;
}

EMILE_API Eina_Bool
emile_cipher_cafile_add(Emile_SSL *emile EINA_UNUSED,
                        const char *file EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_cipher_cert_add(Emile_SSL *emile EINA_UNUSED,
                      const char *file EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_cipher_privkey_add(Emile_SSL *emile EINA_UNUSED,
                         const char *file EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_cipher_crl_add(Emile_SSL *emile EINA_UNUSED,
                     const char *file EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API int
emile_cipher_read(Emile_SSL *emile EINA_UNUSED,
                  Eina_Binbuf *buffer EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API int
emile_cipher_write(Emile_SSL *emile EINA_UNUSED,
                   const Eina_Binbuf *buffer EINA_UNUSED)
{
   return EINA_FALSE;
}


EMILE_API const char *
emile_cipher_error_get(const Emile_SSL *emile EINA_UNUSED)
{
   return NULL;
}

EMILE_API Eina_Bool
emile_cipher_verify_name_set(Emile_SSL *emile EINA_UNUSED,
                             const char *name EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API const char *
emile_cipher_verify_name_get(const Emile_SSL *emile EINA_UNUSED)
{
   return NULL;
}

EMILE_API void
emile_cipher_verify_set(Emile_SSL *emile EINA_UNUSED,
                        Eina_Bool verify EINA_UNUSED)
{
}

EMILE_API void
emile_cipher_verify_basic_set(Emile_SSL *emile EINA_UNUSED,
                              Eina_Bool verify_basic EINA_UNUSED)
{
}

EMILE_API Eina_Bool
emile_cipher_verify_get(const Emile_SSL *emile EINA_UNUSED)
{
   return EINA_FALSE;
}

EMILE_API Eina_Bool
emile_cipher_verify_basic_get(const Emile_SSL *emile EINA_UNUSED)
{
   return EINA_FALSE;
}
