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
# include <openssl/dh.h>
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

static const char*
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

static Ecore_Con_Ssl_Error  SSL_SUFFIX(_ecore_con_ssl_server_prepare)(Ecore_Con_Server *svr, int ssl_type);
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
 * @return 1 if SSL is available and provided by gnutls, 2 if provided by openssl,
 * 0 if it is not available.
 * @ingroup Ecore_Con_Client_Group
 */
EAPI int
ecore_con_ssl_available_get(void)
{
   return _ECORE_CON_SSL_AVAILABLE;
}

#if 0
EAPI Eina_Bool
ecore_con_ssl_server_reinit()
{

}
#endif

Ecore_Con_Ssl_Error
ecore_con_ssl_server_prepare(Ecore_Con_Server *svr, int ssl_type)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_prepare) (svr, ssl_type);
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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_gnutls(Ecore_Con_Server *svr, int ssl_type)
{
   int ret;

   if (!ssl_type)
     return ECORE_CON_SSL_ERROR_NONE;

   if ((ssl_type & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT)
     {
        if ((server_cert) && (server_cert->cert) && (svr->created))
          { /* load server cert */
             svr->cert = server_cert->cert;
             server_cert->count++;
          }
        else if ((client_cert) && (client_cert->cert))
          { /* load client cert */
             svr->cert = client_cert->cert;
             client_cert->count++;
          }
     }
   else
     /* if LOAD_CERT is not specified, allocate here */
     SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_allocate_credentials(&svr->cert));

   if (svr->created)
     {
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_init(&svr->dh_params));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_generate2(svr->dh_params, 1024));

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_server_credentials(&svr->anoncred_s));
        /* TODO: implement PSK */
       // SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_server_credentials(&svr->pskcred_s));

        gnutls_anon_set_server_dh_params(svr->anoncred_s, svr->dh_params);
        gnutls_certificate_set_dh_params(svr->cert, svr->dh_params);
        //gnutls_psk_set_server_dh_params(svr->pskcred_s, svr->dh_params);
     }
   else
     {
        //SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_client_credentials(&svr->pskcred_c));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_client_credentials(&svr->anoncred_c));
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
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
   const int mixed_proto[] =
     {
#ifdef GNUTLS_VERSION_MAX
        GNUTLS_VERSION_MAX,
#endif
        GNUTLS_TLS1_1,
        GNUTLS_TLS1_0,
        GNUTLS_SSL3,
        0
     };
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
              proto = ssl3_proto;
              break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
              proto = tls_proto;
              break;

           case ECORE_CON_USE_MIXED:
           case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
              proto = mixed_proto;
              break;

           default:
              return ECORE_CON_SSL_ERROR_NONE;
          }

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&svr->session, GNUTLS_CLIENT));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_set_default_priority(svr->session));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_client(svr->session));

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_CERTIFICATE, svr->cert));
        //SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_PSK, svr->pskcred_c));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_ANON, svr->anoncred_c));
        if (!((svr->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT))
          {
             int kx[] = { GNUTLS_KX_DHE_RSA, GNUTLS_KX_RSA, GNUTLS_KX_ANON_DH, 0 };
             int cipher[] = { GNUTLS_CIPHER_AES_256_CBC, GNUTLS_CIPHER_AES_128_CBC, GNUTLS_CIPHER_3DES_CBC, 0 };
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_cipher_set_priority(svr->session, cipher));
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_kx_set_priority(svr->session, kx));
          }

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_protocol_set_priority(svr->session, proto));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_compression_set_priority(svr->session, compress));

        gnutls_dh_set_prime_bits(svr->session, 512);
        gnutls_transport_set_ptr(svr->session, (gnutls_transport_ptr_t)svr->fd);
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        ret = gnutls_handshake(svr->session);
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));
        if (!ret)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(svr->session)));
   ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(svr->session)));
   ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(svr->session)));
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

   if ((svr->type & ECORE_CON_SSL) && svr->created)
     {
        if ((server_cert) && (--server_cert->count < 1) && (svr->cert == server_cert->cert))
          { /* only ever free this if it is used and dead */
             gnutls_certificate_free_credentials(server_cert->cert);
             free(server_cert);
             server_cert = NULL;
          }
        else if (svr->cert && ((!server_cert) || (svr->cert != server_cert->cert)))
          /* otherwise just free the anon cert */
          gnutls_certificate_free_credentials(svr->cert);

        if (svr->dh_params)
          {
             gnutls_dh_params_deinit(svr->dh_params);
             svr->dh_params = NULL;
          }
        if (svr->anoncred_s)
          gnutls_anon_free_server_credentials(svr->anoncred_s);
        if (svr->pskcred_s)
          gnutls_psk_free_server_credentials(svr->pskcred_s);

        svr->anoncred_s = NULL;
        svr->pskcred_s = NULL;
     }
   else if (svr->type & ECORE_CON_SSL)
     {
        if ((client_cert) && (--client_cert->count < 1) && (svr->cert == client_cert->cert))
          { /* only ever free this if it is used and dead */
             gnutls_certificate_free_credentials(client_cert->cert);
             free(client_cert);
             client_cert = NULL;
          }
        else if (svr->cert && ((!client_cert) || (svr->cert != client_cert->cert)))
          /* otherwise just free the anon cert */
          gnutls_certificate_free_credentials(svr->cert);

        if (svr->anoncred_c)
          gnutls_anon_free_client_credentials(svr->anoncred_c);
        if (svr->pskcred_c)
          gnutls_psk_free_client_credentials(svr->pskcred_c);

        svr->anoncred_c = NULL;
        svr->pskcred_c = NULL;
     }

   svr->session = NULL;
   svr->cert = NULL;

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


static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_gnutls(Ecore_Con_Client *cl)
{
   const int *proto = NULL;
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
   const int mixed_proto[] =
     {
#ifdef GNUTLS_VERSION_MAX
        GNUTLS_VERSION_MAX,
#endif
        GNUTLS_TLS1_1,
        GNUTLS_TLS1_0,
        GNUTLS_SSL3,
        0 };
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
              proto = ssl3_proto;
              break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
              proto = tls_proto;
              break;

           case ECORE_CON_USE_MIXED:
           case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
              proto = mixed_proto;
              break;

           default:
              return ECORE_CON_SSL_ERROR_NONE;
          }

        _client_connected++;

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&cl->session, GNUTLS_SERVER));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_key_generate(&cl->session_ticket));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_server(cl->session, &cl->session_ticket));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_set_default_priority(cl->session));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_protocol_set_priority(cl->session, proto));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_compression_set_priority(cl->session, compress));

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_ANON, cl->host_server->anoncred_s));
        //SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_PSK, cl->host_server->pskcred_s));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(cl->session, GNUTLS_CRD_CERTIFICATE, cl->host_server->cert));
        if (!((cl->host_server->type & ECORE_CON_SSL) & ECORE_CON_LOAD_CERT))
          {
             int kx[] = { GNUTLS_KX_ANON_DH, GNUTLS_KX_RSA, GNUTLS_KX_DHE_RSA, 0 };
             SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_kx_set_priority(cl->session, kx));
          }

        gnutls_certificate_server_set_request(cl->session, GNUTLS_CERT_REQUEST);

        gnutls_dh_set_prime_bits(cl->session, 2048);
        gnutls_transport_set_ptr(cl->session, (gnutls_transport_ptr_t)cl->fd);
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        ret = gnutls_handshake(cl->session);
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));

        if (!ret)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   /* TODO: add cert verification support */
   return ECORE_CON_SSL_ERROR_NONE;

error:
   ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(cl->session)));
   ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(cl->session)));
   ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(cl->session)));
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

   if (((cl->host_server->type & ECORE_CON_TYPE) & ECORE_CON_LOAD_CERT) &&
       (client_cert) &&
       (client_cert->cert) && (--client_cert->count < 1))
     {
        free(client_cert);
        client_cert = NULL;
     }

   cl->session = NULL;

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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_openssl(Ecore_Con_Server *svr, int ssl_type)
{
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
         break;
     }

   if ((client_cert) && (client_cert->cert) && (private_key->key) && (svr->created) &&
       ((ssl_type & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT))
     { /* this is a server */
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(svr->ssl_ctx, client_cert->cert) < 1);
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_PrivateKey(svr->ssl_ctx, private_key->key) < 1);
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_check_private_key(svr->ssl_ctx) < 1);

        client_cert->count++;
        private_key->count++;
     }

   if ((server_cert) && (server_cert->cert) && (!svr->created) &&
       ((ssl_type & ECORE_CON_LOAD_CERT)))
     { /* this is a client */
        SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(svr->ssl_ctx, server_cert->cert) < 1);

        server_cert->count++;
     }
   else if (!(ssl_type & ECORE_CON_LOAD_CERT) && svr->created)
     {
        DH *dh_params;
        SSL_ERROR_CHECK_GOTO_ERROR(!(dh_params = DH_new()));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_parameters_ex(dh_params, 1024, DH_GENERATOR_5, NULL));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_check(dh_params, &dh));
        SSL_ERROR_CHECK_GOTO_ERROR((dh & DH_CHECK_P_NOT_PRIME) || (dh & DH_CHECK_P_NOT_SAFE_PRIME));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_key(dh_params));
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_tmp_dh(svr->ssl_ctx, dh_params));
        DH_free(dh_params);
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:@STRENGTH"));
     }
   else if (!(ssl_type & ECORE_CON_LOAD_CERT))
     SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:!ECDH:RSA:AES:!PSK:@STRENGTH"));

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
     do
       {
          unsigned long err;

          err = ERR_get_error();
          if (!err) break;
          ERR("openssl error: %s", ERR_reason_error_string(err));
       } while (1);
   _ecore_con_ssl_server_shutdown_openssl(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_openssl(Ecore_Con_Server *svr)
{
   int err, ret = -1;

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
        ret = SSL_do_handshake(svr->ssl);
        err = SSL_get_error(svr->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL));

        if (ret == 1)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   do
     {
        unsigned long sslerr;

        sslerr = ERR_get_error();
        if (!sslerr) break;
        ERR("openssl error: %s", ERR_reason_error_string(sslerr));
     } while (1);
   _ecore_con_ssl_server_shutdown_openssl(svr);
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

   svr->ssl = NULL;
   svr->ssl_ctx = NULL;
   svr->ssl_err = SSL_ERROR_NONE;

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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_openssl(Ecore_Con_Client *cl)
{
   int err, ret = -1;
   switch (cl->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;
      case ECORE_CON_SSL_STATE_INIT:
        SSL_ERROR_CHECK_GOTO_ERROR(!(cl->ssl = SSL_new(cl->host_server->ssl_ctx)));

        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(cl->ssl, cl->fd));
        SSL_set_accept_state(cl->ssl);
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        ret = SSL_do_handshake(cl->ssl);
        err = SSL_get_error(cl->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL));
        if (ret == 1)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   do
     {
        unsigned long sslerr;

        sslerr = ERR_get_error();
        if (!sslerr) break;
        ERR("openssl error: %s", ERR_reason_error_string(sslerr));
     } while (1);
   _ecore_con_ssl_client_shutdown_openssl(cl);
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

   cl->ssl = NULL;
   cl->ssl_err = SSL_ERROR_NONE;

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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_none(Ecore_Con_Server *svr __UNUSED__, int ssl_type __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_none(const char *cert_file __UNUSED__)
{
   return EINA_TRUE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_DISCONNECTED;
}

static int
_ecore_con_ssl_server_read_none(Ecore_Con_Server *svr __UNUSED__, unsigned char *buf __UNUSED__,
                                int size __UNUSED__)
{
   return -1;
}

static int
_ecore_con_ssl_server_write_none(Ecore_Con_Server *svr __UNUSED__, unsigned char *buf __UNUSED__,
                                 int size __UNUSED__)
{
   return -1;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_none(Ecore_Con_Client *cl __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_client_cert_add_none(const char *cert_file __UNUSED__,
                                    const char *crl_file __UNUSED__,
                                    const char *key_file __UNUSED__)
{
   return EINA_TRUE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_none(Ecore_Con_Client *cl __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_client_read_none(Ecore_Con_Client *cl __UNUSED__, unsigned char *buf __UNUSED__,
                                int size __UNUSED__)
{
   return -1;
}

static int
_ecore_con_ssl_client_write_none(Ecore_Con_Client *cl __UNUSED__, unsigned char *buf __UNUSED__,
                                 int size __UNUSED__)
{
   return -1;
}

#endif
