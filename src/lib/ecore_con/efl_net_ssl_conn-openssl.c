#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

/* OpenSSL's BIO is the abstraction for I/O, provide one for Efl.Io.* */
static int
efl_net_socket_bio_create(BIO *b)
{
   b->init = 1;
   b->num = 0;
   b->ptr = NULL;
   b->flags = 0;
   return 1;
}

static int
efl_net_socket_bio_destroy(BIO *b)
{
   if (!b) return 0;
   b->init = 0;
   b->ptr = NULL;
   b->flags = 0;
   return 1;
}

static int
efl_net_socket_bio_read(BIO *b, char *buf, int len)
{
   Eina_Rw_Slice slice = {
     .mem = buf,
     .len = len
   };
   Eo *sock = b->ptr;
   Eina_Error err;

   if ((!buf) || (len <= 0)) return 0;
   if (!sock) return 0;

   if (!efl_io_reader_can_read_get(sock))
     {
        DBG("socket=%p would block if read!", sock);
        BIO_set_retry_read(b);
        return -1;
     }

   err = efl_io_reader_read(sock, &slice);
   BIO_clear_retry_flags(b);
   if (err)
     {
        if (err == EAGAIN)
          BIO_set_retry_write(b);
        return -1;
     }

   return slice.len;
}

static int
efl_net_socket_bio_write(BIO *b, const char *buf, int len)
{
   Eina_Slice slice = {
     .mem = buf,
     .len = len
   };
   Eo *sock = b->ptr;
   Eina_Error err;

   if ((!buf) || (len <= 0)) return 0;
   if (!sock) return 0;

   if (!efl_io_writer_can_write_get(sock))
     {
        DBG("socket=%p would block if written!", sock);
        BIO_set_retry_write(b);
        return -1;
     }

   err = efl_io_writer_write(sock, &slice, NULL);
   BIO_clear_retry_flags(b);
   if (err)
     {
        if (err == EAGAIN)
          BIO_set_retry_write(b);
        return -1;
     }

   return slice.len;
}

static long
efl_net_socket_bio_ctrl(BIO *b EINA_UNUSED, int cmd, long num EINA_UNUSED, void *ptr EINA_UNUSED)
{
   if (cmd == BIO_CTRL_FLUSH)
     /* looks mandatory, but doesn't have a meaning here */
     return 1;
   return 0;
}

static int
efl_net_socket_bio_puts(BIO *b, const char *str)
{
   return efl_net_socket_bio_write(b, str, strlen(str));
}

static BIO_METHOD efl_net_socket_bio = {
  0x400, /* 0x400 means source & sink */
  "efl_net_socket wrapper",
  efl_net_socket_bio_write,
  efl_net_socket_bio_read,
  efl_net_socket_bio_puts,
  NULL, /* no gets */
  efl_net_socket_bio_ctrl,
  efl_net_socket_bio_create,
  efl_net_socket_bio_destroy
};

struct _Efl_Net_Ssl_Conn
{
   SSL *ssl;
   BIO *bio;
   const char *hostname;
   Eina_Bool hostname_verify;
   Eina_Bool did_certificates;
};

#define EFL_NET_SOCKET_SSL_CIPHERS "aRSA+HIGH:+kEDH:+kRSA:!kSRP:!kPSK:+3DES:!MD5"

#define _efl_net_ssl_conn_session_debug(conn) \
  __efl_net_ssl_conn_session_debug(__FILE__, __LINE__, __FUNCTION__, conn)
static void
__efl_net_ssl_conn_session_debug(const char *file, int line, const char *fname, Efl_Net_Ssl_Conn *conn)
{
   STACK_OF(X509) * sk_X509;
   STACK_OF(SSL_CIPHER) *sk_CIPHER;

   if (!eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG)) return;

   sk_X509 = SSL_get_peer_cert_chain(conn->ssl);
   if (!sk_X509)
     eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                    "ssl_conn=%p No peer certificate chain", conn);
   else
     {
        char subject[4096], issuer[4096];
        int i;

        eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                       "ssl_conn=%p Peer Certificates:", conn);
        for (i = 0; i < sk_X509_num(sk_X509); i++)
          {
             X509 *item = sk_X509_value(sk_X509, i);
             eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                            "  #%02d %s (Issuer: %s)",
                            i,
                            X509_NAME_oneline(X509_get_subject_name(item), subject, sizeof(subject)),
                            X509_NAME_oneline(X509_get_issuer_name(item), issuer, sizeof(issuer)));
             if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG + 1))
               X509_print_fp(stderr, item);
          }
     }

   sk_CIPHER = SSL_get_ciphers(conn->ssl);
   if (!sk_CIPHER)
     eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                    "ssl_conn=%p No ciphers", conn);
   else
     {
        char shared[8192];
        const SSL_CIPHER *cipher;
        char *p;
        int i;

        eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                       "ssl_conn=%p Ciphers:", conn);
        for (i = 0; i < sk_SSL_CIPHER_num(sk_CIPHER); i++)
          {
             cipher = sk_SSL_CIPHER_value(sk_CIPHER, i);
             eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                            "  #%03d %s", i, SSL_CIPHER_get_name(cipher));
          }

        p = SSL_get_shared_ciphers(conn->ssl, shared, sizeof(shared));
        if (!p)
          eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                         "ssl_conn=%p No Client (Shared) Ciphers", conn);
        else
          {
             eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                            "ssl_conn=%p Client (Shared) Ciphers:", conn);
             i = 0;
             do
               {
                  char *n = strchr(p, ':');
                  if (n)
                    {
                       *n = '\0';
                       n++;
                    }

                  eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                                 "  #%03d %s", i, p);
                  p = n;
                  i++;
               }
             while (p);
          }

        cipher = SSL_get_current_cipher(conn->ssl);
        if (!cipher)
          eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                         "ssl_conn=%p No cipher in use", conn);
        else
          eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                         "ssl_conn=%p Current Cipher: %s (%s)",
                         conn,
                         SSL_CIPHER_get_version(cipher),
                         SSL_CIPHER_get_name(cipher));
     }

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG + 1))
     SSL_SESSION_print_fp(stderr, SSL_get_session(conn->ssl));

   eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG, file, fname, line,
                  "ssl_conn=%p end of SSL session information", conn);
}

#define _efl_net_ssl_conn_check_errors() \
  __efl_net_ssl_conn_check_errors(__FILE__, __LINE__, __FUNCTION__)
static unsigned long
__efl_net_ssl_conn_check_errors(const char *file, int line, const char *fname)
{
   unsigned long first = 0;
   do
     {
        const char *_ssl_err_file, *_ssl_err_data;
        int _ssl_err_line, _ssl_err_flags;
        unsigned long _ssl_err = ERR_get_error_line_data(&_ssl_err_file, &_ssl_err_line, &_ssl_err_data, &_ssl_err_flags);
        if (!_ssl_err) break;
        if (!first) first = _ssl_err;
        eina_log_print(_ecore_con_log_dom, EINA_LOG_LEVEL_ERR, file, fname, line,
                       "OpenSSL error %s:%d%s%s: %s",
                       _ssl_err_file, _ssl_err_line,
                       (_ssl_err_flags & ERR_TXT_STRING) ? " " : "",
                       (_ssl_err_flags & ERR_TXT_STRING) ? _ssl_err_data : "",
                       ERR_reason_error_string(_ssl_err));
     }
   while (1);
   return first;
}

static Eina_Error
efl_net_ssl_conn_setup(Efl_Net_Ssl_Conn *conn, Eina_Bool is_dialer, Efl_Net_Socket *sock, Efl_Net_Ssl_Context *context)
{
   char vbuf[32];
   const char *ssl_ver_str = NULL;
   int ssl_ver;
   static const struct {
      int ver;
      const char *str;
   } *ssl_ver_itr, ssl_ver_map[] = {
     {SSL3_VERSION, "SSLv3.0"},
     {TLS1_VERSION, "TLSv1.0"},
     {TLS1_1_VERSION, "TLSv1.1"},
     {TLS1_2_VERSION, "TLSv1.2"},
     {DTLS1_VERSION, "DTLSv1.0"},
     {DTLS1_2_VERSION, "DTLSv1.2"},
     {DTLS1_BAD_VER, "DTLSv1.0"},
     {0, NULL}
   };

   EINA_SAFETY_ON_TRUE_RETURN_VAL(conn->ssl != NULL, EALREADY);

   conn->ssl = efl_net_ssl_context_connection_new(context);
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn->ssl, ENOSYS);

   conn->bio = BIO_new(&efl_net_socket_bio);
   EINA_SAFETY_ON_NULL_GOTO(conn->bio, error_bio);

   conn->bio->ptr = sock;

   SSL_set_bio(conn->ssl, conn->bio, conn->bio);
   if (is_dialer)
     SSL_set_connect_state(conn->ssl);
   else
     SSL_set_accept_state(conn->ssl);

   ssl_ver = SSL_version(conn->ssl);
   for (ssl_ver_itr = ssl_ver_map; ssl_ver_itr->str != NULL; ssl_ver_itr++)
     {
        if (ssl_ver_itr->ver == ssl_ver)
          {
             ssl_ver_str = ssl_ver_itr->str;
             break;
          }
     }
   if (!ssl_ver_str)
     {
        snprintf(vbuf, sizeof(vbuf), "%#x", ssl_ver);
        ssl_ver_str = vbuf;
     }
   DBG("Using SSL %s", ssl_ver_str);

   return 0;

 error_bio:
   SSL_shutdown(conn->ssl);
   SSL_free(conn->ssl);
   conn->ssl = NULL;
   return ENOSYS;
}

static void
efl_net_ssl_conn_teardown(Efl_Net_Ssl_Conn *conn)
{
   if (conn->bio)
     {
        /* NOTE: no BIO_free() as it's done by SSL_free(). */
     }

   if (conn->ssl)
     {
        if (!SSL_shutdown(conn->ssl))
          SSL_shutdown(conn->ssl);

        SSL_free(conn->ssl);
        conn->ssl = NULL;
     }

   eina_stringshare_replace(&conn->hostname, NULL);
}

static Eina_Error
efl_net_ssl_conn_write(Efl_Net_Ssl_Conn *conn, Eina_Slice *slice)
{
   int r = SSL_write(conn->ssl, slice->mem, slice->len);
   if (r < 0)
     {
        int ssl_err = SSL_get_error(conn->ssl, r);

        slice->len = 0;
        if (ssl_err == SSL_ERROR_WANT_WRITE) return EAGAIN;
        _efl_net_ssl_conn_check_errors();
        ERR("ssl_conn=%p could not write", conn);
        return EINVAL;
     }
   slice->len = r;
   return 0;
}

static Eina_Error
efl_net_ssl_conn_read(Efl_Net_Ssl_Conn *conn, Eina_Rw_Slice *slice)
{
   int r = SSL_read(conn->ssl, slice->mem, slice->len);
   if (r < 0)
     {
        int ssl_err = SSL_get_error(conn->ssl, r);

        slice->len = 0;
        if (ssl_err == SSL_ERROR_WANT_READ) return EAGAIN;
        _efl_net_ssl_conn_check_errors();
        ERR("ssl_conn=%p could not read", conn);
        return EINVAL;
     }
   slice->len = r;
   return 0;
}

static Eina_Error
_efl_net_ssl_conn_hostname_verify(Efl_Net_Ssl_Conn *conn)
{
   X509 *x509;
   struct sockaddr_storage addr;
   const char *label;
   int family = AF_INET;
   int r;

   if ((!conn->hostname) || (conn->hostname[0] == '\0'))
     {
        ERR("ssl_conn=%p no hostname, cannot verify", conn);
        return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
     }

   x509 = SSL_get_peer_certificate(conn->ssl);
   if (!x509)
     {
        ERR("ssl_conn=%p no peer certificate!", conn);
        return EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE;
     }

   if (strchr(conn->hostname, ':')) family = AF_INET6;
   if (inet_pton(family, conn->hostname, &addr) == 1)
     {
        label = "IP address";
        r = X509_check_ip_asc(x509, conn->hostname, 0);
     }
   else
     {
        label = "hostname";
        r = X509_check_host(x509, conn->hostname, 0, 0, NULL);
     }

   if (r != 1)
     {
        ERR("ssl_conn=%p %s='%s' doesn't match certificate.",
            conn, label, conn->hostname);
        return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
     }

   DBG("ssl_conn=%p %s='%s' matches certificate.", conn, label, conn->hostname);

   return 0;
}

static Eina_Error
efl_net_ssl_conn_handshake(Efl_Net_Ssl_Conn *conn, Eina_Bool *done)
{
   int r = SSL_do_handshake(conn->ssl);
   long err_ssl;
   const char *err_file;
   const char *err_data;
   int err_line, err_flags;

   *done = EINA_FALSE;

   if (r == 1)
     {
        _efl_net_ssl_conn_session_debug(conn);

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

   r = SSL_get_error(conn->ssl, r);
   if ((r == SSL_ERROR_WANT_READ) || (r == SSL_ERROR_WANT_WRITE))
     {
        DBG("ssl_conn=%p handshake needs more data...", conn);
        return 0;
     }

   err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
   _efl_net_ssl_conn_check_errors();

   if (!err_ssl)
     DBG("ssl_conn=%p handshake error=%#x (SSL_ERROR_SSL=%#x)", conn, r, SSL_ERROR_SSL);
   else
     DBG("ssl_conn=%p handshake error=%#x (SSL_ERROR_SSL=%#x) [%s:%d%s%s %#lx='%s']",
         conn, r, SSL_ERROR_SSL,
         err_file, err_line,
         (err_flags & ERR_TXT_STRING) ? " " : "",
         (err_flags & ERR_TXT_STRING) ? err_data : "",
         err_ssl, ERR_reason_error_string(err_ssl));
   if (r == SSL_ERROR_SSL)
     {
        _efl_net_ssl_conn_session_debug(conn);

        if ((ERR_GET_LIB(err_ssl) == ERR_LIB_SSL) &&
            (ERR_GET_REASON(err_ssl) == SSL_R_CERTIFICATE_VERIFY_FAILED))
          {
             WRN("ssl_conn=%p certificate verification failed, handshake failed", conn);
             return EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED;
          }
        WRN("ssl_conn=%p handshake failed: %s", conn, ERR_reason_error_string(err_ssl));
        return EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE;
     }

   return 0;
}

static Eina_Error
efl_net_ssl_conn_verify_mode_set(Efl_Net_Ssl_Conn *conn, Efl_Net_Ssl_Verify_Mode verify_mode)
{
   int ssl_mode;

   switch (verify_mode)
     {
      case EFL_NET_SSL_VERIFY_MODE_NONE:
         ssl_mode = SSL_VERIFY_NONE;
         break;
      case EFL_NET_SSL_VERIFY_MODE_OPTIONAL:
         ssl_mode = SSL_VERIFY_PEER;
         break;
      case EFL_NET_SSL_VERIFY_MODE_REQUIRED:
         ssl_mode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
         break;
      default:
         ERR("unknown verify_mode=%d", verify_mode);
         return EINVAL;
     }

   SSL_set_verify(conn->ssl, ssl_mode, SSL_get_verify_callback(conn->ssl));
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
   eina_stringshare_replace(&conn->hostname, hostname);
   if (hostname && (!conn->hostname)) return ENOMEM;
   return 0;
}
