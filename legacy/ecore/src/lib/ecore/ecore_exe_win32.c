/*
 * TODO:
 * - manage I/O pipes (several ones, and stdin)
 * - manage SetConsoleCtrlHandler ?
 * - the child process seems to still run after the DEL event
 * - add log messages
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <process.h>

#define ECORE_EXE_WIN32_TIMEOUT 3000

typedef enum
{
   ECORE_EXE_WIN32_SIGINT,
   ECORE_EXE_WIN32_SIGQUIT,
   ECORE_EXE_WIN32_SIGTERM,
   ECORE_EXE_WIN32_SIGKILL
} Ecore_Exe_Win32_Signal;

struct _Ecore_Exe
{
   EINA_INLIST;
                          ECORE_MAGIC;

   HANDLE                 process2;
   HANDLE                 process; /* CloseHandle */
   HANDLE                 process_thread;
   DWORD                  process_id;
   DWORD                  thread_id;
   void                  *data;
   char                  *tag;
   char                  *cmd;
   Ecore_Exe_Flags        flags;
   Ecore_Exe_Win32_Signal sig;
   Ecore_Win32_Handler   *h_close;
   struct
   {
      HANDLE      child_pipe;
      HANDLE      child_pipe_x;
      Ecore_Pipe *p;
      HANDLE      thread;
      void       *data_buf;
      int         data_size;
   } pipe_read;
   struct
   {
      HANDLE               child_pipe;
      HANDLE               child_pipe_x;
      HANDLE               thread;
      Ecore_Win32_Handler *h;
      void                *data_buf;
      int                  data_size;
   } pipe_write;
   struct
   {
      HANDLE      child_pipe;
      HANDLE      child_pipe_x;
      Ecore_Pipe *p;
      HANDLE      thread;
      void       *data_buf;
      int         data_size;
   } pipe_error;
   Eina_Bool    close_stdin : 1;
   Eina_Bool    is_suspended : 1;

   Ecore_Exe_Cb pre_free_cb;
};

static Ecore_Exe *exes = NULL;

static int           _ecore_exe_win32_pipes_set(Ecore_Exe *exe);
static void          _ecore_exe_win32_pipes_close(Ecore_Exe *exe);

static BOOL CALLBACK _ecore_exe_enum_windows_procedure(HWND   window,
                                                       LPARAM data);
static void          _ecore_exe_event_add_free(void *data,
                                               void *ev);
static void          _ecore_exe_event_del_free(void *data,
                                               void *ev);
static void          _ecore_exe_event_exe_data_free(void *data,
                                                    void *ev);
static int           _ecore_exe_win32_pipe_thread_generic_cb(void           *data,
                                                             Ecore_Exe_Flags flags);
static DWORD WINAPI  _ecore_exe_win32_pipe_thread_read_cb(void *data);
static DWORD WINAPI  _ecore_exe_win32_pipe_thread_error_cb(void *data);
static Eina_Bool     _ecore_exe_close_cb(void                *data,
                                         Ecore_Win32_Handler *wh);
static void          _ecore_exe_pipe_read_cb(void        *data,
                                             void        *buf,
                                             unsigned int size);
static int  _ecore_exe_pipe_write_cb(void                *data,
                                     Ecore_Win32_Handler *wh);
static void _ecore_exe_pipe_error_cb(void        *data,
                                     void        *buf,
                                     unsigned int size);

EAPI int ECORE_EXE_EVENT_ADD = 0;
EAPI int ECORE_EXE_EVENT_DEL = 0;
EAPI int ECORE_EXE_EVENT_DATA = 0;
EAPI int ECORE_EXE_EVENT_ERROR = 0;

void
_ecore_exe_init(void)
{
   ECORE_EXE_EVENT_ADD = ecore_event_type_new();
   ECORE_EXE_EVENT_DEL = ecore_event_type_new();
   ECORE_EXE_EVENT_DATA = ecore_event_type_new();
   ECORE_EXE_EVENT_ERROR = ecore_event_type_new();
}

void
_ecore_exe_shutdown(void)
{
   while (exes)
     ecore_exe_free(exes);
}

static int run_pri = NORMAL_PRIORITY_CLASS;

EAPI void
ecore_exe_run_priority_set(int pri)
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

EAPI int
ecore_exe_run_priority_get(void)
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

EAPI Ecore_Exe *
ecore_exe_run(const char *exe_cmd,
              const void *data)
{
   return ecore_exe_pipe_run(exe_cmd, 0, data);
}

EAPI Ecore_Exe *
ecore_exe_pipe_run(const char     *exe_cmd,
                   Ecore_Exe_Flags flags,
                   const void     *data)
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   Ecore_Exe_Event_Add *e;
   Ecore_Exe *exe;
   char *ret = NULL;

   exe = calloc(1, sizeof(Ecore_Exe));
   if (!exe)
     return NULL;

   if ((flags & ECORE_EXE_PIPE_AUTO) && (!(flags & ECORE_EXE_PIPE_ERROR))
       && (!(flags & ECORE_EXE_PIPE_READ)))
     /* We need something to auto pipe. */
     flags |= ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_ERROR;

   exe->flags = flags;
   if (exe->flags & ECORE_EXE_PIPE_READ)
     if (!_ecore_exe_win32_pipes_set(exe))
       goto free_exe;

   if (exe->flags & ECORE_EXE_PIPE_WRITE)
     if (!_ecore_exe_win32_pipes_set(exe))
       goto close_pipes;

   if (exe->flags & ECORE_EXE_PIPE_ERROR)
     if (!_ecore_exe_win32_pipes_set(exe))
       goto close_pipes;

   if ((exe->flags & ECORE_EXE_USE_SH) ||
       ((ret = strrstr(exe_cmd, ".bat")) && (ret[4] == '\0')))
     {
        char buf[PATH_MAX];
        snprintf(buf, PATH_MAX, "cmd.exe /c %s", exe_cmd);
        exe->cmd = strdup(buf);
     }
   else
     exe->cmd = strdup(exe_cmd);

   if (!exe->cmd)
     goto close_pipes;

   ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO);
   si.hStdOutput = exe->pipe_read.child_pipe_x;
   si.hStdInput = exe->pipe_write.child_pipe;
   si.hStdError = exe->pipe_error.child_pipe_x;
   si.dwFlags |= STARTF_USESTDHANDLES;

   /* FIXME: gerer la priorite */

   if (!CreateProcess(NULL, exe->cmd, NULL, NULL, EINA_TRUE,
                      run_pri | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
     goto free_exe_cmd;

   /* be sure that the child process is running */
   /* FIXME: This does not work if the child is an EFL-based app */
   /* if (WaitForInputIdle(pi.hProcess, INFINITE) == WAIT_FAILED) */
   /*   goto free_exe_cmd; */

   ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
   exe->process = pi.hProcess;
   exe->process_thread = pi.hThread;
   exe->process_id = pi.dwProcessId;
   exe->thread_id = pi.dwThreadId;
   exe->data = (void *)data;

   if (!(exe->process2 = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE | SYNCHRONIZE,
                                     EINA_FALSE, pi.dwProcessId)))
     goto close_thread;

   exe->h_close = ecore_main_win32_handler_add(exe->process2, _ecore_exe_close_cb, exe);
   if (!exe->h_close) goto close_process2;

   if (ResumeThread(exe->process_thread) == ((DWORD)-1))
     goto close_process2;

   exes = (Ecore_Exe *)eina_inlist_append(EINA_INLIST_GET(exes), EINA_INLIST_GET(exe));

   e = (Ecore_Exe_Event_Add *)calloc(1, sizeof(Ecore_Exe_Event_Add));
   if (!e) goto delete_h_close;

   e->exe = exe;
   ecore_event_add(ECORE_EXE_EVENT_ADD, e,
                   _ecore_exe_event_add_free, NULL);

   return exe;

delete_h_close:
   ecore_main_win32_handler_del(exe->h_close);
close_process2:
   CloseHandle(exe->process2);
close_thread:
   CloseHandle(exe->process_thread);
   CloseHandle(exe->process);
free_exe_cmd:
   free(exe->cmd);
close_pipes:
   _ecore_exe_win32_pipes_close(exe);
free_exe:
   free(exe);
   return NULL;
}

EAPI void
ecore_exe_callback_pre_free_set(Ecore_Exe   *exe,
                                Ecore_Exe_Cb func)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
                         "ecore_exe_callback_pre_free_set");
        return;
     }
   exe->pre_free_cb = func;
}

EAPI Eina_Bool
ecore_exe_send(Ecore_Exe  *exe,
               const void *data,
               int         size)
{
   void *buf;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_send");
        return 0;
     }

   if (exe->close_stdin)
     {
        ERR("Ecore_Exe %p stdin is closed! Cannot send %d bytes from %p",
            exe, size, data);
        return 0;
     }

   if (!exe->pipe_write.child_pipe)
     {
        ERR("Ecore_Exe %p created without ECORE_EXE_PIPE_WRITE! "
            "Cannot send %d bytes from %p", exe, size, data);
        return 0;
     }

   buf = realloc(exe->pipe_write.data_buf, exe->pipe_write.data_size + size);
   if (!buf) return 0;

   exe->pipe_write.data_buf = buf;
   memcpy((char *)exe->pipe_write.data_buf + exe->pipe_write.data_size, data, size);
   exe->pipe_write.data_size += size;

   /* if (exe->pipe_write.) */
   /*    ecore_main_fd_handler_active_set(exe->pipe_write.h, ECORE_FD_WRITE); */

   return 1;
}

EAPI void
ecore_exe_close_stdin(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_close_stdin");
        return;
     }
   exe->close_stdin = 1;
}

/* Not used on Windows */
EAPI void
ecore_exe_auto_limits_set(Ecore_Exe *exe __UNUSED__,
                          int        start_bytes __UNUSED__,
                          int        end_bytes __UNUSED__,
                          int        start_lines __UNUSED__,
                          int        end_lines __UNUSED__)
{
}

EAPI Ecore_Exe_Event_Data *
ecore_exe_event_data_get(Ecore_Exe      *exe,
                         Ecore_Exe_Flags flags)
{
   Ecore_Exe_Event_Data *e = NULL;
   unsigned char *inbuf;
   int inbuf_num;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_event_data_get");
        return NULL;
     }

   /* Sort out what sort of event we are, */
   /* And get the data. */
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
        e->exe = exe;
        e->data = inbuf;
        e->size = inbuf_num;
     }

   return e;
}

EAPI void
ecore_exe_event_data_free(Ecore_Exe_Event_Data *e)
{
   if (!e) return;
   IF_FREE(e->lines);
   IF_FREE(e->data);
   free(e);
}

EAPI void *
ecore_exe_free(Ecore_Exe *exe)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_free");
        return NULL;
     }

   data = exe->data;

   if (exe->pre_free_cb)
     exe->pre_free_cb(data, exe);

   CloseHandle(exe->process2);
   CloseHandle(exe->process_thread);
   CloseHandle(exe->process);
   free(exe->cmd);
   _ecore_exe_win32_pipes_close(exe);
   exes = (Ecore_Exe *)eina_inlist_remove(EINA_INLIST_GET(exes), EINA_INLIST_GET(exe));
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   if (exe->tag) free(exe->tag);
   free(exe);

   return data;
}

EAPI pid_t
ecore_exe_pid_get(const Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_pid_get");
        return -1;
     }
   return exe->process_id;
}

EAPI void
ecore_exe_tag_set(Ecore_Exe  *exe,
                  const char *tag)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_tag_set");
        return;
     }
   IF_FREE(exe->tag);
   if (tag)
     exe->tag = strdup(tag);
}

EAPI const char *
ecore_exe_tag_get(const Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_tag_get");
        return NULL;
     }
   return exe->tag;
}

EAPI const char *
ecore_exe_cmd_get(const Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_cmd_get");
        return NULL;
     }
   return exe->cmd;
}

EAPI void *
ecore_exe_data_get(const Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_data_get");
        return NULL;
     }
   return exe->data;
}

EAPI Ecore_Exe_Flags
ecore_exe_flags_get(const Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_data_get");
        return 0;
     }
   return exe->flags;
}

EAPI void
ecore_exe_pause(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_pause");
        return;
     }

   if (exe->is_suspended)
     return;

   if (SuspendThread(exe->process_thread) != (DWORD)-1)
     exe->is_suspended = 1;
}

EAPI void
ecore_exe_continue(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_continue");
        return;
     }

   if (!exe->is_suspended)
     return;

   if (ResumeThread(exe->process_thread) != (DWORD)-1)
     exe->is_suspended = 0;
}

EAPI void
ecore_exe_interrupt(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_interrupt");
        return;
     }

   CloseHandle(exe->process_thread);
   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGINT;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe)) ;
}

EAPI void
ecore_exe_quit(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_quit");
        return;
     }

   CloseHandle(exe->process_thread);
   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGQUIT;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe)) ;
}

EAPI void
ecore_exe_terminate(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_terminate");
        return;
     }

/*    CloseHandle(exe->thread); */
   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGTERM;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe)) ;
}

EAPI void
ecore_exe_kill(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_kill");
        return;
     }

   CloseHandle(exe->process_thread);
   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGKILL;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe)) ;
}

EAPI void
ecore_exe_signal(Ecore_Exe *exe,
                 int        num __UNUSED__)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_signal");
        return;
     }

   /* does nothing */
}

EAPI void
ecore_exe_hup(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_hup");
        return;
     }

   /* does nothing */
}

/* FIXME: manage error mode */
static int
_ecore_exe_win32_pipe_thread_generic_cb(void           *data,
                                        Ecore_Exe_Flags flags)
{
#define BUFSIZE 2048
   char buf[BUFSIZE];
   Ecore_Exe *exe;
   char *current_buf = NULL;
   HANDLE child_pipe;
   Ecore_Pipe *ecore_pipe;
   Ecore_Exe_Event_Data *event;
   DWORD size;
   DWORD current_size = 0;
   BOOL res;

   exe = (Ecore_Exe *)data;

   /* Sort out what sort of handler we are. */
   /* And get any left over data from last time. */
   if ((exe->flags & ECORE_EXE_PIPE_READ) && (flags == ECORE_EXE_PIPE_READ))
     {
        child_pipe = exe->pipe_read.child_pipe;
        ecore_pipe = exe->pipe_read.p;
        flags = ECORE_EXE_PIPE_READ;
     }
   else if ((exe->flags & ECORE_EXE_PIPE_ERROR) && (flags == ECORE_EXE_PIPE_ERROR))
     {
        child_pipe = exe->pipe_error.child_pipe;
        ecore_pipe = exe->pipe_error.p;
        flags = ECORE_EXE_PIPE_ERROR;
     }
   else
     return 0;

   while (1)
     {
        if (!PeekNamedPipe(child_pipe, buf, sizeof(buf), &size, &current_size, NULL))
          continue;
        if (size == 0)
          continue;
        current_buf = (char *)malloc(current_size);
        if (!current_buf)
          continue;
        res = ReadFile(child_pipe, current_buf, current_size, &size, NULL);
        if (!res || (size == 0))
          {
             free(current_buf);
             current_buf = NULL;
             continue;
          }
        if (current_size != size)
          {
             free(current_buf);
             current_buf = NULL;
             continue;
          }
        current_size = size;

        if (flags == ECORE_EXE_PIPE_READ)
          {
             exe->pipe_read.data_buf = current_buf;
             exe->pipe_read.data_size = current_size;
          }
        else
          {
             exe->pipe_error.data_buf = current_buf;
             exe->pipe_error.data_size = current_size;
          }

        event = ecore_exe_event_data_get(exe, flags);
        if (event)
          ecore_pipe_write(ecore_pipe, &event, sizeof(event));

        current_buf = NULL;
        current_size = 0;
     }

   return 1;
}

static DWORD WINAPI
_ecore_exe_win32_pipe_thread_read_cb(void *data)
{
   return _ecore_exe_win32_pipe_thread_generic_cb(data, ECORE_EXE_PIPE_READ);
}

static DWORD WINAPI
_ecore_exe_win32_pipe_thread_error_cb(void *data)
{
   return _ecore_exe_win32_pipe_thread_generic_cb(data, ECORE_EXE_PIPE_ERROR);
}

static int
_ecore_exe_win32_pipes_set(Ecore_Exe *exe)
{
   SECURITY_ATTRIBUTES sa;
   HANDLE child_pipe;
   HANDLE child_pipe_x;

   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle = EINA_TRUE;
   sa.lpSecurityDescriptor = NULL;

   if (!CreatePipe(&child_pipe, &child_pipe_x, &sa, 0))
     return 0;
   if (exe->flags & ECORE_EXE_PIPE_WRITE)
     {
        if (!SetHandleInformation(child_pipe_x, HANDLE_FLAG_INHERIT, 0))
          goto close_pipe;
     }
   else
     {
        if (!SetHandleInformation(child_pipe, HANDLE_FLAG_INHERIT, 0))
          goto close_pipe;
     }

   if (exe->flags & ECORE_EXE_PIPE_READ)
     {
        exe->pipe_read.child_pipe = child_pipe;
        exe->pipe_read.child_pipe_x = child_pipe_x;
        exe->pipe_read.p = ecore_pipe_add(_ecore_exe_pipe_read_cb, exe);
        exe->pipe_read.thread = CreateThread(NULL, 0,
                                             _ecore_exe_win32_pipe_thread_read_cb,
                                             exe, 0, NULL);
     }
   else if (exe->flags & ECORE_EXE_PIPE_WRITE)
     {
        exe->pipe_write.child_pipe = child_pipe;
        exe->pipe_write.child_pipe_x = child_pipe_x;
/*         exe->pipe_write.thread = CreateThread(NULL, 0, */
/*                                              _ecore_exe_win32_pipe_thread_cb, */
/*                                              exe, 0, NULL); */
     }
   else
     {
        exe->pipe_error.child_pipe = child_pipe;
        exe->pipe_error.child_pipe_x = child_pipe_x;
        exe->pipe_error.p = ecore_pipe_add(_ecore_exe_pipe_error_cb, exe);
        exe->pipe_error.thread = CreateThread(NULL, 0,
                                              _ecore_exe_win32_pipe_thread_error_cb,
                                              exe, 0, NULL);
     }

   return 1;

close_pipe:
   CloseHandle(child_pipe);
   CloseHandle(child_pipe_x);

   return 0;
}

static void
_ecore_exe_win32_pipes_close(Ecore_Exe *exe)
{
   if (exe->flags & ECORE_EXE_PIPE_READ)
     {
        if (exe->pipe_read.child_pipe)
          {
             CloseHandle(exe->pipe_read.child_pipe);
             exe->pipe_read.child_pipe = NULL;
          }
        if (exe->pipe_read.child_pipe_x)
          {
             CloseHandle(exe->pipe_read.child_pipe_x);
             exe->pipe_read.child_pipe_x = NULL;
          }
     }

   if (exe->flags & ECORE_EXE_PIPE_WRITE)
     {
        if (exe->pipe_write.child_pipe)
          {
             CloseHandle(exe->pipe_write.child_pipe);
             exe->pipe_write.child_pipe = NULL;
          }
        if (exe->pipe_write.child_pipe_x)
          {
             CloseHandle(exe->pipe_write.child_pipe_x);
             exe->pipe_write.child_pipe_x = NULL;
          }
     }

   if (exe->flags & ECORE_EXE_PIPE_ERROR)
     {
        if (exe->pipe_error.child_pipe)
          {
             CloseHandle(exe->pipe_error.child_pipe);
             exe->pipe_error.child_pipe = NULL;
          }
        if (exe->pipe_error.child_pipe_x)
          {
             CloseHandle(exe->pipe_error.child_pipe_x);
             exe->pipe_error.child_pipe_x = NULL;
          }
     }
}

static DWORD WINAPI
_ecore_exe_thread_procedure(LPVOID data __UNUSED__)
{
   GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
   GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
   return 1;
}

static BOOL CALLBACK
_ecore_exe_enum_windows_procedure(HWND   window,
                                  LPARAM data)
{
   Ecore_Exe *exe;
   DWORD thread_id;

   exe = (Ecore_Exe *)data;
   thread_id = GetWindowThreadProcessId(window, NULL);

   if (thread_id == exe->thread_id)
     {
        /* Ctrl-C or Ctrl-Break */
         if (CreateRemoteThread(exe->process, NULL, 0,
                                (LPTHREAD_START_ROUTINE)_ecore_exe_thread_procedure, NULL,
                                0, NULL))
           {
              printf ("remote thread\n");
              return EINA_FALSE;
           }

         if ((exe->sig == ECORE_EXE_WIN32_SIGINT) ||
             (exe->sig == ECORE_EXE_WIN32_SIGQUIT))
           {
              printf ("int or quit\n");
              return EINA_FALSE;
           }

         /* WM_CLOSE message */
         PostMessage(window, WM_CLOSE, 0, 0);
         if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
           {
              printf ("CLOSE\n");
              return EINA_FALSE;
           }

         /* WM_QUIT message */
         PostMessage(window, WM_QUIT, 0, 0);
         if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
           {
              printf ("QUIT\n");
              return EINA_FALSE;
           }

         /* Exit process */
         if (CreateRemoteThread(exe->process, NULL, 0,
                                (LPTHREAD_START_ROUTINE)ExitProcess, NULL,
                                0, NULL))
           {
              printf ("remote thread 2\n");
              return EINA_FALSE;
           }

         if (exe->sig == ECORE_EXE_WIN32_SIGTERM)
           {
              printf ("term\n");
              return EINA_FALSE;
           }

         TerminateProcess(exe->process, 0);

         return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_ecore_exe_event_add_free(void *data __UNUSED__,
                          void *ev)
{
   Ecore_Exe_Event_Add *e;

   e = (Ecore_Exe_Event_Add *)ev;
   free(e);
}

static void
_ecore_exe_event_del_free(void *data __UNUSED__,
                          void *ev)
{
   Ecore_Exe_Event_Del *e;

   e = (Ecore_Exe_Event_Del *)ev;
   if (e->exe)
     ecore_exe_free(e->exe);
   free(e);
}

static void
_ecore_exe_event_exe_data_free(void *data __UNUSED__,
                               void *ev)
{
   Ecore_Exe_Event_Data *e;

   e = (Ecore_Exe_Event_Data *)ev;
   ecore_exe_event_data_free(e);
}

static Eina_Bool
_ecore_exe_close_cb(void                *data,
                    Ecore_Win32_Handler *wh __UNUSED__)
{
   Ecore_Exe_Event_Del *e;
   Ecore_Exe *exe;
   DWORD exit_code = 0;

   e = calloc(1, sizeof(Ecore_Exe_Event_Del));
   if (!e) return 0;

   exe = (Ecore_Exe *)data;

   if (GetExitCodeProcess(exe->process2, &exit_code))
     {
        e->exit_code = exit_code;
        e->exited = 1;
     }
   else
     {
        char *msg;

        msg = evil_last_error_get();
        printf("%s\n", msg);
        free(msg);
     }
   e->pid = exe->process_id;
   e->exe = exe;

   ecore_event_add(ECORE_EXE_EVENT_DEL, e,
                   _ecore_exe_event_del_free, NULL);

   return 0;
}

static void
_ecore_exe_pipe_read_cb(void        *data,
                        void        *buf,
                        unsigned int size)
{
   Ecore_Exe_Event_Data *e;

   e = *((Ecore_Exe_Event_Data **)buf);
   if (e)
     ecore_event_add(ECORE_EXE_EVENT_DATA, e,
                     _ecore_exe_event_exe_data_free,
                     NULL);
}

static int
_ecore_exe_pipe_write_cb(void                *data,
                         Ecore_Win32_Handler *wh __UNUSED__)
{
   char buf[READBUFSIZ];
   Ecore_Exe *exe;
   DWORD num_exe;
   BOOL res;

   exe = (Ecore_Exe *)data;

   res = WriteFile(exe->pipe_write.child_pipe_x, buf, READBUFSIZ, &num_exe, NULL);
   if (!res || num_exe == 0)
     {
        /* FIXME: what to do here ?? */
     }

   if (exe->close_stdin == 1)
     {
        if (exe->pipe_write.h)
          {
             ecore_main_win32_handler_del(exe->pipe_write.h);
             exe->pipe_write.h = NULL;
          }
        exe->pipe_write.h = NULL;
        CloseHandle(exe->pipe_write.child_pipe);
        exe->pipe_write.child_pipe = NULL;
     }

   return 1;
}

static void
_ecore_exe_pipe_error_cb(void        *data,
                         void        *buf,
                         unsigned int size)
{
   Ecore_Exe_Event_Data *e;

   e = *((Ecore_Exe_Event_Data **)buf);
   if (e)
     ecore_event_add(ECORE_EXE_EVENT_ERROR, e,
                     _ecore_exe_event_exe_data_free,
                     NULL);
}

