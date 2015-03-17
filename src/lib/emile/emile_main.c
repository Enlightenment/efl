#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#endif /* ifdef HAVE_GNUTLS */

#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/evp.h>
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
   if (_emile_cipher_inited) return EINA_TRUE;

   if (!_emile_cipher_init()) return EINA_FALSE;

   _emile_cipher_inited = EINA_TRUE;

   return EINA_TRUE;
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

   eina_log_timing(_emile_log_dom_global,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

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

   eina_log_timing(_emile_log_dom_global,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   if (_emile_cipher_init)
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
#ifdef HAVE_OPENSSL
        EVP_cleanup();
        ERR_free_strings();
#endif /* ifdef HAVE_OPENSSL */
     }

   eina_log_domain_unregister(_emile_log_dom_global);
   _emile_log_dom_global = -1;

   eina_shutdown();

   return _emile_init_count;
}
