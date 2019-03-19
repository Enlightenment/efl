#define EFL_NET_SOCKET_WINDOWS_PROTECTED 1
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
      DWORD base;
      Efl_Net_Socket_Windows_Operation *pending;
   } recv;
   struct {
      union {
         uint8_t *bytes;
         void *mem;
      };
      DWORD len;
      DWORD used;
      Efl_Net_Socket_Windows_Operation *pending;
   } send;
   HANDLE handle;
   Eina_Bool can_read;
   Eina_Bool eos;
   Eina_Bool pending_eos;
   Eina_Bool can_write;
   Eina_Bool io_started;
   Eina_Bool close_on_exec;
   Eina_Bool close_on_invalidate;
} Efl_Net_Socket_Windows_Data;

struct _Efl_Net_Socket_Windows_Operation
{
   OVERLAPPED base;
   Efl_Net_Socket_Windows_Operation_Success_Cb success_cb;
   Efl_Net_Socket_Windows_Operation_Failure_Cb failure_cb;
   const void *data;
   Ecore_Win32_Handler *event_handler;
   Eo *o;
   Eina_Bool deleting;
};

/*
 * differences to evil_format_message():
 *  - shorter string
 *  - no newline
 *  - fallback to error code if format fails
 */
char *
_efl_net_windows_error_msg_get(DWORD win32err)
{
   LPTSTR msg;
   char  *str;
   char  *disp;
   int len, reqlen;

   if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      win32err,
                      0, /* Default language */
                      (LPTSTR)&msg,
                      0,
                      NULL))
     goto fallback;

#ifdef UNICODE
   str = evil_wchar_to_char(msg);
#else
   str = msg;
#endif /* UNICODE */

   len = strlen(str);
   if ((len > 0) && (str[len - 1] == '\n'))
     str[--len] = '\0';
   if ((len > 0) && (str[len - 1] == '\r'))
     str[--len] = '\0';
   if ((len > 0) && (str[len - 1] == '.'))
     str[--len] = '\0';

   reqlen = snprintf("", 0, "%lu(%s)", win32err, str);
   if (reqlen < 1) goto error;
   disp = malloc(reqlen + 1);
   if (!disp) goto error;
   snprintf(disp, reqlen + 1, "%lu(%s)", win32err, str);

#ifdef UNICODE
   free(str);
#endif /* UNICODE */
   LocalFree(msg);

   return disp;

 error:
#ifdef UNICODE
   free(str);
#endif /* UNICODE */
   LocalFree(msg);
 fallback:
   {
      char buf[64];
      snprintf(buf, sizeof(buf), "%ld", win32err);
      return strdup(buf);
   }
}

static void
_efl_net_socket_windows_handle_close(HANDLE h)
{
   if (!FlushFileBuffers(h))
     {
        DWORD win32err = GetLastError();
        if (win32err != ERROR_PIPE_NOT_CONNECTED)
          {
             char *msg = _efl_net_windows_error_msg_get(GetLastError());
             WRN("HANDLE=%p could not flush buffers: %s", h, msg);
             free(msg);
          }
     }
   if (!DisconnectNamedPipe(h))
     {
        DWORD win32err = GetLastError();
        if ((win32err != ERROR_NOT_SUPPORTED) && /* dialer socket don't support it */
            (win32err != ERROR_PIPE_NOT_CONNECTED))
          {
             char *msg = _efl_net_windows_error_msg_get(win32err);
             WRN("HANDLE=%p could not disconnect: %s", h, msg);
             free(msg);
          }
     }
   CloseHandle(h);
   DBG("HANDLE=%p closed", h);
}

static Eina_Bool _efl_net_socket_windows_operation_event(void *, Ecore_Win32_Handler *wh);

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

   op->base.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!op->base.hEvent)
     {
        char *msg = _efl_net_windows_error_msg_get(GetLastError());
        ERR("socket=%p success_cb=%p failure_cb=%p data=%p: error=%s",
            op->o, op->success_cb, op->failure_cb, op->data, msg);
        free(msg);
        goto error_event;
     }

   op->event_handler = ecore_main_win32_handler_add(op->base.hEvent, _efl_net_socket_windows_operation_event, op);
   if (!op->event_handler)
     {
        ERR("socket=%p success_cb=%p failure_cb=%p data=%p: could not create event handler",
            op->o, op->success_cb, op->failure_cb, op->data);
        goto error_handler;
     }

   op->success_cb = success_cb;
   op->failure_cb = failure_cb;
   op->data = data;
   op->o = o;
   pd->pending_ops = eina_list_append(pd->pending_ops, op);

   DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p",
       op, op->o, op->success_cb, op->failure_cb, op->data);

   return op;

 error_handler:
   CloseHandle(op->base.hEvent);
 error_event:
   free(op);
   return NULL;
}

static Eina_Error
_efl_net_socket_windows_operation_done(Efl_Net_Socket_Windows_Operation *op, DWORD win32err, DWORD used_size)
{
   Efl_Net_Socket_Windows_Data *pd;
   Eina_Error err = 0;

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p error=%s used_size=%lu",
            op, op->o, op->success_cb, op->failure_cb, op->data, msg, used_size);
        free(msg);
     }

   op->deleting = EINA_TRUE;

   efl_ref(op->o);
   pd = efl_data_scope_get(op->o, EFL_NET_SOCKET_WINDOWS_CLASS);
   if (pd)
     pd->pending_ops = eina_list_remove(pd->pending_ops, op);

   if (win32err)
     err = op->failure_cb((void *)op->data, op->o, win32err);
   else
     op->success_cb((void *)op->data, op->o, used_size);

   if (op->event_handler)
     {
        if (WaitForSingleObject(pd->handle, 0) != WAIT_OBJECT_0)
          {
             DWORD used_size = 0;
             if (GetOverlappedResult(pd->handle, &op->base, &used_size, FALSE))
               {
                  DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, FALSE)",
                      op, op->o, op->success_cb, op->failure_cb, op->data, pd->handle, &op->base, used_size);
               }
             else
               {
                  win32err = GetLastError();
                  if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
                    {
                       char *msg = _efl_net_windows_error_msg_get(win32err);
                       DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, TRUE)=%s",
                           op, op->o, op->success_cb, op->failure_cb, op->data, pd->handle, &op->base, used_size, msg);
                       free(msg);
                    }

                  if (win32err == ERROR_IO_INCOMPLETE)
                    {
                       DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p: still pending I/O...",
                           op, op->o, op->success_cb, op->failure_cb, op->data);
                       efl_unref(op->o);
                       op->o = NULL;
                       return 0;
                    }
               }
          }

        ecore_main_win32_handler_del(op->event_handler);
     }

#ifndef ERROR_HANDLES_CLOSED
#define ERROR_HANDLES_CLOSED 676
#endif
   if ((win32err == ERROR_HANDLES_CLOSED) && !efl_io_closer_closed_get(op->o))
     efl_io_closer_close(op->o);
   efl_unref(op->o);

   CloseHandle(op->base.hEvent);
   free(op);
   return err;
}

Eina_Error
_efl_net_socket_windows_operation_failed(Efl_Net_Socket_Windows_Operation *op, DWORD win32err)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(op->deleting, EINVAL);

   return _efl_net_socket_windows_operation_done(op, win32err, 0);
}

Eina_Error
_efl_net_socket_windows_operation_succeeded(Efl_Net_Socket_Windows_Operation *op, DWORD used_size)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(op->deleting, EINVAL);

   return _efl_net_socket_windows_operation_done(op, 0, used_size);
}

static Eina_Bool
_efl_net_socket_windows_operation_event(void *data, Ecore_Win32_Handler *wh EINA_UNUSED)
{
   Efl_Net_Socket_Windows_Operation *op = data;
   HANDLE h = _efl_net_socket_windows_handle_get(op->o);
   DWORD used_size = 0;

   if ((op->deleting) || (h == INVALID_HANDLE_VALUE))
     {
        DBG("op=%p was deleted and pending I/O completed!", op);
        CloseHandle(op->base.hEvent);
        free(op);
        return ECORE_CALLBACK_CANCEL;
     }

   op->event_handler = NULL;

   if (GetOverlappedResult(h, &op->base, &used_size, FALSE))
     {
        DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, FALSE)",
            op, op->o, op->success_cb, op->failure_cb, op->data, h, &op->base, used_size);
        _efl_net_socket_windows_operation_succeeded(op, used_size);
     }
   else
     {
        DWORD win32err = GetLastError();
        if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
          {
             char *msg = _efl_net_windows_error_msg_get(win32err);
             DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, FALSE)=%s",
                 op, op->o, op->success_cb, op->failure_cb, op->data, h, &op->base, used_size, msg);
             free(msg);
          }

        _efl_net_socket_windows_operation_failed(op, win32err);
     }

   return ECORE_CALLBACK_CANCEL;
}

Eina_Error
_efl_net_socket_windows_init(Eo *o, HANDLE h)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(h == INVALID_HANDLE_VALUE, EINVAL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->handle != INVALID_HANDLE_VALUE, EALREADY);

   pd->handle = h;

   DBG("socket=%p adopted handle=%p", o, h);
   return 0;
}

static Eina_Error _efl_net_socket_windows_recv(Eo *o, Efl_Net_Socket_Windows_Data *pd);

static Eina_Error
_efl_net_socket_windows_recv_success(void *data EINA_UNUSED, Eo *o, DWORD used_size)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);

   pd->recv.pending = NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->recv.used + used_size > pd->recv.len, EINVAL);

   pd->recv.used += used_size;

   /* calls back the user, may read()/write()/close() */
   efl_io_reader_can_read_set(o, pd->recv.used > 0);

   if (pd->handle == INVALID_HANDLE_VALUE) return 0;
   if (pd->recv.used == pd->recv.len) return 0;
   if (pd->recv.pending) return 0;

   return _efl_net_socket_windows_recv(o, pd);
}

static Eina_Error
_efl_net_socket_windows_recv_failure(void *data EINA_UNUSED, Eo *o, DWORD win32err)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("socket=%p recv error=%s", o, msg);
        free(msg);
     }

   pd->recv.pending = NULL;

   if (pd->recv.used > 0)
     pd->pending_eos = EINA_TRUE; /* eos when buffer drains */
   else
     {
        efl_io_writer_can_write_set(o, EINA_FALSE);
        efl_io_reader_can_read_set(o, EINA_FALSE);
        efl_io_reader_eos_set(o, EINA_TRUE);
     }

   switch (win32err)
     {
      case ERROR_INVALID_USER_BUFFER: return EFAULT;
      case ERROR_NOT_ENOUGH_MEMORY: return ENOMEM;
      case ERROR_OPERATION_ABORTED: return ECANCELED;
      case ERROR_BROKEN_PIPE: return EPIPE;
      default: return EIO;
     }
}

static Eina_Error
_efl_net_socket_windows_recv(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   Efl_Net_Socket_Windows_Operation *op;
   OVERLAPPED *ovl;
   DWORD used_size = 0, win32err;
   BOOL r;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->recv.pending != NULL, EINPROGRESS);

   if (pd->handle == INVALID_HANDLE_VALUE) return EBADF;
   if (pd->recv.len == 0) return ENOMEM;

   if (pd->recv.base > 0)
     {
        DWORD todo = pd->recv.used - pd->recv.base;
        if (todo > 0)
          memmove(pd->recv.bytes, pd->recv.bytes + pd->recv.base, todo);
        pd->recv.used -= pd->recv.base;
        pd->recv.base = 0;
     }

   if (pd->recv.used == pd->recv.len) return ENOSPC;

   op = _efl_net_socket_windows_operation_new(o,
                                              _efl_net_socket_windows_recv_success,
                                              _efl_net_socket_windows_recv_failure,
                                              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);
   ovl = _efl_net_socket_windows_operation_overlapped_get(op);
   r = ReadFile(pd->handle,
                pd->recv.bytes + pd->recv.used,
                pd->recv.len - pd->recv.used,
                &used_size, ovl);
   win32err = GetLastError();
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("socket=%p ReadFile(%p, %p, %lu, &size=%lu, %p)=%s",
            o, pd->handle, pd->recv.bytes + pd->recv.used,
            pd->recv.len - pd->recv.used, used_size, ovl, msg);
        free(msg);
     }

   if (!r)
     {
        if (win32err == ERROR_IO_PENDING)
          {
             pd->recv.pending = op;
             return 0;
          }
        else
          {
             return _efl_net_socket_windows_operation_failed(op, win32err);
          }
     }

   return _efl_net_socket_windows_operation_succeeded(op, used_size);
}

static Eina_Error _efl_net_socket_windows_send(Eo *o, Efl_Net_Socket_Windows_Data *pd);

static Eina_Error
_efl_net_socket_windows_send_success(void *data EINA_UNUSED, Eo *o, DWORD used_size)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);

   pd->send.pending = NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->send.used < used_size, EINVAL);

   if (used_size > 0)
     {
        DWORD todo = pd->send.used - used_size;
        if (todo > 0)
          memmove(pd->send.bytes, pd->send.bytes + used_size, todo);
        pd->send.used -= used_size;
     }

   /* calls back the user, may read()/write()/close() */
   /* only can_write if we're fully done with previous request! */
   efl_io_writer_can_write_set(o, pd->send.used == 0);

   if (pd->handle == INVALID_HANDLE_VALUE) return 0;
   if (pd->send.used == 0) return 0;
   if (pd->send.pending) return 0;

   return _efl_net_socket_windows_send(o, pd);
}

static Eina_Error
_efl_net_socket_windows_send_failure(void *data EINA_UNUSED, Eo *o, DWORD win32err)
{
   Efl_Net_Socket_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("socket=%p send error=%s", o, msg);
        free(msg);
     }

   pd->send.pending = NULL;

   efl_io_writer_can_write_set(o, EINA_FALSE);

   if (pd->recv.used > 0)
     pd->pending_eos = EINA_TRUE; /* eos when buffer drains */
   else
     {
        efl_io_reader_can_read_set(o, EINA_FALSE);
        efl_io_reader_eos_set(o, EINA_TRUE);
     }

   switch (win32err)
     {
      case ERROR_INVALID_USER_BUFFER: return EFAULT;
      case ERROR_NOT_ENOUGH_MEMORY: return ENOMEM;
      case ERROR_OPERATION_ABORTED: return ECANCELED;
      case ERROR_BROKEN_PIPE: return EPIPE;
      default: return EIO;
     }
}

static Eina_Error
_efl_net_socket_windows_send(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   Efl_Net_Socket_Windows_Operation *op;
   OVERLAPPED *ovl;
   DWORD used_size = 0, win32err;
   BOOL r;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->send.pending != NULL, EINPROGRESS);

   if (pd->handle == INVALID_HANDLE_VALUE) return EBADF;
   if (pd->send.used == 0) return 0;

   op = _efl_net_socket_windows_operation_new(o,
                                              _efl_net_socket_windows_send_success,
                                              _efl_net_socket_windows_send_failure,
                                              NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);

   ovl = _efl_net_socket_windows_operation_overlapped_get(op);

   r = WriteFile(pd->handle,
                 pd->send.bytes,
                 pd->send.used,
                 &used_size, ovl);
   win32err = GetLastError();
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("socket=%p WriteFile(%p, %p, %lu, &size=%lu, %p)=%s",
            o, pd->handle, pd->send.bytes, pd->send.used, used_size, ovl, msg);
        free(msg);
     }

   if (!r)
     {
        if (win32err == ERROR_IO_PENDING)
          {
             pd->send.pending = op;
             return 0;
          }
        else
          {
             return _efl_net_socket_windows_operation_failed(op, win32err);
          }
     }

   return _efl_net_socket_windows_operation_succeeded(op, used_size);
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

   efl_io_writer_can_write_set(o, EINA_TRUE);
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
_efl_net_socket_windows_efl_object_invalidate(Eo *o, Efl_Net_Socket_Windows_Data *pd)
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
_efl_net_socket_windows_efl_object_destructor(Eo *o, Efl_Net_Socket_Windows_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_local, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);

   free(pd->recv.mem);
   pd->recv.mem = NULL;
   pd->recv.len = 0;
   pd->recv.used = 0;
   pd->recv.base = 0;

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

   DBG("socket=%p closing handle=%p", o, pd->handle);

   if (pd->pending_ops)
     {
        if (pd->send.pending)
          {
             Efl_Net_Socket_Windows_Operation *op = pd->send.pending;
             DWORD used_size = 0;
             DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p: wait %lu bytes pending write...",
                 op, op->o, op->success_cb, op->failure_cb, op->data, pd->send.used);
             if (GetOverlappedResult(pd->handle, &op->base, &used_size, TRUE))
               {
                  DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, TRUE)",
                      op, op->o, op->success_cb, op->failure_cb, op->data, pd->handle, &op->base, used_size);
               }
             else
               {
                  DWORD win32err = GetLastError();
                  if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
                    {
                       char *msg = _efl_net_windows_error_msg_get(win32err);
                       DBG("op=%p (socket=%p) success_cb=%p failure_cb=%p data=%p GetOverlappedResult(%p, %p, &size=%lu, TRUE)=%s",
                           op, op->o, op->success_cb, op->failure_cb, op->data, pd->handle, &op->base, used_size, msg);
                       free(msg);
                    }
               }
          }
        if (!FlushFileBuffers(pd->handle))
          {
             char *msg = _efl_net_windows_error_msg_get(GetLastError());
             WRN("HANDLE=%p could not flush buffers: %s", pd->handle, msg);
             free(msg);
          }
        if (CancelIo(pd->handle))
          DBG("socket=%p CancelIo(%p)", o, pd->handle);
        else if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
          {
             char *msg = _efl_net_windows_error_msg_get(GetLastError());
             WRN("socket=%p CancelIo(%p)=%s", o, pd->handle, msg);
             free(msg);
          }
     }

   while (pd->pending_ops)
     _efl_net_socket_windows_operation_failed(pd->pending_ops->data, ERROR_OPERATION_ABORTED);

   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);

   h = InterlockedExchangePointer(&pd->handle, INVALID_HANDLE_VALUE);
   if (h != INVALID_HANDLE_VALUE)
     _efl_net_socket_windows_handle_close(h);

   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);

   return 0;
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_closer_closed_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->handle == INVALID_HANDLE_VALUE;
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, Eina_Bool close_on_exec)
{
   DBG("close on exec is not supported on windows");
   pd->close_on_exec = close_on_exec;
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_closer_close_on_exec_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->close_on_exec;
}

EOLIAN static void
_efl_net_socket_windows_efl_io_closer_close_on_invalidate_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, Eina_Bool close_on_invalidate)
{
   pd->close_on_invalidate = close_on_invalidate;
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_closer_close_on_invalidate_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->close_on_invalidate;
}

EOLIAN static Eina_Error
_efl_net_socket_windows_efl_io_reader_read(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Slice ro_slice;
   DWORD remaining;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->recv.base > pd->recv.used, EINVAL);

   ro_slice.len = pd->recv.used - pd->recv.base;
   if (ro_slice.len == 0)
     {
        rw_slice->len = 0;
        if (pd->pending_eos)
          {
             efl_io_reader_eos_set(o, EINA_TRUE);
             return 0;
          }
        return EAGAIN;
     }
   ro_slice.bytes = pd->recv.bytes + pd->recv.base;

   *rw_slice = eina_rw_slice_copy(*rw_slice, ro_slice);

   pd->recv.base += rw_slice->len;
   remaining = pd->recv.used - pd->recv.base;

   efl_io_reader_can_read_set(o, remaining > 0);

   if ((pd->pending_eos) && (remaining == 0))
     {
        efl_io_reader_eos_set(o, EINA_TRUE);
        return 0;
     }

   if (!pd->recv.pending)
     {
        DBG("recv more from socket=%p", o);
        _efl_net_socket_windows_recv(o, pd);
        return 0;
     }

   return 0;
}

EOLIAN static void
_efl_net_socket_windows_efl_io_reader_can_read_set(Eo *o, Efl_Net_Socket_Windows_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o) && can_read);
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, &can_read);
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
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
_efl_net_socket_windows_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
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
   EINA_SAFETY_ON_NULL_GOTO(pd->send.mem, error);

   /* note: do not queue more data if one is already pending,
    * otherwise we over-commit a lot and on write() -> close(), it
    * would take us more than simply waiting on the pending send to
    * complete.
    */
   if (pd->send.pending)
     {
        efl_io_writer_can_write_set(o, EINA_FALSE);
        err = EAGAIN;
        goto error;
     }
   err = EINVAL;
   EINA_SAFETY_ON_TRUE_GOTO(pd->send.used != 0, error);

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
   pd->send.used += todo;

   efl_io_writer_can_write_set(o, EINA_FALSE);

   DBG("send %lu bytes more to socket=%p", pd->send.used, o);
   return _efl_net_socket_windows_send(o, pd);

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
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, &can_write);
}

EOLIAN static Eina_Bool
_efl_net_socket_windows_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_net_socket_windows_efl_net_socket_address_local_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_local, address);
}

EOLIAN static const char *
_efl_net_socket_windows_efl_net_socket_address_local_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->address_local;
}

EOLIAN static void
_efl_net_socket_windows_efl_net_socket_address_remote_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_remote, address);
}

EOLIAN static const char *
_efl_net_socket_windows_efl_net_socket_address_remote_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Windows_Data *pd)
{
   return pd->address_remote;
}

#include "efl_net_socket_windows.eo.c"
