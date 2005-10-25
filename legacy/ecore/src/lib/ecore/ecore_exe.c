#include "ecore_private.h"
#include "Ecore.h"

#include <errno.h>
#include <sys/wait.h>

#ifndef WIN32
static Ecore_Exe *exes = NULL;

/**
 * @defgroup Ecore_Exe_Basic_Group Process Spawning Functions
 *
 * Functions that deal with spawned processes.
 */

/**
 * Spawns a child process.
 *
 * This function forks and runs the given command using @c /bin/sh.
 *
 * Note that the process handle is only valid until a child process
 * terminated event is received.  After all handlers for the child process
 * terminated event have been called, the handle will be freed by Ecore.
 * 
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 * @ingroup Ecore_Exe_Basic_Group
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
	exes = _ecore_list2_append(exes, exe);
	return exe;
     }
   setsid();
   execl("/bin/sh", "/bin/sh", "-c", exe_cmd, (char *)NULL);
   exit(127);
   return NULL;
}


static char *shell = 0;

/**
 * Spawns a child process with its stdin/out available for communication.
 *
 * This function does the same thing as ecore_exe_run(), but also makes the
 * standard in and/or out from the child process available for reading or
 * writing. To write use ecore_exe_pipe_write(). To read listen to 
 * ECORE_EVENT_EXE_DATA events (set up a handler). Ecore may buffer read data
 * until a newline character if asked to wit the @p flags. All data will be
 * included in the events (newlines will not be stripped). This will only
 * happen if the process is run with ECORE_EXE_PIPE_READ enabled in the flags.
 * 
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   flags   The flag parameters for how to deal with inter-process I/O
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 * @ingroup Ecore_Exe_Basic_Group
 */
Ecore_Exe *
ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data)
{
   Ecore_Exe *exe = NULL;
   pid_t pid = 0;
   int dataPipe[2] = { -1, -1 };
   int statusPipe[2] = { -1, -1 };
   int n;
   volatile int vfork_exec_errno = 0;
   char **args;

   /* FIXME: 
    * set up fd handler in ecore_exe struct 
    * see ecore_con for code and examples on this (fd's there are to a socket
    * but otherwise work the same as here). the ECORE_EVENT_EXE_EXIT event
    * aces like the client del event from ecore_con - signalling that the
    * connection is closed. once this event has been handled the child
    * ecore_exe struct is freed automatically and is no longer valid.
    * 
    * when fd handlers report data - if line buffering is nto enabled instantly
    * copy data to a exe data event struct and add the event like ecore_con. if
    * line buffering is enabled, parse new data block for a \n. if there is
    * none, then simply append to read buf. if there are 1 or more, append
    * until, and including the first \n, to the existing read ubf (if any) then
    * generate data event for that. repeat for each other \n found until no \n
    * chars are left, then take trailing data (if any) and put in read buf
    * waiting for more data.
    */

   if (!exe_cmd) return NULL;

   if ((flags & (ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE)) == 0) return ecore_exe_run(exe_cmd, data);

   if (shell == 0) 
      {
         shell = getenv("SHELL");
	if (shell == 0)
	   shell = "/bin/sh";
      }

   args = (char **) calloc(4, sizeof(char *));
   n = 0;
   args[n++] = shell;
   args[n++] = "-c";
   args[n++] = exe_cmd;
   args[n++] = 0;

   if (pipe(dataPipe) < 0 || pipe(statusPipe) < 0)
      printf("Failed to create pipe\n");
   /* FIXME: I should check this. */
   signal(SIGPIPE, SIG_IGN);	/* we only want EPIPE on errors */
   pid = fork();

   if (pid == 0)
      {		/* child */
         setsid();

         if (flags & ECORE_EXE_PIPE_READ)
	    {
	       dup2(dataPipe[1], STDOUT_FILENO);
	       dup2(dataPipe[1], STDERR_FILENO);
	    }
	 else
	    {
	       close(dataPipe[1]);
               close(STDOUT_FILENO);
               close(STDERR_FILENO);
	    }
         if (flags & ECORE_EXE_PIPE_WRITE)
	    {
	       dup2(dataPipe[0], STDIN_FILENO);
	    }
	 else
	    {
	       close(dataPipe[0]);
               close(STDIN_FILENO);
	    }
	 close(statusPipe[0]);
	 fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC);	/* close on exec shows sucess */

	 errno = 0;
	 execvp(shell, (char **) args);

	 vfork_exec_errno = errno;
	 close(statusPipe[1]);
	 _exit(-1);
      }
   else if (pid > 0)
      {	/* parent */
         if (! (flags & ECORE_EXE_PIPE_READ))
	    close(dataPipe[0]);
         if (! (flags & ECORE_EXE_PIPE_WRITE))
	    close(dataPipe[1]);
	 close(statusPipe[1]);

	 while (1)
	    {
               char buf;

	       n = read(statusPipe[0], &buf, 1);

	       if (n == 0 && vfork_exec_errno != 0)
	          {
		     errno = vfork_exec_errno;
		     printf("Could not exec process\n");
		  }
	       break;
	    }
         close(statusPipe[0]);

      }
   else
      {
	  printf("Failed to fork process\n");
	  pid = 0;
      }

   n = 0;
   if (pid)
      {
	  if (WIFEXITED(n))
	    {
               if (flags & ECORE_EXE_PIPE_READ)
	          close(dataPipe[0]);
               if (flags & ECORE_EXE_PIPE_WRITE)
	          close(dataPipe[1]);

		n = WEXITSTATUS(n);
		printf("Process %s returned %i\n", exe_cmd, n);
		pid = 0;
	    }
         else
	    {
	       n = -1;

	       exe = calloc(1, sizeof(Ecore_Exe));
	       if (!exe)
	          {
                     if (flags & ECORE_EXE_PIPE_READ)
	                close(dataPipe[0]);
                     if (flags & ECORE_EXE_PIPE_WRITE)
	                close(dataPipe[1]);
	             kill(pid, SIGKILL);
                     printf("No memory for Ecore_Exe %s\n", exe_cmd);
	             return NULL;
	          }
	       ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
	       exe->pid = pid;
	       exe->flags = flags;
	       exe->data = (void *)data;
               if (flags & ECORE_EXE_PIPE_READ)
	          exe->child_fd_read = dataPipe[0];
               if (flags & ECORE_EXE_PIPE_WRITE)
	          exe->child_fd_write = dataPipe[1];
	       exes = _ecore_list2_append(exes, exe);
            }
      }

   free(args);
   errno = n;

   return exe;
}

/**
 * Writes data to the given child process which it recieves on stdin.
 * 
 * This function writes to a child processes standard in, with unlimited
 * buffering. This call will never block. It may fail if the system runs out
 * of memory.
 * 
 * @param exe  The child process to write to
 * @param data The data to write
 * @param size The size of the data to write, in bytes
 * @return 1 if successful, 0 on failure.
 * @ingroup Ecore_Exe_Basic_Group
 */
int
ecore_exe_pipe_write(Ecore_Exe *exe, void *data, int size)
{
   /* FIXME: add data to buffer and flag fd handlers to wake up when write
    * to child fd is available, and when it is, flush as much data as possible
    * at that time (much like ecore_con). this means the parent is mallocing
    * its own write buffer in process space - giving us potentially huge
    * buffers, so synchronisation needs to be done at a higher level here as
    * buffers could just get huge
    */
   return 0;
}

/**
 * Sets the string tag for the given process handle
 *
 * @param   exe The given process handle.
 * @param   tag The string tag to set on the process handle.
 * @ingroup Ecore_Exe_Basic_Group
 */
void
ecore_exe_tag_set(Ecore_Exe *exe, const char *tag)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_tag_set");
	return;
     }
   if (exe->tag) free(exe->tag);
   exe->tag = NULL;
   if (tag) exe->tag = strdup(tag);
}

/**
 * Retrieves the tag attached to the given process handle. There is no need to
 * free it as it just returns the internal pointer value. This value is only
 * valid as long as the @p exe is valid or until the tag is set to something
 * else on this @p exe.
 * 
 * @param   exe The given process handle.
 * @return  The string attached to @p exe.
 * @ingroup Ecore_Exe_Basic_Group
 */
char *
ecore_exe_tag_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_tag_get");
	return NULL;
     }
   return exe->tag;
}

/**
 * Frees the given process handle.
 *
 * Note that the process that the handle represents is unaffected by this
 * function.
 *
 * @param   exe The given process handle.
 * @return  The data attached to the handle when @ref ecore_exe_run was
 *          called.
 * @ingroup Ecore_Exe_Basic_Group
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
 * Retrieves the process ID of the given spawned process.
 * @param   exe Handle to the given spawned process.
 * @return  The process ID on success.  @c -1 otherwise.
 * @ingroup Ecore_Exe_Basic_Group
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
 * Retrieves the data attached to the given process handle.
 * @param   exe The given process handle.
 * @return  The data pointer attached to @p exe.
 * @ingroup Ecore_Exe_Basic_Group
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
 * @defgroup Ecore_Exe_Signal_Group Spawned Process Signal Functions
 *
 * Functions that send signals to spawned processes.
 */

/**
 * Pauses the given process by sending it a @c SIGSTOP signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
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
 * Continues the given paused process by sending it a @c SIGCONT signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
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
 * Sends the given spawned process a terminate (@c SIGTERM) signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
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
 * Kills the given spawned process by sending it a @c SIGKILL signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
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
 * Sends a @c SIGUSR signal to the given spawned process.
 * @param   exe Process handle to the given process.
 * @param   num The number user signal to send.  Must be either 1 or 2, or
 *              the signal will be ignored.
 * @ingroup Ecore_Exe_Signal_Group
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
 * Sends a @c SIGHUP signal to the given spawned process.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
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
   Ecore_List2 *l;
   
   for (l = (Ecore_List2 *)exes; l; l = l->next)
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

   /* FIXME: close fdhanlders and free buffers if they exist */
   data = exe->data;
   exes = _ecore_list2_remove(exes, exe);
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   if (exe->tag) free(exe->tag);
   free(exe);
   return data;
}
#endif
