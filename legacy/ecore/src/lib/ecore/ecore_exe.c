#include "ecore_private.h"
#include "Ecore.h"

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

static Ecore_Exe *exes = NULL;

/**
 * Spawn off a child process and return a handle to that process.
 * @param exe_cmd The command-line to run as if typed in a shell
 * @param data A generic data pointer to attach to the process handle
 * @return A process handle to the spawned off process
 * 
 * This function will fork and execute the command line @p exe_cmd as specified
 * using the system shell (/bin/sh). The data pointer @p data is attached to
 * the process handle returned. The application does not need to free or keep
 * the process handle returned unless it has a use for it. It should be noted
 * that the process handle is only valid until a child process terminated
 * event is recieved. After all handlers for this child process terminated
 * event have been called, this process handle will be freed and cleaned up
 * by Ecore, and so any references to it will become invalid.
 */
Ecore_Exe *
ecore_exe_run(const char *exe_cmd, const void *data)
{
   Ecore_Exe *exe;
   pid_t pid;

   if (!exe_cmd) return NULL;
   pid = fork();   
   if (pid)
     {
	exe = calloc(1, sizeof(Ecore_Exe));
	if (!exe)
	  {
	     kill(pid, SIGKILL);
	     return NULL;
	  }
	ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
	exe->pid = pid;
	exe->data = (void *)data;
	exes = _ecore_list_append(exes, exe);
	return exe;
     }
   setsid();
   execl("/bin/sh", "/bin/sh", "-c", exe_cmd, NULL);
   exit(0);
   return NULL;
}

/**
 * Free an exe handle.
 * @param exe
 * @return The data pointer set on execution of the program
 * 
 * This frees an exe handle (but does not affect the process that was spawned
 * that this handle was a result of) and returns the data pointer set on
 * executable start. This does mean there is no handle for the spawned
 * process anymore.
 */
void *
ecore_exe_free(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_free");
	return NULL;
     }
   return _ecore_exe_free(exe);
}

/**
 * Get the process ID of a spawned process.
 * @param exe The process handle returned by ecore_exe_run()
 * @return A system process ID of the process handle
 * 
 * This function returns the system process ID of a spawned off child process.
 */
pid_t
ecore_exe_pid_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_pid_get");
	return -1;
     }
   return exe->pid;
}

/**
 * Get the data pointer attached to a process handle.
 * @param exe The process handle returned by ecore_exe_run()
 * @return An pointer to the attached data of the process handle
 * 
 * This function returns the data pointer attached to the spawned off process
 * whose handle is @p exe.
 */
void *
ecore_exe_data_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_data_get");
	return NULL;
     }
   return exe->data;
}

/**
 * Pause a spawned process.
 * @param exe The process handle to control
 * 
 * This function pauses a process that was spawned.
 */
void
ecore_exe_pause(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_pause");
	return;
     }
   kill(exe->pid, SIGSTOP);
}

/**
 * Continue a paused process.
 * @param exe The process handle to control
 * 
 * This Continues a process. This is only useful if the process has already
 * been paused by something like ecore_exe_pause().
 */
void
ecore_exe_continue(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_continue");
	return;
     }
   kill(exe->pid, SIGCONT);
}

/**
 * Terminate a process.
 * @param exe The process handle to control
 * 
 * This function asks a process to terminate.
 */
void
ecore_exe_terminate(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_terminate");
	return;
     }
   kill(exe->pid, SIGTERM);
}

/**
 * Kill a process.
 * @param exe The process handle to control
 * 
 * This function ills off a process, and that process has no choice and will
 * exit as a result of this function, without having a chance to clean up,
 * save data, or safely shut down.
 */
void
ecore_exe_kill(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_kill");
	return;
     }
   kill(exe->pid, SIGKILL);
}

/**
 * Send a user signal to a process.
 * @param exe The process handle to control
 * @param num The signal number to send to the process
 * 
 * This function sends a user signal (SIGUSR) to a process. @p num determines
 * what numbered user signal to send. This may be either 1 or 2. Other values
 * are illegal and will be ignored, with this function doing nothing.
 */
void
ecore_exe_signal(Ecore_Exe *exe, int num)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_signal");
	return;
     }
   if (num == 1)
     kill(exe->pid, SIGUSR1);
   else if (num == 2)
     kill(exe->pid, SIGUSR2);
}

/**
 * Send a HUP signal to a process.
 * @param exe The process handle to control
 * 
 * This function sends a HUP signal to the specified process.
 */
void
ecore_exe_hup(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_hup");
	return;
     }
   kill(exe->pid, SIGHUP);
}

void
_ecore_exe_shutdown(void)
{
   while (exes) _ecore_exe_free(exes);
}

Ecore_Exe *
_ecore_exe_find(pid_t pid)
{
   Ecore_Oldlist *l;
   
   for (l = (Ecore_Oldlist *)exes; l; l = l->next)
     {
	Ecore_Exe *exe;
	
	exe = (Ecore_Exe *)l;
	if (exe->pid == pid) return exe;
     }
   return NULL;
}

void *
_ecore_exe_free(Ecore_Exe *exe)
{
   void *data;
   
   data = exe->data;
   exes = _ecore_list_remove(exes, exe);
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   free(exe);
   return data;
}
