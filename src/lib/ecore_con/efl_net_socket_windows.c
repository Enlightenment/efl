#define EFL_NET_SOCKET_WINDOWS_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#error "This version of Windows is too old"
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define MY_CLASS EFL_NET_SOCKET_WINDOWS_CLASS

#define BUFFER_SIZE (4 * 4096)

typedef struct _Efl_Net_Socket_Windows_Data
{
   Eina_Stringshare *address_local;
   Eina_Stringshare *address_remote;
   Eina_List *pending_ops;
   struct {
      union {
         uint8_t *bytes;
         void *mem;
      };
      DWORD len;
      DWORD used;
      Eina_Bool pending;
   } recv;
   struct {
      union {
         uint8_t *bytes;
         void *mem;
      };
      DWORD len;
      DWORD used;
      Eina_Bool pending;
   } send;
   HANDLE handle;
   PTP_IO io;
   Eina_Bool can_read;
   Eina_Bool eos;
   Eina_Bool pending_eos;
   Eina_Bool can_write;
   Eina_Bool io_started;
} Efl_Net_Socket_Windows_Data;

struct _Efl_Net_Socket_Windows_Operation
{
   OVERLAPPED base;
   Efl_Net_Socket_Windows_Operation_Success_Cb success_cb;
   Efl_Net_Socket_Windows_Operation_Failure_Cb failure_cb;
   const void *data;
   Eo *o;
   Eina_Bool deleting;
};

Efl_Net_Socket_Windows_Operation *
_efl_net_socket_windows_operation_new(Eo *o, Efl_Net_Socket_Windows_Operation_Success_Cb success_cb, Efl_Net_Socket_Windows_Operation_Failure_Cb failure_cb, const void *data)
{
   Efl_Net_Socket_Windows_Operation *op;
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, EFL_NET_SOCKET_WINDOWS_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(success_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(failure_cb, NULL);

   op = calloc(1, sizeof(*op));
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, NULL);

   op->success_cb = success_cb;
   op->failure_cb = failure_cb;
   op->data = data;
   op->o = o;
   pd->pending_ops = eina_list_append(pd->pending_ops, op);
   StartThreadpoolIo(pd->io);

   // TODO vtorri: I'm not sure the same pd->io can be used for concurrent
   // operations, like read + write. If it's not, then we'll have to
   // CreateThreadpoolIo() for each operation (op->io)...

   DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p",
       op, op->o, op->success_cb, op->failure_cb, op->data);

   return op;
}

static void
_efl_net_socket_windows_operation_done(Efl_Net_Socket_Windows_Operation *op, Eina_Error err, Eina_Rw_Slice slice)
{
   Efl_Net_Socket_Windows_Data *pd;

   DBG("op=%p (socket=%p), success_cb=%p, failure_cb=%p, data=%p, err=%d (%s), slice=" EINA_SLICE_FMT,
       op, op->o, op->success_cb, op->failure_cb, op->data,
       err, eina_error_msg_get(err), EINA_SLICE_PRINT(slice));

   op->deleting = EINA_TRUE;

   pd = efl_data_scope_get(op->o, EFL_NET_SOCKET_WINDOWS_CLASS);
   if (pd)
     {
        pd->pending_ops = eina_list_remove(pd->pending_ops, op);
        CancelThreadpoolIo(pd->io);
     }

   if (err)
     op->failure_cb((void *)op->data, op->o, err);
   else
     op->success_cb((void *)op->data, op->o, slice);

   free(op);
}

void
_efl_net_socket_windows_operation_failed(Efl_Net_Socket_Windows_Operation *op, Eina_Error err)
{
   EINA_SAFETY_ON_NULL_RETURN(op);
   EINA_SAFETY_ON_TRUE_RETURN(op->deleting);

   _efl_net_socket_windows_operation_done(op, err, (Eina_Rw_Slice){});
}

void
_efl_net_socket_windows_operation_succeeded(Efl_Net_Socket_Windows_Operation *op, Eina_Rw_Slice slice)
{
   EINA_SAFETY_ON_NULL_RETURN(op);
   EINA_SAFETY_ON_TRUE_RETURN(op->deleting);

   _efl_net_socket_windows_operation_done(op, 0, slice);
}

static void CALLBACK
_efl_net_socket_windows_io_completed(PTP_CALLBACK_INSTANCE inst EINA_UNUSED,
                                     PVOID data,
                                     PVOID overlapped,
                                     ULONG result,
                                     ULONG_PTR bytes_nbr,
                                     PTP_IO io EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Efl_Net_Socket_Windows_Operation *op = overlapped;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   // TODO: check if this happens on the main thread, otherwise migrate to main!

   if (result == NO_ERROR)
     _efl_net_socket_windows_operation_succeeded(op, (Eina_Rw_Slice){.len = bytes_nbr}); // TODO k-s: slice with actual memory
   else
     _efl_net_socket_windows_operation_failed(op, result);
}

Eina_Error
_efl_net_socket_windows_init(Eo *o, HANDLE h)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(h == INVALID_HANDLE_VALUE, EINVAL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->handle != INVALID_HANDLE_VALUE, EALREADY);

   pd->io = CreateThreadpoolIo(h, _efl_net_socket_windows_io_completed, o, 0);
   if (!pd->io)
     return GetLastError(); // TODO vtorri: is this compatible with errno/strerror()?

   pd->handle = h;

   DBG("socket=%p adopted handle=%p, ThreadpoolIo=%p", o, h, pd->io);
   return 0;
}

static Eina_Error _efl_net_socket_windows_recv(Eo *o, Efl_Net_Socket_Windows_Data *pd);

static void
_efl_net_socket_windows_recv_success(void *data EINA_UNUSED, Eo *o, Eina_Rw_Slice slice)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->recv.used += slice.len;
   pd->recv.pending = EINA_FALSE;

   efl_io_reader_can_read_set(o, pd->recv.used > 0);

   if (pd->handle == INVALID_HANDLE_VALUE) return;
   if (pd->recv.used == pd->recv.len) return;

   _efl_net_socket_windows_recv(o, pd);
}

static void
_efl_net_socket_windows_recv_failure(void *data EINA_UNUSED, Eo *o, Eina_Error err)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   // TODO k-s
   ERR("TODO: handle err=%d (%s)", err, eina_error_msg_get(err));
   pd->recv.pending = EINA_FALSE;
   pd->pending_eos = EINA_TRUE; /* eos when buffer drains */
}

static Eina_Error
_efl_net_socket_windows_recv(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   Efl_Net_Socket_Windows_Operation *op;
   OVERLAPPED *ovl;
   DWORD used_size = 0;

   if (pd->handle == INVALID_HANDLE_VALUE) return EBADF;
   if (pd->recv.len == 0) return ENOMEM;
   if (pd->recv.used == pd->recv.len) return ENOSPC;

   op = _efl_net_socket_windows_operation_new(o,
                                              _efl_net_socket_windows_recv_success,
                                              _efl_net_socket_windows_recv_failure,
                                              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);

   ovl = _efl_net_socket_windows_operation_overlapped_get(op);

   if (!ReadFile(pd->handle,
                 pd->recv.bytes + pd->recv.used,
                 pd->recv.len - pd->recv.used,
                 &used_size, ovl))
     {
        Eina_Error err = GetLastError();
        if (err == ERROR_IO_PENDING)
          {
             pd->recv.pending = EINA_TRUE;
             return 0;
          }
        else
          {
             _efl_net_socket_windows_operation_failed(op, err);
             return err;
          }
     }

   _efl_net_socket_windows_operation_succeeded(op, (Eina_Rw_Slice){
        .mem = pd->recv.mem, .len = pd->recv.used + used_size});
   return 0;
}

static Eina_Error _efl_net_socket_windows_send(Eo *o, Efl_Net_Socket_Windows_Data *pd);

static void
_efl_net_socket_windows_send_success(void *data EINA_UNUSED, Eo *o, Eina_Rw_Slice slice)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   if (slice.len)
     memmove(pd->send.bytes, pd->send.bytes + slice.len, pd->send.used - slice.len);

   pd->send.used -= slice.len;
   pd->send.pending = EINA_FALSE;

   efl_io_writer_can_write_set(o, pd->send.used < pd->send.len);

   if (pd->handle == INVALID_HANDLE_VALUE) return;
   if (pd->send.used == 0) return;

   _efl_net_socket_windows_send(o, pd);
}

static void
_efl_net_socket_windows_send_failure(void *data EINA_UNUSED, Eo *o, Eina_Error err)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   // TODO k-s
   ERR("TODO: handle err=%d (%s)", err, eina_error_msg_get(err));
   pd->send.pending = EINA_FALSE;
}

static Eina_Error
_efl_net_socket_windows_send(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   Efl_Net_Socket_Windows_Operation *op;
   OVERLAPPED *ovl;
   DWORD used_size = 0;

   if (pd->handle == INVALID_HANDLE_VALUE) return EBADF;
   if (pd->send.used == 0) return EINVAL;

   op = _efl_net_socket_windows_operation_new(o,
                                              _efl_net_socket_windows_send_success,
                                              _efl_net_socket_windows_send_failure,
                                              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);

   ovl = _efl_net_socket_windows_operation_overlapped_get(op);

   if (!WriteFile(pd->handle,
                  pd->send.bytes,
                  pd->send.used,
                  &used_size, ovl))
     {
        Eina_Error err = GetLastError();
        if (err == ERROR_IO_PENDING)
          {
             pd->send.pending = EINA_TRUE;
             return 0;
          }
        else
          {
             _efl_net_socket_windows_operation_failed(op, err);
             return err;
          }
     }

   _efl_net_socket_windows_operation_succeeded(op, (Eina_Rw_Slice){
        .mem = pd->send.mem, .len = used_size});
   return 0;
}


Eina_Error
_efl_net_socket_windows_io_start(Eo *o)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->io_started, EALREADY);

   if (!pd->recv.mem)
     {
        pd->recv.mem = malloc(BUFFER_SIZE);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->recv.mem, ENOMEM);
        pd->recv.len = BUFFER_SIZE;
        pd->recv.used = 0;
     }

   if (!pd->send.mem)
     {
        pd->send.mem = malloc(BUFFER_SIZE);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->send.mem, ENOMEM);
        pd->send.len = BUFFER_SIZE;
        pd->send.used = 0;
     }

   DBG("socket=%p starting I/O...", o);
   err = _efl_net_socket_windows_recv(o, pd);
   if (err) return err;

   pd->io_started = EINA_TRUE;
   return 0;
}

HANDLE
_efl_net_socket_windows_handle_get(const Eo *o)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, INVALID_HANDLE_VALUE);
   return pd->handle;
}

EOLIAN static Eo *
_efl_net_socket_windows_efl_object_constructor(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   pd->handle = INVALID_HANDLE_VALUE;

   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_socket_windows_efl_object_destructor(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   while (pd->pending_ops)
     _efl_net_socket_windows_operation_failed(pd->pending_ops->data, ECANCELED);

   if (efl_io_closer_close_on_destructor_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_local, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);

   free(pd->recv.mem);
   pd->recv.mem = NULL;
   pd->recv.len = 0;
   pd->recv.used = 0;

   free(pd->send.mem);
   pd->send.mem = NULL;
   pd->send.len = 0;
   pd->send.used = 0;
}

EOLIAN static Eina_Error
_efl_net_socket_windows_efl_io_closer_close(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   HANDLE h;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);

   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);

   if (pd->io)
     {
        CloseThreadpoolIo(pd->io);
        pd->io = NULL;
     }

   h = InterlockedExchangePointer(&pd->handle, INVALID_HANDLE_VALUE);
   if (h != INVALID_HANDLE_VALUE)
     CloseHandle(h);

   return 0;
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_closer_closed_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->handle == INVALID_HANDLE_VALUE;
}

EOLIAN static Eina_Error
_efl_net_socket_windows_efl_io_reader_read(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Slice ro_slice;
   DWORD remaining;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);

   ro_slice.len = pd->recv.used;
   if (ro_slice.len == 0)
     {
        rw_slice->len = 0;
        if (pd->pending_eos)
          {
             efl_io_reader_eos_set(o, EINA_TRUE);
             efl_io_closer_close(o);
          }
        return EAGAIN;
     }
   ro_slice.bytes = pd->recv.bytes;

   *rw_slice = eina_rw_slice_copy(*rw_slice, ro_slice);

   remaining = pd->recv.used - rw_slice->len;
   if (remaining)
     memmove(pd->recv.bytes, pd->recv.bytes + rw_slice->len, remaining);

   pd->recv.used = remaining;
   efl_io_reader_can_read_set(o, remaining > 0);

   if ((pd->pending_eos) && (remaining == 0))
     {
        efl_io_reader_eos_set(o, EINA_TRUE);
        efl_io_closer_close(o);
        return 0;
     }

   if ((!pd->recv.pending) && (pd->recv.used < pd->recv.len))
     {
        DBG("recv %lu bytes more from socket=%p", pd->recv.len - pd->recv.used, o);
        return _efl_net_socket_windows_recv(o, pd);
     }

   return 0;
}

EOLIAN static void
_efl_net_socket_windows_efl_io_reader_can_read_set(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o) && can_read);
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_reader_can_read_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_net_socket_windows_efl_io_reader_eos_set(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Bool is_eos)
{
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos)
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_reader_eos_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->eos;
}

EOLIAN static Eina_Error
_efl_net_socket_windows_efl_io_writer_write(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   Eina_Error err = EINVAL;
   DWORD available, todo;

   EINA_SAFETY_ON_NULL_RETURN_VAL(slice, EINVAL);
   EINA_SAFETY_ON_NULL_GOTO(slice->mem, error);
   EINA_SAFETY_ON_TRUE_GOTO(efl_io_closer_closed_get(o), error);
   err = ENOMEM;
   EINA_SAFETY_ON_TRUE_GOTO(pd->send.mem != NULL, error);

   if (pd->send.len <= pd->send.used)
     {
        efl_io_writer_can_write_set(o, EINA_FALSE);
        return EAGAIN;
     }

   available = pd->send.len - pd->send.used;
   if (slice->len < available)
     todo = slice->len;
   else
     todo = available;

   memcpy(pd->send.bytes + pd->send.used, slice->mem, todo);
   if (remaining)
     {
        remaining->len = slice->len - todo;
        remaining->bytes = slice->bytes + todo;
     }
   slice->len = todo;

   efl_io_writer_can_write_set(o, pd->send.used < pd->send.len);

   if ((!pd->send.pending) && (pd->send.used > 0))
     {
        DBG("send %lu bytes more to socket=%p", pd->send.used, o);
        return _efl_net_socket_windows_send(o, pd);
     }

   return 0;

 error:
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   return err;
}

EOLIAN static void
_efl_net_socket_windows_efl_io_writer_can_write_set(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o) && can_write);
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_writer_can_write_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_net_socket_windows_efl_net_socket_address_local_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_local, address);
}

EOLIAN static const char *
_efl_net_socket_windows_efl_net_socket_address_local_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->address_local;
}

EOLIAN static void
_efl_net_socket_windows_efl_net_socket_address_remote_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_remote, address);
}

EOLIAN static const char *
_efl_net_socket_windows_efl_net_socket_address_remote_get(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->address_remote;
}

#include "efl_net_socket_windows.eo.c"
