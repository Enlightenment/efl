/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#if USE_GNUTLS
# include <gnutls/gnutls.h>
#elif USE_OPENSSL
# include <openssl/ssl.h>
#endif

#include "ecore_con_private.h"

static int _init_count = 0;

#if USE_GNUTLS
static int _client_connected = 0;

# define SSL_SUFFIX(ssl_func) ssl_func##_gnutls
# define _ECORE_CON_SSL_AVAILABLE 1

#elif USE_OPENSSL
# define SSL_SUFFIX(ssl_func) ssl_func##_openssl
# define _ECORE_CON_SSL_AVAILABLE 2

#else
# define SSL_SUFFIX(ssl_func) ssl_func##_none
# define _ECORE_CON_SSL_AVAILABLE 0

#endif

static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_init)(void);
static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_shutdown)(void);

static void
SSL_SUFFIX(_ecore_con_ssl_server_prepare)(Ecore_Con_Server *svr);
static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_server_init)(Ecore_Con_Server *svr);
static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_server_shutdown)(Ecore_Con_Server *svr);
static Ecore_Con_State
SSL_SUFFIX(_ecore_con_ssl_server_try)(Ecore_Con_Server *svr);
static int
SSL_SUFFIX(_ecore_con_ssl_server_read)(Ecore_Con_Server *svr, char *buf, int size);
static int
SSL_SUFFIX(_ecore_con_ssl_server_write)(Ecore_Con_Server *svr, char *buf, int size);

static void
SSL_SUFFIX(_ecore_con_ssl_client_prepare)(Ecore_Con_Client *cl);
static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_client_init)(Ecore_Con_Client *cl);
static Ecore_Con_Ssl_Error
SSL_SUFFIX(_ecore_con_ssl_client_shutdown)(Ecore_Con_Client *cl);
static int
SSL_SUFFIX(_ecore_con_ssl_client_read)(Ecore_Con_Client *cl, char *buf, int size);
static int
SSL_SUFFIX(_ecore_con_ssl_client_write)(Ecore_Con_Client *cl, char *buf, int size);

/*
 * General SSL API
 */

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_init(void)
{
  if (!_init_count++)
    SSL_SUFFIX(_ecore_con_ssl_init)();

  return _init_count;
}

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_shutdown(void)
{
  if (!--_init_count)
    SSL_SUFFIX(_ecore_con_ssl_shutdown)();

  return _init_count;
}

/**
 * Returns if SSL support is available
 * @return  1 if SSL is available, 0 if it is not.
 * @ingroup Ecore_Con_Client_Group
 */
EAPI int
ecore_con_ssl_available_get(void)
{
  return _ECORE_CON_SSL_AVAILABLE;
}


EAPI void
ecore_con_ssl_server_prepare(Ecore_Con_Server *svr)
{
  SSL_SUFFIX(_ecore_con_ssl_server_prepare)(svr);
}

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_server_init(Ecore_Con_Server *svr)
{
  return SSL_SUFFIX(_ecore_con_ssl_server_init)(svr);
}

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_server_shutdown(Ecore_Con_Server *svr)
{
  return SSL_SUFFIX(_ecore_con_ssl_server_shutdown)(svr);
}

EAPI Ecore_Con_State
ecore_con_ssl_server_try(Ecore_Con_Server *svr)
{
  return SSL_SUFFIX(_ecore_con_ssl_server_try)(svr);
}

EAPI int
ecore_con_ssl_server_read(Ecore_Con_Server *svr, char *buf, int size)
{
  return SSL_SUFFIX(_ecore_con_ssl_server_read)(svr, buf, size);
}

EAPI int
ecore_con_ssl_server_write(Ecore_Con_Server *svr, char *buf, int size)
{
  return SSL_SUFFIX(_ecore_con_ssl_server_write)(svr, buf, size);
}

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_client_init(Ecore_Con_Client *cl)
{
  return SSL_SUFFIX(_ecore_con_ssl_client_init)(cl);
}

EAPI Ecore_Con_Ssl_Error
ecore_con_ssl_client_shutdown(Ecore_Con_Client *cl)
{
  return SSL_SUFFIX(_ecore_con_ssl_client_shutdown)(cl);
}

EAPI int
ecore_con_ssl_client_read(Ecore_Con_Client *cl, char *buf, int size)
{
  return SSL_SUFFIX(_ecore_con_ssl_client_read)(cl, buf, size);
}

EAPI int
ecore_con_ssl_client_write(Ecore_Con_Client *cl, char *buf, int size)
{
  return SSL_SUFFIX(_ecore_con_ssl_client_write)(cl, buf, size);
}

#if USE_GNUTLS

/*
 * GnuTLS
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_gnutls(void)
{
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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_gnutls(Ecore_Con_Server *svr)
{
  const int *proto = NULL;
  gnutls_dh_params_t dh_params;
  int ret;
  const int kx[] = { GNUTLS_KX_ANON_DH, 0 };
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
      return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;
    case ECORE_CON_USE_SSL3:
      proto = ssl3_proto;
      break;
    case ECORE_CON_USE_TLS:
      proto = tls_proto;
      break;
    default:
      return ECORE_CON_SSL_ERROR_NONE;
    }

  gnutls_anon_allocate_client_credentials(&(svr->anoncred_c));
  gnutls_init(&(svr->session), GNUTLS_CLIENT);
  gnutls_set_default_priority(svr->session);
  gnutls_kx_set_priority(svr->session, kx);
  gnutls_credentials_set(svr->session, GNUTLS_CRD_ANON, svr->anoncred_c);
  gnutls_kx_set_priority(svr->session, kx);
  gnutls_protocol_set_priority(svr->session, proto);
  gnutls_dh_set_prime_bits(svr->session, 512);

  gnutls_transport_set_ptr(svr->session, (gnutls_transport_ptr_t)svr->fd);

  while ((ret = gnutls_handshake(svr->session)) < 0)
    {
      if ((ret == GNUTLS_E_AGAIN) ||
	  (ret == GNUTLS_E_INTERRUPTED))
	continue;

      _ecore_con_ssl_server_shutdown_gnutls(svr);
      return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
    }

  return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_gnutls(Ecore_Con_Server *svr)
{
  if (svr->session)
    {
      gnutls_bye(svr->session, GNUTLS_SHUT_RDWR);
      gnutls_deinit(svr->session);
    }
  if (svr->anoncred_c)
    gnutls_anon_free_client_credentials(svr->anoncred_c);
  _ecore_con_ssl_server_prepare_gnutls(svr);

  return ECORE_CON_SSL_ERROR_NONE;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_gnutls(Ecore_Con_Server *svr)
{
   return ECORE_CON_CONNECTED;
}

static int
_ecore_con_ssl_server_read_gnutls(Ecore_Con_Server *svr, char *buf, int size)
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
_ecore_con_ssl_server_write_gnutls(Ecore_Con_Server *svr, char *buf, int size)
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
    cl->server->anoncred_s = NULL;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_gnutls(Ecore_Con_Client *cl)
{
  const int *proto = NULL;
  gnutls_dh_params_t dh_params;
  int ret;
  const int kx[] = { GNUTLS_KX_ANON_DH, 0 };
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
      return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;
    case ECORE_CON_USE_SSL3:
      proto = ssl3_proto;
      break;
    case ECORE_CON_USE_TLS:
      proto = tls_proto;
      break;
    default:
      return ECORE_CON_SSL_ERROR_NONE;
    }

  _client_connected++;
  if (!cl->server->anoncred_s)
    {
      gnutls_anon_allocate_server_credentials(&(cl->server->anoncred_s));
      gnutls_dh_params_init(&dh_params);
      gnutls_dh_params_generate2(dh_params, 512);
      gnutls_anon_set_server_dh_params(cl->server->anoncred_s, dh_params);
    }

  gnutls_init(&(cl->session), GNUTLS_SERVER);
  gnutls_set_default_priority(cl->session);
  gnutls_credentials_set(cl->session, GNUTLS_CRD_ANON, cl->server->anoncred_s);

  gnutls_kx_set_priority(cl->session, kx);

  gnutls_protocol_set_priority(cl->session, proto);

  gnutls_dh_set_prime_bits(cl->session, 512);

  gnutls_transport_set_ptr(cl->session, (gnutls_transport_ptr_t)cl->fd);

  while ((ret = gnutls_handshake(cl->session)) < 0)
    {
      if ((ret == GNUTLS_E_AGAIN) ||
	  (ret == GNUTLS_E_INTERRUPTED))
	continue;

      _ecore_con_ssl_client_shutdown_gnutls(cl);
      return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
    }

  return ECORE_CON_SSL_ERROR_NONE;
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
  _ecore_con_ssl_client_prepare_gnutls(cl);

  return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_gnutls(Ecore_Con_Client *cl, char *buf, int size)
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
_ecore_con_ssl_client_write_gnutls(Ecore_Con_Client *cl, char *buf, int size)
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

#elif USE_OPENSSL

/*
 * OpenSSL
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_openssl(void)
{
  SSL_library_init();
  SSL_load_error_strings();

  return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_openssl(void)
{
  // FIXME nothing to do ?
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
      /* Unsafe version of SSL */
      if (!(svr->ssl_ctx = SSL_CTX_new(SSLv2_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    case ECORE_CON_USE_SSL3:
      if (!(svr->ssl_ctx = SSL_CTX_new(SSLv3_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    case ECORE_CON_USE_TLS:
      if (!(svr->ssl_ctx = SSL_CTX_new(TLSv1_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    default:
      return ECORE_CON_SSL_ERROR_NONE;
    }
  if (!(svr->ssl = SSL_new(svr->ssl_ctx)))
    {
      SSL_CTX_free(svr->ssl_ctx);
      return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
    }

  SSL_set_fd(svr->ssl, svr->fd);

  return ECORE_CON_SSL_ERROR_NONE;
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
  if (svr->ssl_ctx) SSL_CTX_free(svr->ssl_ctx);

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
     ecore_main_fd_handler_active_set(svr->fd_handler, flag);

   return ECORE_CON_INPROGRESS;
}

static int
_ecore_con_ssl_server_read_openssl(Ecore_Con_Server *svr, char *buf, int size)
{
  int num;

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
_ecore_con_ssl_server_write_openssl(Ecore_Con_Server *svr, char *buf, int size)
{
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
      /* Unsafe version of SSL */
      if (!(cl->ssl_ctx = SSL_CTX_new(SSLv2_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    case ECORE_CON_USE_SSL3:
      if (!(cl->ssl_ctx = SSL_CTX_new(SSLv3_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    case ECORE_CON_USE_TLS:
      if (!(cl->ssl_ctx = SSL_CTX_new(TLSv1_client_method())))
	return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
      break;
    default:
      return ECORE_CON_SSL_ERROR_NONE;
    }
  if (!(cl->ssl = SSL_new(cl->ssl_ctx)))
    {
      SSL_CTX_free(cl->ssl_ctx);
      return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
    }

  SSL_set_fd(cl->ssl, cl->fd);

  return ECORE_CON_SSL_ERROR_NONE;
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
  if (cl->ssl_ctx) SSL_CTX_free(cl->ssl_ctx);

  _ecore_con_ssl_client_prepare_openssl(cl);

  return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_openssl(Ecore_Con_Client *cl, char *buf, int size)
{
  int num;

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
_ecore_con_ssl_client_write_openssl(Ecore_Con_Client *cl, char *buf, int size)
{
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

static void
_ecore_con_ssl_server_prepare_none(Ecore_Con_Server *svr)
{
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_none(Ecore_Con_Server *svr)
{
  return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
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
_ecore_con_ssl_server_read_none(Ecore_Con_Server *svr, char *buf, int size)
{
  return -1;
}

static int
_ecore_con_ssl_server_write_none(Ecore_Con_Server *svr, char *buf, int size)
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

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_none(Ecore_Con_Client *cl)
{
  return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_client_read_none(Ecore_Con_Client *cl, char *buf, int size)
{
  return -1;
}

static int
_ecore_con_ssl_client_write_none(Ecore_Con_Client *cl, char *buf, int size)
{
  return -1;
}

#endif
