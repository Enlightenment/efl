#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/dh.h>

struct _Efl_Net_Ssl_Ctx
{
   SSL_CTX *ssl_ctx;
   Eina_Bool did_certificates;
   Eina_Bool is_dialer;
};

#ifndef TLS_server_method
# define TLS_server_method TLSv1_2_server_method
#endif

#ifndef TLS_client_method
# define TLS_client_method TLSv1_2_client_method
#endif

#define EFL_NET_SSL_CONTEXT_CIPHERS "aRSA+HIGH:+kEDH:+kRSA:!kSRP:!kPSK:+3DES:!MD5"

#define _efl_net_ssl_ctx_check_errors() \
  __efl_net_ssl_ctx_check_errors(__FILE__, __LINE__, __FUNCTION__)
static unsigned long
__efl_net_ssl_ctx_check_errors(const char *file, int line, const char *fname)
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
_efl_net_ssl_ctx_load_lists(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Ctx_Config cfg)
{
   Eina_List *n, *n_next;
   const char *path;
   unsigned certificates_count = eina_list_count(*cfg.certificates);
   unsigned private_keys_count = eina_list_count(*cfg.private_keys);
   unsigned certificate_revocation_lists_count = eina_list_count(*cfg.certificate_revocation_lists);
   unsigned certificate_authorities_count = eina_list_count(*cfg.certificate_authorities);
   long err_ssl;
   const char *err_file;
   const char *err_data;
   int err_line, err_flags;
   X509_STORE *x509_store;
   X509_LOOKUP *x509_lookup;
   unsigned long x509_store_flags = 0;

#ifdef X509_V_FLAG_TRUSTED_FIRST
   x509_store_flags |= X509_V_FLAG_TRUSTED_FIRST;
#endif

   if (cfg.load_defaults)
     {
        if (SSL_CTX_set_default_verify_paths(ctx->ssl_ctx) != 1)
          {
             _efl_net_ssl_ctx_check_errors();
             ERR("ssl_ctx=%p could not load default paths", ctx);
             return ENOSYS;
          }
        DBG("ssl_ctx=%p loaded default paths", ctx);
     }
   else
     DBG("ssl_ctx=%p did not load default paths", ctx);

   EINA_LIST_FOREACH_SAFE(*cfg.certificates, n, n_next, path)
     {
        if ((SSL_CTX_use_certificate_file(ctx->ssl_ctx, path, SSL_FILETYPE_PEM) != 1) &&
            (SSL_CTX_use_certificate_file(ctx->ssl_ctx, path, SSL_FILETYPE_ASN1) != 1))
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);

_efl_net_ssl_ctx_check_errors();
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p could not use certificate from %s [%s:%d%s%s '%s']",
                 ctx, path,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             eina_stringshare_del(path);
             *cfg.certificates = eina_list_remove_list(*cfg.certificates, n);
             continue;
          }

        DBG("ssl_ctx=%p loaded certificate '%s'", ctx, path);
        ctx->did_certificates = EINA_TRUE;
     }
   if (certificates_count && !*cfg.certificates)
     {
        ERR("ssl_ctx=%p none of the required certificates were loaded!", ctx);
        return EINVAL;
     }

   EINA_LIST_FOREACH_SAFE(*cfg.private_keys, n, n_next, path)
     {
        if ((SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, path, SSL_FILETYPE_PEM) != 1) &&
            (SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, path, SSL_FILETYPE_ASN1) != 1))
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p could not use private key from %s [%s:%d%s%s '%s']",
                 ctx, path,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             eina_stringshare_del(path);
             *cfg.private_keys = eina_list_remove_list(*cfg.private_keys, n);
             continue;
          }

        if (SSL_CTX_check_private_key(ctx->ssl_ctx) != 1)
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p could not check private key from %s [%s:%d%s%s '%s']",
                 ctx, path,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             continue;
          }

        DBG("ssl_ctx=%p loaded private key '%s'", ctx, path);
     }
   if (private_keys_count && !*cfg.private_keys)
     {
        ERR("ssl_ctx=%p none of the required private keys were loaded!", ctx);
        return EINVAL;
     }

   x509_store = SSL_CTX_get_cert_store(ctx->ssl_ctx);
   if (!x509_store)
     {
        _efl_net_ssl_ctx_check_errors();
        ERR("ssl_ctx=%p SSL has no X509 certificate store", ctx);
        return ENOSYS;
     }
   x509_lookup = X509_STORE_add_lookup(x509_store, X509_LOOKUP_file());
   if (!x509_lookup)
     {
        _efl_net_ssl_ctx_check_errors();
        ERR("ssl_ctx=%p could not add X509 file lookup", ctx);
        return ENOSYS;
     }

   EINA_LIST_FOREACH_SAFE(*cfg.certificate_revocation_lists, n, n_next, path)
     {
        if ((X509_load_crl_file(x509_lookup, path, X509_FILETYPE_PEM) != 1) &&
            (X509_load_crl_file(x509_lookup, path, X509_FILETYPE_ASN1) != 1))
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p could not use certificate revocation lists from %s [%s:%d%s%s '%s']",
                 ctx, path,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             eina_stringshare_del(path);
             *cfg.certificate_revocation_lists = eina_list_remove_list(*cfg.certificate_revocation_lists, n);
             continue;
          }

        DBG("ssl_ctx=%p loaded certificate revocation lists '%s'", ctx, path);
        x509_store_flags |= X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL;
     }
   if (certificate_revocation_lists_count && !*cfg.certificate_revocation_lists)
     {
        ERR("ssl_ctx=%p none of the required certificate revocation lists were loaded!", ctx);
        return EINVAL;
     }
   X509_STORE_set_flags(x509_store, x509_store_flags);

   EINA_LIST_FOREACH_SAFE(*cfg.certificate_authorities, n, n_next, path)
     {
        struct stat st;
        const char *cafile = NULL, *cadir = NULL;

        if (stat(path, &st) != 0)
          {
             ERR("ssl_ctx=%p could not load certificate authorities from '%s': %s", ctx, path, strerror(errno));
             eina_stringshare_del(path);
             *cfg.certificate_authorities = eina_list_remove_list(*cfg.certificate_authorities, n);
             continue;
          }
        else if (S_ISDIR(st.st_mode)) cadir = path;
        else cafile = path;

        if (SSL_CTX_load_verify_locations(ctx->ssl_ctx, cafile, cadir) != 1)
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p could not use certificate authorities from %s [%s:%d%s%s '%s']",
                 ctx, path,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             eina_stringshare_del(path);
             *cfg.certificate_authorities = eina_list_remove_list(*cfg.certificate_authorities, n);
             continue;
          }

        DBG("ssl_ctx=%p loaded certificate authorities '%s'", ctx, path);
     }
   if (certificate_authorities_count && !*cfg.certificate_authorities)
     {
        ERR("ssl_ctx=%p none of the required certificate authorities were loaded!", ctx);
        return EINVAL;
     }

   if (!ctx->did_certificates)
     {
        if (!SSL_CTX_set_cipher_list(ctx->ssl_ctx, EFL_NET_SSL_CONTEXT_CIPHERS))
          {
             err_ssl = ERR_peek_error_line_data(&err_file, &err_line, &err_data, &err_flags);
             _efl_net_ssl_ctx_check_errors();

             ERR("ssl_ctx=%p Could not set ciphers '%s' [%s:%d%s%s '%s']",
                 ctx, EFL_NET_SSL_CONTEXT_CIPHERS,
                 err_file, err_line,
                 (err_flags & ERR_TXT_STRING) ? " " : "",
                 (err_flags & ERR_TXT_STRING) ? err_data : "",
                 ERR_reason_error_string(err_ssl));
             return EINVAL;
          }
     }

   return 0;
}

static void *
efl_net_ssl_ctx_connection_new(Efl_Net_Ssl_Ctx *ctx)
{
   return SSL_new(ctx->ssl_ctx);
}

static Eina_Error
efl_net_ssl_ctx_setup(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Ctx_Config cfg)
{
   Eina_Error err;
   unsigned long options;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(ctx->ssl_ctx != NULL, EALREADY);

   ctx->is_dialer = cfg.is_dialer;
   if (ctx->is_dialer)
     {
        switch (cfg.cipher)
          {
           case EFL_NET_SSL_CIPHER_AUTO:
              ctx->ssl_ctx = SSL_CTX_new(TLS_client_method());
              break;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
           case EFL_NET_SSL_CIPHER_TLSV1:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_client_method());
              break;
           case EFL_NET_SSL_CIPHER_TLSV1_1:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_1_client_method());
              break;
           case EFL_NET_SSL_CIPHER_TLSV1_2:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_2_client_method());
              break;
#else
           case EFL_NET_SSL_CIPHER_TLSV1:
           case EFL_NET_SSL_CIPHER_TLSV1_1:
           case EFL_NET_SSL_CIPHER_TLSV1_2:
              ctx->ssl_ctx = SSL_CTX_new(TLS_client_method());
              break;
#endif
           default:
              ERR("ssl_ctx=%p unsupported cipher %d", ctx, cfg.cipher);
              return EINVAL;
          }

        EINA_SAFETY_ON_NULL_RETURN_VAL(ctx->ssl_ctx, ENOSYS);
     }
   else
     {
        switch (cfg.cipher)
          {
           case EFL_NET_SSL_CIPHER_AUTO:
              ctx->ssl_ctx = SSL_CTX_new(TLS_server_method());
              break;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
           case EFL_NET_SSL_CIPHER_TLSV1:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_server_method());
              break;
           case EFL_NET_SSL_CIPHER_TLSV1_1:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_1_server_method());
              break;
           case EFL_NET_SSL_CIPHER_TLSV1_2:
              ctx->ssl_ctx = SSL_CTX_new(TLSv1_2_server_method());
              break;
#else
           case EFL_NET_SSL_CIPHER_TLSV1:
           case EFL_NET_SSL_CIPHER_TLSV1_1:
           case EFL_NET_SSL_CIPHER_TLSV1_2:
              ctx->ssl_ctx = SSL_CTX_new(TLS_server_method());
              break;
#endif
           default:
              ERR("ssl_ctx=%p unsupported cipher %d", ctx, cfg.cipher);
              return EINVAL;
          }

        EINA_SAFETY_ON_NULL_RETURN_VAL(ctx->ssl_ctx, ENOSYS);
     }

   options = SSL_CTX_get_options(ctx->ssl_ctx);
   options |= SSL_OP_NO_SSLv2;
   options |= SSL_OP_SINGLE_DH_USE;
   options |= SSL_OP_NO_SSLv3;

   SSL_CTX_set_options(ctx->ssl_ctx, options);

   err = _efl_net_ssl_ctx_load_lists(ctx, cfg);
   if (err)
     {
        ERR("ssl_ctx=%p failed to load certificate, private keys, CRL or CA", ctx);
        goto error;
     }

   return 0;

 error:
   SSL_CTX_free(ctx->ssl_ctx);
   ctx->ssl_ctx = NULL;
   return err;
}

static void
efl_net_ssl_ctx_teardown(Efl_Net_Ssl_Ctx *ctx)
{
   if (ctx->ssl_ctx)
     {
        SSL_CTX_free(ctx->ssl_ctx);
        ctx->ssl_ctx = NULL;
     }
}

static Eina_Error
efl_net_ssl_ctx_verify_mode_set(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Verify_Mode verify_mode)
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

   SSL_CTX_set_verify(ctx->ssl_ctx, ssl_mode, SSL_CTX_get_verify_callback(ctx->ssl_ctx));
   return 0;
}

static Eina_Error
efl_net_ssl_ctx_hostname_verify_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, Eina_Bool hostname_verify EINA_UNUSED)
{
   return 0;
}

static Eina_Error
efl_net_ssl_ctx_hostname_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, const char *hostname EINA_UNUSED)
{
   return 0;
}
