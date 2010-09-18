#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#elif USE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include "Ecore.h"
#include "ecore_con_private.h"
#include <sys/mman.h>
#include <errno.h>

static int _init_con_ssl_init_count = 0;

#if USE_GNUTLS
# ifdef EFL_HAVE_PTHREAD
#include <pthread.h>
GCRY_THREAD_OPTION_PTHREAD_IMPL;
# endif

static int _client_connected = 0;
# define SSL_SUFFIX(ssl_func) ssl_func ## _gnutls
# define _ECORE_CON_SSL_AVAILABLE 1

#elif USE_OPENSSL

# define SSL_SUFFIX(ssl_func) ssl_func ## _openssl
# define _ECORE_CON_SSL_AVAILABLE 2

#else
# define SSL_SUFFIX(ssl_func) ssl_func ## _none
# define _ECORE_CON_SSL_AVAILABLE 0

#endif

#if USE_GNUTLS
typedef struct _cert_thingy
{
   gnutls_certificate_credentials_t cert;
   int count;
} gnutls;
static gnutls *client_cert = NULL;
static gnutls *server_cert = NULL;
#elif USE_OPENSSL
typedef struct _cert_thingy
{
   X509 *cert;
   int count;
} openssl;
typedef struct _key_thingy
{
   EVP_PKEY *key;
   int count;
} openssl_pkey;
static openssl_pkey *private_key = NULL;
static openssl *client_cert = NULL;
static openssl *server_cert = NULL;
#endif

#define SSL_ERROR_CHECK_GOTO_ERROR(X) \
do \
  { \
     if ((X)) \
       { \
          ERR("Error at %s:%s:%d!", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
          goto error; \
       } \
  } \
while (0)


static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_init) (void);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_shutdown) (void);

static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_client_cert_add) (const char *
                                                             cert_file,
                                                             const char *
                                                             crl_file,
                                                             const char *
                                                             key_file);
static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_server_cert_add) (const char *cert);

static void      SSL_SUFFIX(_ecore_con_ssl_server_prepare) (Ecore_Con_Server *
                                                            svr);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_server_init) (Ecore_Con_Server * svr);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (Ecore_Con_Server *
                                            svr);
static Ecore_Con_State
                 SSL_SUFFIX(_ecore_con_ssl_server_try) (Ecore_Con_Server * svr);
static int
                 SSL_SUFFIX(_ecore_con_ssl_server_read) (Ecore_Con_Server * svr,
                                        unsigned char *buf, int size);
static int
                 SSL_SUFFIX(_ecore_con_ssl_server_write) (Ecore_Con_Server *
                                         svr,
                                         unsigned char *buf, int size);

static void
                 SSL_SUFFIX(_ecore_con_ssl_client_prepare) (Ecore_Con_Client *
                                           cl);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_client_init) (Ecore_Con_Client * cl);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (Ecore_Con_Client *
                                            cl);
static int
                 SSL_SUFFIX(_ecore_con_ssl_client_read) (Ecore_Con_Client * cl,
                                        unsigned char *buf, int size);
static int
                 SSL_SUFFIX(_ecore_con_ssl_client_write) (Ecore_Con_Client * cl,
                                         unsigned char *buf, int size);

/*
 * General SSL API
 */

Ecore_Con_Ssl_Error
ecore_con_ssl_init(void)
{
   if (!_init_con_ssl_init_count++)
        SSL_SUFFIX(_ecore_con_ssl_init) ();

   return _init_con_ssl_init_count;
}

Ecore_Con_Ssl_Error
ecore_con_ssl_shutdown(void)
{
   if (!--_init_con_ssl_init_count)
     {
#if USE_OPENSSL || USE_GNUTLS
        if (client_cert)
           client_cert->count = 0;

        if (server_cert)
           server_cert->count = 0;
#endif
#if USE_OPENSSL && !USE_GNUTLS
        if (private_key)
           private_key->count = 0;

#endif
        SSL_SUFFIX(_ecore_con_ssl_shutdown) ();
     }

   return _init_con_ssl_init_count;
}

/**
 * Returns if SSL support is available
 * @return  1 if SSL is available, 0 if it is not.
 * @ingroup Ecore_Con_Client_Group
 */
int
ecore_con_ssl_available_get(void)
{
   return _ECORE_CON_SSL_AVAILABLE;
}


void
ecore_con_ssl_server_prepare(Ecore_Con_Server *svr)
{
        SSL_SUFFIX(_ecore_con_ssl_server_prepare) (svr);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_init(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_init) (svr);
}

Eina_Bool
ecore_con_ssl_server_cert_add(const char *cert)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_cert_add) (cert);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_shutdown(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (svr);
}

Ecore_Con_State
ecore_con_ssl_server_try(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_try) (svr);
}

int
ecore_con_ssl_server_read(Ecore_Con_Server *svr, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_read) (svr, buf, size);
}

int
ecore_con_ssl_server_write(Ecore_Con_Server *svr, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_write) (svr, buf, size);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_init(Ecore_Con_Client *cl)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_init) (cl);
}

Eina_Bool
ecore_con_ssl_client_cert_add(const char *cert_file,
                              const char *crl_file,
                              const char *key_file)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_cert_add) (cert_file, crl_file,
                                                      key_file);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_shutdown(Ecore_Con_Client *cl)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (cl);
}

int
ecore_con_ssl_client_read(Ecore_Con_Client *cl, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_read) (cl, buf, size);
}

int
ecore_con_ssl_client_write(Ecore_Con_Client *cl, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_write) (cl, buf, size);
}

#if USE_GNUTLS

/*
 * GnuTLS
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_gnutls(void)
{
#ifdef EFL_HAVE_PTHREAD
   if (gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
      WRN(
         "YOU ARE USING PTHREADS, BUT I CANNOT INITIALIZE THREADSAFE GCRYPT OPERATIONS!");

#endif
   if (gnutls_global_init())
      return ECORE_CON_SSL_ERROR_INIT_FAILED;

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_gnutls(void)
{
   gnutls_global_deinit();

   return ECORE_CON_SSL_ERROR_NONE;
}

static void
_ecore_con_ssl_server_prepare_gnutls(Ecore_Con_Server *svr)
{
   svr->session = NULL;
   svr->anoncred_c = NULL;
   return;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_gnutls(Ecore_Con_Server *svr)
{
   const int *proto = NULL;
   const int compress[] = { GNUTLS_COMP_DEFLATE, GNUTLS_COMP_NULL, 0 };
   int ret = 0;
   const int ssl3_proto[] = { GNUTLS_SSL3, 0 };
   const int tls_proto[] = {
      GNUTLS_TLS1_0,
      GNUTLS_TLS1_1,
#ifdef USE_GNUTLS2
      GNUTLS_TLS1_2,
#endif
      0
   };

   switch (svr->type & ECORE_CON_SSL)
     {
      case ECORE_CON_USE_SSL2: /* not supported because of security issues */
      case ECORE_CON_USE_SSL2 | ECORE_CON_LOAD_CERT: /* not supported because of security issues */
         return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
         proto = ssl3_proto;
         break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
         proto = tls_proto;
         break;

      default:
         return ECORE_CON_SSL_ERROR_NONE;
     }

   if ((server_cert) && (server_cert->cert) &&
       ((svr->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT)
     {
        svr->cert = server_cert->cert;
        server_cert->count++;
     }

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&(svr->session), GNUTLS_CLIENT));
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_set_default_priority(svr->session));

   if (svr->cert)
      SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_CERTIFICATE,
                             svr->cert));
   else
     {
        const int kx[] = { GNUTLS_KX_ANON_DH, 0 };
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_kx_set_priority(svr->session, kx));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_client_credentials(&svr->anoncred_c));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_ANON, svr->anoncred_c));
     }

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_protocol_set_priority(svr->session, proto));
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_compression_set_priority(svr->session, compress));
   gnutls_dh_set_prime_bits(svr->session, 2048);

   gnutls_transport_set_ptr(svr->session, (gnutls_transport_ptr_t)svr->fd);

   while ((ret = gnutls_handshake(svr->session)) < 0)
     {
        SSL_ERROR_CHECK_GOTO_ERROR((ret != GNUTLS_E_AGAIN) &&
            (ret != GNUTLS_E_INTERRUPTED));
     }

   return ECORE_CON_SSL_ERROR_NONE;
   
error:
   ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_gnutls(const char *cert_file)
{
   gnutls_certificate_credentials_t cert = NULL;

   /* cert load */
   if (gnutls_certificate_set_x509_trust_file(cert, cert_file,
                                              GNUTLS_X509_FMT_PEM) < 0)
      goto on_error;

   if (!server_cert)
     {
        server_cert = malloc(sizeof(gnutls));
        if (!server_cert)
           return EINA_FALSE;
     }
   else if ((server_cert->cert) && ((--server_cert->count) < 1))
      gnutls_certificate_free_credentials(server_cert->cert);

   server_cert->cert = cert;
   server_cert->count = 1;

   return EINA_TRUE;

on_error:
   if (cert)
      gnutls_certificate_free_credentials(cert);

   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_gnutls(Ecore_Con_Server *svr)
{
   if (svr->session)
     {
        gnutls_bye(svr->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(svr->session);
     }

   if (((svr->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
       (server_cert) &&
       (server_cert->cert) && (--server_cert->count < 1))
     {
        gnutls_certificate_free_credentials(server_cert->cert);
        free(server_cert);
        server_cert = NULL;
     }
   else if (svr->anoncred_c)
      gnutls_anon_free_client_credentials(svr->anoncred_c);

   _ecore_con_ssl_server_prepare_gnutls(svr);

   return ECORE_CON_SSL_ERROR_NONE;
}

/* this is a stub function, the handshake is done in _init_gnutls */
static Ecore_Con_State
_ecore_con_ssl_server_try_gnutls(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_CONNECTED;
}

static int
_ecore_con_ssl_server_read_gnutls(Ecore_Con_Server *svr, unsigned char *buf,
                                  int size)
{
   int num;

   num = gnutls_record_recv(svr->session, buf, size);
   if (num > 0)
      return num;

   if ((num == GNUTLS_E_AGAIN) ||
       (num == GNUTLS_E_REHANDSHAKE) ||
       (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

static int
_ecore_con_ssl_server_write_gnutls(Ecore_Con_Server *svr, unsigned char *buf,
                                   int size)
{
   int num;

   num = gnutls_record_send(svr->session, buf, size);
   if (num > 0)
      return num;

   if ((num == GNUTLS_E_AGAIN) ||
       (num == GNUTLS_E_REHANDSHAKE) ||
       (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

static void
_ecore_con_ssl_client_prepare_gnutls(Ecore_Con_Client *cl)
{
   cl->session = NULL;
   if (!_client_connected)
     {
        cl->server->anoncred_s = NULL;
        cl->server->cert = NULL;
     }
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_gnutls(Ecore_Con_Client *cl)
{
   const int *proto = NULL;
   gnutls_dh_params_t dh_params;
   int ret;
   const int compress[] = { GNUTLS_COMP_DEFLATE, GNUTLS_COMP_NULL, 0 };
   const int ssl3_proto[] = { GNUTLS_SSL3, 0 };
   const int tls_proto[] = {
      GNUTLS_TLS1_0,
      GNUTLS_TLS1_1,
#ifdef USE_GNUTLS2
      GNUTLS_TLS1_2,
#endif
      0
   };

   switch (cl->server->type & ECORE_CON_SSL)
     {
      case ECORE_CON_USE_SSL2: /* not supported because of security issues */
      case ECORE_CON_USE_SSL2 | ECORE_CON_LOAD_CERT: /* not supported because of security issues */
         return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
         proto = ssl3_proto;
         break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
         proto = tls_proto;
         break;

      default:
         return ECORE_CON_SSL_ERROR_NONE;
     }

   _client_connected++;

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_init(&dh_params));

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_generate2(dh_params, 1024));

   if ((client_cert) && (client_cert->cert) &&
       ((cl->server->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT)
     {
        cl->server->cert = client_cert->cert;
        client_cert->count++;
        gnutls_certificate_set_dh_params(cl->server->cert, dh_params);
     }

   if ((!cl->server->anoncred_s) && (!cl->server->cert))
     {
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_server_credentials(&(cl->server->anoncred_s)));
        gnutls_anon_set_server_dh_params(cl->server->anoncred_s, dh_params);
     }

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&(cl->session), GNUTLS_SERVER));
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_set_default_priority(cl->session));
   if (cl->server->cert)
     {
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session,
                               GNUTLS_CRD_CERTIFICATE,
                               cl->server->cert));
        gnutls_certificate_server_set_request(cl->session, GNUTLS_CERT_REQUEST);
     }
   else
     {
        const int kx[] = { GNUTLS_KX_ANON_DH, 0 };
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_kx_set_priority(cl->session, kx));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_ANON,
                             cl->server->anoncred_s));
     }

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_protocol_set_priority(cl->session, proto));
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_compression_set_priority(cl->session, compress));

   gnutls_transport_set_ptr(cl->session, (gnutls_transport_ptr_t)cl->fd);

   while ((ret = gnutls_handshake(cl->session)) < 0)
     {
        SSL_ERROR_CHECK_GOTO_ERROR((ret != GNUTLS_E_AGAIN) &&
            (ret != GNUTLS_E_INTERRUPTED));
     }

   /* TODO: add cert verification support */
   return ECORE_CON_SSL_ERROR_NONE;

error:
   ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
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
     }

   if (cl->server->anoncred_s && !--_client_connected)
      gnutls_anon_free_server_credentials(cl->server->anoncred_s);

   if (((cl->server->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
       (client_cert) &&
       (client_cert->cert) && (--client_cert->count < 1))
     {
        gnutls_certificate_free_credentials(client_cert->cert);
        free(client_cert);
        client_cert = NULL;
     }

   _ecore_con_ssl_client_prepare_gnutls(cl);

   return ECORE_CON_SSL_ERROR_NONE;
}

static Eina_Bool
_ecore_con_ssl_client_cert_add_gnutls(const char *cert_file,
                                      const char *crl_file __UNUSED__,
                                      const char *key_file)
{
   gnutls_certificate_credentials_t cert = NULL;

   if (gnutls_certificate_allocate_credentials(&cert) < 0)
      return EINA_FALSE;

   /* cert load */
   if (gnutls_certificate_set_x509_trust_file(cert, cert_file,
                                              GNUTLS_X509_FMT_PEM) < 0)
      goto on_error;

   /* private key load */
   if (gnutls_certificate_set_x509_key_file(cert, cert_file, key_file,
                                            GNUTLS_X509_FMT_PEM) < 0)
      goto on_error;

#if 0
   //TODO: uncomment once we implement cert checking
   if (crl_file)
      /* CRL file load */
      if (gnutls_certificate_set_x509_crl_mem(cert, crl_file,
                                              GNUTLS_X509_FMT_PEM) < 0)
         goto on_error;

}
#endif
   if (!client_cert)
     {
        client_cert = malloc(sizeof(gnutls));
        if (!client_cert)
           return EINA_FALSE;
     }
   else if ((client_cert->cert) && ((--client_cert->count) < 1))
      gnutls_certificate_free_credentials(client_cert->cert);

   client_cert->cert = cert;
   client_cert->count = 1;

   return EINA_TRUE;

on_error:
   if (cert)
      gnutls_certificate_free_credentials(cert);

   return EINA_FALSE;
}

static int
_ecore_con_ssl_client_read_gnutls(Ecore_Con_Client *cl, unsigned char *buf,
                                  int size)
{
   int num;

   num = gnutls_record_recv(cl->session, buf, size);
   if (num > 0)
      return num;

   if ((num == GNUTLS_E_AGAIN) ||
       (num == GNUTLS_E_REHANDSHAKE) ||
       (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

static int
_ecore_con_ssl_client_write_gnutls(Ecore_Con_Client *cl, unsigned char *buf,
                                   int size)
{
   int num;

   num = gnutls_record_send(cl->session, buf, size);
   if (num > 0)
      return num;

   if ((num == GNUTLS_E_AGAIN) ||
       (num == GNUTLS_E_REHANDSHAKE) ||
       (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

#elif USE_OPENSSL && !USE_GNUTLS

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

static void
_ecore_con_ssl_server_prepare_openssl(Ecore_Con_Server *svr)
{
   svr->ssl = NULL;
   svr->ssl_ctx = NULL;
   svr->ssl_err = SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_openssl(Ecore_Con_Server *svr)
{
   switch (svr->type & ECORE_CON_SSL)
     {
      case ECORE_CON_USE_SSL2:
      case ECORE_CON_USE_SSL2 | ECORE_CON_LOAD_CERT:
         /* Unsafe version of SSL */
         if (!(svr->ssl_ctx =
                  SSL_CTX_new(SSLv2_client_method())))
            return
               ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
         if (!(svr->ssl_ctx =
                  SSL_CTX_new(SSLv3_client_method())))
            return
               ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
         if (!(svr->ssl_ctx =
                  SSL_CTX_new(TLSv1_client_method())))
            return
               ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      default:
         return ECORE_CON_SSL_ERROR_NONE;
     }
   if (!(svr->ssl = SSL_new(svr->ssl_ctx)))
     {
        SSL_CTX_free(svr->ssl_ctx);
        return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
     }

   if ((server_cert) && (server_cert->cert) &&
       ((svr->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT)
     {
        //FIXME: just log and go on without cert if loading fails?
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(svr->ssl_ctx, server_cert->cert) < 1);

        server_cert->count++;
     }

   SSL_set_connect_state(svr->ssl);
   SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(svr->ssl, svr->fd));

   return ECORE_CON_SSL_ERROR_NONE;

error:
   if (svr->ssl)
     SSL_free(svr->ssl);
   if (svr->ssl_ctx)
     SSL_CTX_free(svr->ssl_ctx);

   ERR("openssl error: %s", ERR_reason_error_string(ERR_get_error()));
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_openssl(const char *cert_file)
{
   FILE *fp = NULL;
   X509 *cert = NULL;

   if (!(fp = fopen(cert_file, "r")))
      goto on_error;

   if (!(cert = PEM_read_X509(fp, NULL, NULL, NULL)))
      goto on_error;

   fclose(fp);

   if (!server_cert)
     {
        server_cert = malloc(sizeof(openssl));
        if (!server_cert)
           return EINA_FALSE;
     }
   else if ((server_cert->cert) && ((--server_cert->count) < 1))
      X509_free(server_cert->cert);

   server_cert->cert = cert;

   server_cert->count = 1;

   return EINA_TRUE;

on_error:
   if (fp)
      fclose(fp);

   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_openssl(Ecore_Con_Server *svr)
{
   if (svr->ssl)
     {
        if (!SSL_shutdown(svr->ssl))
           SSL_shutdown(svr->ssl);

        SSL_free(svr->ssl);
     }

   if (((svr->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
       (server_cert) && (server_cert->cert) &&
       (--server_cert->count < 1))
     {
        X509_free(server_cert->cert);
        free(server_cert);
        server_cert = NULL;
     }

   if (svr->ssl_ctx)
      SSL_CTX_free(svr->ssl_ctx);

   _ecore_con_ssl_server_prepare_openssl(svr);

   return ECORE_CON_SSL_ERROR_NONE;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_openssl(Ecore_Con_Server *svr)
{
   int res, flag = 0;

   if ((res = SSL_connect(svr->ssl)) == 1)
      return ECORE_CON_CONNECTED;

   svr->ssl_err = SSL_get_error(svr->ssl, res);

   switch (svr->ssl_err)
     {
      case SSL_ERROR_NONE:
         return ECORE_CON_CONNECTED;

      case SSL_ERROR_WANT_READ:
         flag = ECORE_FD_READ;
         break;

      case SSL_ERROR_WANT_WRITE:
         flag = ECORE_FD_WRITE;
         break;

      default:
         return ECORE_CON_DISCONNECTED;
     }

   if (svr->fd_handler && flag)
           ecore_main_fd_handler_active_set(svr->fd_handler,
                                       flag);

   return ECORE_CON_INPROGRESS;
}

static int
_ecore_con_ssl_server_read_openssl(Ecore_Con_Server *svr, unsigned char *buf,
                                   int size)
{
   int num;

   num = SSL_read(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(svr->fd_handler,
                                            ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              svr->fd_handler,
              ECORE_FD_WRITE);
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
_ecore_con_ssl_server_write_openssl(Ecore_Con_Server *svr, unsigned char *buf,
                                    int size)
{
   int num;

   num = SSL_write(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(svr->fd_handler,
                                            ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              svr->fd_handler,
              ECORE_FD_WRITE);
     }

   if ((svr->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (svr->ssl_err == SSL_ERROR_SYSCALL) ||
       (svr->ssl_err == SSL_ERROR_SSL))
      return -1;

   if (num < 0)
      return 0;

   return num;
}

static void
_ecore_con_ssl_client_prepare_openssl(Ecore_Con_Client *cl)
{
   cl->ssl = NULL;
   cl->ssl_ctx = NULL;
   cl->ssl_err = SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_openssl(Ecore_Con_Client *cl)
{
   switch (cl->server->type & ECORE_CON_SSL)
     {
      case ECORE_CON_USE_SSL2:
      case ECORE_CON_USE_SSL2 | ECORE_CON_LOAD_CERT:
         /* Unsafe version of SSL */
         if (!(cl->ssl_ctx = SSL_CTX_new(SSLv2_client_method())))
            return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
         if (!(cl->ssl_ctx = SSL_CTX_new(SSLv3_client_method())))
            return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
         if (!(cl->ssl_ctx = SSL_CTX_new(TLSv1_client_method())))
            return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;

         break;

      default:
         return ECORE_CON_SSL_ERROR_NONE;
     }
   SSL_ERROR_CHECK_GOTO_ERROR(!(cl->ssl = SSL_new(cl->ssl_ctx)));

   if ((client_cert) && (client_cert->cert) && (private_key->key) &&
       ((cl->server->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT)
     {
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(cl->ssl_ctx, client_cert->cert) < 1);
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_PrivateKey(cl->ssl_ctx, private_key->key) < 1);
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_check_private_key(cl->ssl_ctx) < 1);

        client_cert->count++;
        private_key->count++;
     }

   SSL_set_accept_state(cl->ssl);
   SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(cl->ssl, cl->fd));
   
   return ECORE_CON_SSL_ERROR_NONE;

error:
   if (cl->ssl)
     SSL_free(cl->ssl);
   if (cl->ssl_ctx)
     SSL_CTX_free(cl->ssl_ctx);

   ERR("openssl error: %s", ERR_reason_error_string(ERR_get_error()));
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}


static Eina_Bool
_ecore_con_ssl_client_cert_add_openssl(const char *cert_file,
                                       const char *crl_file __UNUSED__,
                                       const char *key_file)
{
   FILE *fp = NULL;
   EVP_PKEY *privkey = NULL;
   X509 *cert = NULL;

   if (!(fp = fopen(cert_file, "r")))
      goto on_error;

   if (!(cert = PEM_read_X509(fp, NULL, NULL, NULL)))
      goto on_error;

   if (key_file)
     {
        fclose(fp);
        if (!(fp = fopen(key_file, "r")))
           goto on_error;
     }

   if (!(privkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL)))
      goto on_error;

        fclose(fp);

   if (!client_cert)
     {
        client_cert = malloc(sizeof(openssl));
           if (!client_cert)
           return EINA_FALSE;
     }
   else if ((client_cert->cert) && (--client_cert->count < 1))
      X509_free(client_cert->cert);

   if (!private_key)
     {
        private_key = malloc(sizeof(openssl_pkey));
           if (!private_key) return EINA_FALSE;
     }
   else if ((private_key->key) && ((--private_key->count) < 1))
      EVP_PKEY_free(private_key->key);

   private_key->key = privkey;
   client_cert->cert = cert;

   private_key->count = client_cert->count = 1;

   return EINA_TRUE;

on_error:
   if (fp)
      fclose(fp);

   if (cert)
      X509_free(cert);

   if (privkey)
      EVP_PKEY_free(privkey);

   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_openssl(Ecore_Con_Client *cl)
{
   if (cl->ssl)
     {
        if (!SSL_shutdown(cl->ssl))
           SSL_shutdown(cl->ssl);

        SSL_free(cl->ssl);
     }

   if (cl->ssl_ctx)
     {
        SSL_CTX_free(cl->ssl_ctx);
        if (((cl->server->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
            (client_cert) && (client_cert->cert) && (--client_cert->count < 1))
          {
             X509_free(client_cert->cert);
             free(client_cert);
             client_cert = NULL;
          }

        if (((cl->server->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
            (private_key) && (private_key->key) && (--private_key->count < 1))
          {
             EVP_PKEY_free(private_key->key);
             free(private_key);
             private_key = NULL;
          }
     }

   _ecore_con_ssl_client_prepare_openssl(cl);

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_openssl(Ecore_Con_Client *cl, unsigned char *buf,
                                   int size)
{
   int num;

   num = SSL_read(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(cl->fd_handler,
                                            ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              cl->fd_handler,
              ECORE_FD_WRITE);
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
_ecore_con_ssl_client_write_openssl(Ecore_Con_Client *cl, unsigned char *buf,
                                    int size)
{
   int num;

   num = SSL_write(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(cl->fd_handler,
                                            ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              cl->fd_handler,
              ECORE_FD_WRITE);
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

static void
_ecore_con_ssl_server_prepare_none(Ecore_Con_Server *svr)
{
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_none(Ecore_Con_Server *svr)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_none(const char *cert_file)
{
   return EINA_TRUE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_none(Ecore_Con_Server *svr)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_none(Ecore_Con_Server *svr)
{
   return ECORE_CON_DISCONNECTED;
}

static int
_ecore_con_ssl_server_read_none(Ecore_Con_Server *svr, unsigned char *buf,
                                int size)
{
   return -1;
}

static int
_ecore_con_ssl_server_write_none(Ecore_Con_Server *svr, unsigned char *buf,
                                 int size)
{
   return -1;
}

static void
_ecore_con_ssl_client_prepare_none(Ecore_Con_Client *cl)
{
   return;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_none(Ecore_Con_Client *cl)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_client_cert_add_none(const char *cert_file,
                                    const char *crl_file,
                                    const char *key_file)
{
   return EINA_TRUE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_none(Ecore_Con_Client *cl)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_client_read_none(Ecore_Con_Client *cl, unsigned char *buf,
                                int size)
{
   return -1;
}

static int
_ecore_con_ssl_client_write_none(Ecore_Con_Client *cl, unsigned char *buf,
                                 int size)
{
   return -1;
}

#endif
