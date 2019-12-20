#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <process.h>

#include <evil_private.h> /* evil_last_error_get */

#include "Ecore.h"
#include "ecore_private.h"

#include "ecore_exe_private.h"

/*
 * TESTS
 *
 * [X] add event
 * [X] data event
 * [X] error event
 * [X] data event buffered
 * [X] del event
 * [ ] batch files
 * [X] exit code
 * [X] inherited env var
 */

/*
 * FIXME :
 *
 * [ ] child program with ecore main loop does not exit and nothing is sent
 * [X] ecore_exe_send fails (race condition ? same problem as above ?)
 */

#define ECORE_EXE_WIN32_TIMEOUT 3000

static int run_pri = NORMAL_PRIORITY_CLASS;

static void
_ecore_exe_threads_terminate(Ecore_Exe *obj)
{
   Ecore_Exe_Data *exe = efl_data_scope_get(obj, ECORE_EXE_CLASS);

   if (!exe) return;
   if (!exe->th) return;
   ecore_thread_cancel(exe->th);
   ecore_thread_wait(exe->th, 0.3);
   exe->th = NULL;
}

static Eina_Bool
_ecore_exe_close_cb(void *data,
                    Ecore_Win32_Handler *wh EINA_UNUSED)
{
   Ecore_Exe_Event_Del *e;
   Ecore_Exe *obj = data;
   Ecore_Exe_Data *exe = efl_data_scope_get(obj, ECORE_EXE_CLASS);
   DWORD exit_code = 0;

   if (!exe) return 0;

   e = calloc(1, sizeof(Ecore_Exe_Event_Del));
   if (!e) return 0;

   /* FIXME : manage the STILL_ACTIVE returned error */
   if (!GetExitCodeProcess(exe->process, &exit_code))
     DBG("%s", evil_last_error_get());

   e->exit_code = exit_code;
   e->exited = 1;
   e->pid = exe->pid;
   e->exe = obj;
   exe->h_close = NULL; // It's going to get deleted in the next callback.

   ecore_event_add(ECORE_EXE_EVENT_DEL, e, _ecore_exe_event_del_free, NULL);

   DBG("Exiting process %s with exit code %d\n", exe->cmd, e->exit_code);
   return 0;
}

typedef struct
{
   Ecore_Exe  *obj;
   HANDLE      read_pipe;
   HANDLE      error_pipe;
   Eina_Bool   read : 1;
   Eina_Bool   error : 1;
} Threaddata;

typedef struct
{
   Ecore_Exe     *obj;
   unsigned char *buf;
   int            buf_size;
   Eina_Bool      read : 1;
   Eina_Bool      error : 1;
} Threadreply;

static void
_ecore_exe_win32_io_poll_thread(void *data, Ecore_Thread *th)
{
   Threaddata *tdat = data;
   Threadreply *trep;
   Eina_Bool data_read, data_error;
   char buf[4096];
   DWORD size, current_size;
   BOOL res;

   while (EINA_TRUE)
     {
        data_read = EINA_FALSE;
        data_error = EINA_FALSE;

        if (tdat->read)
          {
             res = PeekNamedPipe(tdat->read_pipe, buf, sizeof(buf),
                                 &size, &current_size, NULL);
             if (res && (size != 0))
               {
                  trep = calloc(1, sizeof(Threadreply));
                  if (trep)
                    {
                       trep->obj = tdat->obj;
                       trep->buf = malloc(current_size);
                       if (trep->buf)
                         {
                            res = ReadFile(tdat->read_pipe, trep->buf,
                                           current_size, &size, NULL);
                            if (!res || (size == 0))
                              {
                                 free(trep->buf);
                                 free(trep);
                              }
                            trep->buf_size = size;
                            trep->read = EINA_TRUE;
                            ecore_thread_feedback(th, trep);
                            data_read = EINA_TRUE;
                         }
                    }
               }
          }
        if (tdat->error)
          {
             res = PeekNamedPipe(tdat->error_pipe, buf, sizeof(buf),
                                 &size, &current_size, NULL);
             if (res && (size != 0))
               {
                  trep = calloc(1, sizeof(Threadreply));
                  if (trep)
                    {
                       trep->obj = tdat->obj;
                       trep->buf = malloc(current_size);
                       if (trep->buf)
                         {
                            res = ReadFile(tdat->error_pipe, trep->buf,
                                           current_size, &size, NULL);
                            if (!res || (size == 0))
                              {
                                 free(trep->buf);
                                 free(trep);
                              }
                            trep->buf_size = size;
                            trep->error = EINA_TRUE;
                            ecore_thread_feedback(th, trep);
                            data_error = EINA_TRUE;
                         }
                    }
               }
          }
        if (ecore_thread_check(th)) break;
        if (!(data_read || data_error)) Sleep(100);
        else if (ecore_thread_check(th)) break;
     }
   free(tdat);
}

static void
_ecore_exe_win32_io_poll_notify(void *data EINA_UNUSED,
                                Ecore_Thread *th EINA_UNUSED, void *msg)
{
   Threadreply *trep = msg;
   Ecore_Exe *obj = trep->obj;
   Ecore_Exe_Data *exe = efl_data_scope_get(obj, ECORE_EXE_CLASS);
   unsigned char *b;

   if (exe)
     {
        Ecore_Exe_Event_Data *event_data;

        if (trep->read)
          {
             if (!exe->pipe_read.data_buf)
               {
                  exe->pipe_read.data_buf = trep->buf;
                  exe->pipe_read.data_size = trep->buf_size;
                  trep->buf = NULL;
               }
             else
               {
                  b = realloc(exe->pipe_read.data_buf,
                              exe->pipe_read.data_size + trep->buf_size);
                  if (b)
                    {
                       memcpy(b + exe->pipe_read.data_size,
                              trep->buf, trep->buf_size);
                       exe->pipe_read.data_buf = b;
                       exe->pipe_read.data_size += trep->buf_size;
                    }
                  else ERR("Out of memory in reading exe pipe data");
               }
             event_data = ecore_exe_event_data_get(obj, ECORE_EXE_PIPE_READ);
             if (event_data)
               {
                  ecore_event_add(ECORE_EXE_EVENT_DATA, event_data,
                                  _ecore_exe_event_exe_data_free, NULL);
                  efl_event_callback_call(obj, ECORE_EXE_EVENT_DATA_GET,
                                          event_data);
               }
          }
        else if (trep->error)
          {
             if (!exe->pipe_error.data_buf)
               {
                  exe->pipe_error.data_buf = trep->buf;
                  exe->pipe_error.data_size = trep->buf_size;
                  trep->buf = NULL;
               }
             else
               {
                  b = realloc(exe->pipe_error.data_buf,
                              exe->pipe_error.data_size + trep->buf_size);
                  if (b)
                    {
                       memcpy(b + exe->pipe_error.data_size,
                              trep->buf, trep->buf_size);
                       exe->pipe_error.data_buf = b;
                       exe->pipe_error.data_size += trep->buf_size;
                    }
                  else ERR("Out of memory in reading exe pipe data");
               }
             event_data = ecore_exe_event_data_get(obj, ECORE_EXE_PIPE_ERROR);
             if (event_data)
               {
                  ecore_event_add(ECORE_EXE_EVENT_ERROR, event_data,
                                  _ecore_exe_event_exe_data_free, NULL);
                  efl_event_callback_call(obj, ECORE_EXE_EVENT_DATA_ERROR,
                                          event_data);
               }
          }
     }
   free(trep->buf);
   free(trep);
}

static DWORD WINAPI
_ecore_exe_thread_procedure(LPVOID data EINA_UNUSED)
{
   GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
   GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
   return 1;
}

static DWORD __stdcall
_ecore_exe_exit_process(void *data)
{
   UINT *code;
   code = (UINT *)data;
   ExitProcess(*code);
   return 1;
}

static BOOL CALLBACK
_ecore_exe_enum_windows_procedure(HWND window,
                                  LPARAM data)
{
   Ecore_Exe *obj = (Ecore_Exe *) data;
   Ecore_Exe_Data *exe = efl_data_scope_get(obj, ECORE_EXE_CLASS);
   DWORD thread_id;
   UINT code = 0;

   if (!exe) return EINA_FALSE;
   thread_id = GetWindowThreadProcessId(window, NULL);

   if (thread_id == exe->thread_id)
     {
        /* Ctrl-C or Ctrl-Break */
        if (CreateRemoteThread(exe->process, NULL, 0,
                               (LPTHREAD_START_ROUTINE)_ecore_exe_thread_procedure, NULL,
                                0, NULL))
          return EINA_FALSE;

        if ((exe->sig == ECORE_EXE_WIN32_SIGINT) ||
            (exe->sig == ECORE_EXE_WIN32_SIGQUIT))
          return EINA_FALSE;

        /* WM_CLOSE message */
        PostMessage(window, WM_CLOSE, 0, 0);
        if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
          return EINA_FALSE;

        /* WM_QUIT message */
        PostMessage(window, WM_QUIT, 0, 0);
        if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
          return EINA_FALSE;

        /* Exit process */
        if (CreateRemoteThread(exe->process, NULL, 0,
                               (LPTHREAD_START_ROUTINE)_ecore_exe_exit_process,
                               &code,
                               0, NULL))
          return EINA_FALSE;

        if (exe->sig == ECORE_EXE_WIN32_SIGTERM)
          return EINA_FALSE;

        TerminateProcess(exe->process, 0);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
_impl_ecore_exe_run_priority_set(int pri)
{
   switch (pri)
     {
      case ECORE_EXE_WIN32_PRIORITY_IDLE:
        run_pri = IDLE_PRIORITY_CLASS;
        break;

      case ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL:
        run_pri = BELOW_NORMAL_PRIORITY_CLASS;
        break;

      case ECORE_EXE_WIN32_PRIORITY_NORMAL:
        run_pri = NORMAL_PRIORITY_CLASS;
        break;

      case ECORE_EXE_WIN32_PRIORITY_ABOVE_NORMAL:
        run_pri = ABOVE_NORMAL_PRIORITY_CLASS;
        break;

      case ECORE_EXE_WIN32_PRIORITY_HIGH:
        run_pri = HIGH_PRIORITY_CLASS;
        break;

      case ECORE_EXE_WIN32_PRIORITY_REALTIME:
        run_pri = REALTIME_PRIORITY_CLASS;
        break;

      default:
        break;
     }
}

int
_impl_ecore_exe_run_priority_get(void)
{
   switch (run_pri)
     {
      case IDLE_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_IDLE;

      case BELOW_NORMAL_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_BELOW_NORMAL;

      case NORMAL_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_NORMAL;

      case ABOVE_NORMAL_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_ABOVE_NORMAL;

      case HIGH_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_HIGH;

      case REALTIME_PRIORITY_CLASS:
        return ECORE_EXE_WIN32_PRIORITY_REALTIME;

      /* default should not be reached */
      default:
        return ECORE_EXE_WIN32_PRIORITY_NORMAL;
     }
}

Eo *
_impl_ecore_exe_efl_object_finalize(Eo *obj, Ecore_Exe_Data *exe)
{
   char exe_cmd_buf[32768];
   SECURITY_ATTRIBUTES sa;
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   HANDLE child_pipe_read = NULL;
   HANDLE child_pipe_error = NULL;
   const char *shell = NULL;
   Ecore_Exe_Event_Add *e;
   Ecore_Exe_Flags flags;
   Eina_Bool use_sh = EINA_FALSE;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   flags = exe->flags;
   DBG("Creating process %s with flags %d", exe->cmd, flags);
   if (!exe->cmd) goto error;

   if ((flags & ECORE_EXE_PIPE_AUTO) && (!(flags & ECORE_EXE_PIPE_ERROR)) &&
       (!(flags & ECORE_EXE_PIPE_READ)))
     /* We need something to auto pipe. */
     flags |= ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_ERROR;

   if ((flags & ECORE_EXE_USE_SH)) use_sh = EINA_TRUE;
   else use_sh = eina_str_has_extension(exe->cmd, ".bat");

   if (use_sh)
     {
        int len;

        shell = "cmd.exe";
        len = snprintf(exe_cmd_buf, sizeof(exe_cmd_buf), "/c %s", exe->cmd);
        if (len >= (int)sizeof(exe_cmd_buf))
          exe_cmd_buf[sizeof(exe_cmd_buf) - 1] = '\0';
     }
   else
     {
        int len;

        /* FIXME : faster with memset() but one must be careful with size */
        len = snprintf(exe_cmd_buf, sizeof(exe_cmd_buf), "%s", exe->cmd);
        if (len >= (int)sizeof(exe_cmd_buf))
          exe_cmd_buf[sizeof(exe_cmd_buf) - 1] = '\0';
     }

   /* stdout, stderr and stdin pipes */
   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle = EINA_TRUE;
   sa.lpSecurityDescriptor = NULL;

   if ((exe->flags & ECORE_EXE_PIPE_READ) ||
       (exe->flags & ECORE_EXE_PIPE_ERROR))
     {
        Threaddata *tdat;

        tdat = calloc(1, sizeof(Threaddata));
        if (tdat)
          {
             tdat->obj = obj;
             /* stdout pipe */
             if (exe->flags & ECORE_EXE_PIPE_READ)
               {
                  if (!CreatePipe(&exe->pipe_read.child_pipe,
                                  &child_pipe_read, &sa, 0))
                    goto error;
                  if (!SetHandleInformation(exe->pipe_read.child_pipe,
                                            HANDLE_FLAG_INHERIT, 0))
                    goto error;
                  tdat->read = EINA_TRUE;
                  tdat->read_pipe = exe->pipe_read.child_pipe;
               }
             /* stderr pipe */
             if (exe->flags & ECORE_EXE_PIPE_ERROR)
               {
                  if (!CreatePipe(&exe->pipe_error.child_pipe,
                                  &child_pipe_error, &sa, 0))
                    goto error;
                  if (!SetHandleInformation(exe->pipe_error.child_pipe,
                                            HANDLE_FLAG_INHERIT, 0))
                    goto error;
                  tdat->error = EINA_TRUE;
                  tdat->error_pipe = exe->pipe_error.child_pipe;
               }
             exe->th = ecore_thread_feedback_run
               (_ecore_exe_win32_io_poll_thread,
                _ecore_exe_win32_io_poll_notify,
                NULL, NULL, tdat, EINA_TRUE);
             if (!exe->th)
               {
                  free(tdat);
                  goto error;
               }
          }
     }

   /* stdin pipe */
   if (exe->flags & ECORE_EXE_PIPE_WRITE)
     {
        if (!CreatePipe(&exe->pipe_write.child_pipe,
                        &exe->pipe_write.child_pipe_x, &sa, 0))
          goto error;
        if (!SetHandleInformation(exe->pipe_write.child_pipe_x,
                                  HANDLE_FLAG_INHERIT, 0))
          goto error;
     }

   /* create child process */

   ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO);
   si.hStdOutput = child_pipe_read;
   si.hStdInput = exe->pipe_write.child_pipe;
   si.hStdError = child_pipe_error;
   si.dwFlags |= STARTF_USESTDHANDLES;

   DBG("CreateProcess: shell:%s child:%s", use_sh ? "yes" : "no", exe_cmd_buf);
   if (!CreateProcess(shell, exe_cmd_buf, NULL, NULL, EINA_TRUE,
                      run_pri | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
     {
        WRN("Failed to create process %s: %s",
            exe_cmd_buf, evil_last_error_get());
        goto error;
     }

   /*
    * Close pipe handles (do not continue to modify the parent).
    * We need to make sure that no handles to the write end of the
    * output and error pipes are maintained in this process or else
    * the pipe will not close when the child process exits and the
    * ReadFile will hang.
    */
   IF_FN_DEL(CloseHandle, child_pipe_read);
   IF_FN_DEL(CloseHandle, child_pipe_error);

   /* be sure that the child process is running */
   /* FIXME: This does not work if the child is an EFL-based app */
   /* if (WaitForInputIdle(pi.hProcess, INFINITE) == WAIT_FAILED) */
   /*   goto error; */
   exe->process = pi.hProcess;
   exe->process_thread = pi.hThread;
   exe->pid = pi.dwProcessId;
   exe->thread_id = pi.dwThreadId;

   exe->h_close = ecore_main_win32_handler_add(exe->process,
                                               _ecore_exe_close_cb, obj);
   if (!exe->h_close) goto error;

   if (ResumeThread(exe->process_thread) == ((DWORD)-1))
     {
        ERR("Could not resume process");
        goto error;
     }

   exe->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   Efl_Loop_Data *loop = efl_data_scope_get(exe->loop, EFL_LOOP_CLASS);
   if (loop) loop->exes = eina_list_append(loop->exes, obj);

   e = calloc(1, sizeof(Ecore_Exe_Event_Add));
   if (!e) goto error;
   e->exe = obj;
   ecore_event_add(ECORE_EXE_EVENT_ADD, e, _ecore_exe_event_add_free, NULL);
   return obj;

error:
   return NULL;
}

Eina_Bool
_impl_ecore_exe_send(Ecore_Exe  *obj,
                     Ecore_Exe_Data *exe,
                     const void *data,
                     int         size)
{
   DWORD num_exe;
   BOOL res;

   res = WriteFile(exe->pipe_write.child_pipe_x, data, size, &num_exe, NULL);
   if (!res || num_exe == 0)
     {
        ERR("Ecore_Exe %p stdin is closed! Cannot send %d bytes from %p",
            obj, size, data);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Ecore_Exe_Event_Data *
_impl_ecore_exe_event_data_get(Ecore_Exe      *obj,
                               Ecore_Exe_Data *exe,
                               Ecore_Exe_Flags flags)
{
   Ecore_Exe_Event_Data *e = NULL;
   unsigned char *inbuf;
   DWORD inbuf_num;
   Eina_Bool is_buffered = EINA_FALSE;

   /* Sort out what sort of event we are. */
   if (flags & ECORE_EXE_PIPE_READ)
     {
        flags = ECORE_EXE_PIPE_READ;
        if (exe->flags & ECORE_EXE_PIPE_READ_LINE_BUFFERED)
          is_buffered = EINA_TRUE;
     }
   else
     {
        flags = ECORE_EXE_PIPE_ERROR;
        if (exe->flags & ECORE_EXE_PIPE_ERROR_LINE_BUFFERED)
          is_buffered = EINA_TRUE;
     }

   /* Get the data. */
   if (flags & ECORE_EXE_PIPE_READ)
     {
        inbuf = exe->pipe_read.data_buf;
        inbuf_num = exe->pipe_read.data_size;
        exe->pipe_read.data_buf = NULL;
        exe->pipe_read.data_size = 0;
     }
   else
     {
        inbuf = exe->pipe_error.data_buf;
        inbuf_num = exe->pipe_error.data_size;
        exe->pipe_error.data_buf = NULL;
        exe->pipe_error.data_size = 0;
     }

   e = calloc(1, sizeof(Ecore_Exe_Event_Data));
   if (e)
     {
        e->exe = obj;
        e->data = inbuf;
        e->size = inbuf_num;

        if (is_buffered) /* Deal with line buffering. */
          {
             char *c;
             DWORD i;
             DWORD max = 0;
             DWORD count = 0;
             DWORD last = 0;

             c = (char *)inbuf;
             for (i = 0; i < inbuf_num; i++)
               {
                  if (inbuf[i] == '\n')
                    {
                       int end;

                       if (count >= max)
                         {
                            Ecore_Exe_Event_Data_Line *lines;

                            max += 10;
                            lines = realloc (e->lines,
                                             sizeof(Ecore_Exe_Event_Data_Line) * (max + 1));
                            if (lines) e->lines = lines;
                            else
                              {
                                 ERR("Out of memory in allocating exe lines");
                                 break;
                              }
                         }

                       if ((i >= 1) && (inbuf[i - 1] == '\r')) end = i - 1;
                       else end = i;
                       inbuf[end] = '\0';
                       e->lines[count].line = c;
                       e->lines[count].size = end - last;
                       last = i + 1;
                       c = (char *)&inbuf[last];
                       count++;
                    }
               }
             if (i > last) /* Partial line left over, save it for next time. */
               {
                  if (count != 0) e->size = last;
                  if (flags & ECORE_EXE_PIPE_READ)
                    {
                       exe->pipe_read.data_size = i - last;
                       exe->pipe_read.data_buf = malloc(exe->pipe_read.data_size);
                       if (exe->pipe_read.data_buf)
                         memcpy(exe->pipe_read.data_buf, c, exe->pipe_read.data_size);
                       else
                         {
                            exe->pipe_read.data_size = 0;
                            ERR("Out of memory in allocating exe pipe data");
                         }
                    }
                  else
                    {
                       exe->pipe_error.data_size = i - last;
                       exe->pipe_error.data_buf = malloc(exe->pipe_error.data_size);
                       if (exe->pipe_error.data_buf)
                         memcpy(exe->pipe_error.data_buf, c, exe->pipe_error.data_size);
                       else
                         {
                            exe->pipe_error.data_size = 0;
                            ERR("Out of memory in allocating exe pipe data");
                         }
                    }
               }
             if (count == 0) /* No lines to send, cancel the event. */
               {
                  _ecore_exe_event_exe_data_free(NULL, e);
                  e = NULL;
               }
             else /* NULL terminate the array, so that people know where the end is. */
               {
                  e->lines[count].line = NULL;
                  e->lines[count].size = 0;
               }
          }
     }
   return e;
}

void
_impl_ecore_exe_efl_object_destructor(Eo *obj, Ecore_Exe_Data *exe)
{
   void *data;

   _ecore_exe_threads_terminate(obj);

   data = exe->data;
   if (exe->pre_free_cb) exe->pre_free_cb(data, obj);

   IF_FN_DEL(ecore_main_win32_handler_del, exe->h_close);

   IF_FN_DEL(CloseHandle, exe->process_thread);
   IF_FN_DEL(CloseHandle, exe->process);
   IF_FN_DEL(CloseHandle, exe->pipe_write.child_pipe);
   IF_FN_DEL(CloseHandle, exe->pipe_write.child_pipe_x);
   IF_FN_DEL(CloseHandle, exe->pipe_error.child_pipe);
   IF_FN_DEL(CloseHandle, exe->pipe_read.child_pipe);

   IF_FREE(exe->cmd);

   Efl_Loop_Data *loop = efl_data_scope_get(exe->loop, EFL_LOOP_CLASS);
   if (loop) loop->exes = eina_list_remove(loop->exes, obj);
   IF_FREE(exe->tag);
}

void
_impl_ecore_exe_pause(Ecore_Exe *obj EINA_UNUSED, Ecore_Exe_Data *exe)
{
   if (exe->is_suspended) return;
   if (SuspendThread(exe->process_thread) != (DWORD)-1) exe->is_suspended = 1;
}

void
_impl_ecore_exe_continue(Ecore_Exe *obj EINA_UNUSED, Ecore_Exe_Data *exe)
{
   if (!exe->is_suspended) return;
   if (ResumeThread(exe->process_thread) != (DWORD)-1) exe->is_suspended = 0;
}

void
_impl_ecore_exe_interrupt(Ecore_Exe *obj, Ecore_Exe_Data *exe)
{
   CloseHandle(exe->process_thread);
   exe->process_thread = NULL;
   CloseHandle(exe->process);
   exe->process = NULL;
   exe->sig = ECORE_EXE_WIN32_SIGINT;
   EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)obj);
}

void
_impl_ecore_exe_quit(Ecore_Exe *obj, Ecore_Exe_Data *exe)
{
   CloseHandle(exe->process_thread);
   exe->process_thread = NULL;
   CloseHandle(exe->process);
   exe->process = NULL;
   exe->sig = ECORE_EXE_WIN32_SIGQUIT;
   EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)obj);
}

void
_impl_ecore_exe_terminate(Ecore_Exe *obj, Ecore_Exe_Data *exe)
{
/*    CloseHandle(exe->thread); */
   CloseHandle(exe->process);
   exe->process = NULL;
   exe->sig = ECORE_EXE_WIN32_SIGTERM;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)obj));
}

void
_impl_ecore_exe_kill(Ecore_Exe *obj, Ecore_Exe_Data *exe)
{
   CloseHandle(exe->process_thread);
   exe->process_thread = NULL;
   CloseHandle(exe->process);
   exe->process = NULL;
   exe->sig = ECORE_EXE_WIN32_SIGKILL;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)obj));
}

void
_impl_ecore_exe_auto_limits_set(Ecore_Exe *obj EINA_UNUSED,
                                Ecore_Exe_Data *exe EINA_UNUSED,
                                int        start_bytes EINA_UNUSED,
                                int        end_bytes EINA_UNUSED,
                                int        start_lines EINA_UNUSED,
                                int        end_lines EINA_UNUSED)
{
   ERR("Not implemented on windows!");
}

void
_impl_ecore_exe_signal(Ecore_Exe *obj EINA_UNUSED,
                       Ecore_Exe_Data *exe EINA_UNUSED,
                       int num EINA_UNUSED)
{
   ERR("Not implemented on windows!");
}

void
_impl_ecore_exe_hup(Ecore_Exe *obj EINA_UNUSED,
                    Ecore_Exe_Data *exe EINA_UNUSED)
{
   ERR("Not implemented on windows!");
}
