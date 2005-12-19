#include "ecore_private.h"
#include "Ecore.h"

#include <errno.h>
#include <sys/wait.h>

#ifndef WIN32

static void _ecore_exe_exec_it(const char *exe_cmd);
static int _ecore_exe_data_read_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_exe_data_write_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_exe_flush(Ecore_Exe *exe);
static void _ecore_exe_event_exe_data_free(void *data __UNUSED__, void *ev);

static Ecore_Exe *exes = NULL;
static char *shell = NULL;

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
/* FIXME: replace this lot with _ecore_exe_exec_it(exe_cmd); once it gets a bit of testing. */
   {
     char use_sh = 1;
     char* buf = NULL;
     char** args = NULL;
     if (! strpbrk(exe_cmd, "|&;<>()$`\\\"'*?#"))
       {
	 if (! (buf = strdup(exe_cmd)))
	   return NULL;
	 char* token = strtok(buf, " \t\n\v");
	 char pre_command = 1;
	 int num_tokens = 0;
	 while(token)
	   {
	     if (token[0] == '~')
	       break;
	     if (pre_command)
	       {
		 if (token[0] == '[')
		   break;
		 if (strchr(token, '='))
		   break;
		 else
		   pre_command = 0;
	       }
	     num_tokens ++;
	     token = strtok(NULL, " \t\n\v");
	   }
	 free(buf);
	 buf = NULL;
	 if (! token && num_tokens)
	   {
	     int i = 0;
	     char* token;
	     if (! (buf = strdup(exe_cmd)))
	       return NULL;
	     token = strtok(buf, " \t\n\v");
	     use_sh = 0;
	     if (! (args = (char**) calloc(num_tokens + 1, sizeof(char*)))) {
	       free (buf);
	       return NULL;
	     }
	     for (i = 0; i < num_tokens; i ++)
	       {
		 if (token)
		   args[i] = token;
		 token = strtok(NULL, " \t\n\v");
	       }
	     args[num_tokens] = NULL;
	   }
       }
     setsid();
     if (use_sh)
       execl("/bin/sh", "/bin/sh", "-c", exe_cmd, (char *)NULL);
     else
       execvp(args[0], args);
     if (buf)
       free(buf);
     if(args)
       free(args);
   }
   exit(127);
   return NULL;
}

/**
 * Spawns a child process with its stdin/out available for communication.
 *
 * This function does the same thing as ecore_exe_run(), but also makes the
 * standard in and/or out from the child process available for reading or
 * writing.  To write use ecore_exe_pipe_write().  To read listen to
 * ECORE_EVENT_EXE_DATA events (set up a handler).  Ecore may buffer read
 * data until a newline character if asked for with the @p flags.  All
 * data will be included in the events (newlines will be replaced with
 * NULLS if line buffered).  ECORE_EVENT_EXE_DATA events will only happen
 * if the process is run with ECORE_EXE_PIPE_READ enabled in the flags.
 * Writing will only be allowed with ECORE_EXE_PIPE_WRITE enabled in the
 * flags.
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
/* FIXME: MAybe we should allow STDERR reading as well. */
   Ecore_Exe *exe = NULL;
   pid_t pid = 0;
   int readPipe[2] = { -1, -1 };
   int writePipe[2] = { -1, -1 };
   int statusPipe[2] = { -1, -1 };
   int n = 0;
   volatile int vfork_exec_errno = 0;

   if (!exe_cmd) return NULL;

   if ((flags & (ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE)) == 0) return ecore_exe_run(exe_cmd, data);

   exe = calloc(1, sizeof(Ecore_Exe));
   if (exe == NULL) return NULL;

   if ((pipe(readPipe) == -1) || (pipe(writePipe) == -1) || (pipe(statusPipe) == -1))
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
         close(STDOUT_FILENO);  /*  FIXME: Check for -1 then errno. */
         close(STDERR_FILENO);  /*  FIXME: Check for -1 then errno. */
         close(STDIN_FILENO);  /*  FIXME: Check for -1 then errno. */
         if (flags & ECORE_EXE_PIPE_READ)
	    {
	       dup2(readPipe[1], STDOUT_FILENO);  /*  FIXME: Check for -1 then errno. */
//	       dup2(dataPipe[1], STDERR_FILENO);  /*  FIXME: Check for -1 then errno. */
	    }
	 else
	    {
	       close(readPipe[1]);  /*  FIXME: Check for -1 then errno. */
	    }
         if (flags & ECORE_EXE_PIPE_WRITE)
	    {
	       dup2(writePipe[0], STDIN_FILENO);  /*  FIXME: Check for -1 then errno. */
	    }
	 else
	    {
	       close(writePipe[0]);  /*  FIXME: Check for -1 then errno. */
	    }
	 close(statusPipe[0]);  /*  FIXME: Check for -1 then errno. */
	 fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC);	/* close on exec shows sucess */    /*  FIXME: Check for -1 then errno. */

         _ecore_exe_exec_it(exe_cmd);

         /* Something went 'orribly wrong. */
	 vfork_exec_errno = errno;
         if (flags & ECORE_EXE_PIPE_READ)
	    close(readPipe[1]);  /*  FIXME: Check for -1 then errno. */
         if (flags & ECORE_EXE_PIPE_WRITE)
	    close(writePipe[0]);  /*  FIXME: Check for -1 then errno. */
	 close(statusPipe[1]);  /*  FIXME: Check for -1 then errno. */
	 _exit(-1);
      }
   else		/* parent */
      {
         if (! (flags & ECORE_EXE_PIPE_READ))
	    close(readPipe[0]);  /*  FIXME: Check for -1 then errno. */
         if (! (flags & ECORE_EXE_PIPE_WRITE))
	    close(writePipe[1]);  /*  FIXME: Check for -1 then errno. */
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
         exe->cmd = strdup(exe_cmd);
         if (flags & ECORE_EXE_PIPE_READ)
	    {
	       exe->child_fd_read = readPipe[0];
	       fcntl(exe->child_fd_read, F_SETFL, O_NONBLOCK);  /*  FIXME: Check for -1 then errno. */
	       exe->read_fd_handler = ecore_main_fd_handler_add(exe->child_fd_read,
	          ECORE_FD_READ, _ecore_exe_data_read_handler, exe,
	          NULL, NULL);
	    }
         if (flags & ECORE_EXE_PIPE_WRITE)
	    {
	       exe->child_fd_write = writePipe[1];
	       fcntl(exe->child_fd_write, F_SETFL, O_NONBLOCK);  /*  FIXME: Check for -1 then errno. */
	       exe->write_fd_handler = ecore_main_fd_handler_add(exe->child_fd_write,
	          ECORE_FD_WRITE, _ecore_exe_data_write_handler, exe,
	          NULL, NULL);
               if (exe->write_fd_handler)
                 ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);
	    }

	 exes = _ecore_list2_append(exes, exe);
         n = 0;
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
   void *buf;

   buf = realloc(exe->write_data_buf, exe->write_data_size + size);
   if (buf == NULL)   return 0;

   exe->write_data_buf = buf;
   memcpy(exe->write_data_buf + exe->write_data_size, data, size);
   exe->write_data_size += size;

   if (exe->write_fd_handler)
     ecore_main_fd_handler_active_set(exe->write_fd_handler, ECORE_FD_WRITE);
      
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

static void
_ecore_exe_exec_it(const char *exe_cmd)
{
   char use_sh = 1;
   char* buf = NULL;
   char** args = NULL;
   int save_errno = 0;

   if (! strpbrk(exe_cmd, "|&;<>()$`\\\"'*?#"))
      {
	 char* token;
	 char pre_command = 1;
	 int num_tokens = 0;

         if (! (buf = strdup(exe_cmd)))
	    return;

	 token = strtok(buf, " \t\n\v");
	 while(token)
	    {
	       if (token[0] == '~')
	          break;
	       if (pre_command)
	          {
		     if (token[0] == '[')
		        break;
		     if (strchr(token, '='))
		        break;
		     else
		        pre_command = 0;
	          }
	       num_tokens ++;
	       token = strtok(NULL, " \t\n\v");
	    }
	 free(buf);
	 buf = NULL;
	 if (! token && num_tokens)
	    {
	       int i = 0;
	       char* token;

	       if (! (buf = strdup(exe_cmd)))
	         return;

	       token = strtok(buf, " \t\n\v");
	       use_sh = 0;
	       if (! (args = (char**) calloc(num_tokens + 1, sizeof(char*)))) 
	          {
	             free (buf);
	             return;
	          }
	       for (i = 0; i < num_tokens; i ++)
	          {
		     if (token)
		        args[i] = token;
		     token = strtok(NULL, " \t\n\v");
	          }
	       args[num_tokens] = NULL;
            }
      }

   setsid();
   if (use_sh)
      {
         if (shell == NULL) 
            {
               shell = getenv("SHELL");
	       if (shell == 0)
	          shell = "/bin/sh";
            }
         errno = 0;
         execl(shell, shell, "-c", exe_cmd, (char *)NULL);
      }
   else
      {
         errno = 0;
         execvp(args[0], args);
      }

   save_errno = errno;
   if (buf)
      free(buf);
   if(args)
      free(args);
   errno = save_errno;
   return;
}

void *
_ecore_exe_free(Ecore_Exe *exe)
{
   void *data;

   data = exe->data;

   if (exe->write_fd_handler)               ecore_main_fd_handler_del(exe->write_fd_handler);
   if (exe->read_fd_handler)                ecore_main_fd_handler_del(exe->read_fd_handler);
   if (exe->write_data_buf)                 free(exe->write_data_buf);
   if (exe->read_data_buf)                  free(exe->read_data_buf);
   if (exe->flags & ECORE_EXE_PIPE_READ)    close(exe->child_fd_read);  /*  FIXME: Check for -1 then errno. */
   if (exe->flags & ECORE_EXE_PIPE_WRITE)   close(exe->child_fd_write);  /*  FIXME: Check for -1 then errno. */
   if (exe->cmd)                            free(exe->cmd);
   
   exes = _ecore_list2_remove(exes, exe);
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   if (exe->tag) free(exe->tag);
   free(exe);
   return data;
}


static int
_ecore_exe_data_read_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Exe *exe;

   exe = data;
   if ((exe->read_fd_handler) && (ecore_main_fd_handler_active_get(exe->read_fd_handler, ECORE_FD_READ)))
      {
         unsigned char *inbuf;
	 int inbuf_num;

         inbuf = exe->read_data_buf;
         inbuf_num = exe->read_data_size;
	 exe->read_data_buf = NULL;
	 exe->read_data_size = 0;

	 for (;;)
	    {
	       int num, lost_exe;
	       char buf[READBUFSIZ];

	       lost_exe = 0;
	       errno = 0;
	       if ((num = read(exe->child_fd_read, buf, READBUFSIZ)) < 1)  /* FIXME: SPEED/SIZE TRADE OFF - add a smaller READBUFSIZE (currently 64k) to inbuf, use that instead of buf, and save ourselves a memcpy(). */
	          {
		     lost_exe = ((errno == EIO) || 
			         (errno == EBADF) ||
				 (errno == EPIPE) || 
				 (errno == EINVAL) ||
				 (errno == ENOSPC));
                     if ((errno != EAGAIN) && (errno != EINTR))
                        perror("_ecore_exe_data_handler() read problem ");
                  }
	       if (num > 0)
	          {
		     inbuf = realloc(inbuf, inbuf_num + num);
		     memcpy(inbuf + inbuf_num, buf, num);
		     inbuf_num += num;
	          }
	       else
	          {
		     if (inbuf) 
		        {
		           Ecore_Event_Exe_Data *e;
		       
		           e = calloc(1, sizeof(Ecore_Event_Exe_Data));
		           if (e)
			      {
			         e->exe = exe;
			         e->data = inbuf;
			         e->size = inbuf_num;

                                 if (exe->flags & ECORE_EXE_PIPE_READ_LINE_BUFFERED)
				    {
				       int max = 0;
				       int count = 0;
				       int i;
				       int last = 0;
				       char *c;

                                       c = inbuf;
				       for (i = 0; i < inbuf_num; i++) /* Find the lines. */
				          {
					     if (inbuf[i] == '\n')
					        {
					           if (count >= max)
					              {
						         max += 10;  /* FIXME: Maybe keep track of the largest number of lines ever sent, and add half that many instead of 10. */
		                                         e->lines = realloc(e->lines, sizeof(Ecore_Event_Exe_Data_Line) * (max + 1)); /* Allow room for the NULL termination. */
						      }
						   /* raster said to leave the line endings as line endings, however -
						    * This is line buffered mode, we are not dealing with binary here, but lines.
						    * If we are not dealing with binary, we must be dealing with ASCII, unicode, or some other text format.
						    * Thus the user is most likely gonna deal with this text as strings.
						    * Thus the user is most likely gonna pass this data to str functions.
						    * rasters way - the endings are always gonna be '\n';  onefangs way - they will always be '\0'
						    * We are handing them the string length as a convenience.
						    * Thus if they really want it in raw format, they can e->lines[i].line[e->lines[i].size - 1] = '\n'; easily enough.
						    * In the default case, we can do this conversion quicker than the user can, as we already have the index and pointer.
						    * Let's make it easy on them to use these as standard C strings.
						    *
						    * onefang is proud to announce that he has just set a new personal record for the
						    * most over documentation of a simple assignment statement.  B-)
						    */
						   inbuf[i] = '\0';
						   e->lines[count].line = c;
						   e->lines[count].size = i - last;
						   last = i + 1;
						   c = &inbuf[last];
					           count++;
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
					  if (i > last) /* Partial line left over, save it for next time. */
					     {
					        e->size = last;
	                                        exe->read_data_size = i - last;
	                                        exe->read_data_buf = malloc(exe->read_data_size);
		                                memcpy(exe->read_data_buf, c, exe->read_data_size);
					     }
				    }

				 if (e)
			            ecore_event_add(ECORE_EVENT_EXE_DATA, e,
					    _ecore_exe_event_exe_data_free, NULL);
			      }
		        }
		     if (lost_exe)
		        {
			   if (exe->exit_event)
			      {
		                 _ecore_event_add(ECORE_EVENT_EXE_EXIT, exe->exit_event, 
				                  _ecore_event_exe_exit_free, NULL);
				 exe->exit_event = NULL;   /* Just being paranoid. */
			      }
			   else
                              ecore_exe_terminate(exe);   /* FIXME: give this some deep thought later. */
                        }
		     break;
	          }
	    }
      }

   return 1;
}

static int
_ecore_exe_data_write_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Exe *exe;

   exe = data;
   if ((exe->write_fd_handler) && (ecore_main_fd_handler_active_get(exe->write_fd_handler, ECORE_FD_WRITE)))
      _ecore_exe_flush(exe);

   return 1;
}

static void
_ecore_exe_flush(Ecore_Exe *exe)
{
   int count;

   /* check whether we need to write anything at all. */
   if ((!exe->child_fd_write) && (!exe->write_data_buf))   return;
   if (exe->write_data_size == exe->write_data_offset)     return;

   count = write(exe->child_fd_write, 
                 exe->write_data_buf  + exe->write_data_offset, 
		 exe->write_data_size - exe->write_data_offset);
   if (count < 1)
      {
         if (errno == EIO   || errno == EBADF ||
 	     errno == EPIPE || errno == EINVAL ||
	     errno == ENOSPC)   /* we lost our server! */
	    {
               ecore_exe_terminate(exe);
               if (exe->write_fd_handler)
                 ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);
	    }
      }
   else
      {
         exe->write_data_offset += count;
         if (exe->write_data_offset >= exe->write_data_size)
            {
	       exe->write_data_size = 0;
	       exe->write_data_offset = 0;
	       free(exe->write_data_buf);
	       exe->write_data_buf = NULL;
               if (exe->write_fd_handler)
                 ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);
            }
      }
}

static void
_ecore_exe_event_exe_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Event_Exe_Data *e;

   e = ev;

   if (e->lines)   free(e->lines);
   if (e->data)    free(e->data);
   free(e);
}
#endif
