#define EFL_NET_SERVER_UDP_CLIENT_PROTECTED 1
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

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef _WIN32
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_SERVER_UDP_CLIENT_CLASS

typedef struct _Efl_Net_Server_Udp_Client_Packet
{
   EINA_INLIST;
   Eina_Rw_Slice slice;
} Efl_Net_Server_Udp_Client_Packet;

static void
_efl_net_server_udp_client_packet_free(Efl_Net_Server_Udp_Client_Packet *pkt)
{
   free(pkt->slice.mem);
   free(pkt);
}

typedef struct _Efl_Net_Server_Udp_Client_Data
{
   Eina_Stringshare *address_local;
   Eina_Stringshare *address_remote;
   Eina_Inlist *packets;
   struct sockaddr *addr_remote;
   socklen_t addr_remote_len;
   SOCKET fd;
   Eina_Bool close_on_invalidate;
   Eina_Bool eos;
   Eina_Bool can_read;
   Eina_Bool can_write;
} Efl_Net_Server_Udp_Client_Data;

EOLIAN static Efl_Object *
_efl_net_server_udp_client_efl_object_finalize(Eo *o, Efl_Net_Server_Udp_Client_Data *pd)
{
   o = efl_finalize(efl_super(o, MY_CLASS));
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->addr_remote, NULL);

   return o;
}

static void
_efl_net_server_udp_client_cleanup(Efl_Net_Server_Udp_Client_Data *pd)
{
   Efl_Net_Server_Udp_Client_Packet *pkt;

   pd->fd = INVALID_SOCKET;
   EINA_INLIST_FREE(pd->packets, pkt)
     {
        pd->packets = eina_inlist_remove(pd->packets, EINA_INLIST_GET(pkt));
        _efl_net_server_udp_client_packet_free(pkt);
     }
}

EOLIAN static void
_efl_net_server_udp_client_efl_object_invalidate(Eo *o, Efl_Net_Server_Udp_Client_Data *pd EINA_UNUSED)
{
   if (efl_io_closer_close_on_invalidate_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_udp_client_efl_object_destructor(Eo *o, Efl_Net_Server_Udp_Client_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   _efl_net_server_udp_client_cleanup(pd);

   eina_stringshare_replace(&pd->address_local, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);
   free(pd->addr_remote);
   pd->addr_remote = NULL;
   pd->addr_remote_len = 0;
}

void
_efl_net_server_udp_client_init(Eo *o, SOCKET fd, const struct sockaddr *addr, socklen_t addrlen, const char *str)
{
   Efl_Net_Server_Udp_Client_Data *pd = efl_data_scope_get(o, MY_CLASS);
   pd->fd = fd;
   pd->addr_remote = malloc(addrlen);
   EINA_SAFETY_ON_NULL_RETURN(pd->addr_remote);
   memcpy(pd->addr_remote, addr, addrlen);
   pd->addr_remote_len = addrlen;
   efl_net_socket_address_remote_set(o, str);
}

void
_efl_net_server_udp_client_feed(Eo *o, Eina_Rw_Slice slice)
{
   Efl_Net_Server_Udp_Client_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Efl_Net_Server_Udp_Client_Packet *pkt;

   pkt = malloc(sizeof(Efl_Net_Server_Udp_Client_Packet));
   EINA_SAFETY_ON_NULL_GOTO(pkt, error);
   pkt->slice = slice;
   pd->packets = eina_inlist_append(pd->packets, EINA_INLIST_GET(pkt));

   efl_io_reader_can_read_set(o, EINA_TRUE);
   return;

 error:
   free(slice.mem);
}

EOLIAN static size_t
_efl_net_server_udp_client_next_datagram_size_query(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   Efl_Net_Server_Udp_Client_Packet *pkt;

   if (!pd->packets) return 0;
   pkt = EINA_INLIST_CONTAINER_GET(pd->packets, Efl_Net_Server_Udp_Client_Packet);
   return pkt->slice.len;
}

EOLIAN static Eina_Error
_efl_net_server_udp_client_efl_io_closer_close(Eo *o, Efl_Net_Server_Udp_Client_Data *pd)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);

   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);

   _efl_net_server_udp_client_cleanup(pd);

   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);

   return 0;
}

EOLIAN static Eina_Error
_efl_net_server_udp_client_efl_io_reader_read(Eo *o, Efl_Net_Server_Udp_Client_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Efl_Net_Server_Udp_Client_Packet *pkt;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);

   if (!pd->packets)
     {
        rw_slice->len = 0;
        rw_slice->mem = NULL;
        efl_io_reader_can_read_set(o, EINA_FALSE);
        return EAGAIN;
     }

   pkt = EINA_INLIST_CONTAINER_GET(pd->packets, Efl_Net_Server_Udp_Client_Packet);
   pd->packets = eina_inlist_remove(pd->packets, pd->packets);

   *rw_slice = eina_rw_slice_copy(*rw_slice, eina_rw_slice_slice_get(pkt->slice));
   _efl_net_server_udp_client_packet_free(pkt);
   efl_io_reader_can_read_set(o, !!pd->packets);

   return 0;
}

EOLIAN static Eina_Error
_efl_net_server_udp_client_efl_io_writer_write(Eo *o, Efl_Net_Server_Udp_Client_Data *pd, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ro_slice, EINVAL);
   if (pd->fd == INVALID_SOCKET) goto error;

   do
     {
        r = sendto(pd->fd, ro_slice->mem, ro_slice->len, 0, pd->addr_remote, pd->addr_remote_len);
        if (r < 0)
          {
             Eina_Error err = efl_net_socket_error_get();

             if (err == EINTR) continue;

             if (remaining) *remaining = *ro_slice;
             ro_slice->len = 0;
             ro_slice->mem = NULL;
             efl_io_writer_can_write_set(o, EINA_FALSE);
             return err;
          }
     }
   while (r < 0);

   if (remaining)
     {
        remaining->len = ro_slice->len - r;
        remaining->bytes = ro_slice->bytes + r;
     }
   ro_slice->len = r;

   return 0;

 error:
   if (remaining) *remaining = *ro_slice;
   ro_slice->len = 0;
   ro_slice->mem = NULL;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_net_server_udp_client_efl_io_reader_can_read_set(Eo *o, Efl_Net_Server_Udp_Client_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->fd == INVALID_SOCKET);
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->eos;
}

EOLIAN static void
_efl_net_server_udp_client_efl_io_reader_eos_set(Eo *o, Efl_Net_Server_Udp_Client_Data *pd, Eina_Bool is_eos)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->fd == INVALID_SOCKET);
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos)
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_net_server_udp_client_efl_io_writer_can_write_set(Eo *o, Efl_Net_Server_Udp_Client_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->fd == INVALID_SOCKET);
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_closer_closed_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->fd == INVALID_SOCKET;
}

EOLIAN static void
_efl_net_server_udp_client_efl_io_closer_close_on_invalidate_set(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd, Eina_Bool close_on_invalidate)
{
   pd->close_on_invalidate = close_on_invalidate;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_closer_close_on_invalidate_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->close_on_invalidate;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_closer_close_on_exec_set(Eo *o, Efl_Net_Server_Udp_Client_Data *pd EINA_UNUSED, Eina_Bool close_on_exec)
{
   if (efl_net_server_fd_close_on_exec_get(efl_parent_get(o)) != close_on_exec)
     {
        ERR("Efl.Net.Server.Udp.Client close-on-exec must be the same as the server setting, no file descriptor is created for each client!");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_client_efl_io_closer_close_on_exec_get(const Eo *o, Efl_Net_Server_Udp_Client_Data *pd EINA_UNUSED)
{
   return efl_net_server_fd_close_on_exec_get(efl_parent_get(o));
}

EOLIAN static void
_efl_net_server_udp_client_efl_net_socket_address_local_set(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_local, address);
}

EOLIAN static const char *
_efl_net_server_udp_client_efl_net_socket_address_local_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->address_local;
}

EOLIAN static void
_efl_net_server_udp_client_efl_net_socket_address_remote_set(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_remote, address);
}

EOLIAN static const char *
_efl_net_server_udp_client_efl_net_socket_address_remote_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Client_Data *pd)
{
   return pd->address_remote;
}

#include "efl_net_server_udp_client.eo.c"
