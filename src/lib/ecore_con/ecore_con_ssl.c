#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#if HAVE_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#elif HAVE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/dh.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include <sys/stat.h>
#include "Ecore.h"
#include "ecore_con_private.h"

EAPI int ECORE_CON_EVENT_CLIENT_UPGRADE = 0;
EAPI int ECORE_CON_EVENT_SERVER_UPGRADE = 0;

static int _init_con_ssl_init_count = 0;

#ifdef HAVE_GNUTLS
GCRY_THREAD_OPTION_PTHREAD_IMPL;

static int _client_connected = 0;

# define SSL_SUFFIX(ssl_func) ssl_func ## _gnutls
# define _ECORE_CON_SSL_AVAILABLE 1

#elif HAVE_OPENSSL

# define SSL_SUFFIX(ssl_func) ssl_func ## _openssl
# define _ECORE_CON_SSL_AVAILABLE 2

#else
# define SSL_SUFFIX(ssl_func) ssl_func ## _none
# define _ECORE_CON_SSL_AVAILABLE 0

#endif

#if HAVE_GNUTLS
static void
_gnutls_print_errors(void *conn, int type, int ret)
{
   char buf[1024];

   if (!ret) return;

   snprintf(buf, sizeof(buf), "GNUTLS error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
   if (type == ECORE_CON_EVENT_CLIENT_ERROR)
     ecore_con_event_client_error(conn, buf);
   else
     ecore_con_event_server_error(conn, buf);
}

static void
_gnutls_print_session(const gnutls_datum_t *cert_list, unsigned int cert_list_size)
{
   char *c = NULL;
   gnutls_x509_crt_t crt;
   unsigned int x;

   if (!eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG)) return;
   for (x = 0; x < cert_list_size; x++)
     {
        gnutls_x509_crt_init(&crt);
        gnutls_x509_crt_import(crt, &cert_list[x], GNUTLS_X509_FMT_DER);
        gnutls_x509_crt_print(crt, GNUTLS_CRT_PRINT_FULL, (gnutls_datum_t *)&c);
        INF("CERTIFICATE:\n%s", c);
        gnutls_free(c);
        gnutls_x509_crt_deinit(crt);
        crt = NULL;
     }
}

#ifdef ISCOMFITOR
static void
_gnutls_log_func(int level,
                 const char *str)
{
   char buf[128];
   strncat(buf, str, strlen(str) - 1);
   DBG("|<%d>| %s", level, buf);
}

#endif

static const char *
SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_description_t status)
{
   switch (status)
     {
      case GNUTLS_HANDSHAKE_HELLO_REQUEST:
        return "Hello request";

      case GNUTLS_HANDSHAKE_CLIENT_HELLO:
        return "Client hello";

      case GNUTLS_HANDSHAKE_SERVER_HELLO:
        return "Server hello";

      case GNUTLS_HANDSHAKE_NEW_SESSION_TICKET:
        return "New session ticket";

      case GNUTLS_HANDSHAKE_CERTIFICATE_PKT:
        return "Certificate packet";

      case GNUTLS_HANDSHAKE_SERVER_KEY_EXCHANGE:
        return "Server key exchange";

      case GNUTLS_HANDSHAKE_CERTIFICATE_REQUEST:
        return "Certificate request";

      case GNUTLS_HANDSHAKE_SERVER_HELLO_DONE:
        return "Server hello done";

      case GNUTLS_HANDSHAKE_CERTIFICATE_VERIFY:
        return "Certificate verify";

      case GNUTLS_HANDSHAKE_CLIENT_KEY_EXCHANGE:
        return "Client key exchange";

      case GNUTLS_HANDSHAKE_FINISHED:
        return "Finished";

      case GNUTLS_HANDSHAKE_SUPPLEMENTAL:
        return "Supplemental";
     }
   return NULL;
}

#elif HAVE_OPENSSL

static void
_openssl_print_verify_error(int error)
{
   switch (error)
     {
#define ERROR(X) \
case (X):        \
  ERR("%s", #X); \
  break
#ifdef X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT
        ERROR(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT);
#endif
#ifdef X509_V_ERR_UNABLE_TO_GET_CRL
        ERROR(X509_V_ERR_UNABLE_TO_GET_CRL);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE
        ERROR(X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE
        ERROR(X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY
        ERROR(X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY);
#endif
#ifdef X509_V_ERR_CERT_SIGNATURE_FAILURE
        ERROR(X509_V_ERR_CERT_SIGNATURE_FAILURE);
#endif
#ifdef X509_V_ERR_CRL_SIGNATURE_FAILURE
        ERROR(X509_V_ERR_CRL_SIGNATURE_FAILURE);
#endif
#ifdef X509_V_ERR_CERT_NOT_YET_VALID
        ERROR(X509_V_ERR_CERT_NOT_YET_VALID);
#endif
#ifdef X509_V_ERR_CERT_HAS_EXPIRED
        ERROR(X509_V_ERR_CERT_HAS_EXPIRED);
#endif
#ifdef X509_V_ERR_CRL_NOT_YET_VALID
        ERROR(X509_V_ERR_CRL_NOT_YET_VALID);
#endif
#ifdef X509_V_ERR_CRL_HAS_EXPIRED
        ERROR(X509_V_ERR_CRL_HAS_EXPIRED);
#endif
#ifdef X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD
        ERROR(X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD
        ERROR(X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD
        ERROR(X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD
        ERROR(X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD);
#endif
#ifdef X509_V_ERR_OUT_OF_MEM
        ERROR(X509_V_ERR_OUT_OF_MEM);
#endif
#ifdef X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
        ERROR(X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT);
#endif
#ifdef X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN
        ERROR(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
#endif
#ifdef X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
        ERROR(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY);
#endif
#ifdef X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE
        ERROR(X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE);
#endif
#ifdef X509_V_ERR_CERT_CHAIN_TOO_LONG
        ERROR(X509_V_ERR_CERT_CHAIN_TOO_LONG);
#endif
#ifdef X509_V_ERR_CERT_REVOKED
        ERROR(X509_V_ERR_CERT_REVOKED);
#endif
#ifdef X509_V_ERR_INVALID_CA
        ERROR(X509_V_ERR_INVALID_CA);
#endif
#ifdef X509_V_ERR_PATH_LENGTH_EXCEEDED
        ERROR(X509_V_ERR_PATH_LENGTH_EXCEEDED);
#endif
#ifdef X509_V_ERR_INVALID_PURPOSE
        ERROR(X509_V_ERR_INVALID_PURPOSE);
#endif
#ifdef X509_V_ERR_CERT_UNTRUSTED
        ERROR(X509_V_ERR_CERT_UNTRUSTED);
#endif
#ifdef X509_V_ERR_CERT_REJECTED
        ERROR(X509_V_ERR_CERT_REJECTED);
#endif
        /* These are 'informational' when looking for issuer cert */
#ifdef X509_V_ERR_SUBJECT_ISSUER_MISMATCH
        ERROR(X509_V_ERR_SUBJECT_ISSUER_MISMATCH);
#endif
#ifdef X509_V_ERR_AKID_SKID_MISMATCH
        ERROR(X509_V_ERR_AKID_SKID_MISMATCH);
#endif
#ifdef X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH
        ERROR(X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_CERTSIGN
        ERROR(X509_V_ERR_KEYUSAGE_NO_CERTSIGN);
#endif

#ifdef X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER
        ERROR(X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER);
#endif
#ifdef X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION
        ERROR(X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_CRL_SIGN
        ERROR(X509_V_ERR_KEYUSAGE_NO_CRL_SIGN);
#endif
#ifdef X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION
        ERROR(X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION);
#endif
#ifdef X509_V_ERR_INVALID_NON_CA
        ERROR(X509_V_ERR_INVALID_NON_CA);
#endif
#ifdef X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED
        ERROR(X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE
        ERROR(X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE);
#endif
#ifdef X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED
        ERROR(X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED);
#endif

#ifdef X509_V_ERR_INVALID_EXTENSION
        ERROR(X509_V_ERR_INVALID_EXTENSION);
#endif
#ifdef X509_V_ERR_INVALID_POLICY_EXTENSION
        ERROR(X509_V_ERR_INVALID_POLICY_EXTENSION);
#endif
#ifdef X509_V_ERR_NO_EXPLICIT_POLICY
        ERROR(X509_V_ERR_NO_EXPLICIT_POLICY);
#endif
#ifdef X509_V_ERR_DIFFERENT_CRL_SCOPE
        ERROR(X509_V_ERR_DIFFERENT_CRL_SCOPE);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE
        ERROR(X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE);
#endif

#ifdef X509_V_ERR_UNNESTED_RESOURCE
        ERROR(X509_V_ERR_UNNESTED_RESOURCE);
#endif

#ifdef X509_V_ERR_PERMITTED_VIOLATION
        ERROR(X509_V_ERR_PERMITTED_VIOLATION);
#endif
#ifdef X509_V_ERR_EXCLUDED_VIOLATION
        ERROR(X509_V_ERR_EXCLUDED_VIOLATION);
#endif
#ifdef X509_V_ERR_SUBTREE_MINMAX
        ERROR(X509_V_ERR_SUBTREE_MINMAX);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE
        ERROR(X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX
        ERROR(X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_NAME_SYNTAX
        ERROR(X509_V_ERR_UNSUPPORTED_NAME_SYNTAX);
#endif
#ifdef X509_V_ERR_CRL_PATH_VALIDATION_ERROR
        ERROR(X509_V_ERR_CRL_PATH_VALIDATION_ERROR);
#endif

        /* The application is not happy */
#ifdef X509_V_ERR_APPLICATION_VERIFICATION
        ERROR(X509_V_ERR_APPLICATION_VERIFICATION);
#endif
     }
#undef ERROR
}

static void
_openssl_print_errors(void *conn, int type)
{
   char buf[1024];
   do
     {
        unsigned long err;

        err = ERR_get_error();
        if (!err) break;
        snprintf(buf, sizeof(buf), "OpenSSL error: %s", ERR_reason_error_string(err));
        if (type == ECORE_CON_EVENT_CLIENT_ERROR)
          ecore_con_event_client_error(conn, buf);
        else
          ecore_con_event_server_error(conn, buf);
     } while (1);
}

static Eina_Bool
_openssl_name_verify(const char *name, const char *svrname)
{
   if (name[0] == '*')
     {
        /* we allow *.domain.TLD with a wildcard, but nothing else */
        const char *p, *s;

        EINA_SAFETY_ON_TRUE_RETURN_VAL((name[1] != '.') || (!name[2]), EINA_FALSE);
        p = strchr(name + 1, '*');
        EINA_SAFETY_ON_TRUE_RETURN_VAL(!!p, EINA_FALSE);
        /* verify that we have a domain of at least *.X.TLD and not *.TLD */
        p = strchr(name + 2, '.');
        EINA_SAFETY_ON_TRUE_RETURN_VAL(!p, EINA_FALSE);
        s = strchr(svrname, '.');
        EINA_SAFETY_ON_TRUE_RETURN_VAL(!s, EINA_FALSE);
        /* same as above for the stored name */
        EINA_SAFETY_ON_TRUE_RETURN_VAL(!strchr(s + 1, '.'), EINA_FALSE);
        if (strcasecmp(s, name + 1))
          {
             ERR("%s != %s", s, name + 1);
             return EINA_FALSE;
          }
     }
   else
   if (strcasecmp(name, svrname))
     {
        ERR("%s != %s", name, svrname);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_openssl_print_session(SSL *ssl)
{
   /* print session info into DBG */
   SSL_SESSION *s;
   STACK_OF(X509) * sk;
   BIO *b;
   char log[4096], *p;
   int x;

   if (!eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG)) return;

   memset(log, 0, sizeof(log));
   b = BIO_new(BIO_s_mem());
   sk = SSL_get_peer_cert_chain(ssl);
   if (sk)
     {
        DBG("CERTIFICATES:");
        for (x = 0; x < sk_X509_num(sk); x++)
          {
             p = X509_NAME_oneline(X509_get_subject_name(sk_X509_value(sk, x)), log, sizeof(log));
             DBG("%2d s:%s", x, p);
             p = X509_NAME_oneline(X509_get_issuer_name(sk_X509_value(sk, x)), log, sizeof(log));
             DBG("   i:%s", p);
             PEM_write_X509(stderr, sk_X509_value(sk, x));
          }
     }
   s = SSL_get_session(ssl);
   SSL_SESSION_print(b, s);
   fprintf(stderr, "\n");
   while (BIO_read(b, log, sizeof(log)) > 0)
     fprintf(stderr, "%s", log);

   BIO_free(b);
}

#endif

#define SSL_ERROR_CHECK_GOTO_ERROR(X)                                           \
  do                                                                            \
    {                                                                           \
       if ((X))                                                                 \
         {                                                                      \
            ERR("Error at %s:%s:%d!", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
            goto error;                                                         \
         }                                                                      \
    }                                                                           \
  while (0)

static Ecore_Con_Ssl_Error
                           SSL_SUFFIX(_ecore_con_ssl_init) (void);
static Ecore_Con_Ssl_Error
                           SSL_SUFFIX(_ecore_con_ssl_shutdown) (void);

static Eina_Bool           SSL_SUFFIX(_ecore_con_ssl_server_cafile_add) (Ecore_Con_Server *svr, const char *ca_file);
static Eina_Bool           SSL_SUFFIX(_ecore_con_ssl_server_crl_add) (Ecore_Con_Server *svr, const char *crl_file);
static Eina_Bool           SSL_SUFFIX(_ecore_con_ssl_server_cert_add) (Ecore_Con_Server *svr, const char *cert);
static Eina_Bool           SSL_SUFFIX(_ecore_con_ssl_server_privkey_add) (Ecore_Con_Server *svr, const char *key_file);

static Ecore_Con_Ssl_Error SSL_SUFFIX(_ecore_con_ssl_server_prepare) (Ecore_Con_Server *svr, int ssl_type);
static Ecore_Con_Ssl_Error SSL_SUFFIX(_ecore_con_ssl_server_init) (Ecore_Con_Server *svr);
static Ecore_Con_Ssl_Error SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (Ecore_Con_Server *svr);
static int                 SSL_SUFFIX(_ecore_con_ssl_server_read) (Ecore_Con_Server *svr, unsigned char *buf, int size);
static int                 SSL_SUFFIX(_ecore_con_ssl_server_write) (Ecore_Con_Server *svr, const unsigned char *buf, int size);

static Ecore_Con_Ssl_Error SSL_SUFFIX(_ecore_con_ssl_client_init) (Ecore_Con_Client *cl);
static Ecore_Con_Ssl_Error SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (Ecore_Con_Client *cl);
static int                 SSL_SUFFIX(_ecore_con_ssl_client_read) (Ecore_Con_Client *cl,
                                                                    unsigned char *buf, int size);
static int                 SSL_SUFFIX(_ecore_con_ssl_client_write) (Ecore_Con_Client *cl,
                                                                     const unsigned char *buf, int size);

/*
 * General SSL API
 */

Ecore_Con_Ssl_Error
ecore_con_ssl_init(void)
{
   if (!_init_con_ssl_init_count++)
     {
        SSL_SUFFIX(_ecore_con_ssl_init) ();
#if _ECORE_CON_SSL_AVAILABLE != 0
        ECORE_CON_EVENT_CLIENT_UPGRADE = ecore_event_type_new();
        ECORE_CON_EVENT_SERVER_UPGRADE = ecore_event_type_new();
#endif
     }

   return _init_con_ssl_init_count;
}

Ecore_Con_Ssl_Error
ecore_con_ssl_shutdown(void)
{
   if (!--_init_con_ssl_init_count)
     SSL_SUFFIX(_ecore_con_ssl_shutdown) ();

   return _init_con_ssl_init_count;
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_prepare(Ecore_Con_Server *svr,
                             int ssl_type)
{
   if (!ssl_type)
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_server_prepare) (svr, ssl_type);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_init(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr || !(svr->type & ECORE_CON_SSL))
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_server_init) (obj);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_shutdown(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr || !(svr->type & ECORE_CON_SSL))
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (obj);
}

int
ecore_con_ssl_server_read(Ecore_Con_Server *svr,
                          unsigned char *buf,
                          int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_read) (svr, buf, size);
}

int
ecore_con_ssl_server_write(Ecore_Con_Server *svr,
                           const unsigned char *buf,
                           int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_write) (svr, buf, size);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_init(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
     return ECORE_CON_SSL_ERROR_NONE;
   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
   if (host_server || !(host_server->type & ECORE_CON_SSL))
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_client_init) (obj);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_shutdown(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
     return ECORE_CON_SSL_ERROR_NONE;
   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
   if (!host_server || !(host_server->type & ECORE_CON_SSL))
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (obj);
}

int
ecore_con_ssl_client_read(Ecore_Con_Client *cl,
                          unsigned char *buf,
                          int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_read) (cl, buf, size);
}

int
ecore_con_ssl_client_write(Ecore_Con_Client *cl,
                           const unsigned char *buf,
                           int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_write) (cl, buf, size);
}

/**
 * Returns if SSL support is available
 * @return 1 if SSL is available and provided by gnutls, 2 if provided by openssl,
 * 0 if it is not available.
 * @ingroup Ecore_Con_Client_Group
 */
EAPI int
ecore_con_ssl_available_get(void)
{
   return _ECORE_CON_SSL_AVAILABLE;
}

/**
 * @addtogroup Ecore_Con_SSL_Group Ecore Connection SSL Functions
 *
 * Functions that operate on Ecore connection objects pertaining to SSL.
 *
 * @{
 */

/**
 * @brief Enable certificate verification on a server object
 *
 * Call this function on a server object before main loop has started
 * to enable verification of certificates against loaded certificates.
 * @param svr The server object
 */
EAPI void
ecore_con_ssl_server_verify(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return;

   svr->verify = EINA_TRUE;
}

/**
 * @brief Enable hostname-based certificate verification on a server object
 *
 * Call this function on a server object before main loop has started
 * to enable verification of certificates using ONLY their hostnames.
 * @param svr The server object
 * @note This function has no effect when used on a listening server created by
 * ecore_con_server_add
 * @since 1.1
 */
EAPI void
ecore_con_ssl_server_verify_basic(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return;

   svr->verify_basic = EINA_TRUE;
}

/**
 * @brief Set the hostname to verify against in certificate verification
 *
 * Sometimes the certificate hostname will not match the hostname that you are
 * connecting to, and will instead match a different name. An example of this is
 * that if you connect to talk.google.com to use Google Talk, you receive Google's
 * certificate for gmail.com. This certificate should be trusted, and so you must call
 * this function with "gmail.com" as @p name.
 * See RFC2818 for more details.
 * @param svr The server object
 * @param name The hostname to verify against
 * @since 1.2
 */
EAPI void
ecore_con_ssl_server_verify_name_set(Ecore_Con_Server *obj, const char *name)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return;

   eina_stringshare_replace(&svr->verify_name, name);
}

/**
 * @brief Get the hostname to verify against in certificate verification
 *
 * This function returns the name which will be used to validate the SSL certificate
 * common name (CN) or alt name (subjectAltName). It will default to the @p name
 * param in ecore_con_server_connect(), but can be changed with ecore_con_ssl_server_verify_name_set().
 * @param svr The server object
 * @return The hostname which will be used
 * @since 1.2
 */
EAPI const char *
ecore_con_ssl_server_verify_name_get(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return NULL;

   return svr->verify_name ? : svr->name;
}

/**
 * @brief Add an ssl certificate for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM certificate.
 * Simply specify the cert here to use it in the server object for connecting or listening.
 * If there is an error loading the certificate, an error will automatically be logged.
 * @param svr The server object
 * @param cert The path to the certificate.
 * @return @c EINA_FALSE if the file cannot be loaded, otherwise @c EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_cert_add(Ecore_Con_Server *obj,
                              const char *cert)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return EINA_FALSE;

   if (!svr->ssl_prepared)
     {
        svr->use_cert = EINA_TRUE;
        svr->type |= ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
        if (ecore_con_ssl_server_prepare(obj, svr->type & ECORE_CON_SSL))
          return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_cert_add) (obj, cert);
}

/**
 * @brief Add an ssl CA file for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM CA file.
 * Simply specify the file here to use it in the server object for connecting or listening.
 * If there is an error loading the CAs, an error will automatically be logged.
 * @param svr The server object
 * @param ca_file The path to the CA file.
 * @return @c EINA_FALSE if the file cannot be loaded, otherwise @c EINA_TRUE.
 * @note since 1.2, this function can load directores
 */

EAPI Eina_Bool
ecore_con_ssl_server_cafile_add(Ecore_Con_Server *obj,
                                const char *ca_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return EINA_FALSE;

   if (!svr->ssl_prepared)
     {
        svr->use_cert = EINA_TRUE;
        svr->type |= ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
        if (ecore_con_ssl_server_prepare(obj, svr->type & ECORE_CON_SSL))
          return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_cafile_add) (obj, ca_file);
}

/**
 * @brief Add an ssl private key for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM private key
 * Simply specify the key file here to use it in the server object for connecting or listening.
 * If there is an error loading the key, an error will automatically be logged.
 * @param svr The server object
 * @param key_file The path to the key file.
 * @return @c EINA_FALSE if the file cannot be loaded, otherwise @c EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_privkey_add(Ecore_Con_Server *obj,
                                 const char *key_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return EINA_FALSE;

   if (!svr->ssl_prepared)
     {
        svr->use_cert = EINA_TRUE;
        svr->type |= ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
        if (ecore_con_ssl_server_prepare(obj, svr->type & ECORE_CON_SSL))
          return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_privkey_add) (obj, key_file);
}

/**
 * @brief Add an ssl CRL for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM CRL file
 * Simply specify the CRL file here to use it in the server object for connecting or listening.
 * If there is an error loading the CRL, an error will automatically be logged.
 * @param svr The server object
 * @param crl_file The path to the CRL file.
 * @return @c EINA_FALSE if the file cannot be loaded, otherwise @c EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_crl_add(Ecore_Con_Server *obj,
                             const char *crl_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return EINA_FALSE;

   if (!svr->ssl_prepared)
     {
        svr->use_cert = EINA_TRUE;
        svr->type |= ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
        if (ecore_con_ssl_server_prepare(obj, svr->type & ECORE_CON_SSL))
          return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_crl_add) (obj, crl_file);
}

/**
 * @brief Upgrade a connection to a specified level of encryption
 *
 * Use this function to begin an SSL handshake on a connection (STARTTLS or similar).
 * Once the upgrade has been completed, an ECORE_CON_EVENT_SERVER_UPGRADE event will be emitted.
 * The connection should be treated as disconnected until the next event.
 * @param svr The server object
 * @param ssl_type The SSL connection type (ONLY).
 * @return @c EINA_FALSE if the connection cannot be upgraded, otherwise @c EINA_TRUE.
 * @note This function is NEVER to be used on a server object created with ecore_con_server_add
 * @warning Setting a wrong value for @p compl_type WILL mess up your program.
 * @since 1.1
 */

EAPI Eina_Bool
ecore_con_ssl_server_upgrade(Ecore_Con_Server *obj, Ecore_Con_Type ssl_type)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr) return EINA_FALSE;
#if _ECORE_CON_SSL_AVAILABLE == 0
   return EINA_FALSE;
#endif

   if (!svr->ssl_prepared)
     {
        if (ecore_con_ssl_server_prepare(obj, ssl_type))
          return EINA_FALSE;
     }
   if (!svr->use_cert)
     svr->type |= ssl_type;
   svr->upgrade = EINA_TRUE;
   svr->handshaking = EINA_TRUE;
   svr->ssl_state = ECORE_CON_SSL_STATE_INIT;
   return !SSL_SUFFIX(_ecore_con_ssl_server_init) (obj);
}

/**
 * @brief Upgrade a connection to a specified level of encryption
 *
 * Use this function to begin an SSL handshake on a connection (STARTTLS or similar).
 * Once the upgrade has been completed, an ECORE_CON_EVENT_CLIENT_UPGRADE event will be emitted.
 * The connection should be treated as disconnected until the next event.
 * @param cl The client object
 * @param ssl_type The SSL connection type (ONLY).
 * @return @c EINA_FALSE if the connection cannot be upgraded, otherwise @c EINA_TRUE.
 * @warning Setting a wrong value for @p compl_type WILL mess up your program.
 * @since 1.1
 */

EAPI Eina_Bool
ecore_con_ssl_client_upgrade(Ecore_Con_Client *obj, Ecore_Con_Type ssl_type)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
     {
        return EINA_FALSE;
     }
#if _ECORE_CON_SSL_AVAILABLE == 0
   return EINA_FALSE;
#endif

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
   if (!host_server->ssl_prepared)
     {
        if (ecore_con_ssl_server_prepare(cl->host_server, ssl_type))
          return EINA_FALSE;
     }
   if (!host_server->use_cert)
     host_server->type |= ssl_type;
   cl->upgrade = EINA_TRUE;
   host_server->upgrade = EINA_TRUE;
   cl->handshaking = EINA_TRUE;
   cl->ssl_state = ECORE_CON_SSL_STATE_INIT;
   return SSL_SUFFIX(_ecore_con_ssl_client_init) (obj);
}

/**
 * @}
 */

#if HAVE_GNUTLS

/*
 * GnuTLS
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_gnutls(void)
{
   if (gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
     WRN("YOU ARE USING PTHREADS, BUT I CANNOT INITIALIZE THREADSAFE GCRYPT OPERATIONS!");
   if (gnutls_global_init())
     return ECORE_CON_SSL_ERROR_INIT_FAILED;

#ifdef ISCOMFITOR
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        gnutls_global_set_log_level(9);
        gnutls_global_set_log_function(_gnutls_log_func);
     }
#endif
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_gnutls(void)
{
   gnutls_global_deinit();

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_gnutls(Ecore_Con_Server *svr,
                                     int ssl_type)
{
   int ret;

   if (ssl_type & ECORE_CON_USE_SSL2)
     return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

   switch (ssl_type)
     {
      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
      case ECORE_CON_USE_MIXED:
      case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
        break;

      default:
        return ECORE_CON_SSL_ERROR_NONE;
     }

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_allocate_credentials(&svr->cert));

   if (svr->use_cert)
     {
        if (svr->created)
          {
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_init(&svr->dh_params));
             INF("Generating DH params");
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_generate2(svr->dh_params, 1024));

             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_server_credentials(&svr->anoncred_s));
             /* TODO: implement PSK */
             //  SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_server_credentials(&svr->pskcred_s));

             gnutls_anon_set_server_dh_params(svr->anoncred_s, svr->dh_params);
             gnutls_certificate_set_dh_params(svr->cert, svr->dh_params);
             //gnutls_psk_set_server_dh_params(svr->pskcred_s, svr->dh_params);
             INF("DH params successfully generated and applied!");
          }
        else
          {
             //SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_client_credentials(&svr->pskcred_c));
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_client_credentials(&svr->anoncred_c));
          }
     }

   svr->ssl_prepared = EINA_TRUE;
   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR, ret);
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_gnutls(Ecore_Con_Server *svr)
{
   const gnutls_datum_t *cert_list;
   unsigned int iter, cert_list_size;
   gnutls_x509_crt_t cert = NULL;
   const char *priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT";
   int ret = 0;

   switch (svr->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;

      case ECORE_CON_SSL_STATE_INIT:
        if (svr->type & ECORE_CON_USE_SSL2) /* not supported because of security issues */
          return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

        switch (svr->type & ECORE_CON_SSL)
          {
           case ECORE_CON_USE_SSL3:
           case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
             priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-TLS1.0:!VERS-TLS1.1:!VERS-TLS1.2";
             break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
             priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0";
             break;

           case ECORE_CON_USE_MIXED:
           case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
             break;

           default:
             return ECORE_CON_SSL_ERROR_NONE;
          }

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&svr->session, GNUTLS_CLIENT));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_client(svr->session));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_server_name_set(svr->session, GNUTLS_NAME_DNS, svr->name, strlen(svr->name)));
        INF("Applying priority string: %s", priority);
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_priority_set_direct(svr->session, priority, NULL));
        gnutls_handshake_set_private_extensions(svr->session, 1);
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_CERTIFICATE, svr->cert));
        // SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_PSK, svr->pskcred_c));
        if (!svr->use_cert)
          SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_ANON, svr->anoncred_c));

        gnutls_dh_set_prime_bits(svr->session, 512);
        gnutls_transport_set_ptr(svr->session, (gnutls_transport_ptr_t)((intptr_t)svr->fd));
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;

      case ECORE_CON_SSL_STATE_HANDSHAKING:
        if (!svr->session)
          {
             DBG("Server was previously lost, going to error condition");
             goto error;
          }
        ret = gnutls_handshake(svr->session);
        DBG("calling gnutls_handshake(): returned with '%s'", gnutls_strerror_name(ret));
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));
        if (!ret)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (gnutls_record_get_direction(svr->session))
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
             else
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
             return ECORE_CON_SSL_ERROR_NONE;
          }

      default:
        break;
     }

   if ((!svr->verify) && (!svr->verify_basic))
     /* not verifying certificates, so we're done! */
     return ECORE_CON_SSL_ERROR_NONE;
   if (svr->verify)
     {
        /* use CRL/CA lists to verify */
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_verify_peers2(svr->session, &iter));
        if (iter & GNUTLS_CERT_INVALID)
          ERR("The certificate is not trusted.");
        else if (iter & GNUTLS_CERT_SIGNER_NOT_FOUND)
          ERR("The certificate hasn't got a known issuer.");
        else if (iter & GNUTLS_CERT_REVOKED)
          ERR("The certificate has been revoked.");
        else if (iter & GNUTLS_CERT_EXPIRED)
          ERR("The certificate has expired");
        else if (iter & GNUTLS_CERT_NOT_ACTIVATED)
          ERR("The certificate is not yet activated");

        if (iter)
          goto error;
     }
   if (gnutls_certificate_type_get(svr->session) != GNUTLS_CRT_X509)
     {
        ERR("Warning: PGP certificates are not yet supported!");
        goto error;
     }

   SSL_ERROR_CHECK_GOTO_ERROR(!(cert_list = gnutls_certificate_get_peers(svr->session, &cert_list_size)));
   SSL_ERROR_CHECK_GOTO_ERROR(!cert_list_size);

   _gnutls_print_session(cert_list, cert_list_size);

   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_init(&cert));
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER));

   SSL_ERROR_CHECK_GOTO_ERROR(!gnutls_x509_crt_check_hostname(cert, svr->verify_name ? : svr->name));
   gnutls_x509_crt_deinit(cert);
   DBG("SSL certificate verification succeeded!");
   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR, ret);
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(svr->session)));
   if (svr->session && (svr->ssl_state != ECORE_CON_SSL_STATE_DONE))
     {
        ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(svr->session)));
        ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(svr->session)));
     }
   if (cert)
     gnutls_x509_crt_deinit(cert);
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_gnutls(Ecore_Con_Server *svr,
                                        const char *ca_file)
{
   struct stat st;
   Eina_Iterator *it;
   const char *file;
   Eina_Bool error = EINA_FALSE;

   if (stat(ca_file, &st)) return EINA_FALSE;
   if (S_ISDIR(st.st_mode))
     {
        it = eina_file_ls(ca_file);
        SSL_ERROR_CHECK_GOTO_ERROR(!it);
        EINA_ITERATOR_FOREACH(it, file)
          {
             if (!error)
               {
                  if (gnutls_certificate_set_x509_trust_file(svr->cert, file, GNUTLS_X509_FMT_PEM) < 1)
                    error++;
               }
             eina_stringshare_del(file);
          }
        eina_iterator_free(it);
     }
   else
     SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_trust_file(svr->cert, ca_file,
                                                                       GNUTLS_X509_FMT_PEM) < 1);

   return !error;
error:
   ERR("Could not load CA file!");
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_gnutls(Ecore_Con_Server *svr,
                                     const char *crl_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_crl_file(svr->cert, crl_file,
                                                                   GNUTLS_X509_FMT_PEM) < 1);

   return EINA_TRUE;
error:
   ERR("Could not load CRL file!");
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_privkey_add_gnutls(Ecore_Con_Server *svr,
                                         const char *key_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_key_file(svr->cert, svr->cert_file, key_file,
                                                                   GNUTLS_X509_FMT_PEM));

   return EINA_TRUE;
error:
   ERR("Could not load certificate/key file!");
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_gnutls(Ecore_Con_Server *svr,
                                      const char *cert_file)
{
   if (!(svr->cert_file = strdup(cert_file)))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_gnutls(Ecore_Con_Server *svr)
{
   if (svr->session)
     {
        gnutls_bye(svr->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(svr->session);
     }

   free(svr->cert_file);
   svr->cert_file = NULL;
   if (svr->cert)
     gnutls_certificate_free_credentials(svr->cert);
   svr->cert = NULL;

   if ((svr->type & ECORE_CON_SSL) && svr->created)
     {
        if (svr->dh_params)
          {
             gnutls_dh_params_deinit(svr->dh_params);
             svr->dh_params = NULL;
          }
        if (svr->anoncred_s)
          gnutls_anon_free_server_credentials(svr->anoncred_s);
        //  if (svr->pskcred_s)
        //   gnutls_psk_free_server_credentials(svr->pskcred_s);

        svr->anoncred_s = NULL;
        svr->pskcred_s = NULL;
     }
   else if (svr->type & ECORE_CON_SSL)
     {
        if (svr->anoncred_c)
          gnutls_anon_free_client_credentials(svr->anoncred_c);
        // if (svr->pskcred_c)
        // gnutls_psk_free_client_credentials(svr->pskcred_c);

        svr->anoncred_c = NULL;
        svr->pskcred_c = NULL;
     }

   svr->session = NULL;

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_server_read_gnutls(Ecore_Con_Server *svr,
                                  unsigned char *buf,
                                  int size)
{
   int num;

   if (svr->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        DBG("Continuing gnutls handshake");
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
        return -1;
     }

   num = gnutls_record_recv(svr->session, buf, size);
   if (num > 0)
     return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;

        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
     }
   else if ((!gnutls_error_is_fatal(num)) && (num != GNUTLS_E_SUCCESS))
     return 0;

   return -1;
}

static int
_ecore_con_ssl_server_write_gnutls(Ecore_Con_Server *svr,
                                   const unsigned char *buf,
                                   int size)
{
   int num;

   if (svr->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        DBG("Continuing gnutls handshake");
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
        return -1;
     }

   num = gnutls_record_send(svr->session, buf, size);
   if (num > 0)
     return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;
/* this is only partly functional I think? */
        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
     }
   else if (!gnutls_error_is_fatal(num))
     return 0;

   return -1;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_gnutls(Ecore_Con_Client *cl)
{
   const gnutls_datum_t *cert_list;
   unsigned int iter, cert_list_size;
   const char *priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT";
   int ret = 0;

   switch (cl->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;

      case ECORE_CON_SSL_STATE_INIT:
        if (cl->host_server->type & ECORE_CON_USE_SSL2) /* not supported because of security issues */
          return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

        switch (cl->host_server->type & ECORE_CON_SSL)
          {
           case ECORE_CON_USE_SSL3:
           case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
             priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-TLS1.0:!VERS-TLS1.1:!VERS-TLS1.2";
             break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
             priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0";
             break;

           case ECORE_CON_USE_MIXED:
           case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
             break;

           default:
             return ECORE_CON_SSL_ERROR_NONE;
          }

        _client_connected++;

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&cl->session, GNUTLS_SERVER));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_key_generate(&cl->session_ticket));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_server(cl->session, &cl->session_ticket));
        INF("Applying priority string: %s", priority);
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_priority_set_direct(cl->session, priority, NULL));
        gnutls_handshake_set_private_extensions(cl->session, 1);
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_CERTIFICATE, cl->host_server->cert));
        //  SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_PSK, cl->host_server->pskcred_s));
        if (!cl->host_server->use_cert)
          SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_ANON, cl->host_server->anoncred_s));

        gnutls_certificate_server_set_request(cl->session, GNUTLS_CERT_REQUEST);

        gnutls_dh_set_prime_bits(cl->session, 2048);
        gnutls_transport_set_ptr(cl->session, (gnutls_transport_ptr_t)((intptr_t)cl->fd));
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;

      case ECORE_CON_SSL_STATE_HANDSHAKING:
        if (!cl->session)
          {
             DBG("Client was previously lost, going to error condition");
             goto error;
          }
        DBG("calling gnutls_handshake()");
        ret = gnutls_handshake(cl->session);
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));

        if (!ret)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (gnutls_record_get_direction(cl->session))
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
             else
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
             return ECORE_CON_SSL_ERROR_NONE;
          }

      default:
        break;
     }

   if (!cl->host_server->verify)
     /* not verifying certificates, so we're done! */
     return ECORE_CON_SSL_ERROR_NONE;
   /* use CRL/CA lists to verify */
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_verify_peers2(cl->session, &iter));
   if (iter & GNUTLS_CERT_INVALID)
     ERR("The certificate is not trusted.");
   else if (iter & GNUTLS_CERT_SIGNER_NOT_FOUND)
     ERR("The certificate hasn't got a known issuer.");
   else if (iter & GNUTLS_CERT_REVOKED)
     ERR("The certificate has been revoked.");
   else if (iter & GNUTLS_CERT_EXPIRED)
     ERR("The certificate has expired");
   else if (iter & GNUTLS_CERT_NOT_ACTIVATED)
     ERR("The certificate is not yet activated");

   if (iter)
     goto error;
   if (gnutls_certificate_type_get(cl->session) != GNUTLS_CRT_X509)
     {
        ERR("Warning: PGP certificates are not yet supported!");
        goto error;
     }

   SSL_ERROR_CHECK_GOTO_ERROR(!(cert_list = gnutls_certificate_get_peers(cl->session, &cert_list_size)));
   SSL_ERROR_CHECK_GOTO_ERROR(!cert_list_size);

   _gnutls_print_session(cert_list, cert_list_size);
/*
   gnutls_x509_crt_t cert = NULL;
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_init(&cert));
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER));

   SSL_ERROR_CHECK_GOTO_ERROR(!gnutls_x509_crt_check_hostname(cert, cl->host_server->name));
   gnutls_x509_crt_deinit(cert);
 */
   DBG("SSL certificate verification succeeded!");
   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(cl, ECORE_CON_EVENT_CLIENT_ERROR, ret);
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(cl->session)));
   if (cl->session && (cl->ssl_state != ECORE_CON_SSL_STATE_DONE))
     {
        ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(cl->session)));
        ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(cl->session)));
     }
/*
   if (cert)
     gnutls_x509_crt_deinit(cert);
 */
   _ecore_con_ssl_client_shutdown_gnutls(cl);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_gnutls(Ecore_Con_Client *cl)
{
   if (cl->session)
     {
        gnutls_bye(cl->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(cl->session);
        gnutls_free(cl->session_ticket.data);
        cl->session_ticket.data = NULL;
     }

   cl->session = NULL;

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_gnutls(Ecore_Con_Client *cl,
                                  unsigned char *buf,
                                  int size)
{
   int num;

   if (cl->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
        return -1;
     }

   num = gnutls_record_recv(cl->session, buf, size);
   if (num > 0)
     return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
        WRN("Rehandshake request ignored");
        return 0;
     }
   else if ((!gnutls_error_is_fatal(num)) && (num != GNUTLS_E_SUCCESS))
     return 0;

   return -1;
}

static int
_ecore_con_ssl_client_write_gnutls(Ecore_Con_Client *cl,
                                   const unsigned char *buf,
                                   int size)
{
   int num;

   if (cl->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
        return -1;
     }

   num = gnutls_record_send(cl->session, buf, size);
   if (num > 0)
     return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
     }
   else if (!gnutls_error_is_fatal(num))
     return 0;

   return -1;
}

#elif HAVE_OPENSSL && !HAVE_GNUTLS

/*
 * OpenSSL
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_openssl(void)
{
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_openssl(void)
{
   ERR_free_strings();
   EVP_cleanup();
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_openssl(Ecore_Con_Server *obj,
                                      int ssl_type)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   long options;
   int dh = 0;

   if (ssl_type & ECORE_CON_USE_SSL2)
     return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

   switch (ssl_type)
     {
      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
        if (!svr->created)
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv3_client_method())));
        else
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv3_server_method())));
        break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
        if (!svr->created)
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(TLSv1_client_method())));
        else
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(TLSv1_server_method())));
        break;

      case ECORE_CON_USE_MIXED:
      case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
        if (!svr->created)
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv23_client_method())));
        else
          SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv23_server_method())));
        options = SSL_CTX_get_options(svr->ssl_ctx);
        SSL_CTX_set_options(svr->ssl_ctx, options | SSL_OP_NO_SSLv2 | SSL_OP_SINGLE_DH_USE);
        break;

      default:
        svr->ssl_prepared = EINA_TRUE;
        return ECORE_CON_SSL_ERROR_NONE;
     }

   if ((!svr->use_cert) && svr->created)
     {
        DH *dh_params;
        INF("Generating DH params");
        SSL_ERROR_CHECK_GOTO_ERROR(!(dh_params = DH_new()));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_parameters_ex(dh_params, 1024, DH_GENERATOR_5, NULL));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_check(dh_params, &dh));
        SSL_ERROR_CHECK_GOTO_ERROR((dh & DH_CHECK_P_NOT_PRIME) || (dh & DH_CHECK_P_NOT_SAFE_PRIME));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_key(dh_params));
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_tmp_dh(svr->ssl_ctx, dh_params));
        DH_free(dh_params);
        INF("DH params successfully generated and applied!");
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:@STRENGTH"));
     }
   else if (!svr->use_cert)
     SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:!ECDH:RSA:AES:!PSK:@STRENGTH"));

   svr->ssl_prepared = EINA_TRUE;
   return ECORE_CON_SSL_ERROR_NONE;

error:
   if (dh)
     {
        if (dh & DH_CHECK_P_NOT_PRIME)
          ERR("openssl error: dh_params could not generate a prime!");
        else
          ERR("openssl error: dh_params could not generate a safe prime!");
     }
   else
     _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   _ecore_con_ssl_server_shutdown_openssl(obj);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_openssl(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int ret = -1;

   switch (svr->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;

      case ECORE_CON_SSL_STATE_INIT:
        SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl = SSL_new(svr->ssl_ctx)));

        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(svr->ssl, svr->fd));
        SSL_set_connect_state(svr->ssl);
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;

      case ECORE_CON_SSL_STATE_HANDSHAKING:
        if (!svr->ssl)
          {
             DBG("Server was previously lost, going to error condition");
             goto error;
          }
        ret = SSL_do_handshake(svr->ssl);
        svr->ssl_err = SSL_get_error(svr->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((svr->ssl_err == SSL_ERROR_SYSCALL) || (svr->ssl_err == SSL_ERROR_SSL));

        if (ret == 1)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (svr->ssl_err == SSL_ERROR_WANT_READ)
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
             else if (svr->ssl_err == SSL_ERROR_WANT_WRITE)
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
             return ECORE_CON_SSL_ERROR_NONE;
          }

      default:
        break;
     }

   _openssl_print_session(svr->ssl);
   if ((!svr->verify) && (!svr->verify_basic))
     /* not verifying certificates, so we're done! */
     return ECORE_CON_SSL_ERROR_NONE;

   {
      X509 *cert;
      SSL_set_verify(svr->ssl, SSL_VERIFY_PEER, NULL);
      /* use CRL/CA lists to verify */
      cert = SSL_get_peer_certificate(svr->ssl);
      if (cert)
        {
           char *c;
           int clen;
           int name = 0;

           if (svr->verify)
             {
                int err;

                err = SSL_get_verify_result(svr->ssl);
                _openssl_print_verify_error(err);
                SSL_ERROR_CHECK_GOTO_ERROR(err);
             }
           clen = X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_subject_alt_name, NULL, 0);
           if (clen > 0)
             name = NID_subject_alt_name;
           else
             clen = X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName, NULL, 0);
           SSL_ERROR_CHECK_GOTO_ERROR(clen < 1);
           if (!name) name = NID_commonName;
           c = alloca(++clen);
           X509_NAME_get_text_by_NID(X509_get_subject_name(cert), name, c, clen);
           INF("CERT NAME: %s\n", c);
           SSL_ERROR_CHECK_GOTO_ERROR(!_openssl_name_verify(c, svr->verify_name ? : svr->name));
        }
   }

   DBG("SSL certificate verification succeeded!");

   return ECORE_CON_SSL_ERROR_NONE;

error:
   _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   _ecore_con_ssl_server_shutdown_openssl(obj);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_openssl(Ecore_Con_Server *obj,
                                         const char *ca_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   struct stat st;

   if (stat(ca_file, &st)) return EINA_FALSE;
   if (S_ISDIR(st.st_mode))
     SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_load_verify_locations(svr->ssl_ctx, NULL, ca_file));
   else
     SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_load_verify_locations(svr->ssl_ctx, ca_file, NULL));
   return EINA_TRUE;

error:
   _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_openssl(Ecore_Con_Server *obj,
                                      const char *crl_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   X509_STORE *st;
   X509_LOOKUP *lu;
   static Eina_Bool flag = EINA_FALSE;

   SSL_ERROR_CHECK_GOTO_ERROR(!(st = SSL_CTX_get_cert_store(svr->ssl_ctx)));
   SSL_ERROR_CHECK_GOTO_ERROR(!(lu = X509_STORE_add_lookup(st, X509_LOOKUP_file())));
   SSL_ERROR_CHECK_GOTO_ERROR(X509_load_crl_file(lu, crl_file, X509_FILETYPE_PEM) < 1);
   if (!flag)
     {
        X509_STORE_set_flags(st, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
        flag = EINA_TRUE;
     }

   return EINA_TRUE;

error:
   _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_privkey_add_openssl(Ecore_Con_Server *obj,
                                          const char *key_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   FILE *fp = NULL;
   EVP_PKEY *privkey = NULL;

   if (!(fp = fopen(key_file, "r")))
     goto error;

   SSL_ERROR_CHECK_GOTO_ERROR(!(privkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL)));

   fclose(fp);
   fp = NULL;
   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_PrivateKey(svr->ssl_ctx, privkey) < 1);
   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_check_private_key(svr->ssl_ctx) < 1);

   return EINA_TRUE;

error:
   if (fp)
     fclose(fp);
   _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_openssl(Ecore_Con_Server *obj,
                                       const char *cert_file)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   FILE *fp = NULL;
   X509 *cert = NULL;

   if (!(fp = fopen(cert_file, "r")))
     goto error;

   SSL_ERROR_CHECK_GOTO_ERROR(!(cert = PEM_read_X509(fp, NULL, NULL, NULL)));

   fclose(fp);
   fp = NULL;
   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(svr->ssl_ctx, cert) < 1);

   return EINA_TRUE;

error:
   if (fp)
     fclose(fp);
   _openssl_print_errors(svr, ECORE_CON_EVENT_SERVER_ERROR);
   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_openssl(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (svr->ssl)
     {
        if (!SSL_shutdown(svr->ssl))
          SSL_shutdown(svr->ssl);

        SSL_free(svr->ssl);
     }

   if (svr->ssl_ctx)
     SSL_CTX_free(svr->ssl_ctx);

   svr->ssl = NULL;
   svr->ssl_ctx = NULL;
   svr->ssl_err = SSL_ERROR_NONE;

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_server_read_openssl(Ecore_Con_Server *obj,
                                   unsigned char *buf,
                                   int size)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int num;

   if (!svr->ssl) return -1;
   num = SSL_read(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_READ)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_WRITE)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
     }

   if ((svr->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (svr->ssl_err == SSL_ERROR_SYSCALL) ||
       (svr->ssl_err == SSL_ERROR_SSL))
     return -1;

   if (num < 0)
     return 0;

   return num;
}

static int
_ecore_con_ssl_server_write_openssl(Ecore_Con_Server *obj,
                                    const unsigned char *buf,
                                    int size)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int num;

   num = SSL_write(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_READ)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_WRITE)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
     }

   if ((svr->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (svr->ssl_err == SSL_ERROR_SYSCALL) ||
       (svr->ssl_err == SSL_ERROR_SSL))
     return -1;

   if (num < 0)
     return 0;

   return num;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_openssl(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
   int ret = -1;
   switch (cl->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;

      case ECORE_CON_SSL_STATE_INIT:
        SSL_ERROR_CHECK_GOTO_ERROR(!(cl->ssl = SSL_new(host_server->ssl_ctx)));

        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(cl->ssl, cl->fd));
        SSL_set_accept_state(cl->ssl);
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;

      case ECORE_CON_SSL_STATE_HANDSHAKING:
        if (!cl->ssl)
          {
             DBG("Client was previously lost, going to error condition");
             goto error;
          }
        ret = SSL_do_handshake(cl->ssl);
        cl->ssl_err = SSL_get_error(cl->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((cl->ssl_err == SSL_ERROR_SYSCALL) || (cl->ssl_err == SSL_ERROR_SSL));
        if (ret == 1)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (cl->ssl_err == SSL_ERROR_WANT_READ)
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
             else if (cl->ssl_err == SSL_ERROR_WANT_WRITE)
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
             return ECORE_CON_SSL_ERROR_NONE;
          }

      default:
        break;
     }

   _openssl_print_session(cl->ssl);
   if (!host_server->verify)
     /* not verifying certificates, so we're done! */
     return ECORE_CON_SSL_ERROR_NONE;
   SSL_set_verify(cl->ssl, SSL_VERIFY_PEER, NULL);
   /* use CRL/CA lists to verify */
   if (SSL_get_peer_certificate(cl->ssl))
     {
        int err;

        err = SSL_get_verify_result(cl->ssl);
        _openssl_print_verify_error(err);
        SSL_ERROR_CHECK_GOTO_ERROR(err);
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   _openssl_print_errors(cl, ECORE_CON_EVENT_CLIENT_ERROR);
   _ecore_con_ssl_client_shutdown_openssl(obj);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_openssl(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (cl->ssl)
     {
        if (!SSL_shutdown(cl->ssl))
          SSL_shutdown(cl->ssl);

        SSL_free(cl->ssl);
     }

   cl->ssl = NULL;
   cl->ssl_err = SSL_ERROR_NONE;

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_openssl(Ecore_Con_Client *obj,
                                   unsigned char *buf,
                                   int size)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   int num;

   if (!cl->ssl) return -1;
   num = SSL_read(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err == SSL_ERROR_WANT_READ)
          ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err == SSL_ERROR_WANT_WRITE)
          ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
     }

   if ((cl->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (cl->ssl_err == SSL_ERROR_SYSCALL) ||
       (cl->ssl_err == SSL_ERROR_SSL))
     return -1;

   if (num < 0)
     return 0;

   return num;
}

static int
_ecore_con_ssl_client_write_openssl(Ecore_Con_Client *obj,
                                    const unsigned char *buf,
                                    int size)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   int num;

   num = SSL_write(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err == SSL_ERROR_WANT_READ)
          ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err == SSL_ERROR_WANT_WRITE)
          ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
     }

   if ((cl->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (cl->ssl_err == SSL_ERROR_SYSCALL) ||
       (cl->ssl_err == SSL_ERROR_SSL))
     return -1;

   if (num < 0)
     return 0;

   return num;
}

#else

/*
 * No Ssl
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_none(void)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_none(void)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_none(Ecore_Con_Server *svr EINA_UNUSED,
                                   int ssl_type EINA_UNUSED)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_none(Ecore_Con_Server *svr EINA_UNUSED)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_none(Ecore_Con_Server *svr EINA_UNUSED,
                                      const char *ca_file EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_none(Ecore_Con_Server *svr EINA_UNUSED,
                                    const char *cert_file EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_privkey_add_none(Ecore_Con_Server *svr EINA_UNUSED,
                                       const char *key_file EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_none(Ecore_Con_Server *svr EINA_UNUSED,
                                   const char *crl_file EINA_UNUSED)
{
   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_none(Ecore_Con_Server *svr EINA_UNUSED)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_server_read_none(Ecore_Con_Server *svr EINA_UNUSED,
                                unsigned char *buf EINA_UNUSED,
                                int size EINA_UNUSED)
{
   return -1;
}

static int
_ecore_con_ssl_server_write_none(Ecore_Con_Server *svr EINA_UNUSED,
                                 const unsigned char *buf EINA_UNUSED,
                                 int size EINA_UNUSED)
{
   return -1;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_none(Ecore_Con_Client *cl EINA_UNUSED)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_none(Ecore_Con_Client *cl EINA_UNUSED)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_client_read_none(Ecore_Con_Client *cl EINA_UNUSED,
                                unsigned char *buf EINA_UNUSED,
                                int size EINA_UNUSED)
{
   return -1;
}

static int
_ecore_con_ssl_client_write_none(Ecore_Con_Client *cl EINA_UNUSED,
                                 const unsigned char *buf EINA_UNUSED,
                                 int size EINA_UNUSED)
{
   return -1;
}

#endif
