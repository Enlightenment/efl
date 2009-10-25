/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * TODO:
 * - manage priority
 * - manage I/O pipes
 * - add events for start/close, maybe others
 * - manage SetConsoleCtrlHandler ?
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_private.h"
#include "Ecore.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

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

   HANDLE process; /* CloseHandle */
   HANDLE thread;
   DWORD  process_id;
   DWORD  thread_id;
   void  *data;
   char  *tag;
   char  *cmd;
   Ecore_Exe_Win32_Signal sig;
   int    is_suspended : 1;
};

static Ecore_Exe *exes = NULL;

static BOOL CALLBACK _ecore_exe_enum_windows_procedure(HWND window, LPARAM data);

void
_ecore_exe_init(void)
{
/*    ECORE_EXE_EVENT_ADD = ecore_event_type_new(); */
/*    ECORE_EXE_EVENT_DEL = ecore_event_type_new(); */
/*    ECORE_EXE_EVENT_DATA = ecore_event_type_new(); */
/*    ECORE_EXE_EVENT_ERROR = ecore_event_type_new(); */
}

void
_ecore_exe_shutdown(void)
{
   while (exes)
      ecore_exe_free(exes);
}

EAPI void ecore_exe_run_priority_set(int pri)
{
}

EAPI int ecore_exe_run_priority_get(void)
{
}

EAPI Ecore_Exe *ecore_exe_run(const char *exe_cmd, const void *data)
{
   return ecore_exe_pipe_run(exe_cmd, 0, data);
}

EAPI Ecore_Exe *ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data)
{
   STARTUPINFO         si;
   PROCESS_INFORMATION pi;
   Ecore_Exe          *exe;
   char               *ret = NULL;

   exe = calloc(1, sizeof(Ecore_Exe));
   if (!exe)
     return NULL;

   if ((flags & ECORE_EXE_USE_SH) ||
       ((ret = strrchr(exe_cmd, ".bat")) && (ret[4] == '\0')))
     {
        char buf[PATH_MAX];
        snprintf(buf, PATH_MAX, "cmd.exe /c %s", exe_cmd);
        exe->cmd = strdup(buf);
     }
   else
     exe->cmd = strdup(exe_cmd);

   if (!exe->cmd)
     goto free_exe;

   ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb = sizeof(STARTUPINFO); 
/*    si.hStdError = g_hChildStd_OUT_Wr; */
/*    si.hStdOutput = g_hChildStd_OUT_Wr; */
/*    si.hStdInput = g_hChildStd_IN_Rd; */
/*    si.dwFlags |= STARTF_USESTDHANDLES; */

   /* FIXME: gerer la priorite */

   if (!CreateProcess(NULL, exe->cmd, NULL, NULL, TRUE,
                      0, NULL, NULL, &si, &pi))
     goto free_exe_cmd;

   /* be sure that the child process is running */
   if (WaitForInputIdle(pi.hProcess, INFINITE) != 0)
     goto free_exe_cmd;

   exe->thread = pi.hThread;
   exe->process_id = pi.dwProcessId;
   exe->thread_id = pi.dwThreadId;
   exe->data = (void *)data;
   CloseHandle(pi.hProcess);

   if (!(exe->process = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE | SYNCHRONIZE,
                                    FALSE, pi.dwProcessId)))
     goto close_thread;

   exes = (Ecore_Exe *)eina_inlist_append(EINA_INLIST_GET(exes), EINA_INLIST_GET(exe));
   return exe;

 close_thread:
   CloseHandle(pi.hThread);
 free_exe_cmd:
   free(exe->cmd);
 free_exe:
   free(exe);
   return NULL;
}

EAPI int ecore_exe_send(Ecore_Exe *exe, void *data, int size)
{
}

EAPI void ecore_exe_close_stdin(Ecore_Exe *exe)
{
}

EAPI void ecore_exe_auto_limits_set(Ecore_Exe *exe, int start_bytes, int end_bytes, int start_lines, int end_lines)
{
}

EAPI Ecore_Exe_Event_Data *ecore_exe_event_data_get(Ecore_Exe *exe, Ecore_Exe_Flags flags)
{
}

EAPI void ecore_exe_event_data_free(Ecore_Exe_Event_Data *data)
{
}

EAPI void *ecore_exe_free(Ecore_Exe *exe)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_free");
	return NULL;
     }

   data = exe->data;

   CloseHandle(exe->process);
   CloseHandle(exe->thread);
   free(exe->cmd);
   exes = (Ecore_Exe *) eina_inlist_remove(EINA_INLIST_GET(exes), EINA_INLIST_GET(exe));
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   if (exe->tag) free(exe->tag);
   free(exe);

   return data;
}

EAPI pid_t ecore_exe_pid_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_pid_get");
	return -1;
     }
   return exe->process_id;
}

EAPI void ecore_exe_tag_set(Ecore_Exe *exe, const char *tag)
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

EAPI char *ecore_exe_tag_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_tag_get");
	return NULL;
     }
   return exe->tag;
}

EAPI char *ecore_exe_cmd_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_cmd_get");
	return NULL;
     }
   return exe->cmd;
}

EAPI void *ecore_exe_data_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_data_get");
	return NULL;
     }
   return exe->data;
}

EAPI void ecore_exe_pause(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_pause");
	return;
     }

   if (exe->is_suspended)
     return;

   if (SuspendThread(exe->thread) != (DWORD)-1)
     exe->is_suspended = 1;
}

EAPI void ecore_exe_continue(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_continue");
	return;
     }

   if (!exe->is_suspended)
     return;

   if (ResumeThread(exe->thread) != (DWORD)-1)
     exe->is_suspended = 0;
}

EAPI void ecore_exe_interrupt(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_interrupt");
	return;
     }

   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGINT;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe));
}

EAPI void ecore_exe_quit(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_quit");
	return;
     }

   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGQUIT;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe));
}

EAPI void ecore_exe_terminate(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_terminate");
	return;
     }

   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGTERM;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe));
}

EAPI void ecore_exe_kill(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_kill");
	return;
     }

   CloseHandle(exe->process);
   exe->sig = ECORE_EXE_WIN32_SIGKILL;
   while (EnumWindows(_ecore_exe_enum_windows_procedure, (LPARAM)exe));
}

EAPI void ecore_exe_signal(Ecore_Exe *exe, int num)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_signal");
	return;
     }

   /* does nothing */
}

EAPI void ecore_exe_hup(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE, "ecore_exe_hup");
	return;
     }

   /* does nothing */
}

static DWORD WINAPI
_ecore_exe_thread_procedure(LPVOID data)
{
   GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
   GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
   return 1;
}

static BOOL CALLBACK
_ecore_exe_enum_windows_procedure(HWND window, LPARAM data)
{
   Ecore_Exe *exe;
   DWORD      thread_id;

   exe = (Ecore_Exe *)data;
   thread_id = GetWindowThreadProcessId(window, NULL);

   if (thread_id == exe->thread_id)
     {
        /* Ctrl-C or Ctrl-Break */
        if (CreateRemoteThread(exe->process, NULL, 0,
                               (LPTHREAD_START_ROUTINE)_ecore_exe_thread_procedure, NULL,
                               0, NULL))
          return FALSE;

        if ((exe->sig == ECORE_EXE_WIN32_SIGINT) ||
            (exe->sig == ECORE_EXE_WIN32_SIGQUIT))
          return FALSE;

        /* WM_CLOSE message */
        PostMessage(window, WM_CLOSE, 0, 0);
        if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
          return FALSE;

        /* WM_QUIT message */
        PostMessage(window, WM_QUIT, 0, 0);
        if (WaitForSingleObject(exe->process, ECORE_EXE_WIN32_TIMEOUT) == WAIT_OBJECT_0)
          return FALSE;

        /* Exit process */
        if (CreateRemoteThread(exe->process, NULL, 0,
                               (LPTHREAD_START_ROUTINE)ExitProcess, NULL,
                               0, NULL))
          return FALSE;
        if (exe->sig == ECORE_EXE_WIN32_SIGTERM)
          return FALSE;

        TerminateProcess(exe->process, 0);

        return FALSE;
     }

   return TRUE;
}

#endif
