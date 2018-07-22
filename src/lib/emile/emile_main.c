#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gcrypt.h>
#endif /* ifdef HAVE_GNUTLS */

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#endif /* ifdef HAVE_OPENSSL */

#include <Eina.h>

#include "Emile.h"
#include "emile_private.h"

static Eina_Bool _emile_cipher_inited = EINA_FALSE;
static unsigned int _emile_init_count = 0;
int _emile_log_dom_global = -1;

EAPI Eina_Bool
emile_cipher_init(void)
{
   if (_emile_cipher_inited)
     return EINA_TRUE;

   if (!_emile_cipher_init())
     return EINA_FALSE;

   _emile_cipher_inited = EINA_TRUE;

   return EINA_TRUE;
}

EAPI Emile_Cipher_Backend
emile_cipher_module_get(void)
{
#ifdef HAVE_GNUTLS
   return EMILE_GNUTLS;
#else
#ifdef HAVE_OPENSSL
   return EMILE_OPENSSL;
#else
   return EMILE_NONE;
#endif
#endif
}

EAPI int
emile_init(void)
{
   if (++_emile_init_count != 1)
     return _emile_init_count;

   if (!eina_init())
     return --_emile_init_count;

   _emile_log_dom_global = eina_log_domain_register("emile", EINA_COLOR_CYAN);
   if (_emile_log_dom_global < 0)
     {
        EINA_LOG_ERR("Emile can not create a general log domain.");
        goto shutdown_eina;
     }

   eina_log_timing(_emile_log_dom_global, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   return _emile_init_count;

shutdown_eina:
   eina_shutdown();

   return --_emile_init_count;
}

EAPI int
emile_shutdown(void)
{
   if (--_emile_init_count != 0)
     return _emile_init_count;

   eina_log_timing(_emile_log_dom_global, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);

   if (_emile_cipher_inited)
     {
#ifdef HAVE_GNUTLS
        /* Note that gnutls has a leak where it doesnt free stuff it alloced
         * on init. valgrind trace here:
         * 21 bytes in 1 blocks are definitely lost in loss record 24 of 194
         *    at 0x4C2B6CD: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
         *    by 0x68AC801: strdup (strdup.c:43)
         *    by 0xD215B6A: p11_kit_registered_module_to_name (in /usr/lib/x86_64-linux-gnu/libp11-kit.so.0.0.0)
         *    by 0x9571574: gnutls_pkcs11_init (in /usr/lib/x86_64-linux-gnu/libgnutls.so.26.21.8)
         *    by 0x955B031: gnutls_global_init (in /usr/lib/x86_64-linux-gnu/libgnutls.so.26.21.8)
         *    by 0x6DFD6D0: eet_init (eet_lib.c:608)
         *
         * yes - i've tried calling gnutls_pkcs11_deinit() by hand but no luck.
         * the leak is in there.
         */
        gnutls_global_deinit();
#endif /* ifdef HAVE_GNUTLS */
#if defined(HAVE_OPENSSL) && (OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER))
        EVP_cleanup();
        ERR_free_strings();
#endif /* if defined(HAVE_OPENSSL) && (OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)) */
     }

   eina_log_domain_unregister(_emile_log_dom_global);
   _emile_log_dom_global = -1;

   eina_shutdown();

   return _emile_init_count;
}

/* For the moment, we have just one function shared accross both cipher
 * backend, so here it is. */
Eina_Bool
emile_pbkdf2_sha1(const char *key, unsigned int key_len, const unsigned char *salt, unsigned int salt_len, unsigned int iter, unsigned char *res, unsigned int res_len)
{
   Eina_Binbuf *step1, *step2;
   unsigned char *buf;
   unsigned char *p = res;
   unsigned char digest[20];
   unsigned char tab[4];
   unsigned int len = res_len;
   unsigned int tmp_len;
   unsigned int i, j, k;

   buf = alloca(salt_len + 4);
   if (!buf)
     return EINA_FALSE;

   step1 = eina_binbuf_manage_new(buf, salt_len + 4, EINA_TRUE);
   if (!step1)
     return EINA_FALSE;
   step2 = eina_binbuf_manage_new(digest, 20, EINA_TRUE);
   if (!step2)
     {
        eina_binbuf_free(step1);
        return EINA_FALSE;
     }

   for (i = 1; len; len -= tmp_len, p += tmp_len, i++)
     {
        tmp_len = (len > 20) ? 20 : len;

        tab[0] = (unsigned char)(i & 0xff000000) >> 24;
        tab[1] = (unsigned char)(i & 0x00ff0000) >> 16;
        tab[2] = (unsigned char)(i & 0x0000ff00) >> 8;
        tab[3] = (unsigned char)(i & 0x000000ff) >> 0;

        memcpy(buf, salt, salt_len);
        memcpy(buf + salt_len, tab, 4);

        if (!emile_binbuf_hmac_sha1(key, key_len, step1, digest))
          return EINA_FALSE;

        memcpy(p, digest, tmp_len);

        for (j = 1; j < iter; j++)
          {
             if (!emile_binbuf_hmac_sha1(key, key_len, step2, digest))
               return EINA_FALSE;
             for (k = 0; k < tmp_len; k++)
               p[k] ^= digest[k];
          }
     }

   eina_binbuf_free(step1);
   eina_binbuf_free(step2);

   return EINA_TRUE;
}

