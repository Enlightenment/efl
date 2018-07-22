#include <gnutls/gnutls.h>

struct _Efl_Net_Ssl_Ctx {
   gnutls_certificate_credentials_t x509_cred;
   gnutls_priority_t priority;
   Eina_Bool is_dialer;
};

static Eina_Error
_efl_net_ssl_ctx_load_lists(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Ctx_Config cfg)
{
   Eina_List *n, *n_next, *pk_node;
   const char *path;
   unsigned certificates_count = eina_list_count(*cfg.certificates);
   unsigned private_keys_count = eina_list_count(*cfg.private_keys);
   unsigned certificate_revocation_lists_count = eina_list_count(*cfg.certificate_revocation_lists);
   unsigned certificate_authorities_count = eina_list_count(*cfg.certificate_authorities);
   int r;

   ctx->is_dialer = cfg.is_dialer;

   if (cfg.load_defaults)
     {
        r = gnutls_certificate_set_x509_system_trust(ctx->x509_cred);
        if (r < 0)
          {
             ERR("ssl_ctx=%p could not load default paths: %s", ctx, gnutls_strerror(r));
             return ENOSYS;
          }
        DBG("ssl_ctx=%p loaded default paths", ctx);
     }
   else
     DBG("ssl_ctx=%p did not load default paths", ctx);

   /* GNUTLS needs certificate-key pairs, so we do:
    *
    *  - if no private keys, use certificate as its own key;
    *
    *  - if a private keys, walk the list alongside certificates, but
    *    do NOT delete elements if list sizes are different. Stop at
    *    last private key, allowing a single private key for multiple
    *    certificates.
    */
   pk_node = *cfg.private_keys;
   EINA_LIST_FOREACH_SAFE(*cfg.certificates, n, n_next, path)
     {
        const char *key = pk_node ? pk_node->data : path;

        r = gnutls_certificate_set_x509_key_file(ctx->x509_cred, path, key, GNUTLS_X509_FMT_PEM);
        if (r < 0)
          {
             ERR("ssl_ctx=%p could not use certificate from '%s' with key '%s': %s",
                 ctx, path, key, gnutls_strerror(r));

             if (pk_node)
               {
                  if (eina_list_count(*cfg.private_keys) == eina_list_count(*cfg.certificates))
                    {
                       pk_node = pk_node->next;
                       eina_stringshare_del(key);
                       *cfg.private_keys = eina_list_remove_list(*cfg.private_keys, pk_node->prev);
                    }
                  else if (pk_node->next) pk_node = pk_node->next;
               }

             eina_stringshare_del(path);
             *cfg.certificates = eina_list_remove_list(*cfg.certificates, n);
             continue;
          }
        else
          {
             if (pk_node->next) pk_node = pk_node->next;
          }

        DBG("ssl_ctx=%p loaded certificate '%s' with key '%s'", ctx, path, key);
     }
   if (certificates_count && !*cfg.certificates)
     {
        ERR("ssl_ctx=%p none of the required certificates were loaded!", ctx);
        return EINVAL;
     }

   if (private_keys_count && !*cfg.private_keys)
     {
        ERR("ssl_ctx=%p none of the required private keys were loaded!", ctx);
        return EINVAL;
     }
   else if (pk_node != eina_list_last(*cfg.private_keys))
     {
        do
          {
             n = pk_node->next;
             path = n->data;
             ERR("ssl_ctx=%p extra private key is unused '%s'", ctx, path);
             eina_stringshare_del(path);
             *cfg.private_keys = eina_list_remove_list(*cfg.private_keys, n);
          }
        while (pk_node->next);
     }

   EINA_LIST_FOREACH_SAFE(*cfg.certificate_revocation_lists, n, n_next, path)
     {
        r = gnutls_certificate_set_x509_crl_file(ctx->x509_cred, path, GNUTLS_X509_FMT_PEM);
        if (r < 0)
          {
             ERR("ssl_ctx=%p could not use certificate revocation lists from %s: %s",
                 ctx, path, gnutls_strerror(r));
             eina_stringshare_del(path);
             *cfg.certificate_revocation_lists = eina_list_remove_list(*cfg.certificate_revocation_lists, n);
             continue;
          }

        DBG("ssl_ctx=%p loaded certificate revocation lists '%s'", ctx, path);
     }
   if (certificate_revocation_lists_count && !*cfg.certificate_revocation_lists)
     {
        ERR("ssl_ctx=%p none of the required certificate revocation lists were loaded!", ctx);
        return EINVAL;
     }

   EINA_LIST_FOREACH_SAFE(*cfg.certificate_authorities, n, n_next, path)
     {
        struct stat st;

        r = 0;
        if (stat(path, &st) != 0)
          {
             ERR("ssl_ctx=%p could not load certificate authorities from '%s': %s", ctx, path, strerror(errno));
             eina_stringshare_del(path);
             *cfg.certificate_authorities = eina_list_remove_list(*cfg.certificate_authorities, n);
             continue;
          }
        else if (S_ISDIR(st.st_mode))
          r = gnutls_certificate_set_x509_trust_dir(ctx->x509_cred, path, GNUTLS_X509_FMT_PEM);
        else
          r = gnutls_certificate_set_x509_trust_file(ctx->x509_cred, path, GNUTLS_X509_FMT_PEM);

        if (r < 0)
          {
             ERR("ssl_ctx=%p could not use certificate authorities from '%s': %s", ctx, path, gnutls_strerror(r));
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

   return 0;
}

static void *
efl_net_ssl_ctx_connection_new(Efl_Net_Ssl_Ctx *ctx)
{
   gnutls_session_t session;
   int r;

   r = gnutls_init(&session, ctx->is_dialer ? GNUTLS_CLIENT : GNUTLS_SERVER);
   if (r < 0)
     {
        ERR("ssl_ctx=%p could not create %s session: %s",
            ctx, ctx->is_dialer ? "dialer" : "server", gnutls_strerror(r));
        return NULL;
     }

   if (!ctx->priority)
     {
        r = gnutls_set_default_priority(session);
        if (r < 0)
          {
             ERR("ssl_ctx=%p could not set default cipher priority: %s", ctx, gnutls_strerror(r));
             goto error;
          }
     }
   else
     {
        r = gnutls_priority_set(session, ctx->priority);
        if (r < 0)
          {
             ERR("ssl_ctx=%p could not set cipher priority: %s", ctx, gnutls_strerror(r));
             goto error;
          }
     }

   r = gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, ctx->x509_cred);
   if (r < 0)
     {
        ERR("ssl_ctx=%p could not set session credentials: %s", ctx, gnutls_strerror(r));
        goto error;
     }

   return session;

 error:
   gnutls_deinit(session);
   return NULL;
}

static Eina_Error
efl_net_ssl_ctx_setup(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Ctx_Config cfg)
{
   Eina_Error err;
   const char *priority;
   int r;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(ctx->x509_cred != NULL, EALREADY);

   switch (cfg.cipher)
     {
      case EFL_NET_SSL_CIPHER_AUTO:
         priority = NULL;
         break;
      case EFL_NET_SSL_CIPHER_TLSV1:
         priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0!VERS-TLS1.1:!VERS-TLS1.2";
         break;
      case EFL_NET_SSL_CIPHER_TLSV1_1:
         priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0:!VERS-TLS1.0:!VERS-TLS1.2";
         break;
      case EFL_NET_SSL_CIPHER_TLSV1_2:
         priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0:!VERS-TLS1.0:!VERS-TLS1.1";
         break;
      default:
         ERR("ssl_ctx=%p unsupported cipher %d", ctx, cfg.cipher);
         return EINVAL;
     }

   if (priority)
     {
        const char *err_pos = NULL;
        r = gnutls_priority_init(&ctx->priority, priority, &err_pos);
        if (r < 0)
          {
             size_t off = err_pos - priority;
             if (r == GNUTLS_E_INVALID_REQUEST)
               {
                  ERR("ssl_ctx=%p invalid syntax on GNUTLS priority string offset %zd: '%s'", ctx, off, priority);
                  return EINVAL;
               }
             ERR("ssl_ctx=%p could not set GNUTLS priority offset %zd '%s': %s", ctx, off, priority, gnutls_strerror(r));
             return EINVAL;
          }
     }

   r = gnutls_certificate_allocate_credentials(&ctx->x509_cred);
   if (r < 0)
     {
        ERR("ssl_ctx=%p could not allocate X509 credentials: %s", ctx, gnutls_strerror(r));
        err = ENOSYS;
        goto err_cert_alloc;
     }

   err = _efl_net_ssl_ctx_load_lists(ctx, cfg);
   if (err)
     {
        ERR("ssl_ctx=%p failed to load certificate, private keys, CRL or CA", ctx);
        goto err_load;
     }

   return 0;

 err_load:
   gnutls_certificate_free_credentials(ctx->x509_cred);
   ctx->x509_cred = NULL;
 err_cert_alloc:
   gnutls_priority_deinit(ctx->priority);
   ctx->priority = NULL;
   return err;
}

static void
efl_net_ssl_ctx_teardown(Efl_Net_Ssl_Ctx *ctx)
{
   if (ctx->x509_cred)
     {
        gnutls_certificate_free_credentials(ctx->x509_cred);
        ctx->x509_cred = NULL;
     }

   if (ctx->priority)
     {
        gnutls_priority_deinit(ctx->priority);
        ctx->priority = NULL;
     }
}

static Eina_Error
efl_net_ssl_ctx_verify_mode_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, Efl_Net_Ssl_Verify_Mode verify_mode EINA_UNUSED)
{
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
