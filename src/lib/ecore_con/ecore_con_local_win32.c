#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <process.h>

#include <Ecore.h>

#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define BUFSIZE 512

static int _ecore_con_local_init_count = 0;

int
ecore_con_local_init(void)
{
   if (++_ecore_con_local_init_count != 1)
     return _ecore_con_local_init_count;

   return _ecore_con_local_init_count;
}

int
ecore_con_local_shutdown(void)
{
   if (--_ecore_con_local_init_count != 0)
     return _ecore_con_local_init_count;

   return _ecore_con_local_init_count;
}

static Eina_Bool
_ecore_con_local_win32_server_read_client_handler(void *data, Ecore_Win32_Handler *wh)
{
   Ecore_Con_Client *obj = data;
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   void *buf;
   DWORD n;
   Eina_Bool broken_pipe = EINA_FALSE;
   Efl_Network_Server_Data *host_svr = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   if (!ResetEvent(host_svr->event_read))
     return ECORE_CALLBACK_RENEW;

   buf = malloc(host_svr->nbr_bytes);
   if (!buf)
     return ECORE_CALLBACK_RENEW;

   if (ReadFile(host_svr->pipe, buf, host_svr->nbr_bytes, &n, NULL))
     {
        if (!cl->delete_me)
          ecore_con_event_client_data(obj, buf, host_svr->nbr_bytes, EINA_FALSE);
        host_svr->want_write = 1;
     }
   else
     {
        if (GetLastError() == ERROR_BROKEN_PIPE)
          broken_pipe = EINA_TRUE;
     }

   if (broken_pipe)
     {
#if 0
        ecore_con_event_client_error(cl, evil_last_error_get());
#endif
        _ecore_con_client_kill(obj);
        return ECORE_CALLBACK_CANCEL;
     }

   if (host_svr->want_write)
     ecore_con_local_win32_client_flush(obj);

   ecore_main_win32_handler_del(wh);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_con_local_win32_server_peek_client_handler(void *data, Ecore_Win32_Handler *wh)
{
   Ecore_Con_Client *obj = data;
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Efl_Network_Server_Data *host_svr = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
#if 0
   char *msg;
#endif

   if (!ResetEvent(host_svr->event_peek))
     return ECORE_CALLBACK_RENEW;

#if 0
   ecore_con_event_server_error(host_svr, evil_last_error_get());
#endif
   _ecore_con_server_kill(cl->host_server);
   return ECORE_CALLBACK_CANCEL;

   ecore_main_win32_handler_del(wh);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_con_local_win32_client_peek_server_handler(void *data, Ecore_Win32_Handler *wh)
{
   Ecore_Con_Server *obj = data;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
#if 0
   char *msg;
#endif

   if (!ResetEvent(svr->event_peek))
     return ECORE_CALLBACK_RENEW;
#if 0
   ecore_con_event_server_error(svr, evil_last_error_get());
#endif
   _ecore_con_server_kill(obj);
   return ECORE_CALLBACK_CANCEL;

   ecore_main_win32_handler_del(wh);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_con_local_win32_client_read_server_handler(void *data, Ecore_Win32_Handler *wh)
{
   Ecore_Con_Server *obj = data;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   void *buf;
   DWORD n;
   Eina_Bool broken_pipe = EINA_FALSE;

   if (!ResetEvent(svr->event_read))
     return ECORE_CALLBACK_RENEW;

   buf = malloc(svr->nbr_bytes);
   if (!buf)
     return ECORE_CALLBACK_RENEW;

   if (ReadFile(svr->pipe, buf, svr->nbr_bytes, &n, NULL))
     {
        if (!svr->delete_me)
          ecore_con_event_server_data(obj, buf, svr->nbr_bytes, EINA_FALSE);
        svr->want_write = 1;
     }
   else
     {
        if (GetLastError() == ERROR_BROKEN_PIPE)
          broken_pipe = EINA_TRUE;
     }

   if (broken_pipe)
     {
#if 0
        ecore_con_event_server_error(svr, evil_last_error_get());
#endif
        _ecore_con_server_kill(obj);
        return ECORE_CALLBACK_CANCEL;
     }

   if (svr->want_write)
     ecore_con_local_win32_server_flush(obj);

   ecore_main_win32_handler_del(wh);

   return ECORE_CALLBACK_DONE;
}

/* thread to read data sent by the server to the client */
static unsigned int __stdcall
_ecore_con_local_win32_client_read_server_thread(void *data)
{
   Ecore_Con_Server *obj = data;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   DWORD nbr_bytes = 0;

   svr->read_stopped = EINA_FALSE;

   while (!svr->read_stop)
     {
        if (PeekNamedPipe(svr->pipe, NULL, 0, NULL, &nbr_bytes, NULL))
          {
             if (nbr_bytes <= 0)
               continue;

             svr->nbr_bytes = nbr_bytes;
             if (!SetEvent(svr->event_read))
               continue;
          }
        else
          {
             if (GetLastError() == ERROR_BROKEN_PIPE)
               {
                  if (!SetEvent(svr->event_peek))
                    continue;
                  break;
               }
          }
     }

   svr->read_stopped = EINA_TRUE;
   _endthreadex(0);
   return 0;
}

/* thread to read data sent by the client to the server */
static unsigned int __stdcall
_ecore_con_local_win32_server_read_client_thread(void *data)
{
   Ecore_Con_Client *obj = data;
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Efl_Network_Server_Data *host_svr = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
   DWORD nbr_bytes = 0;

   host_svr->read_stopped = EINA_FALSE;

   while (!host_svr->read_stop)
     {
        if (PeekNamedPipe(host_svr->pipe, NULL, 0, NULL, &nbr_bytes, NULL))
          {
             if (nbr_bytes <= 0)
               continue;

             host_svr->nbr_bytes = nbr_bytes;
             if (!SetEvent(host_svr->event_read))
               continue;
          }
        else
          {
             if (GetLastError() == ERROR_BROKEN_PIPE)
               {
                  if (!SetEvent(host_svr->event_peek))
                    continue;
                  break;
               }
          }
     }

   host_svr->read_stopped = EINA_TRUE;
   _endthreadex(0);
   return 0;
}

static Eina_Bool
_ecore_con_local_win32_client_add(void *data, Ecore_Win32_Handler *wh)
{
   Ecore_Con_Server *obj = data;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Win32_Handler *handler_read;
   Ecore_Win32_Handler *handler_peek;

   if (!svr->pipe)
     return ECORE_CALLBACK_CANCEL;

   if (svr->delete_me)
     return ECORE_CALLBACK_CANCEL;

   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     return ECORE_CALLBACK_CANCEL;

   Ecore_Con_Client *cl_obj = efl_add(EFL_NETWORK_CLIENT_CLASS, efl_main_loop_get());
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   if (!cl)
     {
        ERR("allocation failed");
        return ECORE_CALLBACK_CANCEL;
     }

   cl->host_server = obj;

   svr->event_read = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!svr->event_read)
     {
        ERR("Can not create event read");
        goto free_cl;
     }

   handler_read = ecore_main_win32_handler_add(svr->event_read,
                                               _ecore_con_local_win32_server_read_client_handler,
                                               obj);
   if (!handler_read)
     {
        ERR("Can not create handler read");
        goto close_event_read;
     }

   svr->event_peek = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!svr->event_peek)
     {
        ERR("Can not create event peek");
        goto del_handler_read;
     }

   handler_peek = ecore_main_win32_handler_add(svr->event_peek,
                                               _ecore_con_local_win32_server_peek_client_handler,
                                               obj);
   if (!handler_peek)
     {
        ERR("Can not create handler peek");
        goto close_event_peek;
     }

   svr->read_stopped = EINA_TRUE;
   svr->thread_read = (HANDLE)_beginthreadex(NULL, 0, _ecore_con_local_win32_server_read_client_thread, cl, CREATE_SUSPENDED, NULL);
   if (!svr->thread_read)
     {
        ERR("Can not launch thread");
        goto del_handler_peek;
     }

   svr->clients = eina_list_append(svr->clients, obj);
   svr->client_count++;

   if (!cl->delete_me)
     ecore_con_event_client_add(obj);

   ecore_main_win32_handler_del(wh);

   ResumeThread(svr->thread_read);
   return ECORE_CALLBACK_DONE;

del_handler_peek:
   ecore_main_win32_handler_del(handler_peek);
close_event_peek:
   CloseHandle(svr->event_peek);
del_handler_read:
   ecore_main_win32_handler_del(handler_read);
close_event_read:
   CloseHandle(svr->event_read);
free_cl:
   efl_del(cl_obj);

   return ECORE_CALLBACK_CANCEL;
}

static unsigned int __stdcall
_ecore_con_local_win32_listening(void *data)
{
   Ecore_Con_Server *obj = data;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   BOOL res;

   while (1)
     {
        res = ConnectNamedPipe(svr->pipe, NULL);
        if (!res)
          {
             ERR("Opening the connection to the client failed");
             CloseHandle(svr->pipe);
             svr->pipe = NULL;
          }
        break;
     }

   DBG("Client connected");

   _endthreadex(0);
   return 0;
}

EAPI char *
ecore_con_local_path_new(Eina_Bool is_system, const char *name, int port)
{
   char buf[256];

   if (!is_system)
     snprintf(buf, sizeof(buf), "\\\\.\\pipe\\%s%ld", name, GetProcessId(GetCurrentProcess()));
   else
     {
        const char *computername;

        computername = getenv("COMPUTERNAME");
        snprintf(buf, sizeof(buf), "\\\\%s\\pipe\\%s%ld", computername, name, GetProcessId(GetCurrentProcess()));
     }

   return strdup(buf);
   (void)port; // CHECK-ME: shouldn't we use port to be similar to UNIX?
}

Eina_Bool
ecore_con_local_listen(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   HANDLE thread_listening;
   Ecore_Win32_Handler *handler;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
        ERR("Your system does not support abstract sockets!");
        return EINA_FALSE;
     }

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_USER)
     svr->path = ecore_con_local_path_new(EINA_FALSE, svr->name, svr->port);
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_SYSTEM)
     svr->path = ecore_con_local_path_new(EINA_TRUE, svr->name, svr->port);

   if (!svr->path)
     {
        ERR("Allocation failed");
        return EINA_FALSE;
     }

   /*
    * synchronuous
    * block mode
    * wait mode
    */
   svr->pipe = CreateNamedPipe(svr->path,
                               PIPE_ACCESS_DUPLEX,
                               PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                               PIPE_UNLIMITED_INSTANCES,
                               BUFSIZE,
                               BUFSIZE,
                               5000,
                               NULL);
   if (svr->pipe == INVALID_HANDLE_VALUE)
     {
        DBG("Creation of the named pipe '%s' failed", svr->path);
        goto free_path;
     }

   /*
    * We use ConnectNamedPipe() to wait for a client to connect.
    * As the function is blocking, to let the main loop continuing
    * its iterations, we call ConnectNamedPipe() in a thread
    */
   thread_listening = (HANDLE)_beginthreadex(NULL, 0, _ecore_con_local_win32_listening, obj, CREATE_SUSPENDED, NULL);
   if (!thread_listening)
     {
        ERR("Creation of the listening thread failed");
        goto close_pipe;
     }

   handler = ecore_main_win32_handler_add(thread_listening,
                                          _ecore_con_local_win32_client_add,
                                          obj);
   if (!handler)
     {
        ERR("Creation of the client add handler failed");
        goto del_handler;
     }

   svr->read_stopped = EINA_TRUE;
   ResumeThread(thread_listening);

   return EINA_TRUE;

del_handler:
   ecore_main_win32_handler_del(handler);
close_pipe:
   CloseHandle(svr->pipe);
free_path:
   free(svr->path);
   svr->path = NULL;

   return EINA_FALSE;
}

void
ecore_con_local_win32_server_del(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     return;

   if (((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_USER) &&
       ((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_SYSTEM))
     return;

   svr->read_stop = 1;
   /* FIXME: we should try to stop these thread in one way or another */
   /* should we use ecore_thread ? */
   /* while (!svr->read_stopped) */
   /*   Sleep(100); */

   if (svr->event_peek)
     CloseHandle(svr->event_peek);
   svr->event_peek = NULL;
   if (svr->event_read)
     CloseHandle(svr->event_read);
   svr->event_read = NULL;
   free(svr->path);
   svr->path = NULL;
   if (svr->pipe)
     CloseHandle(svr->pipe);
   svr->pipe = NULL;
}

void
ecore_con_local_win32_client_del(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Efl_Network_Server_Data *svr = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     return;

   if (((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_USER) &&
       ((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_SYSTEM))
     return;

   svr->read_stop = 1;
   while (!svr->read_stopped)
     Sleep(100);

   if (svr->event_peek)
     CloseHandle(svr->event_peek);
   svr->event_peek = NULL;
   if (svr->event_read)
     CloseHandle(svr->event_read);
   svr->event_read = NULL;
   free(svr->path);
   svr->path = NULL;
   if (svr->pipe)
     CloseHandle(svr->pipe);
   svr->pipe = NULL;
}

Eina_Bool
ecore_con_local_connect(Ecore_Con_Server *obj,
                        Eina_Bool (*cb_done)(void *data,
                                             Ecore_Fd_Handler *fd_handler))
{
#warning "I am pretty sure cb_done should be used."
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   char *buf = NULL;
   Ecore_Win32_Handler *handler_read;
   Ecore_Win32_Handler *handler_peek;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     {
        WRN("Your system does not support abstract sockets!");
        return EINA_FALSE;
     }

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_USER)
     buf = ecore_con_local_path_new(EINA_FALSE, svr->name, svr->port);
   else if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_SYSTEM)
     buf = ecore_con_local_path_new(EINA_TRUE, svr->name, svr->port);

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EINA_FALSE);

   while (1)
     {
        svr->pipe = CreateFile(buf,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);
        if (svr->pipe != INVALID_HANDLE_VALUE)
          break;

        /* if pipe not busy, we exit */
        if (GetLastError() != ERROR_PIPE_BUSY)
          {
             DBG("Connection to a server failed");
             free(buf);
             return EINA_FALSE;
          }

        /* pipe busy, so we wait for it */
        if (!WaitNamedPipe(buf, NMPWAIT_WAIT_FOREVER))
          {
             DBG("Can not wait for a server");
             goto close_pipe;
          }
     }

   svr->path = buf;
   buf = NULL;

   svr->event_read = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!svr->event_read)
     {
        ERR("Can not create event read");
        goto free_path;
     }

   handler_read = ecore_main_win32_handler_add(svr->event_read,
                                               _ecore_con_local_win32_client_read_server_handler,
                                               obj);
   if (!handler_read)
     {
        ERR("Can not create handler read");
        goto close_event_read;
     }

   svr->event_peek = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!svr->event_peek)
     {
        ERR("Can not create event peek");
        goto del_handler_read;
     }

   handler_peek = ecore_main_win32_handler_add(svr->event_peek,
                                               _ecore_con_local_win32_client_peek_server_handler,
                                               obj);
   if (!handler_peek)
     {
        ERR("Can not create handler peek");
        goto close_event_peek;
     }

   svr->thread_read = (HANDLE)_beginthreadex(NULL, 0, _ecore_con_local_win32_client_read_server_thread, obj, CREATE_SUSPENDED, NULL);
   if (!svr->thread_read)
     {
        ERR("Can not launch thread");
        goto del_handler_peek;
     }

   if (!svr->delete_me) ecore_con_event_server_add(obj);

   ResumeThread(svr->thread_read);
   free(buf);

   return EINA_TRUE;

del_handler_peek:
   ecore_main_win32_handler_del(handler_peek);
close_event_peek:
   CloseHandle(svr->event_peek);
del_handler_read:
   ecore_main_win32_handler_del(handler_read);
close_event_read:
   CloseHandle(svr->event_read);
free_path:
   free(svr->path);
   svr->path = NULL;
close_pipe:
   CloseHandle(svr->pipe);
   free(buf);

   return EINA_FALSE;
}

Eina_Bool
ecore_con_local_win32_server_flush(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   size_t num;
   BOOL res;
   DWORD written;

   /* This check should never be true */
   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_LOCAL_ABSTRACT)
     return EINA_TRUE;

   if (((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_USER) &&
       ((svr->type & ECORE_CON_TYPE) != ECORE_CON_LOCAL_SYSTEM))
     return EINA_FALSE;

   num = eina_binbuf_length_get(svr->buf) - svr->write_buf_offset;
   if (num == 0) return EINA_TRUE;

   res = WriteFile(svr->pipe, eina_binbuf_string_get(svr->buf) + svr->write_buf_offset, num, &written, NULL);
   if (!res)
     {
        ecore_con_event_server_error(obj, evil_last_error_get());
        _ecore_con_server_kill(obj);
     }

   svr->write_buf_offset += written;
   if (svr->write_buf_offset >= eina_binbuf_length_get(svr->buf))
     {
        svr->write_buf_offset = 0;
        eina_binbuf_free(svr->buf);
        svr->buf = NULL;
        svr->want_write = 0;
     }
   else if (written < (DWORD)num)
     svr->want_write = 1;

   return EINA_TRUE;
}

Eina_Bool
ecore_con_local_win32_client_flush(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Type type;
   size_t num;
   BOOL res;
   DWORD written;
   Efl_Network_Server_Data *svr = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   type = svr->type & ECORE_CON_TYPE;

   /* This check should never be true */
   if (type == ECORE_CON_LOCAL_ABSTRACT)
     return EINA_TRUE;

   if ((type != ECORE_CON_LOCAL_USER) &&
       (type != ECORE_CON_LOCAL_SYSTEM))
     return EINA_FALSE;

   num = eina_binbuf_length_get(cl->buf) - cl->buf_offset;
   if (num == 0) return EINA_TRUE;

   res = WriteFile(svr->pipe, eina_binbuf_string_get(cl->buf) + cl->buf_offset, num, &written, NULL);
   if (!res)
     {
        ecore_con_event_client_error(obj, evil_last_error_get());
        _ecore_con_client_kill(obj);
     }

   cl->buf_offset += written;
   if (cl->buf_offset >= eina_binbuf_length_get(cl->buf))
     {
        cl->buf_offset = 0;
        eina_binbuf_free(cl->buf);
        cl->buf = NULL;
        svr->want_write = 0;
     }
   else if (written < (DWORD)num)
     svr->want_write = 1;

   return EINA_TRUE;
}

