#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

struct _Efl_Net_Ssl_Conn {
   gnutls_session_t session;
   gnutls_datum_t ticket;
   Eo *sock;
   const char *hostname;
   Efl_Net_Ssl_Verify_Mode verify_mode;
   Eina_Bool hostname_verify;
   Eina_Bool is_dialer;
};

static ssize_t
_efl_net_ssl_conn_write(gnutls_transport_ptr_t transp, const void *buf, size_t len)
{
   Eina_Slice slice = {
     .mem = buf,
     .len = len
   };
   Efl_Net_Ssl_Conn *conn = transp;
   Eina_Error err;

   if ((!buf) || (len == 0)) return 0;
   if (!conn) return 0;

   if (!efl_io_writer_can_write_get(conn->sock))
     {
        DBG("socket=%p would block if written!", conn->sock);
        gnutls_transport_set_errno(conn->session, EAGAIN);
        return -1;
     }

   err = efl_io_writer_write(conn->sock, &slice, NULL);
   if (err)
     {
        gnutls_transport_set_errno(conn->session, err);
        return -1;
     }

   gnutls_transport_set_errno(conn->session, 0);
   return slice.len;
}

static ssize_t
_efl_net_ssl_conn_read(gnutls_transport_ptr_t transp, void *buf, size_t len)
{
   Eina_Rw_Slice slice = {
     .mem = buf,
     .len = len
   };
   Efl_Net_Ssl_Conn *conn = transp;
   Eina_Error err;

   if ((!buf) || (len == 0)) return 0;
   if (!conn) return 0;

   if (!efl_io_reader_can_read_get(conn->sock))
     {
        DBG("socket=%p would block if read!", conn->sock);
        gnutls_transport_set_errno(conn->session, EAGAIN);
        return -1;
     }

   err = efl_io_reader_read(conn->sock, &slice);
   if (err)
     {
        gnutls_transport_set_errno(conn->session, err);
        return -1;
     }

   gnutls_transport_set_errno(conn->session, 0);
   return slice.len;
}

static Eina_Error
efl_net_ssl_conn_setup(Efl_Net_Ssl_Conn *conn, Eina_Bool is_dialer, Efl_Net_Socket *sock, Efl_Net_Ssl_Context *context)
{
   gnutls_certificate_request_t req;
   int r;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(conn->session != NULL, EALREADY);

   conn->is_dialer = is_dialer;

   conn->session = efl_net_ssl_context_connection_new(context);
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn->session, ENOSYS);

   gnutls_handshake_set_private_extensions(conn->session, 1);

   switch (conn->verify_mode)
     {
      case EFL_NET_SSL_VERIFY_MODE_NONE:
         req = GNUTLS_CERT_IGNORE;
         break;
      case EFL_NET_SSL_VERIFY_MODE_OPTIONAL:
         req = GNUTLS_CERT_REQUEST;
         break;
      case EFL_NET_SSL_VERIFY_MODE_REQUIRED:
      default:
         req = GNUTLS_CERT_REQUIRE;
     }
   gnutls_certificate_server_set_request(conn->session, req);

   if (is_dialer)
     {
        r = gnutls_session_ticket_enable_client(conn->session);
        if (r < 0)
          {
             ERR("ssl_conn=%p could not enable session's ticket client: %s", conn, gnutls_strerror(r));
             goto error;
          }
     }
   else
     {
        r = gnutls_session_ticket_key_generate(&conn->ticket);
        if (r < 0)
          {
             ERR("ssl_conn=%p could not generate session ticket: %s", conn, gnutls_strerror(r));
             goto error;
          }

        r = gnutls_session_ticket_enable_server(conn->session, &conn->ticket);
        if (r < 0)
          {
             ERR("ssl_conn=%p could not enable session's ticket server: %s", conn, gnutls_strerror(r));
             goto error_ticket;
          }
     }

   conn->sock = sock;
   gnutls_transport_set_ptr(conn->session, conn);
   gnutls_transport_set_push_function(conn->session, _efl_net_ssl_conn_write);
   gnutls_transport_set_pull_function(conn->session, _efl_net_ssl_conn_read);

   return 0;

 error_ticket:
   gnutls_free(conn->ticket.data);
   conn->ticket.data = NULL;

 error:
   gnutls_deinit(conn->session);
   conn->session = NULL;
   return ENOSYS;
}

static void
efl_net_ssl_conn_teardown(Efl_Net_Ssl_Conn *conn)
{
   if (conn->session)
     {
        gnutls_bye(conn->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(conn->session);
        conn->session = NULL;
     }

   if (conn->ticket.data)
     {
        gnutls_free(conn->ticket.data);
        conn->ticket.data = NULL;
     }

   eina_stringshare_replace(&conn->hostname, NULL);
}

static Eina_Error
efl_net_ssl_conn_write(Efl_Net_Ssl_Conn *conn, Eina_Slice *slice)
{
   ssize_t r = gnutls_record_send(conn->session, slice->mem, slice->len);
   if (r < 0)
     {
        slice->len = 0;
        if (gnutls_error_is_fatal(r))
          {
             ERR("ssl_conn=%p could not send %zd bytes: %s", conn, slice->len, gnutls_strerror(r));
             return EINVAL;
          }
        DBG("ssl_conn=%p could not send %zd bytes: %s", conn, slice->len, gnutls_strerror(r));
        return EAGAIN;
     }
   slice->len = r;
   return 0;
}

static Eina_Error
efl_net_ssl_conn_read(Efl_Net_Ssl_Conn *conn, Eina_Rw_Slice *slice)
{
   ssize_t r = gnutls_record_recv(conn->session, slice->mem, slice->len);
   if (r < 0)
     {
        slice->len = 0;
        if (gnutls_error_is_fatal(r))
          {
             ERR("ssl_conn=%p could not receive %zd bytes: %s", conn, slice->len, gnutls_strerror(r));
             return EINVAL;
          }
        DBG("ssl_conn=%p could not receive %zd bytes: %s", conn, slice->len, gnutls_strerror(r));
        return EAGAIN;
     }
   slice->len = r;
   return 0;
}

static Eina_Error
_efl_net_ssl_conn_verify(Efl_Net_Ssl_Conn *conn)
{
   unsigned status = 0;
   int r;

   r = gnutls_certificate_verify_peers2(conn->session, &status);
   if (r < 0)
     {
        ERR("ssl_conn=%p could not verify peer: %s", conn, gnutls_strerror(r));
        return EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE;
     }

   if (!status) return 0;

   if (status & GNUTLS_CERT_INVALID)
     WRN("ssl_conn=%p The certificate is not trusted.", conn);
   if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
     WRN("ssl_conn=%p The certificate hasn't got a known issuer.", conn);
   if (status & GNUTLS_CERT_REVOKED)
     WRN("ssl_conn=%p The certificate has been revoked.", conn);
   if (status & GNUTLS_CERT_EXPIRED)
     WRN("ssl_conn=%p The certificate has expired", conn);
   if (status & GNUTLS_CERT_NOT_ACTIVATED)
     WRN("ssl_conn=%p The certificate is not yet activated", conn);

   return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
}

static Eina_Error
_efl_net_ssl_conn_hostname_verify(Efl_Net_Ssl_Conn *conn)
{
   const gnutls_datum_t *list;
   unsigned int size;
   gnutls_x509_crt_t cert = NULL;
   int r;

   if ((!conn->hostname) || (conn->hostname[0] == '\0'))
     {
        ERR("ssl_conn=%p no hostname, cannot verify", conn);
        return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
     }

   if (gnutls_certificate_type_get(conn->session) != GNUTLS_CRT_X509)
     {
        ERR("ssl_conn=%p PGP certificates are not yet supported!", conn);
        return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
     }

   list = gnutls_certificate_get_peers(conn->session, &size);
   if (!list)
     {
        ERR("ssl_conn=%p no peer certificate!", conn);
        return EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE;
     }

   r = gnutls_x509_crt_init(&cert);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(r < 0, EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED);

   r = gnutls_x509_crt_import(cert, &list[0], GNUTLS_X509_FMT_DER);
   if (r < 0)
     {
        ERR("ssl_conn=%p could not import x509 certificate to verify: %s", conn, gnutls_strerror(r));
        gnutls_x509_crt_deinit(cert);
        return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
     }

   r = gnutls_x509_crt_check_hostname(cert, conn->hostname);
   gnutls_x509_crt_deinit(cert);

   if (r == 1)
     return 0;

   ERR("ssl_conn=%p hostname='%s' doesn't match certificate.",
       conn, conn->hostname);
   return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
}

static Eina_Error
efl_net_ssl_conn_handshake(Efl_Net_Ssl_Conn *conn, Eina_Bool *done)
{
   int r = gnutls_handshake(conn->session);
   if (r < 0)
     {
        *done = EINA_FALSE;
        if (gnutls_error_is_fatal(r))
          {
             ERR("ssl_conn=%p could not handshake: %s", conn, gnutls_strerror(r));
             return EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE;
          }

        DBG("ssl_conn=%p did not finish handshake: %s", conn, gnutls_strerror(r));
        return 0;
     }

   if (conn->verify_mode != EFL_NET_SSL_VERIFY_MODE_NONE)
     {
        Eina_Error err = _efl_net_ssl_conn_verify(conn);
        if (err)
          return err;
     }

   if (conn->hostname_verify)
     {
        Eina_Error err = _efl_net_ssl_conn_hostname_verify(conn);
        if (err)
          return err;
     }

   *done = EINA_TRUE;
   DBG("ssl_conn=%p handshake finished!", conn);
   return 0;
}

static Eina_Error
efl_net_ssl_conn_verify_mode_set(Efl_Net_Ssl_Conn *conn, Efl_Net_Ssl_Verify_Mode verify_mode)
{
   gnutls_certificate_request_t req;
   conn->verify_mode = verify_mode;

   switch (conn->verify_mode)
     {
      case EFL_NET_SSL_VERIFY_MODE_NONE:
         req = GNUTLS_CERT_IGNORE;
         break;
      case EFL_NET_SSL_VERIFY_MODE_OPTIONAL:
         req = GNUTLS_CERT_REQUEST;
         break;
      case EFL_NET_SSL_VERIFY_MODE_REQUIRED:
      default:
         req = GNUTLS_CERT_REQUIRE;
     }
   gnutls_certificate_server_set_request(conn->session, req);

   return 0;
}

static Eina_Error
efl_net_ssl_conn_hostname_verify_set(Efl_Net_Ssl_Conn *conn, Eina_Bool hostname_verify)
{
   conn->hostname_verify = hostname_verify;
   return 0;
}

static Eina_Error
efl_net_ssl_conn_hostname_override_set(Efl_Net_Ssl_Conn *conn, const char *hostname)
{
   int r;
   eina_stringshare_replace(&conn->hostname, hostname);
   if (!hostname) hostname = "";
   r = gnutls_server_name_set(conn->session, GNUTLS_NAME_DNS, hostname, strlen(hostname));
   if (r < 0)
     {
        ERR("ssl_conn=%p could not set server name '%s': %s", conn, hostname, gnutls_strerror(r));
        return EINVAL;
     }
   return 0;
}
