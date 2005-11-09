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
   int n = 0;
   volatile int vfork_exec_errno = 0;

   /* FIXME: 
    * set up fd handler in ecore_exe struct 
    * see ecore_con for code and examples on this (fd's there are to a socket
    * but otherwise work the same as here). the ECORE_EVENT_EXE_EXIT event
    * aces like the client del event from ecore_con - signalling that the
    * connection is closed. once this event has been handled the child
    * ecore_exe struct is freed automatically and is no longer valid.
    * 
    * _ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
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

   exe = calloc(1, sizeof(Ecore_Exe));
   if (exe == NULL) return NULL;

   exe->args[n++] = shell;
   exe->args[n++] = "-c";
   exe->args[n++] = exe_cmd;
   exe->args[n++] = NULL;

   if ((pipe(dataPipe) == -1) || pipe(statusPipe) == -1)
      printf("Failed to create pipes\n");
   /* FIXME: I should double check this.  After a quick look around, this is already done, but via a more modern method. */
   /* signal(SIGPIPE, SIG_IGN);	/* we only want EPIPE on errors */
   pid = fork();

   if (pid == -1)
      {
	  printf("Failed to fork process\n");
	  pid = 0;
      }
   else if (pid == 0)		/* child */
      {
         setsid();  /*  FIXME: Check for -1 then errno. */

         close(STDOUT_FILENO);  /*  FIXME: Check for -1 then errno. */
         close(STDERR_FILENO);  /*  FIXME: Check for -1 then errno. */
         close(STDIN_FILENO);  /*  FIXME: Check for -1 then errno. */
         if (flags & ECORE_EXE_PIPE_READ)
	    {
	       dup2(dataPipe[1], STDOUT_FILENO);  /*  FIXME: Check for -1 then errno. */
	       dup2(dataPipe[1], STDERR_FILENO);  /*  FIXME: Check for -1 then errno. */
	    }
	 else
	    {
	       close(dataPipe[1]);  /*  FIXME: Check for -1 then errno. */
	    }
         if (flags & ECORE_EXE_PIPE_WRITE)
	    {
	       dup2(dataPipe[0], STDIN_FILENO);  /*  FIXME: Check for -1 then errno. */
	    }
	 else
	    {
	       close(dataPipe[0]);  /*  FIXME: Check for -1 then errno. */
	    }
	 close(statusPipe[0]);  /*  FIXME: Check for -1 then errno. */
	 fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC);	/* close on exec shows sucess */    /*  FIXME: Check for -1 then errno. */

	 errno = 0;
	 execvp(shell, (char **) exe->args);

         /* Something went 'orribly wrong. */
	 vfork_exec_errno = errno;
//         if (! (flags & ECORE_EXE_PIPE_READ))
//	    close(dataPipe[1]);  /*  FIXME: Check for -1 then errno. */
//         if (! (flags & ECORE_EXE_PIPE_WRITE))
//	    close(dataPipe[0]);  /*  FIXME: Check for -1 then errno. */
//	 close(statusPipe[1]);  /*  FIXME: Check for -1 then errno. */
	 _exit(-1);
      }
   else		/* parent */
      {
         if (! (flags & ECORE_EXE_PIPE_READ))
	    close(dataPipe[0]);  /*  FIXME: Check for -1 then errno. */
         if (! (flags & ECORE_EXE_PIPE_WRITE))
	    close(dataPipe[1]);  /*  FIXME: Check for -1 then errno. */
	 close(statusPipe[1]);  /*  FIXME: Check for -1 then errno. */

         /* FIXME: after having a good look at the current e fd handling, investigate fcntl(dataPipe[x], F_SETSIG, ...) */

	 while (1)  /* Wait for it to start executing. */
	    {
               char buf;

	       n = read(statusPipe[0], &buf, 1);

	       if ((n == -1) && ((errno == EAGAIN) || (errno == EINTR)))
		  continue;	/* try it again */
	       if (n == 0)
	          {
	             if (vfork_exec_errno != 0)
		        {
		           n = vfork_exec_errno;
		           printf("Could not exec process\n"); /* FIXME: maybe set the pid to 0? */
		        }
		     break;
	          }
	    }
         close(statusPipe[0]);

      }

   if (pid)
      {
         ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
	 exe->pid = pid;
	 exe->flags = flags;
	 exe->data = (void *)data;
         exe->cmd = exe_cmd;  /* FIXME: should calloc and cpy. */
         if (flags & ECORE_EXE_PIPE_READ)
	    exe->child_fd_read = dataPipe[0];
         if (flags & ECORE_EXE_PIPE_WRITE)
	    exe->child_fd_write = dataPipe[1];
	 exes = _ecore_list2_append(exes, exe);
         n = 0;
         printf("Ecore_Exe %s success!\n", exe_cmd);
      }

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
    *
    * But for now, a quick and dirty test implementation -
    */
   ssize_t outsize = write(exe->child_fd_write, data, size);
   if (outsize == -1)
      {
         /* FIXME: should check errno to see what went wrong. */
         return 0;
      }
   else
      return 1;
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
 * Makes sure the process is dead, one way or another.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
void
ecore_exe_kill_maybe(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
        /* Since Ecore_Exe's can be freed without the users knowledge, we need a way to kill them without bitchin'. */
	/* FIXME: On the other hand, handling the exe exit event may be the way to go. */
	return;
     }
   kill(exe->pid, SIGTERM);
/* FIXME: should pause for a bit. */
   kill(exe->pid, SIGKILL);
}

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

printf("FREEING Ecore_Exe %s\n", exe->cmd);
   data = exe->data;

   /* FIXME: close fdhanlders and free buffers if they exist */
   if (exe->flags & ECORE_EXE_PIPE_READ)
      close(exe->child_fd_read);  /*  FIXME: Check for -1 then errno. */
   if (exe->flags & ECORE_EXE_PIPE_WRITE)
      close(exe->child_fd_write);  /*  FIXME: Check for -1 then errno. */

   exes = _ecore_list2_remove(exes, exe);
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   if (exe->tag) free(exe->tag);
   free(exe);
   return data;
}
#endif
