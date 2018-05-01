#define EFL_NET_SOCKET_SSL_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct _Efl_Net_Ssl_Conn Efl_Net_Ssl_Conn;

/**
 * Setups the SSL context
 *
 * Update the given lists, removing invalid entries. If all entries
 * failed in a list, return EINVAL.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_setup(Efl_Net_Ssl_Conn *conn, Eina_Bool is_dialer, Efl_Net_Socket *sock, Efl_Net_Ssl_Context *context);

/**
 * Cleans up the SSL associated to this context.
 * @internal
 */
static void efl_net_ssl_conn_teardown(Efl_Net_Ssl_Conn *conn);

/**
 * Send data to remote peer.
 *
 * This should be called once handshake is finished, otherwise it may
 * lead the handshake to fail.
 *
 * @param slice[inout] takes the amount of bytes to write and
 *        source memory, will store written length in slice->len.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_write(Efl_Net_Ssl_Conn *conn, Eina_Slice *slice);

/**
 * Receive data from remote peer.
 *
 * This should be called once handshake is finished, otherwise it may
 * lead the handshake to fail.
 *
 * Note that even if the socket 'can_read', eventually it couldn't
 * decipher a byte and it will return slice->len == 0 with EAGAIN as
 * error.
 *
 * @param slice[inout] takes the amount of bytes to read and
 *        destination memory, will store read length in slice->len.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_read(Efl_Net_Ssl_Conn *conn, Eina_Rw_Slice *slice);

/**
 * Attempt to finish the handshake.
 *
 * This should not block, if it's not finished yet, just set done =
 * false.
 *
 * Errors, such as failed handshake, should be returned as Eina_Error.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_handshake(Efl_Net_Ssl_Conn *conn, Eina_Bool *done);

/**
 * Configure how to verify peer.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_verify_mode_set(Efl_Net_Ssl_Conn *conn, Efl_Net_Ssl_Verify_Mode verify_mode);

/**
 * Configure whenever to check for hostname.
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_hostname_verify_set(Efl_Net_Ssl_Conn *conn, Eina_Bool hostname_verify);

/**
 * Overrides the hostname to use.
 *
 * @note duplicate hostname if needed!
 *
 * @internal
 */
static Eina_Error efl_net_ssl_conn_hostname_override_set(Efl_Net_Ssl_Conn *conn, const char *hostname);

#if HAVE_OPENSSL
#include "efl_net_ssl_conn-openssl.c"
#elif HAVE_GNUTLS
#include "efl_net_ssl_conn-gnutls.c"
#else
#include "efl_net_ssl_conn-none.c"
#endif

#define MY_CLASS EFL_NET_SOCKET_SSL_CLASS

typedef struct _Efl_Net_Socket_Ssl_Data
{
   Eo *sock;
   Efl_Net_Ssl_Context *context;
   const char *hostname_override;
   Efl_Net_Ssl_Conn ssl_conn;
   Efl_Net_Ssl_Verify_Mode verify_mode;
   Eina_Bool hostname_verify;
   Eina_Bool did_handshake;
   Eina_Bool torndown;
   Eina_Bool can_read;
   Eina_Bool eos;
   Eina_Bool can_write;
} Efl_Net_Socket_Ssl_Data;

static void
efl_net_socket_ssl_sock_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   efl_io_reader_eos_set(o, EINA_TRUE);
}

static void
efl_net_socket_ssl_handshake_try(Eo *o, Efl_Net_Socket_Ssl_Data *pd)
{
   Eina_Error err;

   if (pd->torndown) return;

   DBG("SSL=%p handshake...", o);

   err = efl_net_ssl_conn_handshake(&pd->ssl_conn, &pd->did_handshake);
   if (err)
     {
        WRN("SSL=%p failed handshake: %s", o, eina_error_msg_get(err));
        efl_event_callback_call(o, EFL_NET_SOCKET_SSL_EVENT_SSL_ERROR, &err);
        efl_io_closer_close(o);
        return;
     }
   if (!pd->did_handshake) return;

   DBG("SSL=%p finished handshake", o);
   efl_io_reader_can_read_set(o, efl_io_reader_can_read_get(pd->sock));
   efl_io_writer_can_write_set(o, efl_io_writer_can_write_get(pd->sock));

   efl_event_callback_call(o, EFL_NET_SOCKET_SSL_EVENT_SSL_READY, NULL);
}

static void
efl_net_socket_ssl_sock_can_read_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (!efl_io_reader_can_read_get(pd->sock))
     goto end;

   if (pd->did_handshake)
     efl_io_reader_can_read_set(o, EINA_TRUE);
   else
     efl_net_socket_ssl_handshake_try(o, pd);

 end:
   efl_unref(o);
}

static void
efl_net_socket_ssl_sock_can_write_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (!efl_io_writer_can_write_get(pd->sock))
     goto end;

   if (pd->did_handshake)
     efl_io_writer_can_write_set(o, EINA_TRUE);
   else
     efl_net_socket_ssl_handshake_try(o, pd);

 end:
   efl_unref(o);
}

static void
efl_net_socket_ssl_sock_closed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
}

static void
efl_net_socket_ssl_sock_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);
   pd->sock = NULL;
   pd->torndown = EINA_TRUE;
   efl_net_ssl_conn_teardown(&pd->ssl_conn);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_net_socket_ssl_sock_cbs,
                           {EFL_IO_READER_EVENT_EOS, efl_net_socket_ssl_sock_eos},
                           {EFL_IO_READER_EVENT_CAN_READ_CHANGED, efl_net_socket_ssl_sock_can_read_changed},
                           {EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, efl_net_socket_ssl_sock_can_write_changed},
                           {EFL_IO_CLOSER_EVENT_CLOSED, efl_net_socket_ssl_sock_closed},
                           {EFL_EVENT_DEL, efl_net_socket_ssl_sock_del});

static void
efl_net_socket_ssl_sock_resolved(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);

   if (pd->torndown) return;

   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
}

static void
efl_net_socket_ssl_sock_connected(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err;

   if (pd->torndown) return;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   err = efl_net_ssl_conn_handshake(&pd->ssl_conn, &pd->did_handshake);
   if (err)
     {
        WRN("SSL=%p failed handshake: %s", o, eina_error_msg_get(err));
        efl_io_closer_close(o);
     }

   efl_unref(o);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_net_socket_ssl_sock_dialer_cbs,
                           {EFL_NET_DIALER_EVENT_RESOLVED, efl_net_socket_ssl_sock_resolved},
                           {EFL_NET_DIALER_EVENT_CONNECTED, efl_net_socket_ssl_sock_connected});

static void
_efl_net_socket_ssl_context_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);
   pd->context = NULL;
}

EOLIAN static void
_efl_net_socket_ssl_adopt(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Efl_Net_Socket *sock, Efl_Net_Ssl_Context *context)
{
   Eina_Error err;
   char *tmp = NULL;
   const char *hostname;
   Eina_Bool is_dialer;

   EINA_SAFETY_ON_TRUE_RETURN(pd->sock != NULL);
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(sock, EFL_NET_SOCKET_INTERFACE));
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(context, EFL_NET_SSL_CONTEXT_CLASS));
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));

   is_dialer = efl_isa(sock, EFL_NET_DIALER_INTERFACE);
   err = efl_net_ssl_conn_setup(&pd->ssl_conn, is_dialer, sock, context);
   if (err)
     {
        ERR("ssl=%p failed to adopt socket (is_dialer=%d) sock=%p", o, is_dialer, sock);
        return;
     }

   pd->context = efl_ref(context);
   efl_event_callback_add(context, EFL_EVENT_DEL, _efl_net_socket_ssl_context_del, o);

   DBG("ssl=%p adopted socket (is_dialer=%d) sock=%p, ssl_conn=%p", o, is_dialer, sock, &pd->ssl_conn);

   if (pd->hostname_verify == 0xff)
     pd->hostname_verify = efl_net_ssl_context_hostname_verify_get(context);

   hostname = pd->hostname_override;
   if (!hostname)
     hostname = pd->hostname_override = eina_stringshare_ref(efl_net_ssl_context_hostname_get(context));
   if (!hostname)
     {
        const char *remote_address = (is_dialer ?
                                      efl_net_dialer_address_dial_get(sock) :
                                      efl_net_socket_address_remote_get(sock));
        if (remote_address)
          {
             const char *host, *port;

             tmp = strdup(remote_address);
             EINA_SAFETY_ON_NULL_RETURN(tmp);
             if (efl_net_ip_port_split(tmp, &host, &port))
               hostname = host;
          }
     }

   if ((uint8_t)pd->verify_mode == 0xff) pd->verify_mode = efl_net_ssl_context_verify_mode_get(context);
   efl_net_ssl_conn_verify_mode_set(&pd->ssl_conn, pd->verify_mode);
   efl_net_ssl_conn_hostname_verify_set(&pd->ssl_conn, pd->hostname_verify);
   efl_net_ssl_conn_hostname_override_set(&pd->ssl_conn, hostname);
   free(tmp);

   pd->sock = efl_ref(sock);
   efl_event_callback_array_add(sock, efl_net_socket_ssl_sock_cbs(), o);

   if (efl_isa(sock, EFL_NET_DIALER_INTERFACE))
     efl_event_callback_array_add(sock, efl_net_socket_ssl_sock_dialer_cbs(), o);

   efl_net_socket_ssl_sock_can_read_changed(o, NULL);
   efl_net_socket_ssl_sock_can_write_changed(o, NULL);
   if (efl_io_closer_closed_get(sock))
     efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_adopted_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, Efl_Net_Socket **sock, Efl_Net_Ssl_Context **context)
{
   if (sock) *sock = pd->sock;
   if (context) *context = pd->context;
   return !!pd->sock;
}

static Efl_Net_Ssl_Verify_Mode
_efl_net_socket_ssl_verify_mode_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->verify_mode;
}

static void
_efl_net_socket_ssl_verify_mode_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, Efl_Net_Ssl_Verify_Mode verify_mode)
{
   pd->verify_mode = verify_mode;
   if (!efl_finalized_get(o)) return;
   EINA_SAFETY_ON_TRUE_RETURN(pd->torndown);

   efl_net_ssl_conn_verify_mode_set(&pd->ssl_conn, pd->verify_mode);
}

static Eina_Bool
_efl_net_socket_ssl_hostname_verify_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->hostname_verify;
}

static void
_efl_net_socket_ssl_hostname_verify_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool hostname_verify)
{
   pd->hostname_verify = hostname_verify;
   if (!efl_finalized_get(o)) return;
   EINA_SAFETY_ON_TRUE_RETURN(pd->torndown);

   efl_net_ssl_conn_hostname_verify_set(&pd->ssl_conn, pd->hostname_verify);
}

static const char *
_efl_net_socket_ssl_hostname_override_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->hostname_override;
}

static void
_efl_net_socket_ssl_hostname_override_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, const char* hostname_override)
{
   char *tmp = NULL;
   const char *hostname;

   eina_stringshare_replace(&pd->hostname_override, hostname_override);

   if (!efl_finalized_get(o)) return;
   EINA_SAFETY_ON_TRUE_RETURN(pd->torndown);

   hostname = pd->hostname_override;
   if (!hostname)
     {
        const char *remote_address = (efl_isa(pd->sock, EFL_NET_DIALER_INTERFACE) ?
                                      efl_net_dialer_address_dial_get(pd->sock) :
                                      efl_net_socket_address_remote_get(pd->sock));
        if (remote_address)
          {
             const char *host, *port;

             tmp = strdup(remote_address);
             EINA_SAFETY_ON_NULL_RETURN(tmp);
             if (efl_net_ip_port_split(tmp, &host, &port))
               hostname = host;
          }
     }

   efl_net_ssl_conn_hostname_override_set(&pd->ssl_conn, hostname);
   free(tmp);
}

EOLIAN static Efl_Object *
_efl_net_socket_ssl_efl_object_finalize(Eo *o, Efl_Net_Socket_Ssl_Data *pd EINA_UNUSED)
{
   Eina_Error err;

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   if (!pd->sock)
     {
        ERR("no Efl.Net.Socket was adopted by this SSL=%p", o);
        return NULL;
     }

   if (efl_isa(pd->sock, EFL_NET_DIALER_INTERFACE))
     {
        if (!efl_net_dialer_connected_get(pd->sock))
          return o;
     }

   err = efl_net_ssl_conn_handshake(&pd->ssl_conn, &pd->did_handshake);
   if (err)
     {
        WRN("SSL=%p failed handshake", o);
        return NULL;
     }

   return o;
}

EOLIAN static Eo *
_efl_net_socket_ssl_efl_object_constructor(Eo *o, Efl_Net_Socket_Ssl_Data *pd)
{
   pd->hostname_verify = 0xff;
   pd->verify_mode = 0xff;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_socket_ssl_efl_object_invalidate(Eo *o, Efl_Net_Socket_Ssl_Data *pd EINA_UNUSED)
{
   if (efl_io_closer_close_on_invalidate_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   pd->torndown = EINA_TRUE;
   efl_net_ssl_conn_teardown(&pd->ssl_conn);
   if (pd->sock)
     {
        efl_event_callback_array_del(pd->sock, efl_net_socket_ssl_sock_cbs(), o);
        if (efl_isa(pd->sock, EFL_NET_DIALER_INTERFACE))
          efl_event_callback_array_del(pd->sock, efl_net_socket_ssl_sock_dialer_cbs(), o);
        efl_unref(pd->sock);
        pd->sock = NULL;
     }

   if (pd->context)
     {
        efl_event_callback_del(pd->context, EFL_EVENT_DEL, _efl_net_socket_ssl_context_del, o);
        efl_unref(pd->context);
        pd->context = NULL;
     }

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_socket_ssl_efl_object_destructor(Eo *o, Efl_Net_Socket_Ssl_Data *pd)
{
   eina_stringshare_replace(&pd->hostname_override, NULL);

   efl_destructor(efl_super(o, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_net_socket_ssl_efl_io_closer_close(Eo *o, Efl_Net_Socket_Ssl_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->sock, EBADF);

   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);
   pd->torndown = EINA_TRUE;
   efl_net_ssl_conn_teardown(&pd->ssl_conn);
   if (efl_io_closer_closed_get(pd->sock))
     return 0;
   return efl_io_closer_close(pd->sock);
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_closer_closed_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return (!pd->sock) || efl_io_closer_closed_get(pd->sock);
}

EOLIAN static Eina_Error
_efl_net_socket_ssl_efl_io_reader_read(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->torndown, EBADF);

   if (!pd->did_handshake)
     {
        rw_slice->mem = NULL;
        rw_slice->len = 0;
        return EAGAIN;
     }

   err = efl_net_ssl_conn_read(&pd->ssl_conn, rw_slice);

   if (rw_slice->len == 0)
     {
        efl_io_reader_can_read_set(o, EINA_FALSE);
        if (err == 0)
          efl_io_reader_eos_set(o, EINA_TRUE);
     }

   return err;
}

EOLIAN static void
_efl_net_socket_ssl_efl_io_reader_can_read_set(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->sock);
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->can_read;
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->eos;
}

EOLIAN static void
_efl_net_socket_ssl_efl_io_reader_eos_set(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool is_eos)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->sock);
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos)
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Error
_efl_net_socket_ssl_efl_io_writer_write(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ro_slice, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->torndown, EBADF);

   if (!pd->did_handshake)
     {
        if (remaining) *remaining = *ro_slice;
        ro_slice->mem = NULL;
        ro_slice->len = 0;
        return EAGAIN;
     }

   if (remaining) *remaining = *ro_slice;

   err = efl_net_ssl_conn_write(&pd->ssl_conn, ro_slice);

   if (remaining)
     {
        remaining->bytes += ro_slice->len;
        remaining->len -= ro_slice->len;
     }

   if (ro_slice->len == 0)
     efl_io_writer_can_write_set(o, EINA_FALSE);

   return err;
}

EOLIAN static void
_efl_net_socket_ssl_efl_io_writer_can_write_set(Eo *o, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->sock);
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->can_write;
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool close_on_exec)
{
   if (pd->sock) efl_io_closer_close_on_exec_set(pd->sock, close_on_exec);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_closer_close_on_exec_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->sock && efl_io_closer_close_on_exec_get(pd->sock);
}

EOLIAN static void
_efl_net_socket_ssl_efl_io_closer_close_on_invalidate_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd, Eina_Bool close_on_invalidate)
{
   if (pd->sock) efl_io_closer_close_on_invalidate_set(pd->sock, close_on_invalidate);
}

EOLIAN static Eina_Bool
_efl_net_socket_ssl_efl_io_closer_close_on_invalidate_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   return pd->sock && efl_io_closer_close_on_invalidate_get(pd->sock);
}

EOLIAN static const char *
_efl_net_socket_ssl_efl_net_socket_address_local_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   if (!pd->sock) return "unbound";
   return efl_net_socket_address_local_get(pd->sock);
}

EOLIAN static const char *
_efl_net_socket_ssl_efl_net_socket_address_remote_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Ssl_Data *pd)
{
   if (!pd->sock) return "unbound";
   return efl_net_socket_address_remote_get(pd->sock);
}

#include "efl_net_socket_ssl.eo.c"
