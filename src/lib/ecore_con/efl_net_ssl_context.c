#define EFL_NET_SSL_CONTEXT_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#include "Emile.h"

/**
 * This function is used by efl_net_socket_ssl to retrieve a new
 * connection based on the implementation-depentent context.
 *
 * @internal
 */
void *efl_net_ssl_context_connection_new(Efl_Net_Ssl_Context *context);

typedef struct _Efl_Net_Ssl_Ctx Efl_Net_Ssl_Ctx;

typedef struct _Efl_Net_Ssl_Ctx_Config {
   Efl_Net_Ssl_Cipher cipher;
   Eina_Bool is_dialer;
   Eina_Bool load_defaults;
   Eina_List **certificates;
   Eina_List **private_keys;
   Eina_List **certificate_revocation_lists;
   Eina_List **certificate_authorities;
} Efl_Net_Ssl_Ctx_Config;

/**
 * Returns the platform dependent context to efl_net_socket_ssl
 * wrapper.
 *
 * @internal
 */
static void *efl_net_ssl_ctx_connection_new(Efl_Net_Ssl_Ctx *ctx);

/**
 * Setups the SSL context
 *
 * Update the given lists, removing invalid entries. If all entries
 * failed in a list, return EINVAL.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_ctx_setup(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Ctx_Config cfg);

/**
 * Cleans up the SSL associated to this context.
 * @internal
 */
static void efl_net_ssl_ctx_teardown(Efl_Net_Ssl_Ctx *ctx);

/**
 * Configure how to verify peer.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_ctx_verify_mode_set(Efl_Net_Ssl_Ctx *ctx, Efl_Net_Ssl_Verify_Mode verify_mode);

/**
 * Configure whenever to check for hostname.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_ctx_hostname_verify_set(Efl_Net_Ssl_Ctx *ctx, Eina_Bool hostname_verify);

/**
 * Configure the hostname to use.
 *
 * @note duplicate hostname if needed!
 *
 * @internal
 */
static Eina_Error efl_net_ssl_ctx_hostname_set(Efl_Net_Ssl_Ctx *ctx, const char *hostname);

#if HAVE_OPENSSL
#include "efl_net_ssl_ctx-openssl.c"
#elif HAVE_GNUTLS
#include "efl_net_ssl_ctx-gnutls.c"
#else
#include "efl_net_ssl_ctx-none.c"
#endif

#define MY_CLASS EFL_NET_SSL_CONTEXT_CLASS

typedef struct _Efl_Net_Ssl_Context_Data
{
   Efl_Net_Ssl_Ctx ssl_ctx;
   Eina_List *certificates;
   Eina_List *private_keys;
   Eina_List *certificate_revocation_lists;
   Eina_List *certificate_authorities;
   const char *hostname;
   Efl_Net_Ssl_Cipher cipher;
   Eina_Bool is_dialer;
   Efl_Net_Ssl_Verify_Mode verify_mode;
   Eina_Bool load_defaults;
   Eina_Bool hostname_verify;
   Eina_Bool did_handshake;
   Eina_Bool can_read;
   Eina_Bool eos;
   Eina_Bool can_write;
} Efl_Net_Ssl_Context_Data;


void *
efl_net_ssl_context_connection_new(Efl_Net_Ssl_Context *context)
{
   Efl_Net_Ssl_Context_Data *pd = efl_data_scope_get(context, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   return efl_net_ssl_ctx_connection_new(&pd->ssl_ctx);
}

EOLIAN static void
_efl_net_ssl_context_setup(Eo *o, Efl_Net_Ssl_Context_Data *pd, Efl_Net_Ssl_Cipher cipher, Eina_Bool is_dialer)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_TRUE_RETURN(cipher > EFL_NET_SSL_CIPHER_TLSV1_2);

   pd->cipher = cipher;
   pd->is_dialer = is_dialer;
}

static Eina_List *
_efl_net_ssl_context_string_iter_to_list(Eina_Iterator *it)
{
   Eina_List *lst = NULL;
   const char *str;
   EINA_ITERATOR_FOREACH(it, str)
     {
        if (!str) continue;
        lst = eina_list_append(lst, eina_stringshare_add(str));
     }
   eina_iterator_free(it);
   return lst;
}

static void
_efl_net_ssl_context_string_list_free(Eina_List **p_lst)
{
   const char *str;
   EINA_LIST_FREE(*p_lst, str)
     eina_stringshare_del(str);
}

static Eina_Iterator *
_efl_net_ssl_context_certificates_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return eina_list_iterator_new(pd->certificates);
}

static void
_efl_net_ssl_context_certificates_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Iterator *it)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   _efl_net_ssl_context_string_list_free(&pd->certificates);
   pd->certificates = _efl_net_ssl_context_string_iter_to_list(it);
}

static Eina_Iterator *
_efl_net_ssl_context_private_keys_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return eina_list_iterator_new(pd->private_keys);
}

static void
_efl_net_ssl_context_private_keys_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Iterator *it)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   _efl_net_ssl_context_string_list_free(&pd->private_keys);
   pd->private_keys = _efl_net_ssl_context_string_iter_to_list(it);
}

static Eina_Iterator *
_efl_net_ssl_context_certificate_revocation_lists_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return eina_list_iterator_new(pd->certificate_revocation_lists);
}

static void
_efl_net_ssl_context_certificate_revocation_lists_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Iterator *it)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   _efl_net_ssl_context_string_list_free(&pd->certificate_revocation_lists);
   pd->certificate_revocation_lists = _efl_net_ssl_context_string_iter_to_list(it);
}

static Eina_Iterator *
_efl_net_ssl_context_certificate_authorities_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return eina_list_iterator_new(pd->certificate_authorities);
}

static void
_efl_net_ssl_context_certificate_authorities_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Iterator *it)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   _efl_net_ssl_context_string_list_free(&pd->certificate_authorities);
   pd->certificate_authorities = _efl_net_ssl_context_string_iter_to_list(it);
}

static Eina_Bool
_efl_net_ssl_context_default_paths_load_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return pd->load_defaults;
}

static void
_efl_net_ssl_context_default_paths_load_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Bool load_defaults)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   pd->load_defaults = load_defaults;
}

static Efl_Net_Ssl_Verify_Mode
_efl_net_ssl_context_verify_mode_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return pd->verify_mode;
}

static void
_efl_net_ssl_context_verify_mode_set(Eo *o, Efl_Net_Ssl_Context_Data *pd, Efl_Net_Ssl_Verify_Mode verify_mode)
{
   pd->verify_mode = verify_mode;
   if (!efl_finalized_get(o)) return;

   efl_net_ssl_ctx_verify_mode_set(&pd->ssl_ctx, pd->verify_mode);
}

static Eina_Bool
_efl_net_ssl_context_hostname_verify_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return pd->hostname_verify;
}

static void
_efl_net_ssl_context_hostname_verify_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, Eina_Bool hostname_verify)
{
   pd->hostname_verify = hostname_verify;
   if (!efl_finalized_get(o)) return;

   efl_net_ssl_ctx_hostname_verify_set(&pd->ssl_ctx, pd->hostname_verify);
}

static const char *
_efl_net_ssl_context_hostname_get(const Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd)
{
   return pd->hostname;
}

static void
_efl_net_ssl_context_hostname_set(Eo *o EINA_UNUSED, Efl_Net_Ssl_Context_Data *pd, const char* hostname)
{
   eina_stringshare_replace(&pd->hostname, hostname);
   if (!efl_finalized_get(o)) return;

   efl_net_ssl_ctx_hostname_set(&pd->ssl_ctx, pd->hostname);
}

EOLIAN static Efl_Object *
_efl_net_ssl_context_efl_object_finalize(Eo *o, Efl_Net_Ssl_Context_Data *pd)
{
   Eina_Error err;
   Efl_Net_Ssl_Ctx_Config cfg;

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   if (!emile_cipher_init())
     {
        ERR("could not initialize cipher subsystem.");
        return NULL;
     }

   if (pd->is_dialer)
     {
        if ((uint8_t)pd->verify_mode == 0xff)
          pd->verify_mode = EFL_NET_SSL_VERIFY_MODE_REQUIRED;
        if (pd->hostname_verify == 0xff)
          pd->hostname_verify = EINA_TRUE;
        if (pd->load_defaults == 0xff)
          pd->load_defaults = EINA_TRUE;
     }
   else
     {
        cfg.is_dialer = EINA_FALSE;
        if ((uint8_t)pd->verify_mode == 0xff)
          pd->verify_mode = EFL_NET_SSL_VERIFY_MODE_NONE;
        if (pd->hostname_verify == 0xff)
          pd->hostname_verify = EINA_FALSE;
        if (pd->load_defaults == 0xff)
          pd->load_defaults = EINA_FALSE;
     }

   cfg.cipher = pd->cipher;
   cfg.is_dialer = pd->is_dialer;
   cfg.load_defaults = pd->load_defaults;
   cfg.certificates = &pd->certificates;
   cfg.private_keys = &pd->private_keys;
   cfg.certificate_revocation_lists = &pd->certificate_revocation_lists;
   cfg.certificate_authorities = &pd->certificate_authorities;

   err = efl_net_ssl_ctx_setup(&pd->ssl_ctx, cfg);
   if (err)
     {
        ERR("o=%p failed to setup context (is_dialer=%d)", o, cfg.is_dialer);
        return NULL;
     }
   DBG("o=%p setup context (is_dialer=%d) ssl_ctx=%p", o, cfg.is_dialer, &pd->ssl_ctx);

   efl_net_ssl_ctx_verify_mode_set(&pd->ssl_ctx, pd->verify_mode);
   efl_net_ssl_ctx_hostname_verify_set(&pd->ssl_ctx, pd->hostname_verify);
   efl_net_ssl_ctx_hostname_set(&pd->ssl_ctx, pd->hostname);

   return o;
}

EOLIAN static Eo *
_efl_net_ssl_context_efl_object_constructor(Eo *o, Efl_Net_Ssl_Context_Data *pd)
{
   pd->cipher = EFL_NET_SSL_CIPHER_AUTO;
   pd->is_dialer = EINA_TRUE;
   pd->load_defaults = 0xff;
   pd->hostname_verify = 0xff;
   pd->verify_mode = 0xff;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_ssl_context_efl_object_destructor(Eo *o, Efl_Net_Ssl_Context_Data *pd)
{
   efl_net_ssl_ctx_teardown(&pd->ssl_ctx);

   _efl_net_ssl_context_string_list_free(&pd->certificates);
   _efl_net_ssl_context_string_list_free(&pd->private_keys);
   _efl_net_ssl_context_string_list_free(&pd->certificate_revocation_lists);
   _efl_net_ssl_context_string_list_free(&pd->certificate_authorities);

   eina_stringshare_replace(&pd->hostname, NULL);

   efl_destructor(efl_super(o, MY_CLASS));
}

static Efl_Net_Ssl_Context *_efl_net_ssl_context_default_dialer = NULL;

static void
_efl_net_ssl_context_default_dialer_del(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   _efl_net_ssl_context_default_dialer = NULL;
}

EOLIAN static Efl_Net_Ssl_Context *
_efl_net_ssl_context_default_dialer_get(void)
{
   if (!_efl_net_ssl_context_default_dialer)
     {
        _efl_net_ssl_context_default_dialer = efl_add(EFL_NET_SSL_CONTEXT_CLASS, efl_main_loop_get(),
                                                      efl_net_ssl_context_verify_mode_set(efl_added, EFL_NET_SSL_VERIFY_MODE_REQUIRED),
                                                      efl_net_ssl_context_hostname_verify_set(efl_added, EINA_TRUE),
                                                      efl_net_ssl_context_default_paths_load_set(efl_added, EINA_TRUE),
                                                      efl_net_ssl_context_setup(efl_added, EFL_NET_SSL_CIPHER_AUTO, EINA_TRUE));
        efl_event_callback_add(_efl_net_ssl_context_default_dialer,
                               EFL_EVENT_DEL,
                               _efl_net_ssl_context_default_dialer_del,
                               NULL);
     }
   return _efl_net_ssl_context_default_dialer;
}

#include "efl_net_ssl_context.eo.c"
